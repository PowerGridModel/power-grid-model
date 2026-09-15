# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# This script modifies the pyproject.toml file to remove specific sections
#   [project.entry-points."cmake.root"] and [project.scripts]
# and deletes the run_pgm_cli.py file from the source directory.
# It is intended to be run as part of the conda build preparation process.
# So that conda environment will not be confused with PyPI style Python shim and entry points.

import re
from pathlib import Path

# Read the root pyproject.toml
pyproject_path = Path(__file__).parent / "pyproject.toml"
content = pyproject_path.read_text()

# Remove [project.entry-points."cmake.root"] section
content = re.sub(r'\n\[project\.entry-points\."cmake\.root"\].*?(?=\n\[|\Z)', "", content, flags=re.DOTALL)

# Remove [project.scripts] section
content = re.sub(r"\n\[project\.scripts\].*?(?=\n\[|\Z)", "", content, flags=re.DOTALL)

# Write back to pyproject.toml
pyproject_path.write_text(content)

# Remove run_pgm_cli.py file
run_pgm_cli_path = (
    Path(__file__).parent / "src" / "power_grid_model" / "_core" / "power_grid_model_c" / "run_pgm_cli.py"
)
run_pgm_cli_path.unlink(missing_ok=True)
