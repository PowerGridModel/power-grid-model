#!/usr/bin/env python3
# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0
"""Build the generated fuzzing material from its hand-maintained sources.

Every harness takes a single document verbatim, so there are only two corpora:

* ``corpus_json/`` — hand-maintained documents of every kind. Shared by every
  harness except ``fuzz_model_batch_update``. This script adds columnar
  variants of its row-oriented documents.
* ``corpus_update/`` — generated here: ``update`` documents targeting the fixed
  grid in ``update_grid.json``, for ``fuzz_model_batch_update``.

It also renders ``update_grid.json`` into ``update_grid.h``, the C string the
batch harness builds its model from, so the grid has a single source.

Run from anywhere; paths are resolved relative to this file.

    python3 tools/make_corpus.py

Regenerating is idempotent: generated outputs are rebuilt from scratch.
"""

from __future__ import annotations

import json
import shutil
from pathlib import Path

FUZZER_DIR = Path(__file__).resolve().parent.parent
CORPUS_JSON = FUZZER_DIR / "corpus_json"
CORPUS_UPDATE = FUZZER_DIR / "corpus_update"
UPDATE_GRID_JSON = FUZZER_DIR / "update_grid.json"
UPDATE_GRID_HEADER = FUZZER_DIR / "update_grid.h"


def load_document(path: Path) -> dict | None:
    try:
        doc = json.loads(path.read_text())
    except (ValueError, UnicodeDecodeError):
        return None
    return doc if isinstance(doc, dict) else None


# ---------------------------------------------------------------------------
# Columnar variants
#
# The columnar serialization ("attributes": {component: [names...]} with data
# rows as positional arrays) is a separate branch of the deserializer and the
# only thing that populates attribute indications. corpus_json has no columnar
# entries, so derive them from the row-oriented ones.
# ---------------------------------------------------------------------------


def to_columnar(doc: dict) -> dict | None:
    data = doc.get("data")
    if not isinstance(data, dict) or not data:
        return None
    if doc.get("is_batch"):
        return None  # keep the transform simple: single datasets only

    attributes: dict[str, list[str]] = {}
    columnar_data: dict[str, list] = {}

    for component, elements in data.items():
        if not isinstance(elements, list) or not elements:
            return None
        if not all(isinstance(e, dict) for e in elements):
            return None
        # Every element must declare the same attributes for a positional
        # encoding to be lossless.
        names = list(elements[0].keys())
        if any(list(e.keys()) != names for e in elements):
            return None
        attributes[component] = names
        columnar_data[component] = [[e[n] for n in names] for e in elements]

    if not attributes:
        return None

    return {
        "version": doc.get("version", "1.0"),
        "type": doc.get("type", "input"),
        "is_batch": False,
        "attributes": attributes,
        "data": columnar_data,
    }


def write_columnar_variants() -> int:
    written = 0
    for path in sorted(CORPUS_JSON.glob("*.json")):
        if path.name.startswith("24_columnar"):
            continue  # generated output; do not feed it back in
        doc = load_document(path)
        if doc is None:
            continue
        columnar = to_columnar(doc)
        if columnar is None:
            continue
        out = CORPUS_JSON / f"24_columnar_{path.stem}.json"
        out.write_text(json.dumps(columnar, indent=2) + "\n")
        written += 1
    return written


# ---------------------------------------------------------------------------
# Update documents for the batch target
#
# They target update_grid.json, the fixed grid fuzz_model_batch_update builds
# its model from, so every component below must exist in that grid.
# ---------------------------------------------------------------------------

# Components whose updatable attributes are simple enough to synthesize, mapped
# to the per-scenario values to sweep.
UPDATABLE = {
    "sym_load": ("p_specified", [1000.0, 50000.0, -25000.0]),
    "asym_load": ("p_specified", [[1000.0, 1000.0, 1000.0], [5000.0, 1000.0, 0.0], [0.0, 0.0, 0.0]]),
    "sym_gen": ("p_specified", [500.0, 25000.0, 0.0]),
    "asym_gen": ("p_specified", [[500.0, 500.0, 500.0], [0.0, 1000.0, 0.0], [0.0, 0.0, 0.0]]),
    "source": ("u_ref", [1.0, 1.05, 0.95]),
    "line": ("from_status", [1, 0, 1]),
    "transformer": ("tap_pos", [0, 1, -1]),
}


def make_update(doc: dict, uniform: bool) -> dict | None:
    """Build an update dataset targeting components the input actually declares.

    Note the shape of ``data``. A batch document is a *list of scenarios*, each
    scenario a map of component -> elements — not a map of component -> list of
    per-scenario lists. The deserializer checks this explicitly ("Map/Array type
    of data does not match is_batch!") and rejects the map form outright, so the
    wrong shape produces a document that never gets past the parser.

    ``uniform`` picks between a rectangular batch (every scenario updates the
    same elements) and a ragged one (scenario N updates N elements). The ragged
    shape is what forces the deserializer down the indptr path, so both are
    generated.
    """
    data = doc.get("data")
    if not isinstance(data, dict) or doc.get("is_batch"):
        return None

    # component -> (attribute, per-scenario values) for components present here
    present = []
    for component, (attribute, values) in UPDATABLE.items():
        elements = data.get(component)
        if not isinstance(elements, list) or not elements:
            continue
        ids = [e.get("id") for e in elements if isinstance(e, dict) and "id" in e]
        if ids:
            present.append((component, attribute, values, ids))

    if not present:
        return None

    n_scenarios = max(len(values) for _, _, values, _ in present)
    scenarios: list[dict] = []
    for s in range(n_scenarios):
        scenario: dict[str, list] = {}
        for component, attribute, values, ids in present:
            if s >= len(values):
                continue
            if uniform:
                chosen = ids
            else:
                # 0 elements in the first scenario, then 1, then 2 — ragged, and
                # an empty scenario is a legitimate edge case in its own right.
                chosen = ids[: min(s, len(ids))]
            scenario[component] = [{"id": i, attribute: values[s]} for i in chosen]
        scenarios.append(scenario)

    return {
        "version": doc.get("version", "1.0"),
        "type": "update",
        "is_batch": True,
        "attributes": {},
        "data": scenarios,
    }


def make_single_update(doc: dict) -> dict | None:
    """The first scenario of the uniform batch as a non-batch document — the
    shape PGM_update_model takes."""
    batch = make_update(doc, uniform=True)
    if batch is None or not batch["data"]:
        return None
    return {**batch, "is_batch": False, "data": batch["data"][0]}


# ---------------------------------------------------------------------------
# Writers
# ---------------------------------------------------------------------------


def reset(directory: Path) -> None:
    if directory.exists():
        shutil.rmtree(directory)
    directory.mkdir(parents=True)


def write_update_grid_header(grid: dict) -> None:
    """Render the grid as compact JSON in a C string literal, split across lines."""
    compact = json.dumps(grid, separators=(",", ":"))
    chunks = [compact[i : i + 100] for i in range(0, len(compact), 100)]
    # json.dumps output contains no control characters, so escaping quotes and
    # backslashes is all a C string literal needs. Escape per chunk so a split
    # never lands inside an escape sequence.
    body = "\n".join('    "' + c.replace("\\", "\\\\").replace('"', '\\"') + '"' for c in chunks)
    UPDATE_GRID_HEADER.write_text(
        "// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>\n"
        "//\n"
        "// SPDX-License-Identifier: MPL-2.0\n"
        "\n"
        "// GENERATED by tools/make_corpus.py from update_grid.json — do not edit.\n"
        "//\n"
        "// The fixed grid that fuzz_model_batch_update applies fuzzed update documents to.\n"
        "\n"
        "#ifndef PGM_FUZZ_UPDATE_GRID_H\n"
        "#define PGM_FUZZ_UPDATE_GRID_H\n"
        "\n"
        "static char const update_grid_json[] =\n"
        f"{body};\n"
        "\n"
        "#endif /* PGM_FUZZ_UPDATE_GRID_H */\n"
    )


def write_update_corpus(grid: dict) -> int:
    reset(CORPUS_UPDATE)
    written = 0
    for label, update in (
        ("batch_uniform", make_update(grid, uniform=True)),
        ("batch_ragged", make_update(grid, uniform=False)),
        ("single", make_single_update(grid)),
    ):
        if update is None:
            continue
        (CORPUS_UPDATE / f"{label}.json").write_text(json.dumps(update, indent=2) + "\n")
        written += 1
    return written


def main() -> None:
    if not CORPUS_JSON.is_dir():
        raise SystemExit(f"corpus_json not found at {CORPUS_JSON}")

    grid = load_document(UPDATE_GRID_JSON)
    if grid is None:
        raise SystemExit(f"{UPDATE_GRID_JSON} is not a JSON object")

    columnar = write_columnar_variants()
    write_update_grid_header(grid)
    update = write_update_corpus(grid)

    print(f"corpus_json:   +{columnar} columnar variants ({len(list(CORPUS_JSON.iterdir()))} total)")
    print(f"corpus_update: {update} entries")
    print("update_grid.h: regenerated")


if __name__ == "__main__":
    main()
