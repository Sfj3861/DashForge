#!/usr/bin/env python3
"""Offline DashForge baseline trainer for normalized JSON event sequences."""
from __future__ import annotations
import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", type=Path, required=True, help="Folder containing normalized *.json sequences")
    parser.add_argument("--output", type=Path, default=Path("dashforge-model.json"))
    args = parser.parse_args()
    files = sorted(args.dataset.glob("*.json"))
    if not files:
        parser.error(f"No JSON training files found in {args.dataset}")
    transitions = defaultdict(Counter)
    event_counts = Counter()
    levels_used = 0
    for path in files:
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
            events = data.get("events")
            if not isinstance(events, list) or not events:
                continue
            sequence = [str(event["type"]) for event in events if isinstance(event, dict) and "type" in event]
            if not sequence:
                continue
            levels_used += 1
            event_counts.update(sequence)
            for previous, following in zip(sequence, sequence[1:]):
                transitions[previous][following] += 1
        except (OSError, json.JSONDecodeError, TypeError) as exc:
            print(f"Skipping {path}: {exc}")
    if not levels_used:
        parser.error("No valid event sequences found.")
    model = {
        "format": "dashforge-transition-model",
        "format_version": 1,
        "training_levels": levels_used,
        "event_counts": dict(event_counts),
        "transitions": {key: dict(value) for key, value in transitions.items()},
        "limitations": [
            "Baseline frequency/transition model, not a neural network.",
            "Requires normalized JSON; GD level import/normalization is not implemented.",
            "Generated sequences must be validated before conversion to playable levels."
        ]
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(model, indent=2), encoding="utf-8")
    print(f"Saved baseline model from {levels_used} level(s) to {args.output}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
