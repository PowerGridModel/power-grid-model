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

from power_grid_model import initialize_array
from power_grid_model.data_types import (
    BatchDataset,
    BatchList,
    ComponentList,
    Dataset,
    PythonDataset,
    SingleDataset,
    SinglePythonDataset,
)
from power_grid_model.utils import export_json_data, import_json_data

BASE_PATH = Path(__file__).parent.parent
DATA_PATH = BASE_PATH / "data"
OUPUT_PATH = BASE_PATH / "output"
EXPORT_OUTPUT = ("POWER_GRID_MODEL_VALIDATION_TEST_EXPORT" in os.environ) and (
    os.environ["POWER_GRID_MODEL_VALIDATION_TEST_EXPORT"] == "ON"
)


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
        pytest_param = [
            case_id,
            case_dir,
            sym,
            calculation_type,
            calculation_method,
            params["rtol"],
            params["atol"],
            params,
        ]
        kwargs = {}
        if "fail" in params:
            kwargs["marks"] = pytest.mark.xfail(reason=params["fail"], raises=AssertionError)
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

            for calculation_method in calculation_methods:
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
    return_dict = {
        "input": import_json_data(data_path / "input.json", "input", ignore_extra=True),
    }
    # import output if relevant
    if (data_path / f"{output_prefix}.json").exists():
        return_dict["output"] = import_json_data(data_path / f"{output_prefix}.json", output_prefix, ignore_extra=True)
    # import update and output batch if relevant
    if (data_path / "update_batch.json").exists():
        return_dict["update_batch"] = import_json_data(data_path / "update_batch.json", "update", ignore_extra=True)
        return_dict["output_batch"] = import_json_data(
            data_path / f"{output_prefix}_batch.json", output_prefix, ignore_extra=True
        )
    return return_dict


def save_json_data(json_file: str, data: Dataset):
    OUPUT_PATH.mkdir(parents=True, exist_ok=True)
    data_file = OUPUT_PATH / json_file
    data_file.parent.mkdir(parents=True, exist_ok=True)
    export_json_data(data_file, data)


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


def convert_list_to_batch_data(list_data: BatchList) -> BatchDataset:
    """
    Convert a list of datasets to one single batch dataset

    Example data formats:
        input:  [{"node": <1d-array>, "line": <1d-array>}, {"node": <1d-array>, "line": <1d-array>}]
        output: {"node": <2d-array>, "line": <2d-array>}
         -or-:  {"indptr": <1d-array>, "data": <1d-array>}
    Args:
        list_data: list of dataset

    Returns:
        batch dataset
        For a certain component, if all the length is the same for all the batches, a 2D array is used
        Otherwise use a dict of indptr/data key
    """

    # List all *unique* types
    components = {x for dataset in list_data for x in dataset.keys()}

    batch_data: BatchDataset = {}
    for component in components:
        # Create a 2D array if the component exists in all datasets and number of objects is the same in each dataset
        comp_exists_in_all_datasets = all(component in x for x in list_data)
        if comp_exists_in_all_datasets:
            all_sizes_are_the_same = all(x[component].size == list_data[0][component].size for x in list_data)
            if all_sizes_are_the_same:
                batch_data[component] = np.stack([x[component] for x in list_data], axis=0)
                continue

        # otherwise use indptr/data dict
        indptr = [0]
        data = []
        for dataset in list_data:
            if component in dataset:
                # If the current dataset contains the component, increase the indptr for this batch and append the data
                objects = dataset[component]
                indptr.append(indptr[-1] + len(objects))
                data.append(objects)

            else:
                # If the current dataset does not contain the component, add the last indptr again.
                indptr.append(indptr[-1])

        # Convert the index pointers to a numpy array and combine the list of object numpy arrays into a singe
        # numpy array. All objects of all batches are now stores in one large array, the index pointers define
        # which elemets of the array (rows) belong to which batch.
        batch_data[component] = {"indptr": np.array(indptr, dtype=np.int64), "data": np.concatenate(data, axis=0)}

    return batch_data


def convert_python_to_numpy(data: PythonDataset, data_type: str, ignore_extra: bool = False) -> Dataset:
    """
    Convert native python data to internal numpy

    Args:
        data: data in dict or list
        data_type: type of data: input, update, sym_output, or asym_output
        ignore_extra: Allow (and ignore) extra attributes in the data

    Returns:
        A single or batch dataset for power-grid-model
    """

    # If the input data is a list, we are dealing with batch data. Each element in the list is a batch. We'll
    # first convert each batch separately, by recursively calling this function for each batch. Then the numpy
    # data for all batches in converted into a proper and compact numpy structure.
    if isinstance(data, list):
        list_data = [
            convert_python_single_dataset_to_single_dataset(json_dict, data_type=data_type, ignore_extra=ignore_extra)
            for json_dict in data
        ]
        return convert_list_to_batch_data(list_data)

    # Otherwise this should be a normal (non-batch) structure, with a list of objects (dictionaries) per component.
    if not isinstance(data, dict):
        raise TypeError("Data should be either a list or a dictionary!")

    return convert_python_single_dataset_to_single_dataset(data=data, data_type=data_type, ignore_extra=ignore_extra)


def convert_python_single_dataset_to_single_dataset(
    data: SinglePythonDataset, data_type: str, ignore_extra: bool = False
) -> SingleDataset:
    """
    Convert native python data to internal numpy

    Args:
        data: data in dict
        data_type: type of data: input, update, sym_output, or asym_output
        ignore_extra: Allow (and ignore) extra attributes in the data

    Returns:
        A single dataset for power-grid-model
    """

    dataset: SingleDataset = {}
    for component, objects in data.items():
        dataset[component] = convert_component_list_to_numpy(
            objects=objects, component=component, data_type=data_type, ignore_extra=ignore_extra
        )

    return dataset


def convert_component_list_to_numpy(
    objects: ComponentList, component: str, data_type: str, ignore_extra: bool = False
) -> np.ndarray:
    """
    Convert native python data to internal numpy

    Args:
        objects: data in dict
        component: the name of the component
        data_type: type of data: input, update, sym_output, or asym_output
        ignore_extra: Allow (and ignore) extra attributes in the data

    Returns:
        A single numpy array
    """

    # We'll initialize an 1d-array with NaN values for all the objects of this component type
    array = initialize_array(data_type, component, len(objects))

    for i, obj in enumerate(objects):
        # As each object is a separate dictionary, and the attributes may differ per object, we need to check
        # all attributes. Non-existing attributes
        for attribute, value in obj.items():
            # If an attribute doesn't exist, the user should explicitly state that she/he is ok with extra
            # information in the data. This is to protect the user from overlooking errors.
            if attribute not in array.dtype.names:
                if ignore_extra:
                    continue
                raise ValueError(
                    f"Invalid attribute '{attribute}' for {component} {data_type} data. "
                    "(Use ignore_extra=True to ignore the extra data and suppress this exception)"
                )

            # Assign the value and raise an error if the value cannot be stored in the specific numpy array data format
            # for this attribute.
            try:
                array[i][attribute] = value
            except ValueError as ex:
                raise ValueError(f"Invalid '{attribute}' value for {component} {data_type} data: {ex}") from ex
    return array
