# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""Format JSON files in-place with a 2-space indent."""

import json
import re
import sys
from pathlib import Path

REPO_PATH = Path(__file__).parent.parent.resolve()
EXCLUDE = [
    re.compile(rf"{REPO_PATH}/docs/.*"),
    re.compile(rf"{REPO_PATH}/package-lock\.json"),
    re.compile(rf"{REPO_PATH}/\.vscode/.*"),
    re.compile(rf"{REPO_PATH}/\.devcontainer/.*"),
    re.compile(rf"{REPO_PATH}/tests/data/.*/input\.json"),
    re.compile(rf"{REPO_PATH}/tests/data/.*/.*_output.*\.json"),
    re.compile(rf"{REPO_PATH}/tests/data/.*/update_batch\.json"),
    re.compile(rf"{REPO_PATH}/tests/unit/deprecated/data/."),
    re.compile(rf"{REPO_PATH}/uv.lock"),
]


def format_file(path: str) -> bool:
    """Reformat a single JSON file in place. Return True if it changed."""
    file_path = Path(path)
    original = file_path.read_text(encoding="utf-8-sig")

    data = json.loads(original)
    formatted = (
        json.dumps(
            data,
            indent=2,
            ensure_ascii=True,
            sort_keys=False,
        )
        + "\n"
    )

    if formatted == original:
        return False

    file_path.write_text(formatted, encoding="utf-8-sig")
    return True


def to_include(paths: list[str]) -> list[str]:
    def _include_path(path: str) -> bool:
        """Return True if the path is a JSON file and not excluded."""
        p = Path(path)
        full_path = str(p.resolve())
        return p.suffix in (".json", ".jsonc") and not any(rex.match(full_path) for rex in EXCLUDE)

    return [path for path in paths if _include_path(path)]


def main(paths: list[str]) -> int:
    format_paths = to_include(paths)
    changed = [path for path in format_paths if format_file(path)]
    for path in changed:
        print(f"reformatted {path}")
    return 1 if changed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
