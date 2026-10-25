#!/usr/bin/env python3
"""Train DashForge's compact object-transition model from native GD level strings.

Input is one or more JSONL files from the FULL geometry-dash-levels dataset.
The *_nodata datasets cannot be used because they omit level_string.

Each level_string is urlsafe-base64(gzip(UTF-8 level string)). This script:
- decodes each native string,
- parses object ID / X / Y properties,
- learns object frequencies, average horizontal spacing/height and transitions,
- writes a small TSV model for the mod to consume offline.

It deliberately does not claim that this is a playable-level exporter or that it
completes the separate 1,820-slot historical catalogue.
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
    # Dataset uses URL-safe base64 of gzip-compressed UTF-8.
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
        if not math.isfinite(x) or not math.isfinite(y):
            continue
        objects.append((x, y, oid))
    objects.sort(key=lambda item: (item[0], item[1]))
    return objects

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", type=Path, nargs="+", required=True,
                    help="One or more full JSONL shards; not *_nodata metadata files")
    ap.add_argument("--model", type=Path, default=Path("resources/dashforge-model.tsv"))
    ap.add_argument("--min-objects", type=int, default=20,
                    help="Skip levels with fewer valid objects than this")
    ap.add_argument("--max-levels", type=int, default=0,
                    help="Optional cap for testing; 0 means no cap")
    args = ap.parse_args()

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
                if args.max_levels and levels_used >= args.max_levels:
                    break
                try:
                    record = json.loads(line)
                except json.JSONDecodeError:
                    continue
                records_seen += 1
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
                # Per-level de-duplication limits the impact of repeated object copies.
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

    if not object_count or not transitions:
        print("ERROR: no usable native level strings found. Ensure inputs are full JSONL shards, not *_nodata files.", file=sys.stderr)
        return 3

    args.model.parent.mkdir(parents=True, exist_ok=True)
    with args.model.open("w", encoding="utf-8", newline="\n") as out:
        out.write("# DashForge learned object model v1\n")
        out.write("# Generated from native GD level strings; tabs separate fields.\n")
        out.write(f"# levels={levels_used} records_seen={records_seen} skipped={levels_skipped}\n")
        for oid, count in sorted(object_count.items()):
            mean_dx = dx_sum[oid] / dx_n[oid] if dx_n[oid] else 30.0
            mean_y = y_sum[oid] / y_n[oid] if y_n[oid] else 0.0
            out.write(f"O\t{oid}\t{count}\t{mean_dx:.4f}\t{mean_y:.4f}\n")
        for (prev, nxt), count in sorted(transitions.items()):
            out.write(f"T\t{prev}\t{nxt}\t{count}\n")

    print(f"Records scanned: {records_seen}")
    print(f"Levels used for learning: {levels_used}")
    print(f"Skipped/missing/invalid levels: {levels_skipped}")
    print(f"Distinct object IDs learned: {len(object_count)}")
    print(f"Distinct object transitions learned: {len(transitions)}")
    print(f"Saved model: {args.model}")
    print("Note: this model learns object IDs and sequence/spacing statistics; it does not yet convert the sequence into real editor objects.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
