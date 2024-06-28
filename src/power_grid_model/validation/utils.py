# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Utilities used for validation. Only errors_to_string() is intended for end users.
"""
import re
from typing import Any, Dict, List, Optional, Union

import numpy as np

from power_grid_model import power_grid_meta_data
from power_grid_model.core.dataset_definitions import ComponentType, DatasetType, _str_to_component_type
from power_grid_model.data_types import SingleDataset
from power_grid_model.validation.errors import ValidationError


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


def update_input_data(input_data: SingleDataset, update_data: SingleDataset):
    """
    Update the input data using the available non-nan values in the update data.
    """

    merged_data = {component: array.copy() for component, array in input_data.items()}
    for component in update_data.keys():
        update_component_data(component, merged_data[component], update_data[component])
    return merged_data


def update_component_data(component: ComponentType, input_data: np.ndarray, update_data: np.ndarray) -> None:
    """
    Update the data in a numpy array, with another numpy array,
    indexed on the "id" field and only non-NaN values are overwritten.
    """
    for field in update_data.dtype.names:
        if field == "id":
            continue
        nan = nan_type(component, field, DatasetType.update)
        if np.isnan(nan):
            mask = ~np.isnan(update_data[field])
        else:
            mask = np.not_equal(update_data[field], nan)

        if mask.ndim == 2:
            for phase in range(mask.shape[1]):
                # find indexers of to-be-updated object
                sub_mask = mask[:, phase]
                idx = get_indexer(input_data["id"], update_data["id"][sub_mask])
                # update
                input_data[field][idx, phase] = update_data[field][sub_mask, phase]
        else:
            # find indexers of to-be-updated object
            idx = get_indexer(input_data["id"], update_data["id"][mask])
            # update
            input_data[field][idx] = update_data[field][mask]


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

    Returns:
        A human readable string representation of a set of errors.
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


def nan_type(component: Union[str, ComponentType], field: str, data_type: DatasetType = DatasetType.input):
    """
    Helper function to retrieve the nan value for a certain field as defined in the power_grid_meta_data.
    """
    component = _str_to_component_type(component)
    return power_grid_meta_data[data_type][component].nans[field]


def get_indexer(source: np.ndarray, target: np.ndarray, default_value: Optional[int] = None) -> np.ndarray:
    """
    Given array of values from a source and a target dataset.
    Find the position of each value in the target dataset in the context of the source dataset.
    This is needed to update values in the dataset by id lookup.
    Internally this is done by sorting the input ids, then using binary search lookup.

    E.g.: Find the position of each id in an update (target) dataset in the input (source) dataset

    >>> input_ids = [1, 2, 3, 4, 5]
    >>> update_ids = [3]
    >>> assert get_indexer(input_ids, update_ids) == np.array([2])

    Args:
        source: array of values in the source dataset
        target: array of values in the target dataset
        default_value: Optional. the default index to provide for target values not in source

    Returns:
        np.ndarray: array of positions of the values from target dataset in the source dataset
            if default_value is None, (source[result] == target)
            else, ((source[result] == target) | (source[result] == default_value))

    Raises:
        IndexError: if default_value is None and there were values in target that were not in source
    """
    permutation_sort = np.argsort(source)  # complexity O(N_input * logN_input)
    indices = np.searchsorted(source, target, sorter=permutation_sort)  # complexity O(N_update * logN_input)

    if default_value is None:
        return permutation_sort[indices]

    clipped_indices = np.take(permutation_sort, indices, mode="clip")
    return np.where(source[clipped_indices] == target, permutation_sort[clipped_indices], default_value)


def set_default_value(
    data: SingleDataset, component: ComponentType, field: str, default_value: Union[int, float, np.ndarray]
):
    """
    This function sets the default value in the data that is to be validated, so the default values are included in the
    validation.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        default_value: Some values are not required, but will receive a default value in the C++ core. To do a proper
        input validation, these default values should be included in the validation. It can be a fixed value for the
        entire column (int/float) or be different for each element (np.ndarray).

    Returns:

    """
    if np.isnan(nan_type(component, field)):
        mask = np.isnan(data[component][field])
    else:
        mask = data[component][field] == nan_type(component, field)
    if isinstance(default_value, np.ndarray):
        data[component][field][mask] = default_value[mask]
    else:
        data[component][field][mask] = default_value


def get_valid_ids(data: SingleDataset, ref_components: Union[ComponentType, List[ComponentType]]) -> List[int]:
    """
    This function returns the valid IDs specified by all ref_components

    Args:
        data: The input/update data set for all components
        ref_components: The component or components in which we want to look for ids

    Returns:
        List[int]: the list of valid IDs
    """
    # For convenience, ref_component may be a string and we'll convert it to a 'list' containing that string as it's
    # single element.
    if isinstance(ref_components, (str, ComponentType)):
        ref_components = [ref_components]

    # Create a set of ids by chaining the ids of all ref_components
    valid_ids = set()
    for ref_component in ref_components:
        if ref_component in data:
            nan = nan_type(ref_component, "id")
            if np.isnan(nan):
                mask = ~np.isnan(data[ref_component]["id"])
            else:
                mask = np.not_equal(data[ref_component]["id"], nan)
            valid_ids.update(data[ref_component]["id"][mask])

    return list(valid_ids)


def get_mask(data: SingleDataset, component: ComponentType, field: str, **filters: Any) -> np.ndarray:
    """
    Get a mask based on the specified filters. E.g. measured_terminal_type=MeasuredTerminalType.source.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_components: The component or components in which we want to look for ids
        **filters: One or more filters on the dataset. E.

    Returns:
        np.ndarray: the mask
    """
    values = data[component][field]
    mask = np.ones(shape=values.shape, dtype=bool)
    for filter_field, filter_value in filters.items():
        mask = np.logical_and(mask, data[component][filter_field] == filter_value)

    return mask
