#!/usr/bin/env python3
"""Train DashForge's compact object-transition model from selected native GD levels.

Use tools/export_corpus_ids.py to export IDs from the curated workbook, then pass
full JSONL shards from the dataset. The *_nodata subsets cannot be used because
they omit level_string. By default, this script refuses to write a model unless
every requested ID has native data; --allow-partial is an explicit opt-in.

The dataset's level_string is urlsafe-base64(gzip(UTF-8 level string)). This
trainer learns object-ID frequencies, horizontal spacing, vertical placement
statistics and pairwise transitions. It does not yet export a playable level.
"""
from __future__ import annotations
import argparse
import base64
import gzip
import json
import math
from collections import Counter, defaultdict
from pathlib import Path
import sys

def decode_level_string(value: str) -> str:
    raw = base64.urlsafe_b64decode(value.encode("ascii"))
    return gzip.decompress(raw).decode("utf-8", errors="replace")

def parse_objects(level_string: str):
    objects = []
    for chunk in level_string.split(";"):
        if not chunk:
            continue
        parts = chunk.split(",")
        if len(parts) < 6 or len(parts) % 2:
            continue
        try:
            props = {int(parts[i]): parts[i + 1] for i in range(0, len(parts) - 1, 2)}
            oid = int(props[1])
            x = float(props[2])
            y = float(props[3])
        except (ValueError, KeyError, IndexError):
            continue
        if not math.isfinite(x) or not math.isfinite(y) or oid <= 0:
            continue
        objects.append((x, y, oid))
    objects.sort(key=lambda item: (item[0], item[1]))
    return objects

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", type=Path, nargs="+", required=True,
                    help="One or more full JSONL shards; not *_nodata metadata files")
    ap.add_argument("--ids-file", type=Path, required=True,
                    help="Text file with one selected corpus level ID per line")
    ap.add_argument("--model", type=Path, default=Path("resources/dashforge-model.tsv"))
    ap.add_argument("--min-objects", type=int, default=20,
                    help="Skip selected levels with fewer valid objects than this")
    ap.add_argument("--max-levels", type=int, default=0,
                    help="Optional cap for testing; 0 means no cap")
    ap.add_argument("--allow-partial", action="store_true",
                    help="Explicitly allow a model trained from only the IDs found in supplied shards")
    args = ap.parse_args()

    if not args.ids_file.exists():
        ap.error(f"IDs file not found: {args.ids_file}")
    wanted = {line.strip() for line in args.ids_file.read_text(encoding="utf-8").splitlines()
              if line.strip().isdigit()}
    if not wanted:
        ap.error("IDs file contains no numeric level IDs")
    found = set()
    object_count = Counter()
    dx_sum = defaultdict(float)
    dx_n = Counter()
    y_sum = defaultdict(float)
    y_n = Counter()
    transitions = Counter()
    levels_used = 0
    levels_skipped = 0
    records_seen = 0

    for source in args.input:
        if not source.exists():
            print(f"ERROR: input file not found: {source}", file=sys.stderr)
            return 2
        print(f"Reading {source} ...", flush=True)
        with source.open("r", encoding="utf-8") as stream:
            for line in stream:
                try:
                    record = json.loads(line)
                except json.JSONDecodeError:
                    continue
                records_seen += 1
                raw_id = record.get("level_id", record.get("id"))
                if raw_id is None or str(raw_id) not in wanted or str(raw_id) in found:
                    continue
                encoded = record.get("level_string")
                if not isinstance(encoded, str) or not encoded:
                    levels_skipped += 1
                    continue
                try:
                    objects = parse_objects(decode_level_string(encoded))
                except (ValueError, OSError, EOFError, base64.binascii.Error):
                    levels_skipped += 1
                    continue
                if len(objects) < args.min_objects:
                    levels_skipped += 1
                    continue
                found.add(str(raw_id))
                sequence = [oid for _, _, oid in objects]
                object_count.update(sequence)
                for (x0, y0, oid0), (x1, y1, oid1) in zip(objects, objects[1:]):
                    dx = x1 - x0
                    if 0 <= dx <= 500:
                        dx_sum[oid1] += dx
                        dx_n[oid1] += 1
                    y_sum[oid1] += y1
                    y_n[oid1] += 1
                    if oid0 != oid1:
                        transitions[(oid0, oid1)] += 1
                levels_used += 1
                if args.max_levels and levels_used >= args.max_levels:
                    break
        if args.max_levels and levels_used >= args.max_levels:
            break
        if found >= wanted:
            break

    missing = sorted(wanted - found, key=int)
    print(f"Requested corpus IDs: {len(wanted)}")
    print(f"Records scanned: {records_seen}")
    print(f"Selected levels used for learning: {levels_used}")
    print(f"Selected levels skipped for missing/invalid/short native data: {levels_skipped}")
    print(f"Requested IDs not found with usable native data: {len(missing)}")
    if missing:
        preview = ", ".join(missing[:30])
        print(f"Missing ID sample: {preview}" + (" ..." if len(missing) > 30 else ""))
    if missing and not args.allow_partial:
        print("ERROR: refusing to write a partial model. Supply all relevant full JSONL shards, or use --allow-partial knowingly.", file=sys.stderr)
        return 4
    if not object_count or not transitions:
        print("ERROR: no usable native level strings found. Ensure inputs are full JSONL shards, not *_nodata files.", file=sys.stderr)
        return 3

    args.model.parent.mkdir(parents=True, exist_ok=True)
    with args.model.open("w", encoding="utf-8", newline="\n") as out:
        out.write("# DashForge learned object model v1\n")
        out.write("# Generated from selected native GD level strings; tabs separate fields.\n")
        out.write(f"# requested={len(wanted)} levels={levels_used} missing={len(missing)} records_seen={records_seen}\n")
        for oid, count in sorted(object_count.items()):
            mean_dx = dx_sum[oid] / dx_n[oid] if dx_n[oid] else 30.0
            mean_y = y_sum[oid] / y_n[oid] if y_n[oid] else 0.0
            out.write(f"O\t{oid}\t{count}\t{mean_dx:.4f}\t{mean_y:.4f}\n")
        for (prev, nxt), count in sorted(transitions.items()):
            out.write(f"T\t{prev}\t{nxt}\t{count}\n")

    manifest = args.model.with_suffix(".manifest.json")
    manifest.write_text(json.dumps({
        "schema_version": 1,
        "requested_ids": len(wanted),
        "levels_used": levels_used,
        "missing_ids": missing,
        "records_scanned": records_seen,
        "native_data_required": True,
        "partial_model": bool(missing),
        "object_id_types": len(object_count),
        "transition_types": len(transitions),
        "source_files": [str(p) for p in args.input],
    }, indent=2), encoding="utf-8")
    print(f"Distinct object IDs learned: {len(object_count)}")
    print(f"Distinct object transitions learned: {len(transitions)}")
    print(f"Saved model: {args.model}")
    print(f"Saved manifest: {manifest}")
    print("Note: the model learns object IDs and transition/spacing statistics; playable editor-object creation/export remains a separate stage.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
