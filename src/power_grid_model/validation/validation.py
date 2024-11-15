# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power Grid Model Validation Functions.

Although all functions are 'public', you probably only need validate_input_data() and validate_batch_data().

"""

# pylint: disable=too-many-lines

import copy
from collections.abc import Sized as ABCSized
from itertools import chain
from typing import cast

import numpy as np

from power_grid_model import ComponentType, DatasetType, power_grid_meta_data
from power_grid_model._utils import (
    compatibility_convert_row_columnar_dataset as _compatibility_convert_row_columnar_dataset,
    convert_batch_dataset_to_batch_list as _convert_batch_dataset_to_batch_list,
)
from power_grid_model.data_types import BatchDataset, Dataset, SingleDataset
from power_grid_model.enum import (
    Branch3Side,
    BranchSide,
    CalculationType,
    FaultPhase,
    FaultType,
    LoadGenType,
    MeasuredTerminalType,
    WindingType,
)
from power_grid_model.validation.errors import (
    IdNotInDatasetError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    ValidationError,
)
from power_grid_model.validation.rules import (
    all_between as _all_between,
    all_between_or_at as _all_between_or_at,
    all_boolean as _all_boolean,
    all_cross_unique as _all_cross_unique,
    all_enabled_identical as _all_enabled_identical,
    all_finite as _all_finite,
    all_greater_or_equal as _all_greater_or_equal,
    all_greater_than_or_equal_to_zero as _all_greater_than_or_equal_to_zero,
    all_greater_than_zero as _all_greater_than_zero,
    all_less_than as _all_less_than,
    all_not_two_values_equal as _all_not_two_values_equal,
    all_not_two_values_zero as _all_not_two_values_zero,
    all_supported_tap_control_side as _all_supported_tap_control_side,
    all_unique as _all_unique,
    all_valid_associated_enum_values as _all_valid_associated_enum_values,
    all_valid_clocks as _all_valid_clocks,
    all_valid_enum_values as _all_valid_enum_values,
    all_valid_fault_phases as _all_valid_fault_phases,
    all_valid_ids as _all_valid_ids,
    ids_valid_in_update_data_set as _ids_valid_in_update_data_set,
    none_missing as _none_missing,
    valid_p_q_sigma as _valid_p_q_sigma,
)
from power_grid_model.validation.utils import _update_input_data


def validate_input_data(
    input_data: SingleDataset, calculation_type: CalculationType | None = None, symmetric: bool = True
) -> list[ValidationError] | None:
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

    Returns:
        None if the data is valid, or a list containing all validation errors.

    Raises:
        Error: KeyError | TypeError | ValueError: if the data structure is invalid.
    """
    # Convert to row based if in columnar or mixed format format
    row_input_data = _compatibility_convert_row_columnar_dataset(input_data, None, DatasetType.input)

    # A deep copy is made of the input data, since default values will be added in the validation process
    input_data_copy = copy.deepcopy(row_input_data)

    assert_valid_data_structure(input_data_copy, DatasetType.input)

    errors: list[ValidationError] = []
    errors += validate_required_values(input_data_copy, calculation_type, symmetric)
    errors += validate_unique_ids_across_components(input_data_copy)
    errors += validate_values(input_data_copy, calculation_type)
    return errors if errors else None


def validate_batch_data(
    input_data: SingleDataset,
    update_data: BatchDataset,
    calculation_type: CalculationType | None = None,
    symmetric: bool = True,
) -> dict[int, list[ValidationError]] | None:
    """
    The input dataset is validated:

        1. Is the data structure correct? (checking data types and numpy array shapes)
        2. Are all input data ID's unique? (checking object identifiers across all components)

    For each batch the update data is validated:
        3. Is the update data structure correct? (checking data types and numpy array shapes)
        4. Are all update ID's valid? (checking object identifiers across update and input data)

    Then (for each batch independently) the input dataset is updated with the batch's update data and validated:
        5. Are all required values provided? (checking NaNs)
        6. Are the supplied values valid? (checking limits and other logic as described in "Graph Data Model")

    Args:
        input_data: A power-grid-model input dataset
        update_data: A power-grid-model update dataset (one or more batches)
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Returns:
        None if the data is valid, or a dictionary containing all validation errors,
        where the key is the batch number (0-indexed).

    Raises:
        Error: KeyError | TypeError | ValueError: if the data structure is invalid.
    """
    # Convert to row based if in columnar or mixed format
    row_input_data = _compatibility_convert_row_columnar_dataset(input_data, None, DatasetType.input)

    # A deep copy is made of the input data, since default values will be added in the validation process
    input_data_copy = copy.deepcopy(row_input_data)
    assert_valid_data_structure(input_data_copy, DatasetType.input)

    input_errors: list[ValidationError] = list(validate_unique_ids_across_components(input_data_copy))

    batch_data = _convert_batch_dataset_to_batch_list(update_data, DatasetType.update)

    errors = {}
    for batch, batch_update_data in enumerate(batch_data):
        row_update_data = _compatibility_convert_row_columnar_dataset(batch_update_data, None, DatasetType.update)
        assert_valid_data_structure(row_update_data, DatasetType.update)
        id_errors: list[IdNotInDatasetError | InvalidIdError] = validate_ids(row_update_data, input_data_copy)

        batch_errors = input_errors + id_errors

        if not id_errors:
            batch_errors = input_errors
            merged_data = _update_input_data(input_data_copy, row_update_data)
            batch_errors += validate_required_values(merged_data, calculation_type, symmetric)
            batch_errors += validate_values(merged_data, calculation_type)

        if batch_errors:
            errors[batch] = batch_errors

    return errors if errors else None


def assert_valid_data_structure(data: Dataset, data_type: DatasetType) -> None:
    """
    Checks if all component names are valid and if the data inside the component matches the required Numpy
    structured array as defined in the Power Grid Model meta data.

    Args:
        data: A power-grid-model input/update dataset
        data_type: 'input' or 'update'

    Raises:
        Error: KeyError, TypeError

    """
    if data_type not in {DatasetType.input, DatasetType.update}:
        raise KeyError(f"Unexpected data type '{data_type}' (should be 'input' or 'update')")

    component_dtype = {component: meta.dtype for component, meta in power_grid_meta_data[data_type].items()}
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


def validate_unique_ids_across_components(data: SingleDataset) -> list[MultiComponentNotUniqueError]:
    """
    Checks if all ids in the input dataset are unique

    Args:
        data: A power-grid-model input dataset

    Returns:
        An empty list if all ids are unique, or a list of MultiComponentNotUniqueErrors for all components that
        have non-unique ids
    """
    return _all_cross_unique(data, [(component, "id") for component in data])


def validate_ids(update_data: SingleDataset, input_data: SingleDataset) -> list[IdNotInDatasetError | InvalidIdError]:
    """
    Checks if all ids of the components in the update data:
     - exist and match those in the input data
     - are not present but qualifies for optional id

    This function should be called for every update dataset in a batch set

    Args:
        update_data: A single update dataset
        input_data: Input dataset

    Returns:
        An empty list if all update data ids are valid, or a list of IdNotInDatasetErrors or InvalidIdError for
        all update components that have invalid ids

    """
    errors = (
        _ids_valid_in_update_data_set(update_data, input_data, component, "update_data") for component in update_data
    )
    return list(chain(*errors))


def _process_power_sigma_and_p_q_sigma(
    data: SingleDataset,
    sensor: ComponentType,
    required_list: dict[ComponentType | str, list[str | list[str]]],
) -> None:
    """
    Helper function to process the required list when both `p_sigma` and `q_sigma` exist
    and valid but `power_sigma` is missing. The field `power_sigma` is set to the norm of
    `p_sigma` and `q_sigma`in this case. Happens only on proxy data (not the original data).
    However, note that this value is eventually not used in the calculation.
    """

    def _check_sensor_in_data(_data, _sensor):
        return _sensor in _data and isinstance(_data[_sensor], np.ndarray)

    def _contains_p_q_sigma(_sensor_data):
        return "p_sigma" in _sensor_data.dtype.names and "q_sigma" in _sensor_data.dtype.names

    def _process_power_sigma_in_list(_sensor_mask, _power_sigma, _p_sigma, _q_sigma):
        _mask = np.logical_not(np.logical_or(np.isnan(_p_sigma), np.isnan(_q_sigma)))
        if _power_sigma.ndim < _mask.ndim:
            _mask = np.any(_mask, axis=tuple(range(_power_sigma.ndim, _mask.ndim)))

        for sublist, should_remove in zip(_sensor_mask, _mask):
            if should_remove and "power_sigma" in sublist:
                sublist = cast(list[str], sublist)
                sublist.remove("power_sigma")

    if _check_sensor_in_data(data, sensor):
        sensor_data = data[sensor]
        sensor_mask = required_list[sensor]
        if _contains_p_q_sigma(sensor_data):
            p_sigma = sensor_data["p_sigma"]
            q_sigma = sensor_data["q_sigma"]
            power_sigma = sensor_data["power_sigma"]

            _process_power_sigma_in_list(sensor_mask, power_sigma, p_sigma, q_sigma)


def validate_required_values(
    data: SingleDataset, calculation_type: CalculationType | None = None, symmetric: bool = True
) -> list[MissingValueError]:
    """
    Checks if all required data is available.

    Args:
        data: A power-grid-model input dataset
        calculation_type: Supply a calculation method, to allow missing values for unused fields
        symmetric: A boolean to state whether input data will be used for a symmetric or asymmetric calculation

    Returns:
        An empty list if all required data is available, or a list of MissingValueErrors.
    """
    # Base
    required: dict[ComponentType | str, list[str | list[str]]] = {"base": ["id"]}

    # Nodes
    required["node"] = required["base"] + ["u_rated"]

    # Branches
    required["branch"] = required["base"] + ["from_node", "to_node", "from_status", "to_status"]
    required["link"] = required["branch"].copy()
    required["line"] = required["branch"] + ["r1", "x1", "c1", "tan1"]
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
        "tap_min",
        "tap_max",
        "tap_size",
    ]
    # Branch3
    required["branch3"] = required["base"] + ["node_1", "node_2", "node_3", "status_1", "status_2", "status_3"]
    required["three_winding_transformer"] = required["branch3"] + [
        "u1",
        "u2",
        "u3",
        "sn_1",
        "sn_2",
        "sn_3",
        "uk_12",
        "uk_13",
        "uk_23",
        "pk_12",
        "pk_13",
        "pk_23",
        "i0",
        "p0",
        "winding_1",
        "winding_2",
        "winding_3",
        "clock_12",
        "clock_13",
        "tap_side",
        "tap_min",
        "tap_max",
        "tap_size",
    ]

    # Regulators
    required["regulator"] = required["base"] + ["regulated_object", "status"]
    required["transformer_tap_regulator"] = required["regulator"]
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required["transformer_tap_regulator"] += ["control_side", "u_set", "u_band"]

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
    # Different requirements for individual sensors. Avoid shallow copy.
    for sensor_type in ("sym_power_sensor", "asym_power_sensor"):
        try:
            required[sensor_type] = [
                required["power_sensor"].copy() for _ in range(data[sensor_type].shape[0])  # type: ignore
            ]
        except KeyError:
            pass

    # Faults
    required["fault"] = required["base"] + ["fault_object"]
    asym_sc = False
    if calculation_type is None or calculation_type == CalculationType.short_circuit:
        required["fault"] += ["status", "fault_type"]
        if "fault" in data:
            for elem in data[ComponentType.fault]["fault_type"]:
                if elem not in (FaultType.three_phase, FaultType.nan):
                    asym_sc = True
                    break

    if not symmetric or asym_sc:
        required["line"] += ["r0", "x0", "c0", "tan0"]
        required["shunt"] += ["g0", "b0"]

    _process_power_sigma_and_p_q_sigma(data, ComponentType.sym_power_sensor, required)
    _process_power_sigma_and_p_q_sigma(data, ComponentType.asym_power_sensor, required)

    return _validate_required_in_data(data, required)


def _validate_required_in_data(data, required):
    """
    Checks if all required data is available.

    Args:
        data: A power-grid-model input dataset
        required: a list of required fields (a list of str), per component when applicaple (a list of str or str lists)

    Returns:
        An empty list if all required data is available, or a list of MissingValueErrors.
    """

    def is_valid_component(data, component):
        return (
            not (isinstance(data[component], np.ndarray) and data[component].size == 0)
            and data[component] is not None
            and isinstance(data[component], ABCSized)
        )

    def is_nested_list(items):
        return isinstance(items, list) and all(isinstance(i, list) for i in items)

    def process_nested_items(component, items, data, results):
        for index, item in enumerate(sublist for sublist in items):
            if index < len(data[component]):
                results.append(_none_missing(data, component, item, index))

    results = []

    for component in data:
        if is_valid_component(data, component):
            items = required.get(component, [])
            if is_nested_list(items):
                process_nested_items(component, items, data, results)
            else:
                results.append(_none_missing(data, component, items, 0))

    return list(chain(*results))


def validate_values(data: SingleDataset, calculation_type: CalculationType | None = None) -> list[ValidationError]:
    """
    For each component supplied in the data, call the appropriate validation function

    Args:
        data: A power-grid-model input dataset
        calculation_type: Supply a calculation method, to allow missing values for unused fields

    Returns:
        An empty list if all required data is valid, or a list of ValidationErrors.

    """
    errors: list[ValidationError] = list(
        _all_finite(
            data,
            {
                ComponentType.sym_power_sensor: ["power_sigma"],
                ComponentType.asym_power_sensor: ["power_sigma"],
                ComponentType.sym_voltage_sensor: ["u_sigma"],
                ComponentType.asym_voltage_sensor: ["u_sigma"],
            },
        )
    )

    component_validators = {
        "node": validate_node,
        "line": validate_line,
        "link": lambda d: validate_branch(d, ComponentType.link),
        "generic_branch": validate_generic_branch,
        "transformer": validate_transformer,
        "three_winding_transformer": validate_three_winding_transformer,
        "source": validate_source,
        "sym_load": lambda d: validate_generic_load_gen(d, ComponentType.sym_load),
        "sym_gen": lambda d: validate_generic_load_gen(d, ComponentType.sym_gen),
        "asym_load": lambda d: validate_generic_load_gen(d, ComponentType.asym_load),
        "asym_gen": lambda d: validate_generic_load_gen(d, ComponentType.asym_gen),
        "shunt": validate_shunt,
    }

    for component, validator in component_validators.items():
        if component in data:
            errors += validator(data)

    if calculation_type in (None, CalculationType.state_estimation):
        if "sym_voltage_sensor" in data:
            errors += validate_generic_voltage_sensor(data, ComponentType.sym_voltage_sensor)
        if "asym_voltage_sensor" in data:
            errors += validate_generic_voltage_sensor(data, ComponentType.asym_voltage_sensor)
        if "sym_power_sensor" in data:
            errors += validate_generic_power_sensor(data, ComponentType.sym_power_sensor)
        if "asym_power_sensor" in data:
            errors += validate_generic_power_sensor(data, ComponentType.asym_power_sensor)

    if calculation_type in (None, CalculationType.short_circuit) and "fault" in data:
        errors += validate_fault(data)

    if calculation_type in (None, CalculationType.power_flow) and "transformer_tap_regulator" in data:
        errors += validate_transformer_tap_regulator(data)

    return errors


# pylint: disable=missing-function-docstring


def validate_base(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors: list[ValidationError] = list(_all_unique(data, component, "id"))
    return errors


def validate_node(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, ComponentType.node)
    errors += _all_greater_than_zero(data, ComponentType.node, "u_rated")
    return errors


def validate_branch(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(data, component, "from_node", ComponentType.node)
    errors += _all_valid_ids(data, component, "to_node", ComponentType.node)
    errors += _all_not_two_values_equal(data, component, "to_node", "from_node")
    errors += _all_boolean(data, component, "from_status")
    errors += _all_boolean(data, component, "to_status")
    return errors


def validate_line(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, ComponentType.line)
    errors += _all_not_two_values_zero(data, ComponentType.line, "r1", "x1")
    errors += _all_not_two_values_zero(data, ComponentType.line, "r0", "x0")
    errors += _all_greater_than_zero(data, ComponentType.line, "i_n")
    return errors


def validate_generic_branch(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, ComponentType.generic_branch)
    errors += _all_greater_than_zero(data, ComponentType.generic_branch, "k")
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.generic_branch, "sn")
    return errors


def validate_transformer(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, ComponentType.transformer)
    errors += _all_greater_than_zero(data, ComponentType.transformer, "u1")
    errors += _all_greater_than_zero(data, ComponentType.transformer, "u2")
    errors += _all_greater_than_zero(data, ComponentType.transformer, "sn")
    errors += _all_greater_or_equal(data, ComponentType.transformer, "uk", "pk/sn")
    errors += _all_between(data, ComponentType.transformer, "uk", 0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.transformer, "pk")
    errors += _all_greater_or_equal(data, ComponentType.transformer, "i0", "p0/sn")
    errors += _all_less_than(data, ComponentType.transformer, "i0", 1)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.transformer, "p0")
    errors += _all_valid_enum_values(data, ComponentType.transformer, "winding_from", WindingType)
    errors += _all_valid_enum_values(data, ComponentType.transformer, "winding_to", WindingType)
    errors += _all_between_or_at(data, ComponentType.transformer, "clock", 0, 12)
    errors += _all_valid_clocks(data, ComponentType.transformer, "clock", "winding_from", "winding_to")
    errors += _all_valid_enum_values(data, ComponentType.transformer, "tap_side", BranchSide)
    errors += _all_between_or_at(
        data, ComponentType.transformer, "tap_pos", "tap_min", "tap_max", data[ComponentType.transformer]["tap_nom"], 0
    )
    errors += _all_between_or_at(data, ComponentType.transformer, "tap_nom", "tap_min", "tap_max", 0)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.transformer, "tap_size")
    errors += _all_greater_or_equal(
        data, ComponentType.transformer, "uk_min", "pk_min/sn", data[ComponentType.transformer]["uk"]
    )
    errors += _all_between(data, ComponentType.transformer, "uk_min", 0, 1, data[ComponentType.transformer]["uk"])
    errors += _all_greater_or_equal(
        data, ComponentType.transformer, "uk_max", "pk_max/sn", data[ComponentType.transformer]["uk"]
    )
    errors += _all_between(data, ComponentType.transformer, "uk_max", 0, 1, data[ComponentType.transformer]["uk"])
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.transformer, "pk_min", data[ComponentType.transformer]["pk"]
    )
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.transformer, "pk_max", data[ComponentType.transformer]["pk"]
    )
    return errors


def validate_branch3(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(data, component, "node_1", ComponentType.node)
    errors += _all_valid_ids(data, component, "node_2", ComponentType.node)
    errors += _all_valid_ids(data, component, "node_3", ComponentType.node)
    errors += _all_not_two_values_equal(data, component, "node_1", "node_2")
    errors += _all_not_two_values_equal(data, component, "node_1", "node_3")
    errors += _all_not_two_values_equal(data, component, "node_2", "node_3")
    errors += _all_boolean(data, component, "status_1")
    errors += _all_boolean(data, component, "status_2")
    errors += _all_boolean(data, component, "status_3")
    return errors


# pylint: disable=R0915
def validate_three_winding_transformer(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch3(data, ComponentType.three_winding_transformer)
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, "u1")
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, "u2")
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, "u3")
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, "sn_1")
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, "sn_2")
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, "sn_3")
    errors += _all_greater_or_equal(data, ComponentType.three_winding_transformer, "uk_12", "pk_12/sn_1")
    errors += _all_greater_or_equal(data, ComponentType.three_winding_transformer, "uk_12", "pk_12/sn_2")
    errors += _all_greater_or_equal(data, ComponentType.three_winding_transformer, "uk_13", "pk_13/sn_1")
    errors += _all_greater_or_equal(data, ComponentType.three_winding_transformer, "uk_13", "pk_13/sn_3")
    errors += _all_greater_or_equal(data, ComponentType.three_winding_transformer, "uk_23", "pk_23/sn_2")
    errors += _all_greater_or_equal(data, ComponentType.three_winding_transformer, "uk_23", "pk_23/sn_3")
    errors += _all_between(data, ComponentType.three_winding_transformer, "uk_12", 0, 1)
    errors += _all_between(data, ComponentType.three_winding_transformer, "uk_13", 0, 1)
    errors += _all_between(data, ComponentType.three_winding_transformer, "uk_23", 0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.three_winding_transformer, "pk_12")
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.three_winding_transformer, "pk_13")
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.three_winding_transformer, "pk_23")
    errors += _all_greater_or_equal(data, ComponentType.three_winding_transformer, "i0", "p0/sn_1")
    errors += _all_less_than(data, ComponentType.three_winding_transformer, "i0", 1)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.three_winding_transformer, "p0")
    errors += _all_valid_enum_values(data, ComponentType.three_winding_transformer, "winding_1", WindingType)
    errors += _all_valid_enum_values(data, ComponentType.three_winding_transformer, "winding_2", WindingType)
    errors += _all_valid_enum_values(data, ComponentType.three_winding_transformer, "winding_3", WindingType)
    errors += _all_between_or_at(data, ComponentType.three_winding_transformer, "clock_12", 0, 12)
    errors += _all_between_or_at(data, ComponentType.three_winding_transformer, "clock_13", 0, 12)
    errors += _all_valid_clocks(data, ComponentType.three_winding_transformer, "clock_12", "winding_1", "winding_2")
    errors += _all_valid_clocks(data, ComponentType.three_winding_transformer, "clock_13", "winding_1", "winding_3")
    errors += _all_valid_enum_values(data, ComponentType.three_winding_transformer, "tap_side", Branch3Side)
    errors += _all_between_or_at(
        data,
        ComponentType.three_winding_transformer,
        "tap_pos",
        "tap_min",
        "tap_max",
        data[ComponentType.three_winding_transformer]["tap_nom"],
        0,
    )
    errors += _all_between_or_at(data, ComponentType.three_winding_transformer, "tap_nom", "tap_min", "tap_max", 0)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.three_winding_transformer, "tap_size")
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_12_min",
        "pk_12_min/sn_1",
        data[ComponentType.three_winding_transformer]["uk_12"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_12_min",
        "pk_12_min/sn_2",
        data[ComponentType.three_winding_transformer]["uk_12"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_13_min",
        "pk_13_min/sn_1",
        data[ComponentType.three_winding_transformer]["uk_13"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_13_min",
        "pk_13_min/sn_3",
        data[ComponentType.three_winding_transformer]["uk_13"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_23_min",
        "pk_23_min/sn_2",
        data[ComponentType.three_winding_transformer]["uk_23"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_23_min",
        "pk_23_min/sn_3",
        data[ComponentType.three_winding_transformer]["uk_23"],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        "uk_12_min",
        0,
        1,
        data[ComponentType.three_winding_transformer]["uk_12"],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        "uk_13_min",
        0,
        1,
        data[ComponentType.three_winding_transformer]["uk_13"],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        "uk_23_min",
        0,
        1,
        data[ComponentType.three_winding_transformer]["uk_23"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_12_max",
        "pk_12_max/sn_1",
        data[ComponentType.three_winding_transformer]["uk_12"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_12_max",
        "pk_12_max/sn_2",
        data[ComponentType.three_winding_transformer]["uk_12"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_13_max",
        "pk_13_max/sn_1",
        data[ComponentType.three_winding_transformer]["uk_13"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_13_max",
        "pk_13_max/sn_3",
        data[ComponentType.three_winding_transformer]["uk_13"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_23_max",
        "pk_23_max/sn_2",
        data[ComponentType.three_winding_transformer]["uk_23"],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        "uk_23_max",
        "pk_23_max/sn_3",
        data[ComponentType.three_winding_transformer]["uk_23"],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        "uk_12_max",
        0,
        1,
        data[ComponentType.three_winding_transformer]["uk_12"],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        "uk_13_max",
        0,
        1,
        data[ComponentType.three_winding_transformer]["uk_13"],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        "uk_23_max",
        0,
        1,
        data[ComponentType.three_winding_transformer]["uk_23"],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        "pk_12_min",
        data[ComponentType.three_winding_transformer]["pk_12"],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        "pk_13_min",
        data[ComponentType.three_winding_transformer]["pk_13"],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        "pk_23_min",
        data[ComponentType.three_winding_transformer]["pk_23"],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        "pk_12_max",
        data[ComponentType.three_winding_transformer]["pk_12"],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        "pk_13_max",
        data[ComponentType.three_winding_transformer]["pk_13"],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        "pk_23_max",
        data[ComponentType.three_winding_transformer]["pk_23"],
    )
    return errors


def validate_appliance(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_boolean(data, component, "status")
    errors += _all_valid_ids(data, component, "node", ComponentType.node)
    return errors


def validate_source(data: SingleDataset) -> list[ValidationError]:
    errors = validate_appliance(data, ComponentType.source)
    errors += _all_greater_than_zero(data, ComponentType.source, "u_ref")
    errors += _all_greater_than_zero(data, ComponentType.source, "sk")
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.source, "rx_ratio")
    errors += _all_greater_than_zero(data, ComponentType.source, "z01_ratio")
    return errors


def validate_generic_load_gen(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_appliance(data, component)
    errors += _all_valid_enum_values(data, component, "type", LoadGenType)
    return errors


def validate_shunt(data: SingleDataset) -> list[ValidationError]:
    errors = validate_appliance(data, ComponentType.shunt)
    return errors


def validate_generic_voltage_sensor(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, "u_sigma")
    errors += _all_greater_than_zero(data, component, "u_measured")
    errors += _all_valid_ids(data, component, "measured_object", ComponentType.node)
    return errors


def validate_generic_power_sensor(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, "power_sigma")
    errors += _all_valid_enum_values(data, component, "measured_terminal_type", MeasuredTerminalType)
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[
            ComponentType.node,
            ComponentType.line,
            ComponentType.generic_branch,
            ComponentType.transformer,
            ComponentType.three_winding_transformer,
            ComponentType.source,
            ComponentType.shunt,
            ComponentType.sym_load,
            ComponentType.asym_load,
            ComponentType.sym_gen,
            ComponentType.asym_gen,
        ],
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[ComponentType.line, ComponentType.generic_branch, ComponentType.transformer],
        measured_terminal_type=MeasuredTerminalType.branch_from,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[ComponentType.line, ComponentType.generic_branch, ComponentType.transformer],
        measured_terminal_type=MeasuredTerminalType.branch_to,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=ComponentType.source,
        measured_terminal_type=MeasuredTerminalType.source,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=ComponentType.shunt,
        measured_terminal_type=MeasuredTerminalType.shunt,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[ComponentType.sym_load, ComponentType.asym_load],
        measured_terminal_type=MeasuredTerminalType.load,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[ComponentType.sym_gen, ComponentType.asym_gen],
        measured_terminal_type=MeasuredTerminalType.generator,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_1,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_2,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_3,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=ComponentType.node,
        measured_terminal_type=MeasuredTerminalType.node,
    )
    if component in ("sym_power_sensor", "asym_power_sensor"):
        errors += _valid_p_q_sigma(data, component)

    return errors


def validate_fault(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, ComponentType.fault)
    errors += _all_boolean(data, ComponentType.fault, "status")
    errors += _all_valid_enum_values(data, ComponentType.fault, "fault_type", FaultType)
    errors += _all_valid_enum_values(data, ComponentType.fault, "fault_phase", FaultPhase)
    errors += _all_valid_fault_phases(data, ComponentType.fault, "fault_type", "fault_phase")
    errors += _all_valid_ids(data, ComponentType.fault, field="fault_object", ref_components=ComponentType.node)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.fault, "r_f")
    errors += _all_enabled_identical(data, ComponentType.fault, "fault_type", "status")
    errors += _all_enabled_identical(data, ComponentType.fault, "fault_phase", "status")
    return errors


def validate_regulator(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(
        data,
        component,
        field="regulated_object",
        ref_components=[ComponentType.transformer, ComponentType.three_winding_transformer],
    )
    return errors


def validate_transformer_tap_regulator(data: SingleDataset) -> list[ValidationError]:
    errors = validate_regulator(data, ComponentType.transformer_tap_regulator)
    errors += _all_boolean(data, ComponentType.transformer_tap_regulator, "status")
    errors += _all_valid_enum_values(
        data, ComponentType.transformer_tap_regulator, "control_side", [BranchSide, Branch3Side]
    )
    errors += _all_valid_associated_enum_values(
        data,
        ComponentType.transformer_tap_regulator,
        "control_side",
        "regulated_object",
        [ComponentType.transformer],
        [BranchSide],
    )
    errors += _all_valid_associated_enum_values(
        data,
        ComponentType.transformer_tap_regulator,
        "control_side",
        "regulated_object",
        [ComponentType.three_winding_transformer],
        [Branch3Side],
    )
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.transformer_tap_regulator, "u_set")
    errors += _all_greater_than_zero(data, ComponentType.transformer_tap_regulator, "u_band")
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.transformer_tap_regulator, "line_drop_compensation_r", 0.0
    )
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.transformer_tap_regulator, "line_drop_compensation_x", 0.0
    )
    errors += _all_supported_tap_control_side(
        data,
        ComponentType.transformer_tap_regulator,
        "control_side",
        "regulated_object",
        [(ComponentType.transformer, "tap_side"), (ComponentType.three_winding_transformer, "tap_side")],
    )
    return errors
