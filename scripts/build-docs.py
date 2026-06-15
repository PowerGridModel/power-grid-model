# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import sys
from pathlib import Path
import re

BASE_DIR = Path("docs/user_manual")
OUTPUT_FILE = BASE_DIR / "components.md"

def extract_index(path: Path):
    # 从 001-components.md 提取 001
    match = re.match(r"(\d+)", path.name)
    return int(match.group(1)) if match else 9999

files = sorted(
    BASE_DIR.glob("*components*.md"),
    key=extract_index
)

if not files:
    print("No component files found.")
    sys.exit(0)

output = []
output.append("<!-- AUTO-GENERATED FILE - DO NOT EDIT -->\n")

for f in files:
    content = f.read_text(encoding="utf-8").strip()

    # 保留结构连续性，但加清晰分隔
    output.append(f"\n\n<!-- ===== {f.name} ===== -->\n")
    output.append(content)

OUTPUT_FILE.write_text("\n".join(output), encoding="utf-8")

print(f"Built: {OUTPUT_FILE}")