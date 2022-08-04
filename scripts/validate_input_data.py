# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path

from power_grid_model import CalculationType
from power_grid_model.utils import import_input_data
from power_grid_model.validation import errors_to_string, validate_input_data

input_file = Path("../tests/data/state_estimation/dummy-test-sym/input.json")

input_data = import_input_data(json_file=input_file)

input_errors = validate_input_data(
    input_data=input_data, calculation_type=CalculationType.state_estimation, symmetric=True
)

print(errors_to_string(input_errors, name=str(input_file), details=True))
