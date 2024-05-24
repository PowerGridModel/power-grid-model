# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from copy import copy
from pathlib import Path
from typing import Callable, Dict, List, Tuple

import numpy as np
import pytest

from power_grid_model._utils import convert_batch_dataset_to_batch_list
from power_grid_model.enum import TapChangingStrategy

from .utils import EXPORT_OUTPUT, PowerGridModelWithExt, compare_result, import_case_data, pytest_cases, save_json_data

calculation_function_arguments_map: Dict[str, Tuple[Callable, List[str]]] = {
    "power_flow": (
        PowerGridModelWithExt.calculate_power_flow_with_ext,
        [
            "symmetric",
            "error_tolerance",
            "max_iterations",
            "calculation_method",
            "update_data",
            "threading",
            "output_component_types",
            "continue_on_batch_error",
            "tap_changing_strategy",
            "experimental_features",
        ],
    ),
    "state_estimation": (
        PowerGridModelWithExt.calculate_state_estimation_with_ext,
        [
            "symmetric",
            "error_tolerance",
            "max_iterations",
            "calculation_method",
            "update_data",
            "threading",
            "output_component_types",
            "continue_on_batch_error",
            "experimental_features",
        ],
    ),
    "short_circuit": (
        PowerGridModelWithExt.calculate_short_circuit_with_ext,
        [
            "calculation_method",
            "update_data",
            "threading",
            "output_component_types",
            "continue_on_batch_error",
            "short_circuit_voltage_scaling",
            "experimental_features",
        ],
    ),
}


def supported_kwargs(kwargs, supported: List[str]):
    return {key: value for key, value in kwargs.items() if key in supported}


def get_kwargs(sym: bool, calculation_type: str, calculation_method: str, params: Dict, **extra_kwargs) -> Dict:
    base_kwargs = {"symmetric": sym, "calculation_method": calculation_method}
    for key, value in params.items():
        if key not in base_kwargs:
            if not isinstance(value, dict):
                base_kwargs[key] = value
            elif calculation_method in value:
                base_kwargs[key] = value[calculation_method]

    for key, value in extra_kwargs.items():
        base_kwargs[key] = value

    if calculation_method == "iec60909":
        base_kwargs["short_circuit_voltage_scaling"] = params["short_circuit_voltage_scaling"]

    if calculation_type == "power_flow":
        strategy_name = params.get("tap_changing_strategy", "disabled")
        base_kwargs["tap_changing_strategy"] = TapChangingStrategy[strategy_name].value

    return base_kwargs


@pytest.mark.parametrize(
    ["case_id", "case_path", "sym", "calculation_type", "calculation_method", "rtol", "atol", "params"],
    pytest_cases(get_batch_cases=False),
)
def test_single_validation(
    case_id: str,
    case_path: Path,
    sym: bool,
    calculation_type: str,
    calculation_method: str,
    rtol: float,
    atol: float,
    params: Dict,
):
    # Initialization
    case_data = import_case_data(case_path, calculation_type=calculation_type, sym=sym)
    model = PowerGridModelWithExt(case_data["input"], system_frequency=50.0)

    # Normal calculation
    calculation_function, calculation_args = calculation_function_arguments_map[calculation_type]

    base_kwargs = get_kwargs(
        sym=sym, calculation_type=calculation_type, calculation_method=calculation_method, params=params
    )
    result = calculation_function(model, **supported_kwargs(kwargs=base_kwargs, supported=calculation_args))

    # export data if needed
    if EXPORT_OUTPUT:
        save_json_data(f"{case_id}.json", result)

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

    # test calculate with only node and source result
    kwargs = dict(base_kwargs, **{"output_component_types": ["node", "source"]})
    result = calculation_function(model, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
    assert set(result.keys()) == {"node", "source"}
    kwargs = dict(base_kwargs, **{"output_component_types": {"node", "source"}})
    result = calculation_function(model, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
    assert set(result.keys()) == {"node", "source"}


@pytest.mark.parametrize(
    ["case_id", "case_path", "sym", "calculation_type", "calculation_method", "rtol", "atol", "params"],
    pytest_cases(get_batch_cases=True),
)
def test_batch_validation(
    case_id: str,
    case_path: Path,
    sym: bool,
    calculation_type: str,
    calculation_method: str,
    rtol: float,
    atol: float,
    params: Dict,
):
    # Initialization
    case_data = import_case_data(case_path, calculation_type=calculation_type, sym=sym)
    model = PowerGridModelWithExt(case_data["input"], system_frequency=50.0)
    update_batch = case_data["update_batch"]
    update_list = convert_batch_dataset_to_batch_list(update_batch)
    reference_output_batch = case_data["output_batch"]
    reference_output_list = convert_batch_dataset_to_batch_list(reference_output_batch)

    base_kwargs = get_kwargs(
        sym=sym, calculation_type=calculation_type, calculation_method=calculation_method, params=params
    )

    calculation_function, calculation_args = calculation_function_arguments_map[calculation_type]

    # export data without comparing first, if needed
    if EXPORT_OUTPUT:
        model_copy = copy(model)
        kwargs = dict(base_kwargs, update_data=update_batch)
        result_batch = calculation_function(model_copy, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
        save_json_data(f"{case_id}.json", result_batch)

    # execute batch calculation by applying update method
    for update_data, reference_result in zip(update_list, reference_output_list):
        model_copy = copy(model)
        model_copy.update(update_data=update_data)
        result = calculation_function(model_copy, **supported_kwargs(kwargs=base_kwargs, supported=calculation_args))
        compare_result(result, reference_result, rtol, atol)

    # execute in batch one go
    for threading in [-1, 0, 1, 2]:
        kwargs = dict(base_kwargs, update_data=update_batch, threading=threading)
        result_batch = calculation_function(model, **supported_kwargs(kwargs=kwargs, supported=calculation_args))
        result_list = convert_batch_dataset_to_batch_list(result_batch)
        for result, reference_result in zip(result_list, reference_output_list):
            compare_result(result, reference_result, rtol, atol)
