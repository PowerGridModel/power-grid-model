# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path

from power_grid_model import CalculationType
from power_grid_model.utils import import_input_data, import_update_data
from power_grid_model.validation import errors_to_string, validate_batch_data

input_file = Path("../tests/data/power_flow/dummy-test-batch/input.json")
update_file = Path("../tests/data/power_flow/dummy-test-batch/update_batch.json")

input_data = import_input_data(json_file=input_file)
update_data = import_update_data(json_file=update_file)

update_errors = validate_batch_data(
    input_data=input_data, update_data=update_data, calculation_type=CalculationType.power_flow, symmetric=True
)

print(errors_to_string(update_errors, name=str(update_file), details=True))
