# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path
from typing import cast

from power_grid_model import CalculationType
from power_grid_model.manual_testing import import_json_data
from power_grid_model.validation import (
    InputData,
    UpdateData,
    errors_to_string,
    validate_batch_data,
)

input_file = Path("../tests/data/power_flow/dummy-test-batch/input.json")
update_file = Path("../tests/data/power_flow/dummy-test-batch/update_batch.json")

input_data = cast(InputData, import_json_data(json_file=input_file, data_type="input"))
update_data = cast(UpdateData, import_json_data(json_file=update_file, data_type="update"))

update_errors = validate_batch_data(
    input_data=input_data, update_data=update_data, calculation_type=CalculationType.power_flow, symmetric=True
)

print(errors_to_string(update_errors, name=str(update_file), details=True))
