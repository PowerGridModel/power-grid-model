# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path
from typing import cast

from power_grid_model import CalculationType
from power_grid_model.manual_testing import import_json_data
from power_grid_model.validation import InputData, errors_to_string, validate_input_data

input_file = Path("../tests/data/state_estimation/dummy-test-sym/input.json")

input_data = cast(InputData, import_json_data(json_file=input_file, data_type="input"))

input_errors = validate_input_data(
    input_data=input_data, calculation_type=CalculationType.state_estimation, symmetric=True
)

print(errors_to_string(input_errors, name=str(input_file), details=True))
