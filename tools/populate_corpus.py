#!/usr/bin/env python3
"""Populate DashForge's 1,820-slot workbook from the public GD rated-level metadata dump.

Important:
- This selects candidate NAMES/IDs from metadata; it does not download native level strings.
- The game's editor version is only a proxy for historical era, not proof of when a level was first rated.
- Ambiguous/unmapped records are not guessed. Empty slots remain visibly pending.
- Run: python tools/populate_corpus.py --input 2026_50k_rated_nodata.jsonl --workbook DashForge_Corpus_1820_plazas.xlsx
  Download the metadata file from:
  https://huggingface.co/datasets/yusp48/geometry-dash-levels
"""
from __future__ import annotations
import argparse, json, random, re, sys
from pathlib import Path
from collections import defaultdict
from openpyxl import load_workbook

ERAS = [f"1.{i}" for i in range(10)] + ["2.0", "2.1", "2.2"]
COHORTS = [
    "Auto", "Easy", "Normal", "Hard", "Harder", "Insane",
    "Easy Demon", "Medium Demon", "Hard Demon", "Insane Demon",
    "Extreme Demon", "N/A (referencia)", "Platformer Moon — no Demon",
    "Platformer Moon — Demon",
]
# Metadata schemas vary; these aliases are deliberately explicit.
ALIASES = {
    "id": ["level_id", "levelID", "id"],
    "name": ["name", "level_name", "levelName"],
    "creator": ["creator", "author", "username", "userName"],
    "version": ["game_version", "gameVersion", "version"],
    "difficulty": ["difficulty_name", "difficultyName", "difficulty", "diff"],
    "stars": ["stars", "star", "star_count"],
    "featured": ["featured", "is_featured", "isFeatured"],
    "platformer": ["platformer", "is_platformer", "isPlatformer", "level_type"],
    "downloads": ["downloads", "download_count", "downloadsCount"],
    "likes": ["likes", "like_count", "likesCount"],
}

def first(record, keys):
    for key in keys:
        if key in record and record[key] not in (None, ""):
            return record[key]
    return None

def version_to_era(value):
    if value is None:
        return None
    s = str(value).strip()
    # Numeric server versions can appear as "22", "21", "20", or "2.2081".
    if s in {"22", "2.2", "2.201", "2.202", "2.203", "2.204", "2.205",
             "2.206", "2.207", "2.2071", "2.2072", "2.2073", "2.2074",
             "2.208", "2.2081", "2.209"} or s.startswith("2.2"):
        return "2.2"
    if s in {"21", "2.1"} or s.startswith("2.1"):
        return "2.1"
    if s in {"20", "2.0"} or s.startswith("2.0"):
        return "2.0"
    m = re.match(r"^(?:1\.)?(\d+)(?:\.|$)", s)
    if m:
        n = int(m.group(1))
        if 0 <= n <= 9:
            return f"1.{n}"
    return None

def normalize_difficulty(value, stars=None):
    if value is None:
        return None
    s = str(value).strip().lower().replace("_", " ").replace("-", " ")
    aliases = {
        "auto": "Auto", "easy": "Easy", "normal": "Normal", "medium": "Normal",
        "hard": "Hard", "harder": "Harder", "insane": "Insane",
        "easy demon": "Easy Demon", "medium demon": "Medium Demon",
        "hard demon": "Hard Demon", "insane demon": "Insane Demon",
        "extreme demon": "Extreme Demon", "demon": "Hard Demon",
        "na": "N/A (referencia)", "n/a": "N/A (referencia)",
        "not rated": "N/A (referencia)", "unrated": "N/A (referencia)",
    }
    if s in aliases:
        return aliases[s]
    # Common numeric difficulty values used by GD server datasets.
    try:
        n = int(float(s))
    except (ValueError, TypeError):
        return None
    numeric = {
        0: "N/A (referencia)", 10: "Auto", 20: "Easy", 30: "Normal",
        40: "Hard", 50: "Harder", 60: "Insane", 70: "Hard Demon",
        71: "Easy Demon", 72: "Medium Demon", 73: "Hard Demon",
        74: "Insane Demon", 75: "Extreme Demon",
    }
    return numeric.get(n)

def is_platformer(record):
    raw = first(record, ALIASES["platformer"])
    if isinstance(raw, bool):
        return raw
    if str(raw).strip().lower() in {"true", "1", "yes", "platformer", "moon"}:
        return True
    return False

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", required=True, help="Path to the dataset's metadata-only JSONL file")
    ap.add_argument("--workbook", required=True, help="Existing DashForge 1,820-slot XLSX")
    ap.add_argument("--seed", type=int, default=3861, help="Deterministic shuffle seed")
    args = ap.parse_args()
    source, output = Path(args.input), Path(args.workbook)
    if not source.exists() or not output.exists():
        sys.exit("ERROR: input JSONL or workbook path does not exist.")

    records = []
    unmapped = 0
    with source.open("r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                rec = json.loads(line)
            except json.JSONDecodeError:
                continue
            name = first(rec, ALIASES["name"])
            lid = first(rec, ALIASES["id"])
            version = first(rec, ALIASES["version"])
            diff_raw = first(rec, ALIASES["difficulty"])
            era = version_to_era(version)
            diff = normalize_difficulty(diff_raw, first(rec, ALIASES["stars"]))
            if not name or not lid or not era or not diff:
                unmapped += 1
                continue
            if is_platformer(rec):
                diff = "Platformer Moon — Demon" if "demon" in str(diff_raw).lower() else "Platformer Moon — no Demon"
            records.append({
                "era": era, "difficulty": diff, "name": str(name),
                "creator": first(rec, ALIASES["creator"]) or "",
                "id": str(lid), "version": str(version),
                "downloads": first(rec, ALIASES["downloads"]) or 0,
                "likes": first(rec, ALIASES["likes"]) or 0,
                "source": "Hugging Face: yusp48/geometry-dash-levels (metadata-only; verify against server)",
            })

    rng = random.Random(args.seed)
    groups = defaultdict(list)
    seen = set()
    for r in records:
        key = (r["era"], r["difficulty"], r["id"])
        if key in seen:
            continue
        seen.add(key)
        groups[(r["era"], r["difficulty"])].append(r)
    for key in groups:
        rng.shuffle(groups[key])

    wb = load_workbook(output)
    ws = wb["Matriz 1820"]
    # Keep manually curated/mandatory seeds; fill only empty-name cells.
    filled = 0
    shortage = []
    used_ids = {str(ws.cell(row, 7).value) for row in range(2, ws.max_row + 1)
                if ws.cell(row, 7).value not in (None, "")}
    for row in range(2, ws.max_row + 1):
        era = str(ws.cell(row, 1).value or "")
        cohort = str(ws.cell(row, 3).value or "")
        name_cell = ws.cell(row, 5)
        if name_cell.value not in (None, ""):
            continue
        candidates = groups.get((era, cohort), [])
        candidate = next((c for c in candidates if c["id"] not in used_ids), None)
        if candidate is None:
            continue
        name_cell.value = candidate["name"]
        ws.cell(row, 6).value = candidate["creator"]
        ws.cell(row, 7).value = candidate["id"]
        ws.cell(row, 8).value = cohort
        ws.cell(row, 9).value = "Platformer" if cohort.startswith("Platformer") else "Classic"
        ws.cell(row, 10).value = "Metadata dataset says rated/featured; verify current rating"
        ws.cell(row, 11).value = candidate["era"] + " (inferred from editor version; not confirmed first publication)"
        ws.cell(row, 12).value = "Metadata candidate — random seeded selection"
        ws.cell(row, 13).value = "No nativo adjunto; pendiente de obtener la cadena del nivel"
        ws.cell(row, 14).value = "Candidate selected from rated-level metadata; verify exact difficulty, era and native data before training"
        ws.cell(row, 15).value = "https://huggingface.co/datasets/yusp48/geometry-dash-levels"
        ws.cell(row, 16).value = f"Editor version in dataset: {candidate['version']}; downloads={candidate['downloads']}; likes={candidate['likes']}"
        used_ids.add(candidate["id"])
        filled += 1

    # Summary tab for reproducibility and honest coverage accounting.
    if "Importación metadatos" in wb.sheetnames:
        del wb["Importación metadatos"]
    sm = wb.create_sheet("Importación metadatos")
    sm.append(["Métrica", "Valor"])
    sm.append(["Registros candidatos parseados", len(records)])
    sm.append(["Registros no mapeados/insuficientes", unmapped])
    sm.append(["Plazas vacías rellenadas en esta ejecución", filled])
    sm.append(["Semilla aleatoria", args.seed])
    sm.append(["Fuente", "https://huggingface.co/datasets/yusp48/geometry-dash-levels"])
    sm.append(["Advertencia", "El editor version no prueba la fecha de publicación ni la época del rate. La metadata no contiene level_string. Verificar cada candidato y recuperar datos nativos antes de entrenar."])
    sm.append(["Advertencia", "Las cohortes de Demon y plataforma pueden tener codificaciones distintas; revisar la hoja Matriz 1820 y corregir cualquier clasificación dudosa."])
    wb.save(output)
    print(f"Parsed candidates: {len(records)}")
    print(f"Unmapped/insufficient records: {unmapped}")
    print(f"Empty matrix slots filled: {filled}")
    print(f"Saved: {output}")

if __name__ == "__main__":
    main()
