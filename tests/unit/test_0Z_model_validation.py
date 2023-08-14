# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from copy import copy
from pathlib import Path
from typing import Callable, Dict, List, Tuple

import numpy as np
import pytest

from power_grid_model import PowerGridModel
from power_grid_model.utils import convert_batch_dataset_to_batch_list

from .utils import EXPORT_OUTPUT, compare_result, import_case_data, pytest_cases, save_json_data

calculation_function_arguments_map: Dict[str, Tuple[Callable, List[str]]] = {
    "power_flow": (
        PowerGridModel.calculate_power_flow,
        [
            "symmetric",
            "error_tolerance",
            "max_iterations",
            "calculation_method",
            "update_data",
            "threading",
            "output_component_types",
            "continue_on_batch_error",
        ],
    ),
    "state_estimation": (
        PowerGridModel.calculate_state_estimation,
        [
            "symmetric",
            "error_tolerance",
            "max_iterations",
            "calculation_method",
            "update_data",
            "threading",
            "output_component_types",
            "continue_on_batch_error",
        ],
    ),
    "short_circuit": (
        PowerGridModel.calculate_short_circuit,
        [
            "calculation_method",
            "update_data",
            "threading",
            "output_component_types",
            "continue_on_batch_error",
        ],
    ),
}


def supported_kwargs(kwargs, supported: List[str]):
    return {key: value for key, value in kwargs.items() if key in supported}


@pytest.mark.parametrize(
    ["case_id", "case_path", "sym", "calculation_type", "calculation_method", "rtol", "atol"],
    pytest_cases(get_batch_cases=False),
)
def test_single_validation(
    case_id: str, case_path: Path, sym: bool, calculation_type: str, calculation_method: str, rtol: float, atol: float
):
    # Initialization
    case_data = import_case_data(case_path, calculation_type=calculation_type, sym=sym)
    model = PowerGridModel(case_data["input"], system_frequency=50.0)

    # Normal calculation
    calculation_function, calculation_args = calculation_function_arguments_map[calculation_type]
    kwargs = {"symmetric": sym, "calculation_method": calculation_method}
    result = calculation_function(model, **supported_kwargs(kwargs=kwargs, supported=calculation_args))

    # Compare the results
    reference_result = case_data["output"]
    compare_result(result, reference_result, rtol, atol)

    # test get indexer
    for component_name, input_array in case_data["input"].items():
        ids_array = input_array["id"].copy()
        np.random.shuffle(ids_array)
        indexer_array = model.get_indexer(component_name, ids_array)
        # check
        assert np.all(input_array["id"][indexer_array] == ids_array)

    # export data if needed
    if EXPORT_OUTPUT:
        save_json_data(f"{case_id}.json", result)

    # test calculate with only node and source result
    kwargs = {"symmetric": sym, "calculation_method": calculation_method, "output_component_types": ["node", "source"]}
    result = calculation_function(model, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
    assert set(result.keys()) == {"node", "source"}
    kwargs = {"symmetric": sym, "calculation_method": calculation_method, "output_component_types": {"node", "source"}}
    result = calculation_function(model, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
    assert set(result.keys()) == {"node", "source"}


@pytest.mark.parametrize(
    ["case_id", "case_path", "sym", "calculation_type", "calculation_method", "rtol", "atol"],
    pytest_cases(get_batch_cases=True),
)
def test_batch_validation(
    case_id: str, case_path: Path, sym: bool, calculation_type: str, calculation_method: str, rtol: float, atol: float
):
    # Initialization
    case_data = import_case_data(case_path, calculation_type=calculation_type, sym=sym)
    model = PowerGridModel(case_data["input"], system_frequency=50.0)
    update_batch = case_data["update_batch"]
    update_list = convert_batch_dataset_to_batch_list(update_batch)
    reference_output_batch = case_data["output_batch"]
    reference_output_list = convert_batch_dataset_to_batch_list(reference_output_batch)

    # execute batch calculation by applying update method
    for update_data, reference_result in zip(update_list, reference_output_list):
        model_copy = copy(model)
        model_copy.update(update_data=update_data)
        calculation_function, calculation_args = calculation_function_arguments_map[calculation_type]
        kwargs = {"symmetric": sym, "calculation_method": calculation_method}
        result = calculation_function(model_copy, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
        compare_result(result, reference_result, rtol, atol)

    # execute in batch one go
    for threading in [-1, 0, 1, 2]:
        calculation_function, calculation_args = calculation_function_arguments_map[calculation_type]
        kwargs = {
            "symmetric": sym,
            "calculation_method": calculation_method,
            "update_data": update_batch,
            "threading": threading,
        }
        result_batch = calculation_function(model, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
        result_list = convert_batch_dataset_to_batch_list(result_batch)
        for result, reference_result in zip(result_list, reference_output_list):
            compare_result(result, reference_result, rtol, atol)

    # export data if needed
    if EXPORT_OUTPUT:
        save_json_data(f"{case_id}.json", result_batch)
