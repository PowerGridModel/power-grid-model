# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import sys
from pathlib import Path

BASE_DIR = Path("docs/user_manual")
OUTPUT_FILE = BASE_DIR / "components.md"

files = sorted(BASE_DIR.glob("*components*.md"))

if not files:
    print("No component files found.")
    sys.exit(0)

output = []

output.append("<!-- AUTO-GENERATED FILE - DO NOT EDIT -->\n")

for f in files:
    content = f.read_text(encoding="utf-8")

    output.append(f"\n\n<!-- ===== {f.name} ===== -->\n")
    output.append(content.strip())

OUTPUT_FILE.write_text("\n".join(output), encoding="utf-8")

print(f"Built: {OUTPUT_FILE}")
