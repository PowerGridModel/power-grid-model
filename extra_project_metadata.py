# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""Provide Python package entry points outside Conda builds.

Regular builds expose the CMake root entry point and Python CLI shim through
package metadata. Conda builds use the native CLI executable directly, so provide empty tables for
these fields to avoid installing a Python shim that could conflict with it.
"""

import os
from collections.abc import Mapping
from typing import Any


def dynamic_metadata(_settings: Mapping[str, Any], _project: Mapping[str, Any]) -> dict[str, Any]:
    if "CONDA_BUILD" in os.environ:
        return {"entry-points": {}, "scripts": {}}

    return {
        "entry-points": {"cmake.root": {"power_grid_model": "power_grid_model._core.power_grid_model_c"}},
        "scripts": {"power-grid-model": "power_grid_model._core.power_grid_model_c.run_pgm_cli:main"},
    }
