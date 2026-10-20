# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import os
from collections.abc import Mapping
from typing import Any


def dynamic_metadata(_settings: Mapping[str, Any], _project: Mapping[str, Any]) -> dict[str, Any]:
    if "CONDA_BUILD" in os.environ:
        return {}

    return {
        "entry-points": {"cmake.root": {"power_grid_model": "power_grid_model._core.power_grid_model_c"}},
        "scripts": {"power-grid-model": "power_grid_model._core.power_grid_model_c.run_pgm_cli:main"},
    }
