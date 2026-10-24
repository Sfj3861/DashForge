# DashForge

DashForge is an experimental Geode mod for Geometry Dash intended to generate level layouts locally. The goal is offline generation, with an optional model trained on user-selected levels. No paid AI API is required by the design.

## Current status: prototype foundation

Included:
- Geode mod metadata and CMake project scaffold.
- Main-menu entry point following the Geode example-mod pattern.
- Deterministic, seeded C++ event-plan generator.
- Versioned starter catalog schema.
- Offline Python baseline trainer for normalized event-sequence JSON.

Not implemented yet:
- Verified object IDs/catalog for a real Geometry Dash version.
- Conversion of generated events to actual Geometry Dash objects.
- Level editor integration, import/export, and playable-level save.
- Parsing user-selected levels from Geometry Dash save data.
- In-game model loading/inference and decoration generation.
- Automated compilation or runtime verification in this environment.

The UI explicitly describes the current generator as an internal prototype plan, not a playable level exporter.

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
    {"type": "ground", "x": 0},
    {"type": "spike", "x": 30},
    {"type": "orb", "x": 75}
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

MIT. See [LICENSE](LICENSE).
