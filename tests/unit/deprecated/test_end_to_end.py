# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path

import pytest

from power_grid_model._utils import convert_batch_dataset_to_batch_list
from power_grid_model.core.power_grid_model import PowerGridModel
from power_grid_model.utils import import_json_data

from ..utils import compare_result


@pytest.fixture()
def deprecated_format_test_case():
    return Path(__file__).parent / "data/dummy-test-batch-dependent-not-cacheable"


def test_end_to_end__deprecated_serialization_format_single(deprecated_format_test_case):
    with pytest.deprecated_call():
        input_data = import_json_data(deprecated_format_test_case / "input.json", "input")
        sym_output = import_json_data(deprecated_format_test_case / "sym_output.json", "sym_output")

    model = PowerGridModel(input_data, system_frequency=50.0)
    result = model.calculate_power_flow()

    compare_result(result, sym_output, 1e-5, 1e-5)


def test_end_to_end__deprecated_serialization_format_batch(deprecated_format_test_case):
    with pytest.deprecated_call():
        input_data = import_json_data(deprecated_format_test_case / "input.json", "input")
        update = import_json_data(deprecated_format_test_case / "update_batch.json", "update")
        sym_output = import_json_data(deprecated_format_test_case / "sym_output_batch.json", "sym_output")

    model = PowerGridModel(input_data, system_frequency=50.0)
    result = model.calculate_power_flow(update_data=update)

    for scenario_result, scenario_expected in zip(
        convert_batch_dataset_to_batch_list(result), convert_batch_dataset_to_batch_list(sym_output)
    ):
        compare_result(scenario_result, scenario_expected, 1e-5, 1e-5)
