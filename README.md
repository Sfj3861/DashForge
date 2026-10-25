# DashForge

DashForge is an experimental Geode mod for Geometry Dash intended to generate level layouts locally. Its training and generation design is offline-first and requires no paid AI API.

## Current status

Implemented in the repository:
- Geode mod metadata, main-menu entry points and deterministic fallback generator.
- In-game corpus-status view with a clear 1,820-slot target and explicit verification states.
- A compact native-object transition model loader in C++.
- Generation now samples object IDs and object-to-object transitions from a trained model when one is installed; otherwise it uses the heuristic fallback.
- A trainer that decodes native Geometry Dash level strings and learns object-ID frequencies, adjacent-object transitions, horizontal spacing and vertical-placement statistics.
- A workbook ID exporter so training can be restricted to selected corpus levels.
- A bundled empty model placeholder, so a clean checkout correctly falls back until a model has been trained.

Still not complete:
- The full, verified list of 1,820 historical/community levels.
- Native level strings for all selected IDs.
- A trained model built from the final 1,820-level corpus; no native dataset is bundled in this repository.
- Real editor-object creation, object-version validation, playable-level save/export and in-game visual validation.
- A confirmed successful build/runtime test in this environment.

**Important:** the current learned output is an object-ID/event plan, not a playable Geometry Dash level. The mod does not yet create actual editor GameObjects from that plan. The training pipeline must be run with full JSONL data and the selected corpus IDs; metadata-only `*_nodata` files cannot train object placement because they omit `level_string`.

## Historical learning corpus

The specification is in [`data/corpus-plan.json`](data/corpus-plan.json), the starter manifest is [`data/community-levels-seed.json`](data/community-levels-seed.json), and the in-game status view is in [`src/Corpus.cpp`](src/Corpus.cpp).

Target:
- 13 eras: 1.0–1.9, 2.0, 2.1 and 2.2 (2.2 patches grouped into 2.2).
- 14 analytical difficulty/mode cohorts.
- 10 levels per cell: 1,820 target slots.
- Official RobTop levels tracked separately and not counted against the 1,820.
- Mandatory examples include SuperSonic, Sonic Wave, Nine Circles, Bloodbath, Sonar, Level Easy, Future Funk, ReTraY and Darnoc's Chill Out.
- Gauntlet substitutes must be marked, and their true historical era must not be rewritten.
- Unknown IDs, ratings, eras and native data must remain pending rather than guessed.

The seed manifest is not the completed corpus. Some entries are metadata candidates; verify their IDs, creator, difficulty and era before training.

## Build

Install the Geode SDK and Geode CLI for the target platform, set `GEODE_SDK`, then run from the repository root:

```sh
geode build
```

CMake requires 3.21+ and the project currently requests C++23. Platform compatibility depends on the Geode SDK and the Geometry Dash version in `mod.json`.

## Train from the curated corpus

### 1. Export IDs from the workbook

The workbook must contain the curated IDs in the `Matriz 1820` sheet, column G by default.

```sh
python -m pip install openpyxl
python tools/export_corpus_ids.py --workbook DashForge_Corpus_1820_plazas.xlsx --output corpus-level-ids.txt
```

Check the report. If the workbook still has empty IDs, the corpus is incomplete.

### 2. Obtain the FULL dataset shards

Use the full `2026_50k_rated` JSONL shards from [the public Geometry Dash level dataset](https://huggingface.co/datasets/yusp48/geometry-dash-levels). Those records contain `level_string`, encoded as URL-safe Base64 of gzip-compressed UTF-8. The smaller `2026_50k_rated_nodata` subset omits it and is metadata-only.

Place the downloaded full shard files on disk. You may pass multiple shards. Do not pass the metadata-only `*_nodata.jsonl` file.

### 3. Train the model on only the selected IDs

```sh
python tools/train_level_model.py \
  --ids-file corpus-level-ids.txt \
  --input ./levels-v2-00000.jsonl ./levels-v2-00001.jsonl \
  --model resources/dashforge-model.tsv
```

Add every relevant shard to `--input` so the trainer can find the selected IDs. By default it refuses to write a model if any selected IDs are missing or have unusable native data. Only use `--allow-partial` if you deliberately accept a partial model; the adjacent manifest records missing IDs. The trainer skips levels with fewer than 20 valid objects by default.

### 4. Rebuild and test

After training, run `geode build` again so the generated `resources/dashforge-model.tsv` is packaged into the mod. In-game, use DashForge's generator entry and check whether the summary says the learned model is active. If no valid model is bundled, the generator reports that it used the heuristic fallback.

## What the model learns

The baseline is a weighted object-transition model, not a neural network. It learns:
- frequency of each native object ID,
- common consecutive object-ID pairs,
- average horizontal spacing associated with each object,
- average vertical position.

The current generator uses these statistics to produce a reproducible learned event/object-ID plan. It does **not** yet place those IDs into the Geometry Dash editor, validate object IDs against the target game build, handle triggers/gameplay semantics or save a playable level. Those are separate required engineering steps before calling generated output a finished level.

## Other trainer

`tools/train_model.py` is the older normalized-JSON frequency/transition baseline. For native Geometry Dash data, use `tools/train_level_model.py`.

## Design principles

- Offline generation; no paid API.
- Reproducible seeds.
- Keep historical era separate from a matrix cell used for a substitute.
- Never treat metadata as native level geometry.
- Never claim the corpus is complete until identities, IDs and native data have been checked.
- Do not overwrite the player's original save data; export to a separate level after editor integration is implemented.

## License

MIT. See [`LICENSE`](LICENSE).
