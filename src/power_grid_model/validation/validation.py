# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power Grid Model Validation Functions.

Although all functions are 'public', you probably only need validate_input_data() and validate_batch_data().

"""
from itertools import chain
from typing import Dict, List, Optional, Union

import numpy as np

from .errors import IdNotInDatasetError, MissingValueError, MultiComponentNotUniqueError, ValidationError
from .rules import (
    none_missing,
    all_finite,
    all_ids_exist_in_data_set,
    all_greater_than_zero,
    all_greater_than_or_equal_to_zero,
    all_greater_or_equal,
    all_less_than,
    all_between,
    all_between_or_at,
    all_unique,
    all_cross_unique,
    all_not_two_values_equal,
    all_valid_enum_values,
    all_boolean,
    all_valid_ids,
    all_not_two_values_zero,
    all_clocks_valid,
)
from .utils import InputData, UpdateData, split_update_data_in_batches, update_input_data
from .. import power_grid_meta_data
from ..enum import BranchSide, CalculationType, LoadGenType, MeasuredTerminalType, WindingType


def validate_input_data(
    input_data: InputData, calculation_type: Optional[CalculationType] = None, symmetric: bool = True
) -> Optional[List[ValidationError]]:
    """
    Validates the entire input dataset:

        1. Is the data structure correct? (checking data types and numpy array shapes)
        2. Are all required values provided? (checking NaNs)
        3. Are all ID's unique? (checking object identifiers across all components)
        4. Are the supplied values valid? (checking limits and other logic as described in "Graph Data Model")

    Args:
        input_data: A power-grid-model input dataset
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Raises:
        KeyError, TypeError or ValueError if the data structure is invalid.

    Returns:
        None if the data is valid, or a list containing all validation errors.
    """
    assert_valid_data_structure(input_data, "input")

    errors: List[ValidationError] = []
    errors += validate_required_values(input_data, calculation_type, symmetric)
    errors += validate_unique_ids_across_components(input_data)
    errors += validate_values(input_data)
    return errors if errors else None


def validate_batch_data(
    input_data: InputData,
    update_data: UpdateData,
    calculation_type: Optional[CalculationType] = None,
    symmetric: bool = True,
) -> Optional[Dict[int, List[ValidationError]]]:
    """
    Ihe input dataset is validated:

        1. Is the data structure correct? (checking data types and numpy array shapes)
        2. Are all input data ID's unique? (checking object identifiers across all components)

    For each batch the update data is validated:
        3. Is the update data structure correct? (checking data types and numpy array shapes)
        4. Are all update ID's valid? (checking object identifiers across update and input data)

    Then (for each batch independently) the input dataset is updated with the batch's update data and validated:
        5. Are all required values provided? (checking NaNs)
        6. Are the supplied values valid? (checking limits and other logic as described in "Graph Data Model")

    Args:
        input_data: a power-grid-model input dataset
        update_data: a power-grid-model update dataset (one or more batches)
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Raises:
        KeyError, TypeError or ValueError if the data structure is invalid.

    Returns:
        None if the data is valid, or a dictionary containing all validation errors,
        where the key is the batch number (0-indexed).
    """
    assert_valid_data_structure(input_data, "input")
    input_errors: List[ValidationError] = validate_unique_ids_across_components(input_data)

    # Splitting update_data_into_batches may raise TypeErrors and ValueErrors
    batch_data = split_update_data_in_batches(update_data)

    errors = {}
    for batch, batch_update_data in enumerate(batch_data):

        assert_valid_data_structure(batch_update_data, "update")
        id_errors: List[ValidationError] = validate_ids_exist(batch_update_data, input_data)

        batch_errors = input_errors + id_errors
        if not id_errors:
            merged_data = update_input_data(input_data, batch_update_data)
            batch_errors += validate_required_values(merged_data, calculation_type, symmetric)
            batch_errors += validate_values(merged_data)

        if batch_errors:
            errors[batch] = batch_errors

    return errors if errors else None


def assert_valid_data_structure(data: Union[InputData, UpdateData], data_type: str) -> None:
    """
    Checks if all component names are valid and if the data inside the component matches the required Numpy
    structured array as defined in the Power Grid Model meta data.

    Args:
        data: a power-grid-model input/update dataset
        data_type: 'input' or 'update'

    Raises: KeyError, TypeError

    """
    if data_type not in {"input", "update"}:
        raise KeyError(f"Unexpected data type '{data_type}' (should be 'input' or 'update')")

    component_dtype = {component: meta["dtype"] for component, meta in power_grid_meta_data[data_type].items()}
    for component, array in data.items():

        # Check if component name is valid
        if component not in component_dtype:
            raise KeyError(f"Unknown component '{component}' in {data_type}_data.")

        # Check if component definition is as expected
        dtype = component_dtype[component]
        if isinstance(array, np.ndarray):
            if array.dtype != dtype:
                if not hasattr(array.dtype, "names") or not array.dtype.names:
                    raise TypeError(
                        f"Unexpected Numpy array ({array.dtype}) for '{component}' {data_type}_data "
                        "(should be a Numpy structured array)."
                    )
                raise TypeError(f"Unexpected Numpy structured array; (expected = {dtype}, actual = {array.dtype}).")
        else:
            raise TypeError(
                f"Unexpected data type {type(array).__name__} for '{component}' {data_type}_data "
                "(should be a Numpy structured array)."
            )


def validate_unique_ids_across_components(data: InputData) -> List[MultiComponentNotUniqueError]:
    """
    Checks if all ids in the input dataset are unique

    Args:
        data: a power-grid-model input dataset

    Returns: an empty list if all ids are unique, or a list of MultiComponentNotUniqueErrors for all components that
             have non-unique ids

    """
    return all_cross_unique(data, [(component, "id") for component in data])


def validate_ids_exist(update_data: Dict[str, np.ndarray], input_data: InputData) -> List[IdNotInDatasetError]:
    """
    Checks if all ids of the components in the update data exist in the input data. This needs to be true, because you
    can only update existing components.

    This function should be called for every update dataset in a batch set

    Args:
        update_data: a single update dataset
        input_data: a power-grid-model input dataset

    Returns: an empty list if all update data ids exist in the input dataset, or a list of IdNotInDatasetErrors for
             all update components of which the id does not exist in the input dataset

    """
    errors = (all_ids_exist_in_data_set(update_data, input_data, component, "input_data") for component in update_data)
    return list(chain(*errors))


def validate_required_values(
    data: InputData, calculation_type: Optional[CalculationType] = None, symmetric: bool = True
) -> List[MissingValueError]:
    """
    Checks if all required data is available.

    Args:
        data: a power-grid-model input dataset
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Returns: an empty list if all required data is available, or a list of MissingValueErrors.

    """
    # Base
    required = {"base": ["id"]}

    # Nodes
    required["node"] = required["base"] + ["u_rated"]

    # Branches
    required["branch"] = required["base"] + ["from_node", "to_node", "from_status", "to_status"]
    required["link"] = required["branch"].copy()
    required["line"] = required["branch"] + ["r1", "x1", "c1", "tan1", "i_n"]
    required["transformer"] = required["branch"] + [
        "u1",
        "u2",
        "sn",
        "uk",
        "pk",
        "i0",
        "p0",
        "winding_from",
        "winding_to",
        "clock",
        "tap_side",
        "tap_pos",
        "tap_min",
        "tap_max",
        "tap_size",
    ]
    # Appliances
    required["appliance"] = required["base"] + ["node", "status"]
    required["source"] = required["appliance"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required["source"] += ["u_ref"]
    required["shunt"] = required["appliance"] + ["g1", "b1"]
    required["generic_load_gen"] = required["appliance"] + ["type"]
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required["generic_load_gen"] += ["p_specified", "q_specified"]
    required["sym_load"] = required["generic_load_gen"].copy()
    required["asym_load"] = required["generic_load_gen"].copy()
    required["sym_gen"] = required["generic_load_gen"].copy()
    required["asym_gen"] = required["generic_load_gen"].copy()

    # Sensors
    required["sensor"] = required["base"] + ["measured_object"]
    required["voltage_sensor"] = required["sensor"].copy()
    required["power_sensor"] = required["sensor"] + ["measured_terminal_type"]
    if calculation_type is None or calculation_type == CalculationType.state_estimation:
        required["voltage_sensor"] += ["u_sigma", "u_measured"]
        required["power_sensor"] += ["power_sigma", "p_measured", "q_measured"]
    required["sym_voltage_sensor"] = required["voltage_sensor"].copy()
    required["asym_voltage_sensor"] = required["voltage_sensor"].copy()
    required["sym_power_sensor"] = required["power_sensor"].copy()
    required["asym_power_sensor"] = required["power_sensor"].copy()

    if not symmetric:
        required["line"] += ["r0", "x0", "c0", "tan0"]
        required["shunt"] += ["g0", "b0"]

    return list(chain(*(none_missing(data, component, required.get(component, [])) for component in data)))


def validate_values(data: InputData) -> List[ValidationError]:  # pylint: disable=too-many-branches
    """
    For each component supplied in the data, call the appropriate validation function

    Args:
        data: a power-grid-model input dataset

    Returns: an empty list if all required data is valid, or a list of ValidationErrors.

    """
    errors: List[ValidationError] = all_finite(data)
    if "node" in data:
        errors += validate_node(data)
    if "line" in data:
        errors += validate_line(data)
    if "link" in data:
        errors += validate_branch(data, "link")
    if "transformer" in data:
        errors += validate_transformer(data)
    if "source" in data:
        errors += validate_source(data)
    if "sym_load" in data:
        errors += validate_generic_load_gen(data, "sym_load")
    if "sym_gen" in data:
        errors += validate_generic_load_gen(data, "sym_gen")
    if "asym_load" in data:
        errors += validate_generic_load_gen(data, "asym_load")
    if "asym_gen" in data:
        errors += validate_generic_load_gen(data, "asym_gen")
    if "shunt" in data:
        errors += validate_shunt(data)
    if "sym_voltage_sensor" in data:
        errors += validate_generic_voltage_sensor(data, "sym_voltage_sensor")
    if "asym_voltage_sensor" in data:
        errors += validate_generic_voltage_sensor(data, "asym_voltage_sensor")
    if "sym_power_sensor" in data:
        errors += validate_generic_power_sensor(data, "sym_power_sensor")
    if "asym_power_sensor" in data:
        errors += validate_generic_power_sensor(data, "asym_power_sensor")
    return errors


# pylint: disable=missing-function-docstring


def validate_base(data: InputData, component: str) -> List[ValidationError]:
    errors = all_unique(data, component, "id")
    return errors


def validate_node(data: InputData) -> List[ValidationError]:
    errors = validate_base(data, "node")
    errors += all_greater_than_zero(data, "node", "u_rated")
    return errors


def validate_branch(data: InputData, component: str) -> List[ValidationError]:
    errors = validate_base(data, component)
    errors += all_valid_ids(data, component, "from_node", "node")
    errors += all_valid_ids(data, component, "to_node", "node")
    errors += all_not_two_values_equal(data, component, "to_node", "from_node")
    errors += all_boolean(data, component, "from_status")
    errors += all_boolean(data, component, "to_status")
    return errors


def validate_line(data: InputData) -> List[ValidationError]:
    errors = validate_branch(data, "line")
    errors += all_not_two_values_zero(data, "line", "r1", "x1")
    errors += all_not_two_values_zero(data, "line", "r0", "x0")
    errors += all_greater_than_zero(data, "line", "i_n")
    return errors


def validate_transformer(data: InputData) -> List[ValidationError]:
    errors = validate_branch(data, "transformer")
    errors += all_greater_than_zero(data, "transformer", "u1")
    errors += all_greater_than_zero(data, "transformer", "u2")
    errors += all_greater_than_zero(data, "transformer", "sn")
    errors += all_greater_or_equal(data, "transformer", "uk", "pk/sn")
    errors += all_between(data, "transformer", "uk", 0, 1)
    errors += all_greater_than_or_equal_to_zero(data, "transformer", "pk")
    errors += all_greater_or_equal(data, "transformer", "i0", "p0/sn")
    errors += all_less_than(data, "transformer", "i0", 1)
    errors += all_greater_than_or_equal_to_zero(data, "transformer", "p0")
    errors += all_valid_enum_values(data, "transformer", "winding_from", WindingType)
    errors += all_valid_enum_values(data, "transformer", "winding_to", WindingType)
    errors += all_between_or_at(data, "transformer", "clock", 0, 12)
    errors += all_clocks_valid(data, "transformer", "clock", "winding_from", "winding_to")
    errors += all_valid_enum_values(data, "transformer", "tap_side", BranchSide)
    errors += all_between_or_at(data, "transformer", "tap_pos", "tap_min", "tap_max")
    errors += all_between_or_at(data, "transformer", "tap_nom", "tap_min", "tap_max")
    errors += all_greater_than_zero(data, "transformer", "tap_size")
    errors += all_greater_or_equal(data, "transformer", "uk_min", "pk_min/sn")
    errors += all_between(data, "transformer", "uk_min", 0, 1)
    errors += all_greater_or_equal(data, "transformer", "uk_max", "pk_max/sn")
    errors += all_between(data, "transformer", "uk_max", 0, 1)
    errors += all_greater_than_or_equal_to_zero(data, "transformer", "uk_min")
    errors += all_greater_than_or_equal_to_zero(data, "transformer", "uk_max")
    errors += all_greater_than_or_equal_to_zero(data, "transformer", "pk_min")
    errors += all_greater_than_or_equal_to_zero(data, "transformer", "pk_max")
    return errors


def validate_appliance(data: InputData, component: str) -> List[ValidationError]:
    errors = validate_base(data, component)
    errors += all_boolean(data, component, "status")
    errors += all_valid_ids(data, component, "node", "node")
    return errors


def validate_source(data: InputData) -> List[ValidationError]:
    errors = validate_appliance(data, "source")
    errors += all_greater_than_zero(data, "source", "u_ref")
    errors += all_greater_than_zero(data, "source", "sk")
    errors += all_greater_than_or_equal_to_zero(data, "source", "rx_ratio")
    errors += all_greater_than_zero(data, "source", "z01_ratio")
    return errors


def validate_generic_load_gen(data: InputData, component: str) -> List[ValidationError]:
    errors = validate_appliance(data, component)
    errors += all_valid_enum_values(data, component, "type", LoadGenType)
    return errors


def validate_shunt(data: InputData) -> List[ValidationError]:
    errors = validate_appliance(data, "shunt")
    return errors


def validate_generic_voltage_sensor(data: InputData, component: str) -> List[ValidationError]:
    errors = validate_base(data, component)
    errors += all_greater_than_zero(data, component, "u_sigma")
    errors += all_greater_than_zero(data, component, "u_measured")
    errors += all_valid_ids(data, component, "measured_object", "node")
    return errors


def validate_generic_power_sensor(data: InputData, component: str) -> List[ValidationError]:
    errors = validate_base(data, component)
    errors += all_greater_than_zero(data, component, "power_sigma")
    errors += all_valid_enum_values(data, component, "measured_terminal_type", MeasuredTerminalType)
    errors += all_valid_ids(
        data,
        component,
        "measured_object",
        ["line", "transformer", "source", "shunt", "sym_load", "asym_load", "sym_gen", "asym_gen"],
    )
    errors += all_valid_ids(
        data,
        component,
        "measured_object",
        ["line", "transformer"],
        measured_terminal_type=MeasuredTerminalType.branch_from,
    )
    errors += all_valid_ids(
        data,
        component,
        "measured_object",
        ["line", "transformer"],
        measured_terminal_type=MeasuredTerminalType.branch_to,
    )
    errors += all_valid_ids(
        data, component, "measured_object", "source", measured_terminal_type=MeasuredTerminalType.source
    )
    errors += all_valid_ids(
        data, component, "measured_object", "shunt", measured_terminal_type=MeasuredTerminalType.shunt
    )
    errors += all_valid_ids(
        data, component, "measured_object", ["sym_load", "asym_load"], measured_terminal_type=MeasuredTerminalType.load
    )
    errors += all_valid_ids(
        data,
        component,
        "measured_object",
        ["sym_gen", "asym_gen"],
        measured_terminal_type=MeasuredTerminalType.generator,
    )

    return errors
