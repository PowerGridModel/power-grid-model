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

from power_grid_model._core.dataset_definitions import (
    ComponentAttribute,
    ComponentType,
    ComponentTypeLike,
    DatasetType,
    _map_to_component_types,
)
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
    InvalidVoltageRegulationError,
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
    return errors or None


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

    return errors or None


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
    return _all_cross_unique(data, [(component, ComponentAttribute.id) for component in data])


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
    required: dict[ComponentTypeLike, list[ComponentAttribute]] = {"base": [ComponentAttribute.id]}

    # Nodes
    required[ComponentType.node] = required["base"] + [ComponentAttribute.u_rated]

    # Branches
    required["branch"] = required["base"] + [
        ComponentAttribute.from_node,
        ComponentAttribute.to_node,
        ComponentAttribute.from_status,
        ComponentAttribute.to_status,
    ]
    required[ComponentType.link] = required["branch"].copy()
    required[ComponentType.line] = required["branch"] + [
        ComponentAttribute.r1,
        ComponentAttribute.x1,
        ComponentAttribute.c1,
        ComponentAttribute.tan1,
    ]
    required[ComponentType.asym_line] = required["branch"] + [
        ComponentAttribute.r_aa,
        ComponentAttribute.r_ba,
        ComponentAttribute.r_bb,
        ComponentAttribute.r_ca,
        ComponentAttribute.r_cb,
        ComponentAttribute.r_cc,
        ComponentAttribute.x_aa,
        ComponentAttribute.x_ba,
        ComponentAttribute.x_bb,
        ComponentAttribute.x_ca,
        ComponentAttribute.x_cb,
        ComponentAttribute.x_cc,
    ]
    required[ComponentType.transformer] = required["branch"] + [
        ComponentAttribute.u1,
        ComponentAttribute.u2,
        ComponentAttribute.sn,
        ComponentAttribute.uk,
        ComponentAttribute.pk,
        ComponentAttribute.i0,
        ComponentAttribute.p0,
        ComponentAttribute.winding_from,
        ComponentAttribute.winding_to,
        ComponentAttribute.clock,
        ComponentAttribute.tap_side,
        ComponentAttribute.tap_min,
        ComponentAttribute.tap_max,
        ComponentAttribute.tap_size,
    ]
    # Branch3
    required["branch3"] = required["base"] + [
        ComponentAttribute.node_1,
        ComponentAttribute.node_2,
        ComponentAttribute.node_3,
        ComponentAttribute.status_1,
        ComponentAttribute.status_2,
        ComponentAttribute.status_3,
    ]
    required[ComponentType.three_winding_transformer] = required["branch3"] + [
        ComponentAttribute.u1,
        ComponentAttribute.u2,
        ComponentAttribute.u3,
        ComponentAttribute.sn_1,
        ComponentAttribute.sn_2,
        ComponentAttribute.sn_3,
        ComponentAttribute.uk_12,
        ComponentAttribute.uk_13,
        ComponentAttribute.uk_23,
        ComponentAttribute.pk_12,
        ComponentAttribute.pk_13,
        ComponentAttribute.pk_23,
        ComponentAttribute.i0,
        ComponentAttribute.p0,
        ComponentAttribute.winding_1,
        ComponentAttribute.winding_2,
        ComponentAttribute.winding_3,
        ComponentAttribute.clock_12,
        ComponentAttribute.clock_13,
        ComponentAttribute.tap_side,
        ComponentAttribute.tap_min,
        ComponentAttribute.tap_max,
        ComponentAttribute.tap_size,
    ]

    # Regulators
    required["regulator"] = required["base"] + [
        ComponentAttribute.regulated_object,
        ComponentAttribute.status,
    ]
    required[ComponentType.transformer_tap_regulator] = required["regulator"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[ComponentType.transformer_tap_regulator] += [
            ComponentAttribute.control_side,
            ComponentAttribute.u_set,
            ComponentAttribute.u_band,
        ]

    required[ComponentType.voltage_regulator] = required["regulator"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[ComponentType.voltage_regulator] += [ComponentAttribute.u_ref]
        # TODO(scud-soptim): add unit test for optional q_min and q_max when limit handling is implemented

    # Appliances
    required["appliance"] = required["base"] + [
        ComponentAttribute.node,
        ComponentAttribute.status,
    ]
    required[ComponentType.source] = required["appliance"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[ComponentType.source] += [ComponentAttribute.u_ref]
    required[ComponentType.shunt] = required["appliance"] + [
        ComponentAttribute.g1,
        ComponentAttribute.b1,
    ]
    required["generic_load_gen"] = required["appliance"] + [ComponentAttribute.type]
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required["generic_load_gen"] += [
            ComponentAttribute.p_specified,
            ComponentAttribute.q_specified,
        ]
    required[ComponentType.sym_load] = required["generic_load_gen"].copy()
    required[ComponentType.asym_load] = required["generic_load_gen"].copy()
    required[ComponentType.sym_gen] = required["generic_load_gen"].copy()
    required[ComponentType.asym_gen] = required["generic_load_gen"].copy()

    # Sensors
    required["sensor"] = required["base"] + [ComponentAttribute.measured_object]
    required["voltage_sensor"] = required["sensor"].copy()
    required["power_sensor"] = required["sensor"] + [ComponentAttribute.measured_terminal_type]
    required["current_sensor"] = required["sensor"] + [
        ComponentAttribute.measured_terminal_type,
        ComponentAttribute.angle_measurement_type,
    ]
    if calculation_type is None or calculation_type == CalculationType.state_estimation:
        required["voltage_sensor"] += [ComponentAttribute.u_sigma, ComponentAttribute.u_measured]
        required["power_sensor"] += [
            ComponentAttribute.p_measured,
            ComponentAttribute.q_measured,
        ]  # power_sigma, p_sigma and q_sigma are checked later
        required["current_sensor"] += [
            ComponentAttribute.i_sigma,
            ComponentAttribute.i_angle_sigma,
            ComponentAttribute.i_measured,
            ComponentAttribute.i_angle_measured,
        ]
    required[ComponentType.sym_voltage_sensor] = required["voltage_sensor"].copy()
    required[ComponentType.asym_voltage_sensor] = required["voltage_sensor"].copy()
    required[ComponentType.sym_current_sensor] = required["current_sensor"].copy()
    required[ComponentType.asym_current_sensor] = required["current_sensor"].copy()

    # Different requirements for individual sensors. Avoid shallow copy.
    for sensor_type in (ComponentType.sym_power_sensor, ComponentType.asym_power_sensor):
        required[sensor_type] = required["power_sensor"].copy()

    # Faults
    required[ComponentType.fault] = required["base"] + [ComponentAttribute.fault_object]
    asym_sc = False
    if calculation_type is None or calculation_type == CalculationType.short_circuit:
        required[ComponentType.fault] += [ComponentAttribute.status, ComponentAttribute.fault_type]
        if ComponentType.fault in data:
            for elem in data[ComponentType.fault][ComponentAttribute.fault_type]:
                if elem not in (FaultType.three_phase, FaultType.nan):
                    asym_sc = True
                    break

    if not symmetric or asym_sc:
        required[ComponentType.line] += [
            ComponentAttribute.r0,
            ComponentAttribute.x0,
            ComponentAttribute.c0,
            ComponentAttribute.tan0,
        ]
        required[ComponentType.shunt] += [ComponentAttribute.g0, ComponentAttribute.b0]

    errors = _validate_required_in_data(data, required)

    if calculation_type is None or calculation_type == CalculationType.state_estimation:
        errors += _validate_required_power_sigma_or_p_q_sigma(data, ComponentType.sym_power_sensor)
        errors += _validate_required_power_sigma_or_p_q_sigma(data, ComponentType.asym_power_sensor)

    return errors


def _validate_required_in_data(data: SingleDataset, required: dict[ComponentTypeLike, list[ComponentAttribute]]):
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
        p_sigma = sensor_data[ComponentAttribute.p_sigma]
        q_sigma = sensor_data[ComponentAttribute.q_sigma]

        asym_axes = tuple(range(sensor_data.ndim, p_sigma.ndim))
        all_pq_sigma_missing_mask = np.all(np.isnan(p_sigma), axis=asym_axes) & np.all(
            np.isnan(q_sigma), axis=asym_axes
        )

        result += _validate_required_in_data(
            {power_sensor: sensor_data[all_pq_sigma_missing_mask]},
            required={power_sensor: [ComponentAttribute.power_sigma]},
        )
        result += _validate_required_in_data(
            {power_sensor: sensor_data[~all_pq_sigma_missing_mask]},
            required={power_sensor: [ComponentAttribute.p_sigma, ComponentAttribute.q_sigma]},
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
                ComponentType.sym_power_sensor: [
                    ComponentAttribute.power_sigma,
                    ComponentAttribute.p_sigma,
                    ComponentAttribute.q_sigma,
                ],
                ComponentType.asym_power_sensor: [
                    ComponentAttribute.power_sigma,
                    ComponentAttribute.p_sigma,
                    ComponentAttribute.q_sigma,
                ],
                ComponentType.sym_voltage_sensor: [ComponentAttribute.u_sigma],
                ComponentType.asym_voltage_sensor: [ComponentAttribute.u_sigma],
                ComponentType.sym_current_sensor: [
                    ComponentAttribute.i_sigma,
                    ComponentAttribute.i_angle_sigma,
                ],
                ComponentType.asym_current_sensor: [ComponentAttribute.i_sigma, ComponentAttribute.i_angle_sigma],
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
        ComponentType.voltage_regulator: validate_voltage_regulator,
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
    errors: list[ValidationError] = list(_all_unique(data, component, ComponentAttribute.id))
    return errors


def validate_node(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, ComponentType.node)
    errors += _all_greater_than_zero(data, ComponentType.node, ComponentAttribute.u_rated)
    return errors


def validate_branch(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(data, component, ComponentAttribute.from_node, ComponentType.node)
    errors += _all_valid_ids(data, component, ComponentAttribute.to_node, ComponentType.node)
    errors += _all_not_two_values_equal(data, component, ComponentAttribute.to_node, ComponentAttribute.from_node)
    errors += _all_boolean(data, component, ComponentAttribute.from_status)
    errors += _all_boolean(data, component, ComponentAttribute.to_status)
    return errors


def validate_line(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, ComponentType.line)
    errors += _all_not_two_values_zero(data, ComponentType.line, ComponentAttribute.r1, ComponentAttribute.x1)
    errors += _all_not_two_values_zero(data, ComponentType.line, ComponentAttribute.r0, ComponentAttribute.x0)
    errors += _all_greater_than_zero(data, ComponentType.line, ComponentAttribute.i_n)
    return errors


def validate_asym_line(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, ComponentType.asym_line)
    errors += _all_greater_than_zero(data, ComponentType.asym_line, ComponentAttribute.i_n)
    required_fields = [
        ComponentAttribute.r_aa,
        ComponentAttribute.r_ba,
        ComponentAttribute.r_bb,
        ComponentAttribute.r_ca,
        ComponentAttribute.r_cb,
        ComponentAttribute.r_cc,
        ComponentAttribute.x_aa,
        ComponentAttribute.x_ba,
        ComponentAttribute.x_bb,
        ComponentAttribute.x_ca,
        ComponentAttribute.x_cb,
        ComponentAttribute.x_cc,
    ]
    optional_r_matrix_fields = [
        ComponentAttribute.r_na,
        ComponentAttribute.r_nb,
        ComponentAttribute.r_nc,
        ComponentAttribute.r_nn,
    ]
    optional_x_matrix_fields = [
        ComponentAttribute.x_na,
        ComponentAttribute.x_nb,
        ComponentAttribute.x_nc,
        ComponentAttribute.x_nn,
    ]
    required_c_matrix_fields = [
        ComponentAttribute.c_aa,
        ComponentAttribute.c_ba,
        ComponentAttribute.c_bb,
        ComponentAttribute.c_ca,
        ComponentAttribute.c_cb,
        ComponentAttribute.c_cc,
    ]
    c_fields = [ComponentAttribute.c0, ComponentAttribute.c1]
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
    errors += _all_greater_than_zero(data, ComponentType.generic_branch, ComponentAttribute.k)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.generic_branch, ComponentAttribute.sn)
    return errors


def validate_transformer(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, ComponentType.transformer)
    errors += _all_greater_than_zero(data, ComponentType.transformer, ComponentAttribute.u1)
    errors += _all_greater_than_zero(data, ComponentType.transformer, ComponentAttribute.u2)
    errors += _all_greater_than_zero(data, ComponentType.transformer, ComponentAttribute.sn)
    errors += _all_greater_or_equal(
        data, ComponentType.transformer, ComponentAttribute.uk, f"{ComponentAttribute.pk}/{ComponentAttribute.sn}"
    )
    errors += _all_between(data, ComponentType.transformer, ComponentAttribute.uk, 0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.transformer, ComponentAttribute.pk)
    errors += _all_greater_or_equal(
        data, ComponentType.transformer, ComponentAttribute.i0, f"{ComponentAttribute.p0}/{ComponentAttribute.sn}"
    )
    errors += _all_less_than(data, ComponentType.transformer, ComponentAttribute.i0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.transformer, ComponentAttribute.p0)
    errors += _all_valid_enum_values(data, ComponentType.transformer, ComponentAttribute.winding_from, WindingType)
    errors += _all_valid_enum_values(data, ComponentType.transformer, ComponentAttribute.winding_to, WindingType)
    errors += _all_between_or_at(data, ComponentType.transformer, ComponentAttribute.clock, -12, 12)
    errors += _all_valid_clocks(
        data,
        ComponentType.transformer,
        ComponentAttribute.clock,
        ComponentAttribute.winding_from,
        ComponentAttribute.winding_to,
    )
    errors += _all_valid_enum_values(data, ComponentType.transformer, ComponentAttribute.tap_side, BranchSide)
    errors += _all_between_or_at(
        data,
        ComponentType.transformer,
        ComponentAttribute.tap_pos,
        ComponentAttribute.tap_min,
        ComponentAttribute.tap_max,
        data[ComponentType.transformer][ComponentAttribute.tap_nom],
        0,
    )
    errors += _all_between_or_at(
        data,
        ComponentType.transformer,
        ComponentAttribute.tap_nom,
        ComponentAttribute.tap_min,
        ComponentAttribute.tap_max,
        0,
    )
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.transformer, ComponentAttribute.tap_size)
    errors += _all_greater_or_equal(
        data,
        ComponentType.transformer,
        ComponentAttribute.uk_min,
        f"{ComponentAttribute.pk_min}/{ComponentAttribute.sn}",
        data[ComponentType.transformer][ComponentAttribute.uk],
    )
    errors += _all_between(
        data,
        ComponentType.transformer,
        ComponentAttribute.uk_min,
        0,
        1,
        data[ComponentType.transformer][ComponentAttribute.uk],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.transformer,
        ComponentAttribute.uk_max,
        f"{ComponentAttribute.pk_max}/{ComponentAttribute.sn}",
        data[ComponentType.transformer][ComponentAttribute.uk],
    )
    errors += _all_between(
        data,
        ComponentType.transformer,
        ComponentAttribute.uk_max,
        0,
        1,
        data[ComponentType.transformer][ComponentAttribute.uk],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.transformer,
        ComponentAttribute.pk_min,
        data[ComponentType.transformer][ComponentAttribute.pk],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.transformer,
        ComponentAttribute.pk_max,
        data[ComponentType.transformer][ComponentAttribute.pk],
    )
    return errors


def validate_branch3(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(data, component, ComponentAttribute.node_1, ComponentType.node)
    errors += _all_valid_ids(data, component, ComponentAttribute.node_2, ComponentType.node)
    errors += _all_valid_ids(data, component, ComponentAttribute.node_3, ComponentType.node)
    errors += _all_not_two_values_equal(data, component, ComponentAttribute.node_1, ComponentAttribute.node_2)
    errors += _all_not_two_values_equal(data, component, ComponentAttribute.node_1, ComponentAttribute.node_3)
    errors += _all_not_two_values_equal(data, component, ComponentAttribute.node_2, ComponentAttribute.node_3)
    errors += _all_boolean(data, component, ComponentAttribute.status_1)
    errors += _all_boolean(data, component, ComponentAttribute.status_2)
    errors += _all_boolean(data, component, ComponentAttribute.status_3)
    return errors


def validate_three_winding_transformer(data: SingleDataset) -> list[ValidationError]:  # noqa: PLR0915
    errors = validate_branch3(data, ComponentType.three_winding_transformer)
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, ComponentAttribute.u1)
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, ComponentAttribute.u2)
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, ComponentAttribute.u3)
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, ComponentAttribute.sn_1)
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, ComponentAttribute.sn_2)
    errors += _all_greater_than_zero(data, ComponentType.three_winding_transformer, ComponentAttribute.sn_3)
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12,
        f"{ComponentAttribute.pk_12}/{ComponentAttribute.sn_1}",
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12,
        f"{ComponentAttribute.pk_12}/{ComponentAttribute.sn_2}",
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13,
        f"{ComponentAttribute.pk_13}/{ComponentAttribute.sn_1}",
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13,
        f"{ComponentAttribute.pk_13}/{ComponentAttribute.sn_3}",
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23,
        f"{ComponentAttribute.pk_23}/{ComponentAttribute.sn_2}",
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23,
        f"{ComponentAttribute.pk_23}/{ComponentAttribute.sn_3}",
    )
    errors += _all_between(data, ComponentType.three_winding_transformer, ComponentAttribute.uk_12, 0, 1)
    errors += _all_between(data, ComponentType.three_winding_transformer, ComponentAttribute.uk_13, 0, 1)
    errors += _all_between(data, ComponentType.three_winding_transformer, ComponentAttribute.uk_23, 0, 1)
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.three_winding_transformer, ComponentAttribute.pk_12
    )
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.three_winding_transformer, ComponentAttribute.pk_13
    )
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.three_winding_transformer, ComponentAttribute.pk_23
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.i0,
        f"{ComponentAttribute.p0}/{ComponentAttribute.sn_1}",
    )
    errors += _all_less_than(data, ComponentType.three_winding_transformer, ComponentAttribute.i0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.three_winding_transformer, ComponentAttribute.p0)
    errors += _all_valid_enum_values(
        data, ComponentType.three_winding_transformer, ComponentAttribute.winding_1, WindingType
    )
    errors += _all_valid_enum_values(
        data, ComponentType.three_winding_transformer, ComponentAttribute.winding_2, WindingType
    )
    errors += _all_valid_enum_values(
        data, ComponentType.three_winding_transformer, ComponentAttribute.winding_3, WindingType
    )
    errors += _all_between_or_at(data, ComponentType.three_winding_transformer, ComponentAttribute.clock_12, -12, 12)
    errors += _all_between_or_at(data, ComponentType.three_winding_transformer, ComponentAttribute.clock_13, -12, 12)
    errors += _all_valid_clocks(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.clock_12,
        ComponentAttribute.winding_1,
        ComponentAttribute.winding_2,
    )
    errors += _all_valid_clocks(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.clock_13,
        ComponentAttribute.winding_1,
        ComponentAttribute.winding_3,
    )
    errors += _all_valid_enum_values(
        data, ComponentType.three_winding_transformer, ComponentAttribute.tap_side, Branch3Side
    )
    errors += _all_between_or_at(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.tap_pos,
        ComponentAttribute.tap_min,
        ComponentAttribute.tap_max,
        data[ComponentType.three_winding_transformer][ComponentAttribute.tap_nom],
        0,
    )
    errors += _all_between_or_at(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.tap_nom,
        ComponentAttribute.tap_min,
        ComponentAttribute.tap_max,
        0,
    )
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.three_winding_transformer, ComponentAttribute.tap_size
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12_min,
        f"{ComponentAttribute.pk_12_min}/{ComponentAttribute.sn_1}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12_min,
        f"{ComponentAttribute.pk_12_min}/{ComponentAttribute.sn_2}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13_min,
        f"{ComponentAttribute.pk_13_min}/{ComponentAttribute.sn_1}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13_min,
        f"{ComponentAttribute.pk_13_min}/{ComponentAttribute.sn_3}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23_min,
        f"{ComponentAttribute.pk_23_min}/{ComponentAttribute.sn_2}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_23],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23_min,
        f"{ComponentAttribute.pk_23_min}/{ComponentAttribute.sn_3}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_23],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12_min,
        0,
        1,
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_12],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13_min,
        0,
        1,
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_13],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23_min,
        0,
        1,
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_23],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12_max,
        f"{ComponentAttribute.pk_12_max}/{ComponentAttribute.sn_1}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12_max,
        f"{ComponentAttribute.pk_12_max}/{ComponentAttribute.sn_2}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13_max,
        f"{ComponentAttribute.pk_13_max}/{ComponentAttribute.sn_1}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13_max,
        f"{ComponentAttribute.pk_13_max}/{ComponentAttribute.sn_3}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23_max,
        f"{ComponentAttribute.pk_23_max}/{ComponentAttribute.sn_2}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_23],
    )
    errors += _all_greater_or_equal(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23_max,
        f"{ComponentAttribute.pk_23_max}/{ComponentAttribute.sn_3}",
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_23],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_12_max,
        0,
        1,
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_12],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_13_max,
        0,
        1,
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_13],
    )
    errors += _all_between(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.uk_23_max,
        0,
        1,
        data[ComponentType.three_winding_transformer][ComponentAttribute.uk_23],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.pk_12_min,
        data[ComponentType.three_winding_transformer][ComponentAttribute.pk_12],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.pk_13_min,
        data[ComponentType.three_winding_transformer][ComponentAttribute.pk_13],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.pk_23_min,
        data[ComponentType.three_winding_transformer][ComponentAttribute.pk_23],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.pk_12_max,
        data[ComponentType.three_winding_transformer][ComponentAttribute.pk_12],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.pk_13_max,
        data[ComponentType.three_winding_transformer][ComponentAttribute.pk_13],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        ComponentType.three_winding_transformer,
        ComponentAttribute.pk_23_max,
        data[ComponentType.three_winding_transformer][ComponentAttribute.pk_23],
    )
    return errors


def validate_appliance(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_boolean(data, component, ComponentAttribute.status)
    errors += _all_valid_ids(data, component, ComponentAttribute.node, ComponentType.node)
    return errors


def validate_source(data: SingleDataset) -> list[ValidationError]:
    errors = validate_appliance(data, ComponentType.source)
    errors += _all_greater_than_zero(data, ComponentType.source, ComponentAttribute.u_ref)
    errors += _all_greater_than_zero(data, ComponentType.source, ComponentAttribute.sk)
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.source, ComponentAttribute.rx_ratio)
    errors += _all_greater_than_zero(data, ComponentType.source, ComponentAttribute.z01_ratio)
    return errors


def validate_generic_load_gen(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_appliance(data, component)
    errors += _all_valid_enum_values(data, component, ComponentAttribute.type, LoadGenType)
    return errors


def validate_shunt(data: SingleDataset) -> list[ValidationError]:
    return validate_appliance(data, ComponentType.shunt)


def validate_voltage_regulator(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, ComponentType.voltage_regulator)
    errors += _all_valid_ids(
        data,
        ComponentType.voltage_regulator,
        ComponentAttribute.regulated_object,
        [ComponentType.sym_gen, ComponentType.asym_gen, ComponentType.sym_load, ComponentType.asym_load],
    )
    errors += _all_boolean(data, ComponentType.voltage_regulator, ComponentAttribute.status)
    errors += _all_unique(data, ComponentType.voltage_regulator, ComponentAttribute.regulated_object)
    errors += _all_greater_than_zero(data, ComponentType.voltage_regulator, ComponentAttribute.u_ref)
    errors += validate_same_u_ref_per_node_voltage_regulator(data)
    return errors


def validate_same_u_ref_per_node_voltage_regulator(
    data: SingleDataset,
) -> list[ValidationError]:
    """Ensure that all voltage regulators and sources connected to the same node have the same u_ref
    - collect u_ref defined by sources
    - if there are multiple sources on the same node, check they have the same u_ref
    - if they have different u_ref, then remove them from consideration and later only check
      that two voltage regulators connected to the same node have the same u_ref, because
      usually the source has priority in defining the node type (Slack vs. PV)
    """

    errors: list[ValidationError] = []
    if ComponentType.voltage_regulator in data:
        node_u_refs = _init_node_u_ref_from_sources(data)
        vr_data = data[ComponentType.voltage_regulator]
        if vr_data.size != 0:
            appliance_to_node: dict[int, int] = _init_appliance_to_node_mapping(data)

            regulator_ids = vr_data[ComponentAttribute.id]
            regulator_status = vr_data[ComponentAttribute.status]
            appliance_ids = vr_data[ComponentAttribute.regulated_object]
            u_refs = vr_data[ComponentAttribute.u_ref]

            # update node_u_refs with voltage regulator u_ref
            node_regulators: dict[int, list[int]] = {}
            for appliance_id, regulator_id, status, u_ref in zip(
                appliance_ids, regulator_ids, regulator_status, u_refs
            ):
                if status == 0:
                    continue  # skip disabled voltage regulators
                node_id = appliance_to_node.get(appliance_id)
                if node_id is not None:
                    node_u_refs.setdefault(node_id, set()).add(u_ref)
                    node_regulators.setdefault(node_id, []).append(regulator_id)

            # get nodes with different u_refs
            error_node_ids = set(
                [
                    node_id
                    for node_id, u_refs in node_u_refs.items()
                    if node_id is not None and u_refs is not None and len(u_refs) > 1
                ]
            )

            # collect voltage regulator ids connected to those nodes
            error_regulator_ids = []
            for node_id in error_node_ids:
                error_regulator_ids.extend(node_regulators[node_id])

            if len(error_regulator_ids) > 0:
                errors.append(
                    InvalidVoltageRegulationError(
                        ComponentType.voltage_regulator, ComponentAttribute.u_ref, error_regulator_ids
                    )
                )

    return errors


def _init_node_u_ref_from_sources(data: SingleDataset):
    """Initialize a mapping of node IDs to u_ref values defined by sources connected to those nodes.
    Multiple sources connected to the same node are possible and the resulting u_ref is effectively
    determined relative to the sk value of those sources. In that case, a voltage regulator at the same node
    probably won't reference the same u_ref value as the sources, so we remove the u_ref of the sources again
    and only check the voltage regulators among each other.
    """
    node_u_refs: dict[int, set[float]] = {}
    if ComponentType.source in data:
        source_data = data[ComponentType.source]
        for idx, node_id in enumerate(source_data[ComponentAttribute.node]):
            status = source_data[ComponentAttribute.status][idx]
            if status != 0:
                u_ref = source_data[ComponentAttribute.u_ref][idx]
                node_u_refs.setdefault(node_id, set()).add(u_ref)

        # clear nodes with different source u_refs from consideration
        for node_id, u_refs in node_u_refs.items():
            if len(u_refs) > 1:
                u_refs.clear()

    return node_u_refs


def _init_appliance_to_node_mapping(data: SingleDataset):
    appliance_to_node: dict[int, int] = {}
    for component_type in [
        ComponentType.sym_gen,
        ComponentType.asym_gen,
        ComponentType.sym_load,
        ComponentType.asym_load,
    ]:
        if component_type in data:
            for appliance in data[component_type]:
                if appliance[ComponentAttribute.status] != 0:
                    appliance_to_node[appliance[ComponentAttribute.id]] = appliance[ComponentAttribute.node]

    return appliance_to_node


def validate_generic_voltage_sensor(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, ComponentAttribute.u_sigma)
    errors += _all_greater_than_zero(data, component, ComponentAttribute.u_measured)
    errors += _all_valid_ids(data, component, ComponentAttribute.measured_object, ComponentType.node)
    return errors


def validate_generic_power_sensor(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, ComponentAttribute.power_sigma)
    errors += _all_valid_enum_values(data, component, ComponentAttribute.measured_terminal_type, MeasuredTerminalType)
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
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
        field=ComponentAttribute.measured_object,
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
        field=ComponentAttribute.measured_object,
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
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.source,
        measured_terminal_type=MeasuredTerminalType.source,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.shunt,
        measured_terminal_type=MeasuredTerminalType.shunt,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=[ComponentType.sym_load, ComponentType.asym_load],
        measured_terminal_type=MeasuredTerminalType.load,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=[ComponentType.sym_gen, ComponentType.asym_gen],
        measured_terminal_type=MeasuredTerminalType.generator,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_1,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_2,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_3,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.node,
        measured_terminal_type=MeasuredTerminalType.node,
    )
    if component in (ComponentType.sym_power_sensor, ComponentType.asym_power_sensor):
        errors += _valid_p_q_sigma(data, component)

    return errors


def validate_generic_current_sensor(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, ComponentAttribute.i_sigma)
    errors += _all_greater_than_zero(data, component, ComponentAttribute.i_angle_sigma)
    errors += _all_valid_enum_values(data, component, ComponentAttribute.measured_terminal_type, MeasuredTerminalType)
    errors += _all_valid_enum_values(data, component, ComponentAttribute.angle_measurement_type, AngleMeasurementType)
    errors += _all_in_valid_values(
        data,
        component,
        ComponentAttribute.measured_terminal_type,
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
        field=ComponentAttribute.measured_object,
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
        field=ComponentAttribute.measured_object,
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
        field=ComponentAttribute.measured_object,
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
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_1,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_2,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.measured_object,
        ref_components=ComponentType.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_3,
    )
    errors += _all_same_current_angle_measurement_type_on_terminal(
        data,
        component,
        measured_object_field=ComponentAttribute.measured_object,
        measured_terminal_type_field=ComponentAttribute.measured_terminal_type,
        angle_measurement_type_field=ComponentAttribute.angle_measurement_type,
    )
    errors += _any_voltage_angle_measurement_if_global_current_measurement(
        data,
        component,
        angle_measurement_type_filter=(ComponentAttribute.angle_measurement_type, AngleMeasurementType.global_angle),
        voltage_sensor_u_angle_measured={
            ComponentType.sym_voltage_sensor: ComponentAttribute.u_angle_measured,
            ComponentType.asym_voltage_sensor: ComponentAttribute.u_angle_measured,
        },
    )

    return errors


def validate_fault(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, ComponentType.fault)
    errors += _all_boolean(data, ComponentType.fault, ComponentAttribute.status)
    errors += _all_valid_enum_values(data, ComponentType.fault, ComponentAttribute.fault_type, FaultType)
    errors += _all_valid_enum_values(data, ComponentType.fault, ComponentAttribute.fault_phase, FaultPhase)
    errors += _all_valid_fault_phases(
        data, ComponentType.fault, ComponentAttribute.fault_type, ComponentAttribute.fault_phase
    )
    errors += _all_valid_ids(
        data, ComponentType.fault, field=ComponentAttribute.fault_object, ref_components=ComponentType.node
    )
    errors += _all_greater_than_or_equal_to_zero(data, ComponentType.fault, ComponentAttribute.r_f)
    errors += _all_enabled_identical(
        data, ComponentType.fault, ComponentAttribute.fault_type, ComponentAttribute.status
    )
    errors += _all_enabled_identical(
        data, ComponentType.fault, ComponentAttribute.fault_phase, ComponentAttribute.status
    )
    return errors


def validate_regulator(data: SingleDataset, component: ComponentType) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(
        data,
        component,
        field=ComponentAttribute.regulated_object,
        ref_components=[ComponentType.transformer, ComponentType.three_winding_transformer],
    )
    return errors


def validate_transformer_tap_regulator(data: SingleDataset) -> list[ValidationError]:
    errors = validate_regulator(data, ComponentType.transformer_tap_regulator)
    errors += _all_boolean(data, ComponentType.transformer_tap_regulator, ComponentAttribute.status)
    errors += _all_unique(data, ComponentType.transformer_tap_regulator, ComponentAttribute.regulated_object)
    errors += _all_valid_enum_values(
        data, ComponentType.transformer_tap_regulator, ComponentAttribute.control_side, [BranchSide, Branch3Side]
    )
    errors += _all_valid_associated_enum_values(
        data,
        ComponentType.transformer_tap_regulator,
        ComponentAttribute.control_side,
        ComponentAttribute.regulated_object,
        [ComponentType.transformer],
        [BranchSide],
    )
    errors += _all_valid_associated_enum_values(
        data,
        ComponentType.transformer_tap_regulator,
        ComponentAttribute.control_side,
        ComponentAttribute.regulated_object,
        [ComponentType.three_winding_transformer],
        [Branch3Side],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.transformer_tap_regulator, ComponentAttribute.u_set
    )
    errors += _all_greater_than_zero(data, ComponentType.transformer_tap_regulator, ComponentAttribute.u_band)
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.transformer_tap_regulator, ComponentAttribute.line_drop_compensation_r, 0.0
    )
    errors += _all_greater_than_or_equal_to_zero(
        data, ComponentType.transformer_tap_regulator, ComponentAttribute.line_drop_compensation_x, 0.0
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
                    measured_object_field=ComponentAttribute.measured_object,
                    measured_terminal_type_field=ComponentAttribute.measured_terminal_type,
                )

    return errors
