# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power Grid Model Validation Functions.

Although all functions are 'public', you probably only need validate_input_data() and validate_batch_data().

"""

import copy
from collections.abc import Sized as ABCSized
from itertools import chain
from typing import Literal

import numpy as np

from power_grid_model._core.dataset_definitions import ComponentType, DatasetType, _map_to_component_types
from power_grid_model._core.power_grid_meta import power_grid_meta_data
from power_grid_model._core.utils import (
    compatibility_convert_row_columnar_dataset as _compatibility_convert_row_columnar_dataset,
    convert_batch_dataset_to_batch_list as _convert_batch_dataset_to_batch_list,
)
from power_grid_model.data_types import BatchDataset, Dataset, SingleDataset
from power_grid_model.enum import (
    AngleMeasurementType,
    Branch3Side,
    BranchSide,
    CalculationType,
    FaultPhase,
    FaultType,
    LoadGenType,
    MeasuredTerminalType,
    WindingType,
)
from power_grid_model.validation._rules import (
    all_between as _all_between,
    all_between_or_at as _all_between_or_at,
    all_boolean as _all_boolean,
    all_cross_unique as _all_cross_unique,
    all_enabled_identical as _all_enabled_identical,
    all_finite as _all_finite,
    all_greater_or_equal as _all_greater_or_equal,
    all_greater_than_or_equal_to_zero as _all_greater_than_or_equal_to_zero,
    all_greater_than_zero as _all_greater_than_zero,
    all_in_valid_values as _all_in_valid_values,
    all_less_than as _all_less_than,
    all_not_two_values_equal as _all_not_two_values_equal,
    all_not_two_values_zero as _all_not_two_values_zero,
    all_same_current_angle_measurement_type_on_terminal as _all_same_current_angle_measurement_type_on_terminal,
    all_same_sensor_type_on_same_terminal as _all_same_sensor_type_on_same_terminal,
    all_unique as _all_unique,
    all_valid_associated_enum_values as _all_valid_associated_enum_values,
    all_valid_clocks as _all_valid_clocks,
    all_valid_enum_values as _all_valid_enum_values,
    all_valid_fault_phases as _all_valid_fault_phases,
    all_valid_ids as _all_valid_ids,
    any_voltage_angle_measurement_if_global_current_measurement as _any_voltage_angle_measurement_if_global_current_measurement,  # noqa: E501
    ids_valid_in_update_data_set as _ids_valid_in_update_data_set,
    no_strict_subset_missing as _no_strict_subset_missing,
    none_missing as _none_missing,
    not_all_missing as _not_all_missing,
    valid_p_q_sigma as _valid_p_q_sigma,
)
from power_grid_model.validation.errors import (
    IdNotInDatasetError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    ValidationError,
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
    input_data = _map_to_component_types(input_data)

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
    input_data = _map_to_component_types(input_data)
    update_data = _map_to_component_types(update_data)

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
            raise KeyError(f"Unknown component '{component}' in {data_type} data.")

        # Check if component definition is as expected
        dtype = component_dtype[component]
        if isinstance(array, np.ndarray):
            if array.dtype != dtype:
                if not hasattr(array.dtype, "names") or not array.dtype.names:
                    raise TypeError(
                        f"Unexpected Numpy array ({array.dtype}) for '{component}' {data_type} data "
                        "(should be a Numpy structured array)."
                    )
                raise TypeError(
                    f"Unexpected Numpy structured array; (expected = {dtype}, actual = {array.dtype}). "
                    f"For component '{component}'."
                )
        else:
            raise TypeError(
                f"Unexpected data type {type(array).__name__} for '{component}' {data_type} data "
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
        _ids_valid_in_update_data_set(update_data, input_data, component, DatasetType.update)
        for component in update_data
    )
    return list(chain(*errors))


def validate_required_values(  # noqa: PLR0915
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
    required: dict[ComponentType | str, list[str]] = {"base": ["id"]}

    # Nodes
    required[ComponentType.node] = required["base"] + ["u_rated"]

    # Branches
    required["branch"] = required["base"] + ["from_node", "to_node", "from_status", "to_status"]
    required[ComponentType.link] = required["branch"].copy()
    required[ComponentType.line] = required["branch"] + ["r1", "x1", "c1", "tan1"]
    required[ComponentType.asym_line] = required["branch"] + [
        "r_aa",
        "r_ba",
        "r_bb",
        "r_ca",
        "r_cb",
        "r_cc",
        "x_aa",
        "x_ba",
        "x_bb",
        "x_ca",
        "x_cb",
        "x_cc",
    ]
    required[ComponentType.transformer] = required["branch"] + [
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
    required[ComponentType.three_winding_transformer] = required["branch3"] + [
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
    required[ComponentType.transformer_tap_regulator] = required["regulator"]
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[ComponentType.transformer_tap_regulator] += ["control_side", "u_set", "u_band"]

    # Appliances
    required["appliance"] = required["base"] + ["node", "status"]
    required[ComponentType.source] = required["appliance"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[ComponentType.source] += ["u_ref"]
    required[ComponentType.shunt] = required["appliance"] + ["g1", "b1"]
    required["generic_load_gen"] = required["appliance"] + ["type"]
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required["generic_load_gen"] += ["p_specified", "q_specified"]
    required[ComponentType.sym_load] = required["generic_load_gen"].copy()
    required[ComponentType.asym_load] = required["generic_load_gen"].copy()
    required[ComponentType.sym_gen] = required["generic_load_gen"].copy()
    required[ComponentType.asym_gen] = required["generic_load_gen"].copy()

    # Sensors
    required["sensor"] = required["base"] + ["measured_object"]
    required["voltage_sensor"] = required["sensor"].copy()
    required["power_sensor"] = required["sensor"] + ["measured_terminal_type"]
    required["current_sensor"] = required["sensor"] + ["measured_terminal_type", "angle_measurement_type"]
    if calculation_type is None or calculation_type == CalculationType.state_estimation:
        required["voltage_sensor"] += ["u_sigma", "u_measured"]
        required["power_sensor"] += ["p_measured", "q_measured"]  # power_sigma, p_sigma and q_sigma are checked later
        required["current_sensor"] += ["i_sigma", "i_angle_sigma", "i_measured", "i_angle_measured"]
    required[ComponentType.sym_voltage_sensor] = required["voltage_sensor"].copy()
    required[ComponentType.asym_voltage_sensor] = required["voltage_sensor"].copy()
    required[ComponentType.sym_current_sensor] = required["current_sensor"].copy()
    required[ComponentType.asym_current_sensor] = required["current_sensor"].copy()

    # Different requirements for individual sensors. Avoid shallow copy.
    for sensor_type in (ComponentType.sym_power_sensor, ComponentType.asym_power_sensor):
        required[sensor_type] = required["power_sensor"].copy()

    # Faults
    required[ComponentType.fault] = required["base"] + ["fault_object"]
    asym_sc = False
    if calculation_type is None or calculation_type == CalculationType.short_circuit:
        required[ComponentType.fault] += ["status", "fault_type"]
        if ComponentType.fault in data:
            for elem in data[ComponentType.fault]["fault_type"]:
                if elem not in (FaultType.three_phase, FaultType.nan):
                    asym_sc = True
                    break

    if not symmetric or asym_sc:
        required[ComponentType.line] += ["r0", "x0", "c0", "tan0"]
        required[ComponentType.shunt] += ["g0", "b0"]

    errors = _validate_required_in_data(data, required)

    if calculation_type is None or calculation_type == CalculationType.state_estimation:
        errors += _validate_required_power_sigma_or_p_q_sigma(data, ComponentType.sym_power_sensor)
        errors += _validate_required_power_sigma_or_p_q_sigma(data, ComponentType.asym_power_sensor)

    return errors


def _validate_required_in_data(data: SingleDataset, required: dict[ComponentType | str, list[str]]):
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

    results: list[MissingValueError] = []

    for component in data:
        if is_valid_component(data, component):
            items = required.get(component, [])
            results += _none_missing(data, component, items)

    return results


def _validate_required_power_sigma_or_p_q_sigma(
    data: SingleDataset,
    power_sensor: Literal[ComponentType.sym_power_sensor, ComponentType.asym_power_sensor],
) -> list[MissingValueError]:
    """
    Check that either `p_sigma` and `q_sigma` are all provided, or that `power_sigma` is provided.

    Args:
        data: SingleDataset, pgm data
        sensor: the power sensor type, either ComponentType.sym_power_sensor or ComponentType.asym_power_sensor
    """
    result: list[MissingValueError] = []

    if power_sensor in data:
        sensor_data = data[power_sensor]
        p_sigma = sensor_data["p_sigma"]
        q_sigma = sensor_data["q_sigma"]

        asym_axes = tuple(range(sensor_data.ndim, p_sigma.ndim))
        all_pq_sigma_missing_mask = np.all(np.isnan(p_sigma), axis=asym_axes) & np.all(
            np.isnan(q_sigma), axis=asym_axes
        )

        result += _validate_required_in_data(
            {power_sensor: sensor_data[all_pq_sigma_missing_mask]}, required={power_sensor: ["power_sigma"]}
        )
        result += _validate_required_in_data(
            {power_sensor: sensor_data[~all_pq_sigma_missing_mask]}, required={power_sensor: ["p_sigma", "q_sigma"]}
        )

    return result


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
            data=data,
            exceptions={
                ComponentType.sym_power_sensor: ["power_sigma", "p_sigma", "q_sigma"],
                ComponentType.asym_power_sensor: ["power_sigma", "p_sigma", "q_sigma"],
                ComponentType.sym_voltage_sensor: ["u_sigma"],
                ComponentType.asym_voltage_sensor: ["u_sigma"],
                ComponentType.sym_current_sensor: ["i_sigma", "i_angle_sigma"],
                ComponentType.asym_current_sensor: ["i_sigma", "i_angle_sigma"],
            },
        )
    )

    component_validators = {
        ComponentType.node: validate_node,
        ComponentType.line: validate_line,
        ComponentType.asym_line: validate_asym_line,
        ComponentType.link: lambda d: validate_branch(d, ComponentType.link),
        ComponentType.generic_branch: validate_generic_branch,
        ComponentType.transformer: validate_transformer,
        ComponentType.three_winding_transformer: validate_three_winding_transformer,
        ComponentType.source: validate_source,
        ComponentType.sym_load: lambda d: validate_generic_load_gen(d, ComponentType.sym_load),
        ComponentType.sym_gen: lambda d: validate_generic_load_gen(d, ComponentType.sym_gen),
        ComponentType.asym_load: lambda d: validate_generic_load_gen(d, ComponentType.asym_load),
        ComponentType.asym_gen: lambda d: validate_generic_load_gen(d, ComponentType.asym_gen),
        ComponentType.shunt: validate_shunt,
    }

    for component, validator in component_validators.items():
        if component in data:
            errors += validator(data)

    if calculation_type in (None, CalculationType.state_estimation):
        if ComponentType.sym_voltage_sensor in data:
            errors += validate_generic_voltage_sensor(data, ComponentType.sym_voltage_sensor)
        if ComponentType.asym_voltage_sensor in data:
            errors += validate_generic_voltage_sensor(data, ComponentType.asym_voltage_sensor)
        if ComponentType.sym_power_sensor in data:
            errors += validate_generic_power_sensor(data, ComponentType.sym_power_sensor)
        if ComponentType.asym_power_sensor in data:
            errors += validate_generic_power_sensor(data, ComponentType.asym_power_sensor)
        if ComponentType.sym_current_sensor in data:
            errors += validate_generic_current_sensor(data, ComponentType.sym_current_sensor)
        if ComponentType.asym_current_sensor in data:
            errors += validate_generic_current_sensor(data, ComponentType.asym_current_sensor)

        errors += validate_no_mixed_sensors_on_same_terminal(data)

    if calculation_type in (None, CalculationType.short_circuit) and ComponentType.fault in data:
        errors += validate_fault(data)

    if calculation_type in (None, CalculationType.power_flow) and ComponentType.transformer_tap_regulator in data:
        errors += validate_transformer_tap_regulator(data)

    return errors


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


def validate_asym_line(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, ComponentType.asym_line)
    errors += _all_greater_than_zero(data, ComponentType.asym_line, "i_n")
    required_fields = ["r_aa", "r_ba", "r_bb", "r_ca", "r_cb", "r_cc", "x_aa", "x_ba", "x_bb", "x_ca", "x_cb", "x_cc"]
    optional_r_matrix_fields = ["r_na", "r_nb", "r_nc", "r_nn"]
    optional_x_matrix_fields = ["x_na", "x_nb", "x_nc", "x_nn"]
    required_c_matrix_fields = ["c_aa", "c_ba", "c_bb", "c_ca", "c_cb", "c_cc"]
    c_fields = ["c0", "c1"]
    for field in (
        required_fields + optional_r_matrix_fields + optional_x_matrix_fields + required_c_matrix_fields + c_fields
    ):
        errors += _all_greater_than_zero(data, ComponentType.asym_line, field)

    errors += _no_strict_subset_missing(
        data, optional_r_matrix_fields + optional_x_matrix_fields, ComponentType.asym_line
    )
    errors += _no_strict_subset_missing(data, required_c_matrix_fields, ComponentType.asym_line)
    errors += _no_strict_subset_missing(data, c_fields, ComponentType.asym_line)
    errors += _not_all_missing(data, required_c_matrix_fields + c_fields, ComponentType.asym_line)

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
    errors += _all_between_or_at(data, ComponentType.transformer, "clock", -12, 12)
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


def validate_three_winding_transformer(data: SingleDataset) -> list[ValidationError]:  # noqa: PLR0915
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
            ComponentType.asym_line,
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
        ref_components=[
            ComponentType.line,
            ComponentType.asym_line,
            ComponentType.generic_branch,
            ComponentType.transformer,
        ],
        measured_terminal_type=MeasuredTerminalType.branch_from,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[
            ComponentType.line,
            ComponentType.asym_line,
            ComponentType.generic_branch,
            ComponentType.transformer,
        ],
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
    if component in (ComponentType.sym_power_sensor, ComponentType.asym_power_sensor):
        errors += _valid_p_q_sigma(data, component)

    return errors


def validate_generic_current_sensor(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, "i_sigma")
    errors += _all_greater_than_zero(data, component, "i_angle_sigma")
    errors += _all_valid_enum_values(data, component, "measured_terminal_type", MeasuredTerminalType)
    errors += _all_valid_enum_values(data, component, "angle_measurement_type", AngleMeasurementType)
    errors += _all_in_valid_values(
        data,
        component,
        "measured_terminal_type",
        [
            MeasuredTerminalType.branch_from,
            MeasuredTerminalType.branch_to,
            MeasuredTerminalType.branch3_1,
            MeasuredTerminalType.branch3_2,
            MeasuredTerminalType.branch3_3,
        ],
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[
            ComponentType.line,
            ComponentType.asym_line,
            ComponentType.generic_branch,
            ComponentType.transformer,
            ComponentType.three_winding_transformer,
        ],
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[
            ComponentType.line,
            ComponentType.asym_line,
            ComponentType.generic_branch,
            ComponentType.transformer,
        ],
        measured_terminal_type=MeasuredTerminalType.branch_from,
    )
    errors += _all_valid_ids(
        data,
        component,
        field="measured_object",
        ref_components=[
            ComponentType.line,
            ComponentType.asym_line,
            ComponentType.generic_branch,
            ComponentType.transformer,
        ],
        measured_terminal_type=MeasuredTerminalType.branch_to,
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
    errors += _all_same_current_angle_measurement_type_on_terminal(
        data,
        component,
        measured_object_field="measured_object",
        measured_terminal_type_field="measured_terminal_type",
        angle_measurement_type_field="angle_measurement_type",
    )
    errors += _any_voltage_angle_measurement_if_global_current_measurement(
        data,
        component,
        angle_measurement_type_filter=("angle_measurement_type", AngleMeasurementType.global_angle),
        voltage_sensor_u_angle_measured={
            ComponentType.sym_voltage_sensor: "u_angle_measured",
            ComponentType.asym_voltage_sensor: "u_angle_measured",
        },
    )

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
    errors += _all_unique(data, ComponentType.transformer_tap_regulator, "regulated_object")
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
    return errors


def validate_no_mixed_sensors_on_same_terminal(data: SingleDataset) -> list[ValidationError]:
    errors: list[ValidationError] = []

    for power_sensor in [ComponentType.sym_power_sensor, ComponentType.asym_power_sensor]:
        for current_sensor in [ComponentType.sym_current_sensor, ComponentType.asym_current_sensor]:
            if power_sensor in data and current_sensor in data:
                errors += _all_same_sensor_type_on_same_terminal(
                    data,
                    power_sensor_type=power_sensor,
                    current_sensor_type=current_sensor,
                    measured_object_field="measured_object",
                    measured_terminal_type_field="measured_terminal_type",
                )

    return errors
