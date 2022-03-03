# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from argparse import ArgumentParser
from pathlib import Path

import pandas as pd
from power_grid_model import PowerGridModel
from power_grid_model.manual_testing import import_json_data

if __name__ == "__main__":
    parser = ArgumentParser(description="Run a load flow calculation on a Power Grid Model .json file")
    parser.add_argument("input", type=Path)
    args = parser.parse_args()

    # input_data
    input_data = import_json_data(Path("OS Lemmer_EN.json"), "input")

    # call constructor
    model = PowerGridModel(input_data, system_frequency=50.0)

    result = model.calculate_power_flow()

    for component, data in result.items():
        print(component.upper())
        print("=" * len(component))
        print(pd.DataFrame(result[component]))
