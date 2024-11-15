# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
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

    component: ComponentType
        The name of the component, which should be an existing key in the data

    field: str
        The name of the column, which should be an field in the component data (numpy structured array)

Output data:
    errors: list[ValidationError]
        A list containing errors; in case of success, `errors` is the empty list: [].

"""
from enum import Enum
from typing import Any, Callable, Type, TypeVar

import numpy as np

from power_grid_model import ComponentType
from power_grid_model._utils import get_comp_size, is_nan_or_default
from power_grid_model.data_types import SingleDataset
from power_grid_model.enum import FaultPhase, FaultType, WindingType
from power_grid_model.validation.errors import (
    ComparisonError,
    FaultPhaseError,
    IdNotInDatasetError,
    InfinityError,
    InvalidAssociatedEnumValueError,
    InvalidEnumValueError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    MultiFieldValidationError,
    NotBetweenError,
    NotBetweenOrAtError,
    NotBooleanError,
    NotGreaterOrEqualError,
    NotGreaterThanError,
    NotIdenticalError,
    NotLessOrEqualError,
    NotLessThanError,
    NotUniqueError,
    SameValueError,
    TransformerClockError,
    TwoValuesZeroError,
    UnsupportedTransformerRegulationError,
    ValidationError,
)
from power_grid_model.validation.utils import (
    _eval_expression,
    _get_indexer,
    _get_mask,
    _get_valid_ids,
    _nan_type,
    _set_default_value,
)

Error = TypeVar("Error", bound=ValidationError)
CompError = TypeVar("CompError", bound=ComparisonError)


def all_greater_than_zero(data: SingleDataset, component: ComponentType, field: str) -> list[NotGreaterThanError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are greater than
    zero. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data (SingleDataset): The input/update data set for all components
        component (ComponentType): The component of interest
        field (str): The field of interest

    Returns:
        A list containing zero or one NotGreaterThanErrors, listing all ids where the value in the field of interest
        was zero or less.
    """
    return all_greater_than(data, component, field, 0.0)


def all_greater_than_or_equal_to_zero(
    data: SingleDataset,
    component: ComponentType,
    field: str,
    default_value: np.ndarray | int | float | None = None,
) -> list[NotGreaterOrEqualError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are greater than,
    or equal to zero. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data (SingleDataset): The input/update data set for all components
        component (ComponentType) The component of interest
        field (str): The field of interest
        default_value (np.ndarray | int | float | None, optional): Some values are not required, but will
            receive a default value in the C++ core. To do a proper input validation, these default values should be
            included in the validation. It can be a fixed value for the entire column (int/float) or be different for
            each element (np.ndarray).

    Returns:
        A list containing zero or one NotGreaterOrEqualErrors, listing all ids where the value in the field of
        interest was less than zero.
    """
    return all_greater_or_equal(data, component, field, 0.0, default_value)


def all_greater_than(
    data: SingleDataset, component: ComponentType, field: str, ref_value: int | float | str
) -> list[NotGreaterThanError]:
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
    data: SingleDataset,
    component: ComponentType,
    field: str,
    ref_value: int | float | str,
    default_value: np.ndarray | int | float | None = None,
) -> list[NotGreaterOrEqualError]:
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
        default_value: Some values are not required, but will receive a default value in the C++ core. To do a proper
        input validation, these default values should be included in the validation. It can be a fixed value for the
        entire column (int/float) or be different for each element (np.ndarray).

    Returns:
        A list containing zero or one NotGreaterOrEqualErrors, listing all ids where the value in the field of
        interest was less than the ref_value.

    """

    def not_greater_or_equal(val: np.ndarray, *ref: np.ndarray):
        return np.less(val, *ref)

    return none_match_comparison(
        data, component, field, not_greater_or_equal, ref_value, NotGreaterOrEqualError, default_value
    )


def all_less_than(
    data: SingleDataset, component: ComponentType, field: str, ref_value: int | float | str
) -> list[NotLessThanError]:
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
    data: SingleDataset, component: ComponentType, field: str, ref_value: int | float | str
) -> list[NotLessOrEqualError]:
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


# pylint: disable=too-many-arguments
def all_between(  # pylint: disable=too-many-positional-arguments
    data: SingleDataset,
    component: ComponentType,
    field: str,
    ref_value_1: int | float | str,
    ref_value_2: int | float | str,
    default_value: np.ndarray | int | float | None = None,
) -> list[NotBetweenError]:
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
        default_value: Some values are not required, but will receive a default value in the C++ core. To do a proper
        input validation, these default values should be included in the validation. It can be a fixed value for the
        entire column (int/float) or be different for each element (np.ndarray).

    Returns:
        A list containing zero or one NotBetweenErrors, listing all ids where the value in the field of interest was
        outside the range defined by the reference values.
    """

    def outside(val: np.ndarray, *ref: np.ndarray) -> np.ndarray:
        return np.logical_or(np.less_equal(val, np.minimum(*ref)), np.greater_equal(val, np.maximum(*ref)))

    return none_match_comparison(
        data, component, field, outside, (ref_value_1, ref_value_2), NotBetweenError, default_value
    )


# pylint: disable=too-many-arguments
def all_between_or_at(  # pylint: disable=too-many-positional-arguments
    data: SingleDataset,
    component: ComponentType,
    field: str,
    ref_value_1: int | float | str,
    ref_value_2: int | float | str,
    default_value_1: np.ndarray | int | float | None = None,
    default_value_2: np.ndarray | int | float | None = None,
) -> list[NotBetweenOrAtError]:
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
        default_value: Some values are not required, but will receive a default value in the C++ core. To do a proper
        input validation, these default values should be included in the validation. It can be a fixed value for the
        entire column (int/float) or be different for each element (np.ndarray).
        default_value_2: Some values can have a double default: the default will be set to another attribute of the
        component, but if that attribute is missing, the default will be set to a fixed value.

    Returns:
        A list containing zero or one NotBetweenOrAtErrors, listing all ids where the value in the field of interest was
        outside the range defined by the reference values.
    """

    def outside(val: np.ndarray, *ref: np.ndarray) -> np.ndarray:
        return np.logical_or(np.less(val, np.minimum(*ref)), np.greater(val, np.maximum(*ref)))

    return none_match_comparison(
        data,
        component,
        field,
        outside,
        (ref_value_1, ref_value_2),
        NotBetweenOrAtError,
        default_value_1,
        default_value_2,
    )


def none_match_comparison(  # pylint: disable=too-many-arguments
    data: SingleDataset,
    component: ComponentType,
    field: str,
    compare_fn: Callable,
    ref_value: ComparisonError.RefType,
    error: Type[CompError] = ComparisonError,  # type: ignore
    default_value_1: np.ndarray | int | float | None = None,
    default_value_2: np.ndarray | int | float | None = None,
) -> list[CompError]:
    # pylint: disable=too-many-positional-arguments
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
        default_value: Some values are not required, but will receive a default value in the C++ core. To do a proper
        input validation, these default values should be included in the validation. It can be a fixed value for the
        entire column (int/float) or be different for each element (np.ndarray).
        default_value_2: Some values can have a double default: the default will be set to another attribute of the
        component, but if that attribute is missing, the default will be set to a fixed value.

    Returns:
        A list containing zero or one comparison errors (should be a subclass of ComparisonError), listing all ids
        where the value in the field of interest matched the comparison.
    """
    if default_value_1 is not None:
        _set_default_value(data=data, component=component, field=field, default_value=default_value_1)
    if default_value_2 is not None:
        _set_default_value(data=data, component=component, field=field, default_value=default_value_2)
    component_data = data[component]
    if not isinstance(component_data, np.ndarray):
        raise NotImplementedError()  # TODO(mgovers): add support for columnar data

    if isinstance(ref_value, tuple):
        ref = tuple(_eval_expression(component_data, v) for v in ref_value)
    else:
        ref = (_eval_expression(component_data, ref_value),)
    matches = compare_fn(component_data[field], *ref)
    if matches.any():
        if matches.ndim > 1:
            matches = matches.any(axis=1)
        ids = component_data["id"][matches].flatten().tolist()
        return [error(component, field, ids, ref_value)]
    return []


def all_identical(data: SingleDataset, component: ComponentType, field: str) -> list[NotIdenticalError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are identical.

    Args:
        data (SingleDataset): The input/update data set for all components
        component (str): The component of interest
        field (str): The field of interest

    Returns:
        A list containing zero or one NotIdenticalError, listing all ids of that component if the value in the field
        of interest was not identical across all components, all values for those ids, the set of unique values in
        that field and the number of unique values in that field.
    """
    field_data = data[component][field]
    if len(field_data) > 0:
        first = field_data[0]
        if np.any(field_data != first):
            return [NotIdenticalError(component, field, data[component]["id"], list(field_data))]

    return []


def all_enabled_identical(
    data: SingleDataset, component: ComponentType, field: str, status_field: str
) -> list[NotIdenticalError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are identical.
    Only entries are checked where the 'status' field is not 0.

    Args:
        data (SingleDataset): The input/update data set for all components
        component (str): The component of interest
        field (str): The field of interest
        status_field (str): The status field based on which to decide whether a component is enabled

    Returns:
        A list containing zero or one NotIdenticalError, listing:

            - all ids of enabled components if the value in the field of interest was not identical across all enabled
              components
            - all values of the 'field' column for enabled components (including duplications)
            - the set of unique such values
            - the amount of unique such values.
    """
    return all_identical(
        {key: (value if key is not component else value[value[status_field] != 0]) for key, value in data.items()},
        component,
        field,
    )


def all_unique(data: SingleDataset, component: ComponentType, field: str) -> list[NotUniqueError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are unique within
    the 'field' column of that component.

    Args:
        data (SingleDataset): The input/update data set for all components
        component (str): The component of interest
        field (str): The field of interest

    Returns:
        A list containing zero or one NotUniqueError, listing all ids where the value in the field of interest was
        not unique. If the field name was 'id' (a very common check), the id is added as many times as it occurred in
        the 'id' column, to maintain object counts.
    """
    field_data = data[component][field]
    _, inverse, counts = np.unique(field_data, return_inverse=True, return_counts=True)
    if any(counts != 1):
        ids = data[component]["id"][(counts != 1)[inverse]].flatten().tolist()
        return [NotUniqueError(component, field, ids)]
    return []


def all_cross_unique(
    data: SingleDataset, fields: list[tuple[ComponentType, str]], cross_only=True
) -> list[MultiComponentNotUniqueError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are unique within
    the 'field' column of that component.

    Args:
        data (SingleDataset): The input/update data set for all components
        fields (list[tuple[str, str]]): The fields of interest, formatted as
            [(component_1, field_1), (component_2, field_2)]
        cross_only (bool, optional): Do not include duplicates within a single field. It is advised that you use
            all_unique() to explicitly check uniqueness within a single field.

    Returns:
        A list containing zero or one MultiComponentNotUniqueError, listing all fields and ids where the value was not
        unique between the fields.
    """
    all_values: dict[int, list[tuple[tuple[ComponentType, str], int]]] = {}
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
    data: SingleDataset, component: ComponentType, field: str, enum: Type[Enum] | list[Type[Enum]]
) -> list[InvalidEnumValueError]:
    """
    Check that for all records of a particular type of component, the values in the 'field' column are valid values for
    the supplied enum class. Returns an empty list on success, or a list containing a single error object on failure.

    Args:
        data (SingleDataset): The input/update data set for all components
        component (ComponentType): The component of interest
        field (str): The field of interest
        enum (Type[Enum] | list[Type[Enum]]): The enum type to validate against, or a list of such enum types

    Returns:
        A list containing zero or one InvalidEnumValueError, listing all ids where the value in the field of interest
        was not a valid value in the supplied enum type.
    """
    enums: list[Type[Enum]] = enum if isinstance(enum, list) else [enum]

    valid = {_nan_type(component, field)}
    for enum_type in enums:
        valid.update(list(enum_type))

    invalid = np.isin(data[component][field], np.array(list(valid), dtype=np.int8), invert=True)
    if invalid.any():
        ids = data[component]["id"][invalid].flatten().tolist()
        return [InvalidEnumValueError(component, field, ids, enum)]
    return []


# pylint: disable=too-many-arguments
def all_valid_associated_enum_values(  # pylint: disable=too-many-positional-arguments
    data: SingleDataset,
    component: ComponentType,
    field: str,
    ref_object_id_field: str,
    ref_components: list[ComponentType],
    enum: Type[Enum] | list[Type[Enum]],
    **filters: Any,
) -> list[InvalidAssociatedEnumValueError]:
    """
    Args:
        data (SingleDataset): The input/update data set for all components
        component (ComponentType): The component of interest
        field (str): The field of interest
        ref_object_id_field (str): The field that contains the referenced component ids
        ref_components (list[ComponentType]): The component or components in which we want to look for ids
        enum (Type[Enum] | list[Type[Enum]]): The enum type to validate against, or a list of such enum types
        **filters: One or more filters on the dataset. E.g. regulated_object="transformer".

    Returns:
        A list containing zero or one InvalidAssociatedEnumValueError, listing all ids where the value in the field
        of interest was not a valid value in the supplied enum type.
    """
    enums: list[Type[Enum]] = enum if isinstance(enum, list) else [enum]

    valid_ids = _get_valid_ids(data=data, ref_components=ref_components)
    mask = np.logical_and(
        _get_mask(data=data, component=component, field=field, **filters),
        np.isin(data[component][ref_object_id_field], valid_ids),
    )

    valid = {_nan_type(component, field)}
    for enum_type in enums:
        valid.update(list(enum_type))

    invalid = np.isin(data[component][field][mask], np.array(list(valid), dtype=np.int8), invert=True)
    if invalid.any():
        ids = data[component]["id"][mask][invalid].flatten().tolist()
        return [InvalidAssociatedEnumValueError(component, [field, ref_object_id_field], ids, enum)]
    return []


def all_valid_ids(
    data: SingleDataset,
    component: ComponentType,
    field: str,
    ref_components: ComponentType | list[ComponentType],
    **filters: Any,
) -> list[InvalidIdError]:
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
    valid_ids = _get_valid_ids(data=data, ref_components=ref_components)
    mask = _get_mask(data=data, component=component, field=field, **filters)

    # Find any values that can't be found in the set of ids
    invalid = np.logical_and(mask, np.isin(data[component][field], valid_ids, invert=True))
    if invalid.any():
        ids = data[component]["id"][invalid].flatten().tolist()
        return [InvalidIdError(component, field, ids, ref_components, filters)]
    return []


def all_boolean(data: SingleDataset, component: ComponentType, field: str) -> list[NotBooleanError]:
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
    data: SingleDataset, component: ComponentType, field_1: str, field_2: str
) -> list[TwoValuesZeroError]:
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


def all_not_two_values_equal(
    data: SingleDataset, component: ComponentType, field_1: str, field_2: str
) -> list[SameValueError]:
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


def ids_valid_in_update_data_set(
    update_data: SingleDataset, ref_data: SingleDataset, component: ComponentType, ref_name: str
) -> list[IdNotInDatasetError | InvalidIdError]:
    """
    Check that for all records of a particular type of component, whether the ids:
    - exist and match those in the reference data set
    - are not present but qualifies for optional id

    Args:
        update_data: The update data set for all components
        ref_data: The reference (input) data set for all components
        component: The component of interest
        ref_name: The name of the reference data set, e.g. 'update_data'
    Returns:
        A list containing zero or one IdNotInDatasetError, listing all ids of the objects in the data set which do not
        exist in the reference data set.
    """
    component_data = update_data[component]
    component_ref_data = ref_data[component]
    if component_ref_data["id"].size == 0:
        return [InvalidIdError(component=component, field="id", ids=None)]
    id_field_is_nan = np.array(is_nan_or_default(component_data["id"]))
    # check whether id qualify for optional
    if component_data["id"].size == 0 or np.all(id_field_is_nan):
        # check if the dimension of the component_data is the same as the component_ref_data
        if get_comp_size(component_data) != get_comp_size(component_ref_data):
            return [InvalidIdError(component=component, field="id", ids=None)]
        return []  # supported optional id

    if np.all(id_field_is_nan) and not np.all(~id_field_is_nan):
        return [InvalidIdError(component=component, field="id", ids=None)]

    # normal check: exist and match with input
    invalid = np.isin(component_data["id"], component_ref_data["id"], invert=True)
    if invalid.any():
        ids = component_data["id"][invalid].flatten().tolist()
        return [IdNotInDatasetError(component, ids, ref_name)]
    return []


def all_finite(data: SingleDataset, exceptions: dict[ComponentType, list[str]] | None = None) -> list[InfinityError]:
    """
    Check that for all records in all component, the values in all columns are finite value, i.e. float values other
    than inf, or -inf. Nan values are ignored, as in all other comparison functions. You can use non_missing() to
    check for missing/nan values. Returns an empty list on success, or a list containing an error object for each
    component/field combination where.

    Args:
        data: The input/update data set for all components
        exceptions:
            A dictionary of fields per component type for which infinite values are supported. Defaults to empty.

    Returns:
        A list containing zero or one NotBooleanError, listing all ids where the value in the field of interest was not
        a valid boolean value.
    """
    errors = []
    for component, array in data.items():
        if not isinstance(array, np.ndarray):
            raise NotImplementedError()  # TODO(mgovers): add support for columnar data

        for field, (dtype, _) in array.dtype.fields.items():
            if not np.issubdtype(dtype, np.floating):
                continue

            if exceptions and field in exceptions.get(component, []):
                continue

            invalid = np.isinf(array[field])
            if invalid.any():
                ids = data[component]["id"][invalid].flatten().tolist()
                errors.append(InfinityError(component, field, ids))
    return errors


def none_missing(
    data: SingleDataset,
    component: ComponentType,
    fields: list[str | list[str]] | str | list[str],
    index: int = 0,
) -> list[MissingValueError]:
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
        if isinstance(field, list):
            field = field[0]
        nan = _nan_type(component, field)
        if np.isnan(nan):
            invalid = np.isnan(data[component][field][index])
        else:
            invalid = np.equal(data[component][field][index], nan)

        if invalid.any():
            if isinstance(invalid, np.ndarray):
                invalid = np.any(invalid)
            ids = data[component]["id"][invalid].flatten().tolist()
            errors.append(MissingValueError(component, field, ids))
    return errors


def valid_p_q_sigma(data: SingleDataset, component: ComponentType) -> list[MultiFieldValidationError]:
    """
    Check validity of the pair `(p_sigma, q_sigma)` for 'sym_power_sensor' and 'asym_power_sensor'.

    Args:
        data: The input/update data set for all components
        component: The component of interest, in this case only 'sym_power_sensor' or 'asym_power_sensor'

    Returns:
        A list containing zero or one MultiFieldValidationError, listing the p_sigma and q_sigma mismatch.
        Note that with asymetric power sensors, partial assignment of p_sigma and q_sigma is also considered mismatch.
    """
    errors = []
    p_sigma = data[component]["p_sigma"]
    q_sigma = data[component]["q_sigma"]
    p_nan = np.isnan(p_sigma)
    q_nan = np.isnan(q_sigma)
    p_inf = np.isinf(p_sigma)
    q_inf = np.isinf(q_sigma)
    if p_sigma.ndim > 1:  # if component == 'asym_power_sensor':
        p_nan = p_nan.any(axis=-1)
        q_nan = q_nan.any(axis=-1)
        p_inf = p_inf.any(axis=-1)
        q_inf = q_inf.any(axis=-1)
    mis_match = p_nan != q_nan
    mis_match |= np.logical_or(p_inf, q_inf)
    if mis_match.any():
        ids = data[component]["id"][mis_match].flatten().tolist()
        errors.append(MultiFieldValidationError(component, ["p_sigma", "q_sigma"], ids))
    return errors


def all_valid_clocks(
    data: SingleDataset, component: ComponentType, clock_field: str, winding_from_field: str, winding_to_field: str
) -> list[TransformerClockError]:
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


def all_valid_fault_phases(
    data: SingleDataset, component: ComponentType, fault_type_field: str, fault_phase_field: str
) -> list[FaultPhaseError]:
    """
    Custom validation rule: Only a subset of fault_phases is supported for each fault type.

    Args:
        data (SingleDataset): The input/update data set for all components
        component (str): The component of interest
        fault_type_field (str): The fault type field
        fault_phase_field (str): The fault phase field

    Returns:
        A list containing zero or more FaultPhaseErrors; listing all the ids of faults where the fault phase was
        invalid, given the fault phase.
    """
    fault_types = data[component][fault_type_field]
    fault_phases = data[component][fault_phase_field]

    supported_combinations: dict[FaultType, list[FaultPhase]] = {
        FaultType.three_phase: [FaultPhase.abc, FaultPhase.default_value, FaultPhase.nan],
        FaultType.single_phase_to_ground: [
            FaultPhase.a,
            FaultPhase.b,
            FaultPhase.c,
            FaultPhase.default_value,
            FaultPhase.nan,
        ],
        FaultType.two_phase: [FaultPhase.ab, FaultPhase.ac, FaultPhase.bc, FaultPhase.default_value, FaultPhase.nan],
        FaultType.two_phase_to_ground: [
            FaultPhase.ab,
            FaultPhase.ac,
            FaultPhase.bc,
            FaultPhase.default_value,
            FaultPhase.nan,
        ],
        FaultType.nan: [],
    }

    def _fault_phase_supported(fault_type: FaultType, fault_phase: FaultPhase):
        return fault_phase not in supported_combinations.get(fault_type, [])

    err = np.vectorize(_fault_phase_supported)(fault_type=fault_types, fault_phase=fault_phases)
    if err.any():
        return [
            FaultPhaseError(
                component=component,
                fields=[fault_type_field, fault_phase_field],
                ids=data[component]["id"][err].flatten().tolist(),
            )
        ]
    return []


def all_supported_tap_control_side(  # pylint: disable=too-many-arguments
    data: SingleDataset,
    component: ComponentType,
    control_side_field: str,
    regulated_object_field: str,
    tap_side_fields: list[tuple[ComponentType, str]],
    **filters: Any,
) -> list[UnsupportedTransformerRegulationError]:
    """
    Args:
        data (SingleDataset): The input/update data set for all components
        component (ComponentType): The component of interest
        control_side_field (str): The field of interest
        regulated_object_field (str): The field that contains the regulated component ids
        tap_side_fields (list[tuple[ComponentType, str]]): The fields of interest per regulated component,
            formatted as [(component_1, field_1), (component_2, field_2)]
        **filters: One or more filters on the dataset. E.g. regulated_object="transformer".

    Returns:
        A list containing zero or more InvalidAssociatedEnumValueErrors; listing all the ids
        of components where the field of interest was invalid, given the referenced object's field.
    """
    mask = _get_mask(data=data, component=component, field=control_side_field, **filters)
    values = data[component][control_side_field][mask]

    invalid = np.zeros_like(mask)

    for ref_component, ref_field in tap_side_fields:
        if ref_component in data:
            indices = _get_indexer(data[ref_component]["id"], data[component][regulated_object_field], default_value=-1)
            found = indices != -1
            ref_comp_values = data[ref_component][ref_field][indices[found]]
            invalid[found] = np.logical_or(invalid[found], values[found] == ref_comp_values)

    if invalid.any():
        return [
            UnsupportedTransformerRegulationError(
                component=component,
                fields=[control_side_field, regulated_object_field],
                ids=data[component]["id"][invalid].flatten().tolist(),
            )
        ]
    return []
