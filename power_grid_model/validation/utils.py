# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
Utilities used for validation. Only errors_to_string() is intended for end users.
"""
import re
from itertools import chain
from typing import Dict, List, Optional, Union

import numpy as np

from .errors import ValidationError
from .. import power_grid_meta_data

InputData = Dict[str, np.ndarray]
UpdateData = Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]
BatchData = List[Dict[str, np.ndarray]]


def eval_expression(data: np.ndarray, expression: Union[int, float, str]) -> np.ndarray:
    """
    Wrapper function that checks the type of the 'expression'. If the expression is a string, it is assumed to be a
    field expression and the expression is validated. Otherwise it is assumed to be a numerical value and the value
    is casted to a numpy 'array'.

    Args:
        data: A numpy structured array
        expression: A numerical value, or a string, representing a (combination of) field(s)

    Returns: The number or an evaluation of the field name(s) in the data, always represented as a Numpy 'array'.

    Examples:
        123 -> np.array(123)
        123.4 -> np.array(123.4)
        'value' -> data['value']
        'foo/bar' -> data['foo'] / data[bar]

    """
    if isinstance(expression, str):
        return eval_field_expression(data, expression)
    return np.array(expression)


def eval_field_expression(data: np.ndarray, expression: str) -> np.ndarray:
    """
    A field expression can either be the name of a field (e.g. 'field_x') in the data, or a ratio between two fields
    (e.g. 'field_x / field_y'). The expression is checked on validity and then the fields are checked to be present in
    the data. If the expression is a single field name, the field is returned. If it is a ratio, the ratio is
    calculated and returned. Values divided by 0 will result in nan values without warning.

    Args:
        data: A numpy structured array
        expression: A string, representing a (combination of) field(s)

    Expression should be a combination of:
      - field names (may contain lower case letters, numbers and underscores)
      - a single mathematical operator /

    Returns: An evaluation of the field name(s) in the data.

    Examples:
        'value' -> data['value']
        'foo/bar' -> data['foo'] / data['bar']

    """

    # Validate the expression
    match = re.fullmatch(r"[a-z][a-z0-9_]*(\s*/\s*[a-z][a-z0-9_]*)?", expression)
    if not match:
        raise ValueError(f"Invalid field expression '{expression}'")

    # Find all field names and check if they exist in the dataset
    fields = [f.strip() for f in expression.split("/")]
    for field in fields:
        if field not in data.dtype.names:
            raise KeyError(f"Invalid field name {field}")

    if len(fields) == 1:
        return data[fields[0]]

    assert len(fields) == 2
    zero_div = np.logical_or(np.equal(data[fields[1]], 0.0), np.logical_not(np.isfinite(data[fields[1]])))
    if np.any(zero_div):
        result = np.full_like(data[fields[0]], np.nan)
        np.true_divide(data[fields[0]], data[fields[1]], out=result, where=~zero_div)
        return result
    return np.true_divide(data[fields[0]], data[fields[1]])


def split_update_data_in_batches(update_data: UpdateData) -> BatchData:
    """

    Args:
        update_data: Dict[str, Union[np.ndarray, Dict[str, np.ndarray]]]

    Returns: List[Dict[str, np.ndarray]]

    """
    batches = []
    for component, data in update_data.items():
        if isinstance(data, np.ndarray):
            component_batches = split_numpy_array_in_batches(data, component)
        elif isinstance(data, dict):
            for key in ["indptr", "data"]:
                if key not in data:
                    raise KeyError(
                        f"Missing '{key}' in sparse update data for '{component}' "
                        "(expected a python dictionary containing two keys: 'indptr' and 'data')."
                    )
            component_batches = split_compressed_sparse_structure_in_batches(data["data"], data["indptr"], component)
        else:
            raise TypeError(
                f"Invalid data type {type(data).__name__} in update data for '{component}' "
                "(should be a Numpy structured array or a python dictionary)."
            )
        if not batches:
            batches = [{} for _ in component_batches]
        elif len(component_batches) != len(batches):
            previous_components = set(chain(*(batch.keys() for batch in batches)))
            if len(previous_components) == 1:
                previous_components = f"'{previous_components.pop()}'"
            else:
                previous_components = "/".join(sorted(previous_components))
            raise ValueError(
                f"Inconsistent number of batches in update data. "
                f"Component '{component}' contains {len(component_batches)} batches, "
                f"while {previous_components} contained {len(batches)} batches."
            )

        for i, batch_data in enumerate(component_batches):
            if batch_data.size > 0:
                batches[i][component] = batch_data
    return batches


def split_numpy_array_in_batches(data: np.ndarray, component: str) -> List[np.ndarray]:
    """
    Split a single dense numpy array into one or more batches

    Args:
        data: A 1D or 2D Numpy structured array. A 1D array is a single table / batch, a 2D array is a batch per table.
        component: The name of the component to which the data belongs, only used for errors.

    Returns:
        A list with a single numpy structured array per batch

    """
    if not isinstance(data, np.ndarray):
        raise TypeError(
            f"Invalid data type {type(data).__name__} in update data for '{component}' "
            "(should be a 1D/2D Numpy structured array)."
        )
    if data.ndim == 1:
        return [data]
    elif data.ndim == 2:
        return [data[i, :] for i in range(data.shape[0])]
    else:
        raise TypeError(
            f"Invalid data dimension {data.ndim} in update data for '{component}' "
            "(should be a 1D/2D Numpy structured array)."
        )


def split_compressed_sparse_structure_in_batches(
    data: np.ndarray, indptr: np.ndarray, component: str
) -> List[np.ndarray]:
    """
    Split a single numpy array representing, a compressed sparse structure, into one or more batches

    Args:
        data: A 1D Numpy structured array
        indptr: A 1D numpy integer array
        component: The name of the component to which the data belongs, only used for errors.

    Returns:
        A list with a single numpy structured array per batch

    """
    if not isinstance(data, np.ndarray) or data.ndim != 1:
        raise TypeError(
            f"Invalid data type {type(data).__name__} in sparse update data for '{component}' "
            "(should be a 1D Numpy structured array (i.e. a single 'table'))."
        )

    if not isinstance(indptr, np.ndarray) or indptr.ndim != 1 or not np.issubdtype(indptr.dtype, np.integer):
        raise TypeError(
            f"Invalid indptr data type {type(indptr).__name__} in update data for '{component}' "
            "(should be a 1D Numpy array (i.e. a single 'list'), "
            "containing indices (i.e. integers))."
        )

    if indptr[0] != 0 or indptr[-1] != len(data) or any(indptr[i] > indptr[i + 1] for i in range(len(indptr) - 1)):
        raise TypeError(
            f"Invalid indptr in update data for '{component}' "
            f"(should start with 0, end with the number of objects ({len(data)}) "
            "and be monotonic increasing)."
        )

    return [data[indptr[i] : indptr[i + 1]] for i in range(len(indptr) - 1)]


def update_input_data(input_data: Dict[str, np.ndarray], update_data: Dict[str, np.ndarray]):
    """
    Update the input data using the available non-nan values in the update data.
    """

    merged_data = {component: array.copy() for component, array in input_data.items()}
    for component, array in update_data.items():
        for field in array.dtype.names:
            if field == "id":
                continue
            nan = nan_type(component, field, "update")
            if np.isnan(nan):
                mask = ~np.isnan(array[field])
            else:
                mask = np.not_equal(array[field], nan)
            data = array[["id", field]][mask]
            idx = np.where(merged_data[component]["id"] == np.reshape(data["id"], (-1, 1)))
            if isinstance(idx, tuple):
                merged_data[component][field][idx[1]] = data[field]
    return merged_data


def errors_to_string(
    errors: Union[List[ValidationError], Dict[int, List[ValidationError]], None],
    name: str = "the data",
    details: bool = False,
    id_lookup: Optional[Union[List[str], Dict[int, str]]] = None,
) -> str:
    """
    Convert a set of errors (list or dict) to a human readable string representation.
    Args:
        errors: The error objects. List for input_data only, dict for batch data.
        name: Human understandable name of the dataset, e.g. input_data, or update_data.
        details: Display object ids and error specific information.
        id_lookup: A list or dict (int->str) containing textual object ids

    Returns: A human readable string representation of a set of errors.
    """
    if errors is None or len(errors) == 0:
        return f"{name}: OK"
    if isinstance(errors, dict):
        return "\n".join(errors_to_string(err, f"{name}, batch #{i}", details) for i, err in sorted(errors.items()))
    if len(errors) == 1 and not details:
        return f"There is a validation error in {name}:\n\t{errors[0]}"
    if len(errors) == 1:
        msg = f"There is a validation error in {name}:\n"
    else:
        msg = f"There are {len(errors)} validation errors in {name}:\n"
    if details:
        for error in errors:
            msg += "\n\t" + str(error) + "\n"
            msg += "".join(f"\t\t{k}: {v}\n" for k, v in error.get_context(id_lookup).items())
    else:
        msg += "\n".join(f"{i + 1:>4}. {err}" for i, err in enumerate(errors))
    return msg


def nan_type(component: str, field: str, data_type="input"):
    """
    Helper function to retrieve the nan value for a certain field as defined in the power_grid_meta_data.
    It silently returns float('nan') if data_type/component/field can't be found.
    """
    return power_grid_meta_data.get(data_type, {}).get(component, {}).get("nans", {}).get(field, float("nan"))
