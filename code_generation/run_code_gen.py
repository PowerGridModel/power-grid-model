# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path

from render_templates import code_gen

if __name__ == "__main__":
    code_gen(Path(__file__).parent / "output")
