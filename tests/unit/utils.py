# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import json
import os
import re
from pathlib import Path
from typing import Any, Dict, List, Optional, Union

import numpy as np
import pytest
from power_grid_model.manual_testing import import_json_data, export_json_data

BASE_PATH = Path(__file__).parent.parent
DATA_PATH = BASE_PATH / "data"
OUPUT_PATH = BASE_PATH / "output"
EXPORT_OUTPUT = ("POWER_GRID_MODEL_VALIDATION_TEST_EXPORT" in os.environ) and (
    os.environ["POWER_GRID_MODEL_VALIDATION_TEST_EXPORT"] == "ON"
)


def pytest_cases(get_batch_cases: bool = False, data_dir: Optional[str] = None, test_cases: Optional[List[str]] = None):
    if data_dir is not None:
        relevant_calculations = [data_dir]
    else:
        # default both calculation type
        relevant_calculations = ["power_flow", "state_estimation"]
    batch_suffix = "_batch" if get_batch_cases else ""
    for calculation_type in relevant_calculations:
        # list of all cases, directories in validation datasets
        calculation_type_dir = DATA_PATH / calculation_type
        test_cases_paths = {
            str(item.relative_to(DATA_PATH)).replace("\\", "/"): item
            for item in calculation_type_dir.glob("**/")
            if (item.is_dir() and (item / "params.json").is_file())
        }
        if test_cases is not None:
            test_cases_paths = {key: value for key, value in test_cases_paths.items() if key in test_cases}
        for case_name, case_dir in test_cases_paths.items():
            with open(case_dir / "params.json") as f:
                params = json.load(f)
            # loop for sym or asym scenario
            for sym in [True, False]:
                output_prefix = "sym_output" if sym else "asym_output"
                # only generate a case if sym or asym output exists
                if (case_dir / f"{output_prefix}{batch_suffix}.json").exists():
                    # Build a recognizable case ID
                    case_id = case_name
                    case_id += "-sym" if sym else "-asym"
                    case_id += "-" + params["calculation_method"]
                    if get_batch_cases:
                        case_id += "-batch"
                    pytest_param = [
                        case_id,
                        case_dir,
                        sym,
                        calculation_type,
                        params["calculation_method"],
                        params["rtol"],
                        params["atol"],
                    ]
                    if get_batch_cases:
                        pytest_param += [params["independent"], params["cache_topology"]]
                    kwargs = {}
                    if "fail" in params:
                        kwargs["marks"] = pytest.mark.xfail(reason=params["fail"], raises=AssertionError)
                    yield pytest.param(*pytest_param, **kwargs, id=case_id)


def bool_params(true_id: str, false_id: Optional[str] = None, **kwargs):
    if false_id is None:
        false_id = f"not-{true_id}"
    yield pytest.param(False, **kwargs, id=false_id)
    yield pytest.param(True, **kwargs, id=true_id)


def dict_params(params: Dict[Any, str], **kwargs):
    for value, param_id in params.items():
        yield pytest.param(value, **kwargs, id=param_id)


def import_case_data(data_path: Path, sym: bool):
    output_prefix = "sym_output" if sym else "asym_output"
    return_dict = {
        "input": import_json_data(data_path / "input.json", "input"),
    }
    # import output if relevant
    if (data_path / f"{output_prefix}.json").exists():
        return_dict["output"] = import_json_data(data_path / f"{output_prefix}.json", output_prefix)
    # import update and output batch if relevant
    if (data_path / "update_batch.json").exists():
        return_dict["update_batch"] = import_json_data(data_path / "update_batch.json", "update")
        return_dict["output_batch"] = import_json_data(data_path / f"{output_prefix}_batch.json", output_prefix)
    return return_dict


def save_json_data(json_file: str, data: Union[dict, list]):
    OUPUT_PATH.mkdir(parents=True, exist_ok=True)
    data_file = OUPUT_PATH / json_file
    data_file.parent.mkdir(parents=True, exist_ok=True)
    export_json_data(data_file, data)


def compare_result(
    actual: Dict[str, np.ndarray], expected: Dict[str, np.ndarray], rtol: float, atol: Union[float, Dict[str, float]]
):
    for key, expected_data in expected.items():
        for col_name in expected_data.dtype.names:
            actual_col = actual[key][col_name]
            expected_col = expected_data[col_name]
            if expected_col.dtype == np.float64:
                expect_all_nan = np.all(np.isnan(expected_col))
            elif expected_col.dtype == np.int8:
                expect_all_nan = np.all(expected_col == np.iinfo("i1").min)
            elif expected_col.dtype == np.int32:
                expect_all_nan = np.all(expected_col == np.iinfo("i4").min)
            else:
                raise Exception(f"Unknown data type {expected_col.dtype}!")

            if not expect_all_nan:
                # permute expected_col if needed
                if expected_col.ndim == 1 and actual_col.ndim == 2:
                    if col_name == "u_angle":
                        # should be 120 and 240 degree lagging
                        expected_col = np.stack(
                            (expected_col, expected_col - 2.0 / 3.0 * np.pi, expected_col + 2.0 / 3.0 * np.pi), axis=-1
                        )
                        # reset angle to -pi to +pi range
                        expected_col = np.angle(np.exp(1j * expected_col))
                    else:
                        expected_col = expected_col.reshape(-1, 1)
                # for u_angle, the angle needs to be normalized
                # because any global angle shift is acceptable
                if col_name == "u_angle":
                    # set the u_angle of 0-th entry to zero
                    actual_col = actual_col - actual_col.ravel()[0]
                    expected_col = expected_col - expected_col.ravel()[0]
                    # convert to unity complex number, to avoid under and over flows
                    actual_col = np.exp(1j * actual_col)
                    expected_col = np.exp(1j * expected_col)
                    # convert to radians
                    actual_col = np.angle(actual_col)
                    expected_col = np.angle(expected_col)

                # Define the absolute tolerance based on the field name
                p = "default"
                if isinstance(atol, dict):
                    a = atol[p]
                    for pattern, tolerance in atol.items():
                        if re.fullmatch(pattern, col_name):
                            a = tolerance
                            p = f"r'{pattern}'"
                            break
                else:
                    a = atol

                if not np.allclose(actual_col, expected_col, rtol=rtol, atol=a):
                    msg = f"Not all values match for {key}.{col_name} (rtol={rtol}, atol={a}, pattern={p})"
                    print(f"\n{msg}")
                    print("Actual:     ", actual_col)
                    print("Expected:   ", expected_col)
                    print("Difference: ", actual_col - expected_col)
                    print("Matches:    ", np.isclose(actual_col, expected_col, rtol=rtol, atol=a))
                    raise AssertionError(msg)
