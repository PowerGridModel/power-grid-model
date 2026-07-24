# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import subprocess

from power_grid_model import __version__


def test_cli_version():
    result = subprocess.run(["power-grid-model", "--version"], capture_output=True, text=True, check=True)
    assert __version__ in result.stdout
