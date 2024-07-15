# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import json
import os
import re
from pathlib import Path
from typing import Any, Dict, List, Optional, Union

import numpy as np
import pytest

from power_grid_model.core.dataset_definitions import DatasetType
from power_grid_model.core.power_grid_model import PowerGridModel
from power_grid_model.data_types import Dataset, PythonDataset, SingleDataset
from power_grid_model.errors import (
    AutomaticTapCalculationError,
    ConflictID,
    ConflictVoltage,
    IDWrongType,
    InvalidBranch,
    InvalidBranch3,
    InvalidCalculationMethod,
    InvalidMeasuredObject,
    InvalidRegulatedObject,
    InvalidTransformerClock,
    NotObservableError,
    PowerGridBatchError,
    PowerGridError,
    PowerGridSerializationError,
)
from power_grid_model.utils import json_deserialize, json_deserialize_from_file, json_serialize_to_file

BASE_PATH = Path(__file__).parent.parent
DATA_PATH = BASE_PATH / "data"
OUPUT_PATH = BASE_PATH / "output"
EXPORT_OUTPUT = ("POWER_GRID_MODEL_VALIDATION_TEST_EXPORT" in os.environ) and (
    os.environ["POWER_GRID_MODEL_VALIDATION_TEST_EXPORT"] == "ON"
)

KNOWN_EXCEPTIONS = {
    ex.__name__: ex
    for ex in (
        PowerGridBatchError,
        PowerGridError,
        ConflictID,
        ConflictVoltage,
        IDWrongType,
        InvalidBranch,
        InvalidBranch3,
        InvalidCalculationMethod,
        InvalidMeasuredObject,
        InvalidRegulatedObject,
        AutomaticTapCalculationError,
        InvalidTransformerClock,
        NotObservableError,
        PowerGridSerializationError,
        AssertionError,
        OSError,
    )
}


class PowerGridModelWithExt(PowerGridModel):
    """Wrapper class around the power grid model to expose extended features."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def calculate_power_flow_with_ext(self, *args, **kwargs):
        """calculate_power_flow with extended features."""
        return self._calculate_power_flow(*args, **kwargs)

    def calculate_state_estimation_with_ext(self, *args, **kwargs):
        """calculate_state_estimation with extended features."""
        return self._calculate_state_estimation(*args, **kwargs)

    def calculate_short_circuit_with_ext(self, *args, **kwargs):
        """calculate_short_circuit with extended features."""
        return self._calculate_short_circuit(*args, **kwargs)


def get_output_type(calculation_type: str, sym: bool) -> str:
    if calculation_type == "short_circuit":
        if sym:
            raise AssertionError(f"Unsupported validation case: calculation_type={calculation_type}, sym={sym}")
        return "sc_output"

    if sym:
        return "sym_output"

    return "asym_output"


def get_test_case_paths(calculation_type: str, test_cases: Optional[List[str]] = None) -> Dict[str, Path]:
    """get a list of all cases, directories in validation datasets"""
    calculation_type_dir = DATA_PATH / calculation_type
    test_case_paths = {
        str(item.relative_to(DATA_PATH)).replace("\\", "/"): item
        for item in calculation_type_dir.glob("**/")
        if (item.is_dir() and (item / "params.json").is_file())
    }
    if test_cases is not None:
        test_case_paths = {key: value for key, value in test_case_paths.items() if key in test_cases}

    return test_case_paths


def add_case(
    case_name: str,
    case_dir: Path,
    params: dict,
    calculation_type: str,
    is_batch: bool,
    calculation_method: str,
    sym: bool,
):
    output_prefix = get_output_type(calculation_type=calculation_type, sym=sym)
    batch_suffix = "_batch" if is_batch else ""

    # only generate a case if sym or asym output exists
    if (case_dir / f"{output_prefix}{batch_suffix}.json").exists():
        # Build a recognizable case ID
        case_id = case_name
        case_id += "-sym" if sym else "-asym"
        case_id += "-" + calculation_method
        if is_batch:
            case_id += "-batch"

        calculation_method_params = dict(params, **params.get("extra_params", {}).get(calculation_method, {}))
        pytest_param = [
            case_id,
            case_dir,
            sym,
            calculation_type,
            calculation_method,
            calculation_method_params["rtol"],
            calculation_method_params["atol"],
            calculation_method_params,
        ]
        kwargs = {}
        if "fail" in calculation_method_params:
            xfail = calculation_method_params["fail"]
            kwargs["marks"] = pytest.mark.xfail(
                reason=xfail["reason"], raises=KNOWN_EXCEPTIONS[xfail.get("raises", "AssertionError")]
            )
        yield pytest.param(*pytest_param, **kwargs, id=case_id)


def _add_cases(case_dir: Path, calculation_type: str, **kwargs):
    with open(case_dir / "params.json") as f:
        params = json.load(f)

    # retrieve calculation method, can be a string or list of strings
    calculation_methods = params["calculation_method"]
    if not isinstance(calculation_methods, list):
        calculation_methods = [calculation_methods]

    # loop for sym or asym scenario
    for calculation_method in calculation_methods:
        for sym in [True, False]:
            if calculation_type == "short_circuit" and sym:
                continue  # only asym short circuit calculations are supported

            yield from add_case(
                case_dir=case_dir,
                params=params,
                calculation_type=calculation_type,
                calculation_method=calculation_method,
                sym=sym,
                **kwargs,
            )


def pytest_cases(get_batch_cases: bool = False, data_dir: Optional[str] = None, test_cases: Optional[List[str]] = None):
    if data_dir is not None:
        relevant_calculations = [data_dir]
    else:
        relevant_calculations = ["power_flow", "state_estimation", "short_circuit"]

    for calculation_type in relevant_calculations:
        test_case_paths = get_test_case_paths(calculation_type=calculation_type, test_cases=test_cases)

        for case_name, case_dir in test_case_paths.items():
            yield from _add_cases(
                case_name=case_name,
                case_dir=case_dir,
                calculation_type=calculation_type,
                is_batch=get_batch_cases,
            )


def bool_params(true_id: str, false_id: Optional[str] = None, **kwargs):
    if false_id is None:
        false_id = f"not-{true_id}"
    yield pytest.param(False, **kwargs, id=false_id)
    yield pytest.param(True, **kwargs, id=true_id)


def dict_params(params: Dict[Any, str], **kwargs):
    for value, param_id in params.items():
        yield pytest.param(value, **kwargs, id=param_id)


def import_case_data(data_path: Path, calculation_type: str, sym: bool):
    output_prefix = get_output_type(calculation_type=calculation_type, sym=sym)
    return_dict = {"input": json_deserialize_from_file(data_path / "input.json")}
    # import output if relevant
    if (data_path / f"{output_prefix}.json").exists():
        return_dict["output"] = json_deserialize_from_file(data_path / f"{output_prefix}.json")
    # import update and output batch if relevant
    if (data_path / "update_batch.json").exists():
        return_dict["update_batch"] = json_deserialize_from_file(data_path / "update_batch.json")
        return_dict["output_batch"] = json_deserialize_from_file(data_path / f"{output_prefix}_batch.json")

    return return_dict


def save_json_data(json_file: str, data: Dataset):
    OUPUT_PATH.mkdir(parents=True, exist_ok=True)
    data_file = OUPUT_PATH / json_file
    data_file.parent.mkdir(parents=True, exist_ok=True)
    json_serialize_to_file(data_file, data)


def compare_result(actual: SingleDataset, expected: SingleDataset, rtol: float, atol: Union[float, Dict[str, float]]):
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

                if col_name.endswith("_angle"):
                    magnitude_name = col_name[: -len("_angle")]
                    if np.all(np.isnan(expected_data[magnitude_name])):
                        continue
                    actual_col = actual[key][magnitude_name] * np.exp(1j * actual_col)
                    expected_col = expected_data[magnitude_name] * np.exp(1j * expected_col)

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

                assert np.allclose(actual_col, expected_col, rtol=rtol, atol=a), (
                    f"Not all values match for {key}.{col_name} (rtol={rtol}, atol={a}, pattern={p})"
                    f"\nActual:     {actual_col}"
                    f"\nExpected:   {expected_col}"
                    f"\nDifference: {actual_col - expected_col}"
                    f"\nMatches:    {np.isclose(actual_col, expected_col, rtol=rtol, atol=a)}"
                )


def convert_python_to_numpy(data: PythonDataset, data_type: DatasetType) -> Dataset:
    """
    Convert native python data to internal numpy

    Args:
        data: data in dict or list
        data_type: type of data: input, update, sym_output, or asym_output

    Returns:
        A single or batch dataset for power-grid-model
    """
    return json_deserialize(
        json.dumps(
            {
                "version": "1.0",
                "is_batch": isinstance(data, list),
                "attributes": {},
                "type": data_type,
                "data": data,
            }
        )
    )
