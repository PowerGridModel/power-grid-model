# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from copy import copy
from pathlib import Path

import numpy as np
import pytest

from power_grid_model import PowerGridModel
from power_grid_model.utils import convert_batch_dataset_to_batch_list

from .utils import EXPORT_OUTPUT, compare_result, import_case_data, pytest_cases, save_json_data

calculation_function_map = {
    "power_flow": PowerGridModel.calculate_power_flow,
    "state_estimation": PowerGridModel.calculate_state_estimation,
}


@pytest.mark.parametrize(
    ["case_id", "case_path", "sym", "calculation_type", "calculation_method", "rtol", "atol"],
    pytest_cases(get_batch_cases=False),
)
def test_single_validation(
    case_id: str, case_path: Path, sym: bool, calculation_type: str, calculation_method: str, rtol: float, atol: float
):
    # Initialization
    case_data = import_case_data(case_path, sym=sym)
    model = PowerGridModel(case_data["input"], system_frequency=50.0)

    # Normal calculation
    result = calculation_function_map[calculation_type](model, symmetric=sym, calculation_method=calculation_method)

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


@pytest.mark.parametrize(
    [
        "case_id",
        "case_path",
        "sym",
        "calculation_type",
        "calculation_method",
        "rtol",
        "atol",
        "independent",
        "cache_topology",
    ],
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
    independent: bool,
    cache_topology: bool,
):
    # Initialization
    case_data = import_case_data(case_path, sym=sym)
    model = PowerGridModel(case_data["input"], system_frequency=50.0)
    update_batch = case_data["update_batch"]
    update_list = convert_batch_dataset_to_batch_list(update_batch)
    reference_output_batch = case_data["output_batch"]
    reference_output_list = convert_batch_dataset_to_batch_list(reference_output_batch)

    # execute batch calculation by applying update method
    for update_data, reference_result in zip(update_list, reference_output_list):
        model_copy = copy(model)
        model_copy.update(update_data=update_data)
        result = calculation_function_map[calculation_type](
            model_copy, symmetric=sym, calculation_method=calculation_method
        )
        compare_result(result, reference_result, rtol, atol)

    # execute in batch one go
    for threading in [-1, 0, 1, 2]:
        result_batch = calculation_function_map[calculation_type](
            model, symmetric=sym, calculation_method=calculation_method, update_data=update_batch, threading=threading
        )
        result_list = convert_batch_dataset_to_batch_list(result_batch)
        for result, reference_result in zip(result_list, reference_output_list):
            compare_result(result, reference_result, rtol, atol)
        # assert batch parameters
        assert independent == model.independent
        assert cache_topology == model.cache_topology

    # export data if needed
    if EXPORT_OUTPUT:
        save_json_data(f"{case_id}.json", result_batch)
