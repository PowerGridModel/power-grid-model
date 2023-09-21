# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import itertools
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict

import pytest

from power_grid_model._utils import convert_batch_dataset_to_batch_list
from power_grid_model.core.power_grid_model import PowerGridModel
from power_grid_model.utils import import_json_data
from tests.unit.utils import compare_result


@dataclass
class TestCase:
    params: Dict[str, Any]
    case_data: Dict[str, Any]


def load_test_case(test_case_dir: Path):
    case_data = {}
    params = {}

    for file_path in test_case_dir.glob("*.json"):
        if file_path.name == "params.json":
            with open(file_path, encoding="utf-8") as fp:
                params = json.load(fp)
        else:
            dataset_type = file_path.with_suffix("").name.replace("_batch", "")
            with pytest.deprecated_call():
                case_data[str(dataset_type)] = import_json_data(file_path, dataset_type)

    params["sym"] = []
    if "sym_output" in case_data:
        params["sym"].append(True)
    if "asym_output" in case_data:
        params["sym"].append(False)

    case_data["output"] = case_data.get("sym_output", case_data.get("asym_output"))

    return TestCase(params=params, case_data=case_data)


def deprecated_format_dummy_test_batch_dependent_not_cacheable():
    test_case_dir = Path(__file__).parent / "data/dummy-test-batch-dependent-not-cacheable"
    return load_test_case(test_case_dir)


@pytest.fixture(params=[deprecated_format_dummy_test_batch_dependent_not_cacheable()])
def deprecated_format_tests(request):
    return request.param


def test_end_to_end__deprecated_serialization_format(deprecated_format_tests):
    params = deprecated_format_tests.params
    case_data = deprecated_format_tests.case_data

    model = PowerGridModel(case_data["input"], system_frequency=50.0)
    update_batch = case_data["update"]

    for symmetric, calculation_method in itertools.product(params["sym"], params["calculation_method"]):
        reference_output_batch = case_data["sym_output" if symmetric else "asym_output"]
        reference_output_list = convert_batch_dataset_to_batch_list(reference_output_batch)

        result_batch = PowerGridModel.calculate_power_flow(
            model, update_data=update_batch, symmetric=symmetric, calculation_method=calculation_method
        )
        result_list = convert_batch_dataset_to_batch_list(result_batch)
        for result, reference_result in zip(result_list, reference_output_list):
            compare_result(result, reference_result, params["rtol"], params["atol"])
