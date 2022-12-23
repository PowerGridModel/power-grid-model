# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
This module contains a set of comparison rules. They all share the same (or similar) logic and interface.

In general each function checks the values in a single 'column' (i.e. field) of a numpy structured array and it
returns an error object containing the component, the field and the ids of the records that did not match the rule.
E.g. all_greater_than_zero(data, 'node', 'u_rated') returns a NotGreaterThanError if any of the node's `u_rated`
values are 0 or less.

In general, the rules are designed to ignore NaN values, except for none_missing() which explicitly checks for NaN
values in the entire data set. It is important to understand that np.less_equal(x) yields different results than
np.logical_not(np.greater(x)) as a NaN comparison always results in False. The most extreme example is that even
np.nan == np.nan yields False.

    np.less_equal(            [0.1, 0.2, 0.3, np.nan], 0.0)  = [False, False, False, False] -> OK
    np.logical_not(np.greater([0.1, 0.2, 0.3, np.nan], 0.0)) = [False, False, False, True] -> Error (false positive)

Input data:

    data: SingleDataset
        The entire input/update data set

    component: str
        The name of the component, which should be an existing key in the data

    field: str
        The name of the column, which should be an field in the component data (numpy structured array)

Output data:
    errors: List[ValidationError]
        A list containing errors; in case of success, `errors` is the empty list: [].

"""
from enum import Enum
from typing import Any, Callable, Dict, List, Tuple, Type, TypeVar, Union

import numpy as np

from power_grid_model.data_types import SingleDataset
from power_grid_model.enum import WindingType
from power_grid_model.validation.errors import (
    ComparisonError,
    IdNotInDatasetError,
    InfinityError,
    InvalidEnumValueError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    NotBetweenError,
    NotBetweenOrAtError,
    NotBooleanError,
    NotGreaterOrEqualError,
    NotGreaterThanError,
    NotLessOrEqualError,
    NotLessThanError,
    NotUniqueError,
    SameValueError,
    TransformerClockError,
    TwoValuesZeroError,
    ValidationError,
)
from power_grid_model.validation.utils import eval_expression, nan_type

Error = TypeVar("Error", bound=ValidationError)
CompError = TypeVar("CompError", bound=ComparisonError)


def all_greater_than_zero(data: SingleDataset, component: str, field: str) -> List[NotGreaterThanError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are greater than
    zero. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest

    Returns:
        A list containing zero or one NotGreaterThanErrors, listing all ids where the value in the field of interest
        was zero or less.
    """
    return all_greater_than(data, component, field, 0.0)


def all_greater_than_or_equal_to_zero(data: SingleDataset, component: str, field: str) -> List[NotGreaterOrEqualError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are greater than,
    or equal to zero. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest

    Returns:
        A list containing zero or one NotGreaterOrEqualErrors, listing all ids where the value in the field of
        interest was less than zero.
    """
    return all_greater_or_equal(data, component, field, 0.0)


def all_greater_than(
    data: SingleDataset, component: str, field: str, ref_value: Union[int, float, str]
) -> List[NotGreaterThanError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are greater than
    the reference value. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_value: The reference value against which all values in the 'field' column are compared. If the reference
        value is a string, it is assumed to be another field (e.g. 'field_x') of the same component, or a ratio between
        two fields (e.g. 'field_x / field_y')

    Returns:
        A list containing zero or one NotGreaterThanErrors, listing all ids where the value in the field of interest
        was less than, or equal to, the ref_value.
    """

    def not_greater(val: np.ndarray, *ref: np.ndarray):
        return np.less_equal(val, *ref)

    return none_match_comparison(data, component, field, not_greater, ref_value, NotGreaterThanError)


def all_greater_or_equal(
    data: SingleDataset, component: str, field: str, ref_value: Union[int, float, str]
) -> List[NotGreaterOrEqualError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are greater than,
    or equal to the reference value. Returns an empty list on success, or a list containing a single error object on
    failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_value: The reference value against which all values in the 'field' column are compared. If the reference
        value is a string, it is assumed to be another field (e.g. 'field_x') of the same component, or a ratio between
        two fields (e.g. 'field_x / field_y')

    Returns:
        A list containing zero or one NotGreaterOrEqualErrors, listing all ids where the value in the field of
        interest was less than the ref_value.

    """

    def not_greater_or_equal(val: np.ndarray, *ref: np.ndarray):
        return np.less(val, *ref)

    return none_match_comparison(data, component, field, not_greater_or_equal, ref_value, NotGreaterOrEqualError)


def all_less_than(
    data: SingleDataset, component: str, field: str, ref_value: Union[int, float, str]
) -> List[NotLessThanError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are less than the
    reference value. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_value: The reference value against which all values in the 'field' column are compared. If the reference
        value is a string, it is assumed to be another field (e.g. 'field_x') of the same component, or a ratio between
        two fields (e.g. 'field_x / field_y')

    Returns:
        A list containing zero or one NotLessThanErrors, listing all ids where the value in the field of interest was
        greater than, or equal to, the ref_value.
    """

    def not_less(val: np.ndarray, *ref: np.ndarray):
        return np.greater_equal(val, *ref)

    return none_match_comparison(data, component, field, not_less, ref_value, NotLessThanError)


def all_less_or_equal(
    data: SingleDataset, component: str, field: str, ref_value: Union[int, float, str]
) -> List[NotLessOrEqualError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are less than,
    or equal to the reference value. Returns an empty list on success, or a list containing a single error object on
    failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_value: The reference value against which all values in the 'field' column are compared. If the reference
        value is a string, it is assumed to be another field (e.g. 'field_x') of the same component, or a ratio between
        two fields (e.g. 'field_x / field_y')

    Returns:
        A list containing zero or one NotLessOrEqualErrors, listing all ids where the value in the field of interest was
        greater than the ref_value.

    """

    def not_less_or_equal(val: np.ndarray, *ref: np.ndarray):
        return np.greater(val, *ref)

    return none_match_comparison(data, component, field, not_less_or_equal, ref_value, NotLessOrEqualError)


def all_between(
    data: SingleDataset,
    component: str,
    field: str,
    ref_value_1: Union[int, float, str],
    ref_value_2: Union[int, float, str],
) -> List[NotBetweenError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are (exclusively)
    between reference value 1 and 2. Value 1 may be smaller, but also larger than value 2. Returns an empty list on
    success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_value_1: The first reference value against which all values in the 'field' column are compared. If the
        reference value is a string, it is assumed to be another field (e.g. 'field_x') of the same component, or a
        ratio between two fields (e.g. 'field_x / field_y')
        ref_value_2: The second reference value against which all values in the 'field' column are compared. If the
        reference value is a string, it is assumed to be another field (e.g. 'field_x') of the same component,
        or a ratio between two fields (e.g. 'field_x / field_y')

    Returns:
        A list containing zero or one NotBetweenErrors, listing all ids where the value in the field of interest was
        outside the range defined by the reference values.
    """

    def outside(val: np.ndarray, *ref: np.ndarray) -> np.ndarray:
        return np.logical_or(np.less_equal(val, np.minimum(*ref)), np.greater_equal(val, np.maximum(*ref)))

    return none_match_comparison(data, component, field, outside, (ref_value_1, ref_value_2), NotBetweenError)


def all_between_or_at(
    data: SingleDataset,
    component: str,
    field: str,
    ref_value_1: Union[int, float, str],
    ref_value_2: Union[int, float, str],
) -> List[NotBetweenOrAtError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are inclusively
    between reference value 1 and 2. Value 1 may be smaller, but also larger than value 2. Returns an empty list on
    success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_value_1: The first reference value against which all values in the 'field' column are compared. If the
        reference value is a string, it is assumed to be another field (e.g. 'field_x') of the same component, or a
        ratio between two fields (e.g. 'field_x / field_y')
        ref_value_2: The second reference value against which all values in the 'field' column are compared. If the
        reference value is a string, it is assumed to be another field (e.g. 'field_x') of the same component,
        or a ratio between two fields (e.g. 'field_x / field_y')

    Returns:
        A list containing zero or one NotBetweenOrAtErrors, listing all ids where the value in the field of interest was
        outside the range defined by the reference values.
    """

    def outside(val: np.ndarray, *ref: np.ndarray) -> np.ndarray:
        return np.logical_or(np.less(val, np.minimum(*ref)), np.greater(val, np.maximum(*ref)))

    return none_match_comparison(data, component, field, outside, (ref_value_1, ref_value_2), NotBetweenOrAtError)


def none_match_comparison(
    data: SingleDataset,
    component: str,
    field: str,
    compare_fn: Callable,
    ref_value: ComparisonError.RefType,
    error: Type[CompError] = ComparisonError,  # type: ignore
) -> List[CompError]:
    # pylint: disable=too-many-arguments
    """
    For all records of a particular type of component, check if the value in the 'field' column match the comparison.
    Returns an empty list if none of the value match the comparison, or a list containing a single error object when at
    the value in 'field' of at least one record matches the comparison.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        compare_fn: A function that takes the data in the 'field' column, and any number of reference values
        ref_value: A reference value, or a tuple of reference values, against which all values in the 'field' column
        are compared using the compare_fn. If a reference value is a string, it is assumed to be another field
        (e.g. 'field_x') of the same component, or a ratio between two fields (e.g. 'field_x / field_y')
        error: The type (class) of error that should be returned in case any of the values match the comparison.

    Returns:
        A list containing zero or one comparison errors (should be a sub class of ComparisonError), listing all ids
        where the value in the field of interest matched the comparison.
    """
    component_data = data[component]
    if isinstance(ref_value, tuple):
        ref = tuple(eval_expression(component_data, v) for v in ref_value)
    else:
        ref = (eval_expression(component_data, ref_value),)
    matches = compare_fn(component_data[field], *ref)
    if matches.any():
        if matches.ndim > 1:
            matches = matches.any(axis=1)
        ids = component_data["id"][matches].flatten().tolist()
        return [error(component, field, ids, ref_value)]
    return []


def all_unique(data: SingleDataset, component: str, field: str) -> List[NotUniqueError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are unique within
    the 'field' column of that component.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest

    Returns:
        A list containing zero or one NotUniqueError, listing all ids where the value in the field of interest was
        not unique. If the field name was 'id' (a very common check), the id is added as many times as it occurred in
        the 'id' column, to maintain object counts.
    """
    _, index, counts = np.unique(data[component][field], return_index=True, return_counts=True)
    if any(counts != 1):
        ids = data[component]["id"][index[counts != 1]].flatten().tolist()
        if field == "id":  # Add ids multiple times
            counts = counts[counts != 1]
            for obj_id, count in zip(ids, counts):
                ids += [obj_id] * (count - 1)
            ids = sorted(ids)
        return [NotUniqueError(component, field, ids)]
    return []


def all_cross_unique(
    data: SingleDataset, fields: List[Tuple[str, str]], cross_only=True
) -> List[MultiComponentNotUniqueError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are unique within
    the 'field' column of that component.

    Args:
        data: The input/update data set for all components
        fields: The fields of interest, formatted as [(component_1, field_1), (component_2, field_2)]
        cross_only: Do not include duplicates within a single field. It is advised that you use all_unique() to
        explicitly check uniqueness within a single field.

    Returns:
        A list containing zero or one MultiComponentNotUniqueError, listing all fields and ids where the value was not
        unique between the fields.
    """
    all_values: Dict[int, List[Tuple[Tuple[str, str], int]]] = {}
    duplicate_ids = set()
    for component, field in fields:
        for obj_id, value in zip(data[component]["id"], data[component][field]):
            component_id = ((component, field), obj_id)
            if value not in all_values:
                all_values[value] = []
            elif not cross_only or not all(f == (component, field) for f, _ in all_values[value]):
                duplicate_ids.update(all_values[value])
                duplicate_ids.add(component_id)
            all_values[value].append(component_id)
    if duplicate_ids:
        fields_with_duplicated_ids = {f for f, _ in duplicate_ids}
        ids_with_duplicated_ids = {(c, i) for (c, _), i in duplicate_ids}
        return [MultiComponentNotUniqueError(list(fields_with_duplicated_ids), list(ids_with_duplicated_ids))]
    return []


def all_valid_enum_values(
    data: SingleDataset, component: str, field: str, enum: Type[Enum]
) -> List[InvalidEnumValueError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are valid values for
    the supplied enum class. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        enum: The enum type to validate against

    Returns:
        A list containing zero or one InvalidEnumValueError, listing all ids where the value in the field of interest
        was not a valid value in the supplied enum type.
    """
    valid = [nan_type(component, field)] + list(enum)
    invalid = np.isin(data[component][field], np.array(valid, dtype=np.int8), invert=True)
    if invalid.any():
        ids = data[component]["id"][invalid].flatten().tolist()
        return [InvalidEnumValueError(component, field, ids, enum)]
    return []


def all_valid_ids(
    data: SingleDataset, component: str, field: str, ref_components: Union[str, List[str]], **filters: Any
) -> List[InvalidIdError]:
    """
    For a column which should contain object identifiers (ids), check if the id exists in the data, for a specific set
    of reference component types. E.g. is the from_node field of each line referring to an existing node id?

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest
        ref_components: The component or components in which we want to look for ids
        **filters: One or more filters on the dataset. E.g. measured_terminal_type=MeasuredTerminalType.source.

    Returns:
        A list containing zero or one InvalidIdError, listing all ids where the value in the field of interest
        was not a valid object identifier.
    """
    # For convenience, ref_component may be a string and we'll convert it to a 'list' containing that string as it's
    # single element.
    if isinstance(ref_components, str):
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

    # Apply the filters (e.g. to select only records with a certain MeasuredTerminalType)
    values = data[component][field]
    mask = np.ones(shape=values.shape, dtype=bool)
    for filter_field, filter_value in filters.items():
        mask = np.logical_and(mask, data[component][filter_field] == filter_value)

    # Find any values that can't be found in the set of ids
    invalid = np.logical_and(mask, np.isin(values, list(valid_ids), invert=True))
    if invalid.any():
        ids = data[component]["id"][invalid].flatten().tolist()
        return [InvalidIdError(component, field, ids, ref_components, filters)]
    return []


def all_boolean(data: SingleDataset, component: str, field: str) -> List[NotBooleanError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are valid boolean
    values, i.e. 0 or 1. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field: The field of interest

    Returns:
        A list containing zero or one NotBooleanError, listing all ids where the value in the field of interest was not
        a valid boolean value.
    """
    invalid = np.isin(data[component][field], [0, 1], invert=True)
    if invalid.any():
        ids = data[component]["id"][invalid].flatten().tolist()
        return [NotBooleanError(component, field, ids)]
    return []


def all_not_two_values_zero(
    data: SingleDataset, component: str, field_1: str, field_2: str
) -> List[TwoValuesZeroError]:
    """
    Check that for all records of a particular type of component, the values in the 'field_1' and 'field_2' column are
    not both zero. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field_1: The first field of interest
        field_2: The second field of interest
    Returns:
        A list containing zero or one TwoValuesZeroError, listing all ids where the value in the two fields of interest
        were both zero.
    """
    invalid = np.logical_and(np.equal(data[component][field_1], 0.0), np.equal(data[component][field_2], 0.0))
    if invalid.any():
        if invalid.ndim > 1:
            invalid = invalid.any(axis=1)
        ids = data[component]["id"][invalid].flatten().tolist()
        return [TwoValuesZeroError(component, [field_1, field_2], ids)]
    return []


def all_not_two_values_equal(data: SingleDataset, component: str, field_1: str, field_2: str) -> List[SameValueError]:
    """
    Check that for all records of a particular type of component, the values in the 'field_1' and 'field_2' column are
    not both the same value. E.g. from_node and to_node of a line. Returns an empty list on success, or a list
    containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        field_1: The first field of interest
        field_2: The second field of interest
    Returns:
        A list containing zero or one SameValueError, listing all ids where the value in the two fields of interest
        were both the same.
    """
    invalid = np.equal(data[component][field_1], data[component][field_2])
    if invalid.any():
        if invalid.ndim > 1:
            invalid = invalid.any(axis=1)
        ids = data[component]["id"][invalid].flatten().tolist()
        return [SameValueError(component, [field_1, field_2], ids)]
    return []


def all_ids_exist_in_data_set(
    data: SingleDataset, ref_data: SingleDataset, component: str, ref_name: str
) -> List[IdNotInDatasetError]:
    """
    Check that for all records of a particular type of component, the ids exist in the reference data set.

    Args:
        data: The (update) data set for all components
        ref_data: The reference (input) data set for all components
        component: The component of interest
        ref_name: The name of the reference data set, e.g. 'input_data'
    Returns:
        A list containing zero or one IdNotInDatasetError, listing all ids of the objects in the data set which do not
        exist in the reference data set.
    """
    invalid = np.isin(data[component]["id"], ref_data[component]["id"], invert=True)
    if invalid.any():
        ids = data[component]["id"][invalid].flatten().tolist()
        return [IdNotInDatasetError(component, ids, ref_name)]
    return []


def all_finite(data: SingleDataset) -> List[InfinityError]:
    """
    Check that for all records in all component, the values in all columns are finite value, i.e. float values other
    than inf, or -inf. Nan values are ignored, as in all other comparison functions. You can use non_missing() to
    check for missing/nan values. Returns an empty list on success, or a list containing an error object for each
    component/field combination where.

    Args:
        data: The input/update data set for all components

    Returns:
        A list containing zero or one NotBooleanError, listing all ids where the value in the field of interest was not
        a valid boolean value.
    """
    errors = []
    for component, array in data.items():
        for field, (dtype, _) in array.dtype.fields.items():
            if not np.issubdtype(dtype, np.floating):
                continue
            invalid = np.isinf(array[field])
            if invalid.any():
                ids = data[component]["id"][invalid].flatten().tolist()
                errors.append(InfinityError(component, field, ids))
    return errors


def none_missing(data: SingleDataset, component: str, fields: Union[str, List[str]]) -> List[MissingValueError]:
    """
    Check that for all records of a particular type of component, the values in the 'fields' columns are not NaN.
    Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        fields: The fields of interest

    Returns:
        A list containing zero or more MissingValueError; one for each field, listing all ids where the value in the
        field was NaN.
    """
    errors = []
    if isinstance(fields, str):
        fields = [fields]
    for field in fields:
        nan = nan_type(component, field)
        if np.isnan(nan):
            invalid = np.isnan(data[component][field])
        else:
            invalid = np.equal(data[component][field], nan)
        if invalid.any():
            if invalid.ndim > 1:
                invalid = invalid.any(axis=1)
            ids = data[component]["id"][invalid].flatten().tolist()
            errors.append(MissingValueError(component, field, ids))
    return errors


def all_valid_clocks(
    data: SingleDataset, component: str, clock_field: str, winding_from_field: str, winding_to_field: str
) -> List[TransformerClockError]:
    """
    Custom validation rule: Odd clock number is only allowed for Dy(n) or Y(N)d configuration.

    Args:
        data: The input/update data set for all components
        component: The component of interest
        clock_field: The clock field
        winding_from_field: The winding from field
        winding_to_field: The winding to field

    Returns:
        A list containing zero or more TransformerClockErrors; listing all the ids of transformers where the clock was
        invalid, given the winding type.
    """

    clk = data[component][clock_field]
    wfr = data[component][winding_from_field]
    wto = data[component][winding_to_field]
    wfr_is_wye = np.isin(wfr, [WindingType.wye, WindingType.wye_n])
    wto_is_wye = np.isin(wto, [WindingType.wye, WindingType.wye_n])
    odd = clk % 2 == 1
    # even number is not possible if one side is wye winding and the other side is not wye winding.
    # odd number is not possible, if both sides are wye winding or both sides are not wye winding.
    err = (~odd & (wfr_is_wye != wto_is_wye)) | (odd & (wfr_is_wye == wto_is_wye))
    if err.any():
        return [
            TransformerClockError(
                component=component,
                fields=[clock_field, winding_from_field, winding_to_field],
                ids=data[component]["id"][err].flatten().tolist(),
            )
        ]
    return []
