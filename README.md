# DashForge

DashForge is an experimental Geode mod for Geometry Dash intended to generate level layouts locally. The goal is offline generation, with an optional model trained on user-selected levels. No paid AI API is required by the design.

## Current status: prototype foundation

Included:
- Geode mod metadata and CMake project scaffold.
- Main-menu entry point with separate generator and corpus-status buttons.
- Deterministic, seeded C++ event-plan generator.
- Starter catalogue of mandatory historical/community level examples with explicit verification states.
- Corpus status report in-game, including the 1,820-slot target and clear warnings about missing verification/native data.
- Versioned corpus specification and JSON seed manifest.
- Offline Python baseline trainer for normalized event-sequence JSON.

Not implemented yet:
- Completed and verified set of all 1,820 named community levels.
- Native level strings for the mandatory seed levels and full corpus.
- Importing/parsing downloaded GD level strings in the mod.
- Verified object IDs/catalog for a real Geometry Dash version.
- Conversion of generated events to actual Geometry Dash objects.
- Level editor integration, import/export, and playable-level save.
- In-game model loading/inference and decoration generation.
- Automated compilation or runtime verification in this environment.

The UI explicitly describes the current generator as an internal prototype plan, not a playable level exporter. The corpus button is a status/seed-catalogue view; it does not claim the 1,820-level dataset is complete or train the generator from level data yet.

## Historical learning corpus

The target and integrity rules are specified in [`data/corpus-plan.json`](data/corpus-plan.json). The in-game seed catalogue is in [`data/community-levels-seed.json`](data/community-levels-seed.json), with a C++ status view in [`src/Corpus.cpp`](src/Corpus.cpp).

Corpus target:
- 13 eras: 1.0–1.9, 2.0, 2.1 and 2.2 (all 2.2-derived patches grouped into 2.2).
- 14 analytical difficulty/mode cohorts.
- 10 community examples per era/cohort cell: 1,820 target slots.
- Include all available official RobTop levels separately, with no 10-per-cell limit.
- Include mandatory examples such as SuperSonic, Sonic Wave, Nine Circles, Bloodbath, Sonar, Level Easy, Future Funk, ReTraY and Darnoc's Chill Out.
- Where eligible rated levels are not available for an early-era cell, Gauntlet substitutes must be flagged and keep their true release era.
- Keep native level strings and source/provenance; metadata, screenshots and videos alone are not sufficient for learning object placement.

The seed catalogue intentionally marks entries as candidates or requiring verification. Some IDs and era/category assignments are provisional; they must be checked against authoritative sources before training. The metadata-only public dataset does not contain native level strings. Do not claim the corpus is complete until every slot, identity, rating/era and native data source has been verified.

## Build

Install the Geode SDK and Geode CLI for your target platform, then set GEODE_SDK to the SDK directory. From the repository root:

```sh
geode build
```

The CMake project requires CMake 3.21+ and a C++20 compiler. Platform support depends on the Geode SDK and GD version declared in mod.json.

## Offline baseline trainer

The trainer expects normalized JSON sequences, not raw GD saves. Example input:

```json
{
  "events": [
    { "type": "ground", "x": 0 },
    { "type": "spike", "x": 30 },
    { "type": "orb", "x": 75 }
  ]
}
```

Train the baseline model:

```sh
python tools/train_model.py --dataset ./normalized-levels --output ./dashforge-model.json
```

This baseline is a frequency/transition model, not a neural network. Importing and normalizing real Geometry Dash levels must be implemented before it can learn directly from levels selected in-game.

## Design principles

- Offline-first: no network calls or paid API dependencies in generation.
- Reproducibility: a seed recreates the same prototype sequence.
- Versioned catalogs: game object definitions must be validated per target GD version.
- Safe saves: do not overwrite original save files; use Geode-supported APIs and a separate output level.
- Honest validation: do not describe generated content as playable until it has been opened and tested in Geometry Dash.

## License

MIT. See [`LICENSE`](LICENSE).
