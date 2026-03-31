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
    AttributeType as AT,
    ComponentType as CT,
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
    return _all_cross_unique(data, [(component, AT.id) for component in data])


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
    required: dict[ComponentTypeLike, list[AT]] = {"base": [AT.id]}

    # Nodes
    required[CT.node] = required["base"] + [AT.u_rated]

    # Branches
    required["branch"] = required["base"] + [
        AT.from_node,
        AT.to_node,
        AT.from_status,
        AT.to_status,
    ]
    required[CT.link] = required["branch"].copy()
    required[CT.line] = required["branch"] + [
        AT.r1,
        AT.x1,
        AT.c1,
        AT.tan1,
    ]
    required[CT.asym_line] = required["branch"] + [
        AT.r_aa,
        AT.r_ba,
        AT.r_bb,
        AT.r_ca,
        AT.r_cb,
        AT.r_cc,
        AT.x_aa,
        AT.x_ba,
        AT.x_bb,
        AT.x_ca,
        AT.x_cb,
        AT.x_cc,
    ]
    required[CT.transformer] = required["branch"] + [
        AT.u1,
        AT.u2,
        AT.sn,
        AT.uk,
        AT.pk,
        AT.i0,
        AT.p0,
        AT.winding_from,
        AT.winding_to,
        AT.clock,
        AT.tap_side,
        AT.tap_min,
        AT.tap_max,
        AT.tap_size,
    ]
    # Branch3
    required["branch3"] = required["base"] + [
        AT.node_1,
        AT.node_2,
        AT.node_3,
        AT.status_1,
        AT.status_2,
        AT.status_3,
    ]
    required[CT.three_winding_transformer] = required["branch3"] + [
        AT.u1,
        AT.u2,
        AT.u3,
        AT.sn_1,
        AT.sn_2,
        AT.sn_3,
        AT.uk_12,
        AT.uk_13,
        AT.uk_23,
        AT.pk_12,
        AT.pk_13,
        AT.pk_23,
        AT.i0,
        AT.p0,
        AT.winding_1,
        AT.winding_2,
        AT.winding_3,
        AT.clock_12,
        AT.clock_13,
        AT.tap_side,
        AT.tap_min,
        AT.tap_max,
        AT.tap_size,
    ]

    # Regulators
    required["regulator"] = required["base"] + [
        AT.regulated_object,
        AT.status,
    ]
    required[CT.transformer_tap_regulator] = required["regulator"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[CT.transformer_tap_regulator] += [
            AT.control_side,
            AT.u_set,
            AT.u_band,
        ]

    required[CT.voltage_regulator] = required["regulator"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[CT.voltage_regulator] += [AT.u_ref]
        # TODO(scud-soptim): add unit test for optional q_min and q_max when limit handling is implemented

    # Appliances
    required["appliance"] = required["base"] + [
        AT.node,
        AT.status,
    ]
    required[CT.source] = required["appliance"].copy()
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required[CT.source] += [AT.u_ref]
    required[CT.shunt] = required["appliance"] + [
        AT.g1,
        AT.b1,
    ]
    required["generic_load_gen"] = required["appliance"] + [AT.type]
    if calculation_type is None or calculation_type == CalculationType.power_flow:
        required["generic_load_gen"] += [
            AT.p_specified,
            AT.q_specified,
        ]
    required[CT.sym_load] = required["generic_load_gen"].copy()
    required[CT.asym_load] = required["generic_load_gen"].copy()
    required[CT.sym_gen] = required["generic_load_gen"].copy()
    required[CT.asym_gen] = required["generic_load_gen"].copy()

    # Sensors
    required["sensor"] = required["base"] + [AT.measured_object]
    required["voltage_sensor"] = required["sensor"].copy()
    required["power_sensor"] = required["sensor"] + [AT.measured_terminal_type]
    required["current_sensor"] = required["sensor"] + [
        AT.measured_terminal_type,
        AT.angle_measurement_type,
    ]
    if calculation_type is None or calculation_type == CalculationType.state_estimation:
        required["voltage_sensor"] += [AT.u_sigma, AT.u_measured]
        required["power_sensor"] += [
            AT.p_measured,
            AT.q_measured,
        ]  # power_sigma, p_sigma and q_sigma are checked later
        required["current_sensor"] += [
            AT.i_sigma,
            AT.i_angle_sigma,
            AT.i_measured,
            AT.i_angle_measured,
        ]
    required[CT.sym_voltage_sensor] = required["voltage_sensor"].copy()
    required[CT.asym_voltage_sensor] = required["voltage_sensor"].copy()
    required[CT.sym_current_sensor] = required["current_sensor"].copy()
    required[CT.asym_current_sensor] = required["current_sensor"].copy()

    # Different requirements for individual sensors. Avoid shallow copy.
    for sensor_type in (CT.sym_power_sensor, CT.asym_power_sensor):
        required[sensor_type] = required["power_sensor"].copy()

    # Faults
    required[CT.fault] = required["base"] + [AT.fault_object]
    asym_sc = False
    if calculation_type is None or calculation_type == CalculationType.short_circuit:
        required[CT.fault] += [AT.status, AT.fault_type]
        if CT.fault in data:
            for elem in data[CT.fault][AT.fault_type]:
                if elem not in (FaultType.three_phase, FaultType.nan):
                    asym_sc = True
                    break

    if not symmetric or asym_sc:
        required[CT.line] += [
            AT.r0,
            AT.x0,
            AT.c0,
            AT.tan0,
        ]
        required[CT.shunt] += [AT.g0, AT.b0]

    errors = _validate_required_in_data(data, required)

    if calculation_type is None or calculation_type == CalculationType.state_estimation:
        errors += _validate_required_power_sigma_or_p_q_sigma(data, CT.sym_power_sensor)
        errors += _validate_required_power_sigma_or_p_q_sigma(data, CT.asym_power_sensor)

    return errors


def _validate_required_in_data(data: SingleDataset, required: dict[ComponentTypeLike, list[AT]]):
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
    power_sensor: Literal[CT.sym_power_sensor, CT.asym_power_sensor],
) -> list[MissingValueError]:
    """
    Check that either `p_sigma` and `q_sigma` are all provided, or that `power_sigma` is provided.

    Args:
        data: SingleDataset, pgm data
        sensor: the power sensor type, either CT.sym_power_sensor or CT.asym_power_sensor
    """
    result: list[MissingValueError] = []

    if power_sensor in data:
        sensor_data = data[power_sensor]
        p_sigma = sensor_data[AT.p_sigma]
        q_sigma = sensor_data[AT.q_sigma]

        asym_axes = tuple(range(sensor_data.ndim, p_sigma.ndim))
        all_pq_sigma_missing_mask = np.all(np.isnan(p_sigma), axis=asym_axes) & np.all(
            np.isnan(q_sigma), axis=asym_axes
        )

        result += _validate_required_in_data(
            {power_sensor: sensor_data[all_pq_sigma_missing_mask]},
            required={power_sensor: [AT.power_sigma]},
        )
        result += _validate_required_in_data(
            {power_sensor: sensor_data[~all_pq_sigma_missing_mask]},
            required={power_sensor: [AT.p_sigma, AT.q_sigma]},
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
                CT.sym_power_sensor: [
                    AT.power_sigma,
                    AT.p_sigma,
                    AT.q_sigma,
                ],
                CT.asym_power_sensor: [
                    AT.power_sigma,
                    AT.p_sigma,
                    AT.q_sigma,
                ],
                CT.sym_voltage_sensor: [AT.u_sigma],
                CT.asym_voltage_sensor: [AT.u_sigma],
                CT.sym_current_sensor: [
                    AT.i_sigma,
                    AT.i_angle_sigma,
                ],
                CT.asym_current_sensor: [AT.i_sigma, AT.i_angle_sigma],
            },
        )
    )

    component_validators = {
        CT.node: validate_node,
        CT.line: validate_line,
        CT.asym_line: validate_asym_line,
        CT.link: lambda d: validate_branch(d, CT.link),
        CT.generic_branch: validate_generic_branch,
        CT.transformer: validate_transformer,
        CT.three_winding_transformer: validate_three_winding_transformer,
        CT.source: validate_source,
        CT.sym_load: lambda d: validate_generic_load_gen(d, CT.sym_load),
        CT.sym_gen: lambda d: validate_generic_load_gen(d, CT.sym_gen),
        CT.asym_load: lambda d: validate_generic_load_gen(d, CT.asym_load),
        CT.asym_gen: lambda d: validate_generic_load_gen(d, CT.asym_gen),
        CT.shunt: validate_shunt,
        CT.voltage_regulator: validate_voltage_regulator,
    }

    for component, validator in component_validators.items():
        if component in data:
            errors += validator(data)

    if calculation_type in (None, CalculationType.state_estimation):
        if CT.sym_voltage_sensor in data:
            errors += validate_generic_voltage_sensor(data, CT.sym_voltage_sensor)
        if CT.asym_voltage_sensor in data:
            errors += validate_generic_voltage_sensor(data, CT.asym_voltage_sensor)
        if CT.sym_power_sensor in data:
            errors += validate_generic_power_sensor(data, CT.sym_power_sensor)
        if CT.asym_power_sensor in data:
            errors += validate_generic_power_sensor(data, CT.asym_power_sensor)
        if CT.sym_current_sensor in data:
            errors += validate_generic_current_sensor(data, CT.sym_current_sensor)
        if CT.asym_current_sensor in data:
            errors += validate_generic_current_sensor(data, CT.asym_current_sensor)

        errors += validate_no_mixed_sensors_on_same_terminal(data)

    if calculation_type in (None, CalculationType.short_circuit) and CT.fault in data:
        errors += validate_fault(data)

    if calculation_type in (None, CalculationType.power_flow) and CT.transformer_tap_regulator in data:
        errors += validate_transformer_tap_regulator(data)

    return errors


def validate_base(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors: list[ValidationError] = list(_all_unique(data, component, AT.id))
    return errors


def validate_node(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, CT.node)
    errors += _all_greater_than_zero(data, CT.node, AT.u_rated)
    return errors


def validate_branch(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(data, component, AT.from_node, CT.node)
    errors += _all_valid_ids(data, component, AT.to_node, CT.node)
    errors += _all_not_two_values_equal(data, component, AT.to_node, AT.from_node)
    errors += _all_boolean(data, component, AT.from_status)
    errors += _all_boolean(data, component, AT.to_status)
    return errors


def validate_line(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, CT.line)
    errors += _all_not_two_values_zero(data, CT.line, AT.r1, AT.x1)
    errors += _all_not_two_values_zero(data, CT.line, AT.r0, AT.x0)
    errors += _all_greater_than_zero(data, CT.line, AT.i_n)
    return errors


def validate_asym_line(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, CT.asym_line)
    errors += _all_greater_than_zero(data, CT.asym_line, AT.i_n)
    required_fields = [
        AT.r_aa,
        AT.r_ba,
        AT.r_bb,
        AT.r_ca,
        AT.r_cb,
        AT.r_cc,
        AT.x_aa,
        AT.x_ba,
        AT.x_bb,
        AT.x_ca,
        AT.x_cb,
        AT.x_cc,
    ]
    optional_r_matrix_fields = [
        AT.r_na,
        AT.r_nb,
        AT.r_nc,
        AT.r_nn,
    ]
    optional_x_matrix_fields = [
        AT.x_na,
        AT.x_nb,
        AT.x_nc,
        AT.x_nn,
    ]
    required_c_matrix_fields = [
        AT.c_aa,
        AT.c_ba,
        AT.c_bb,
        AT.c_ca,
        AT.c_cb,
        AT.c_cc,
    ]
    c_fields = [AT.c0, AT.c1]
    for field in (
        required_fields + optional_r_matrix_fields + optional_x_matrix_fields + required_c_matrix_fields + c_fields
    ):
        errors += _all_greater_than_zero(data, CT.asym_line, field)

    errors += _no_strict_subset_missing(data, optional_r_matrix_fields + optional_x_matrix_fields, CT.asym_line)
    errors += _no_strict_subset_missing(data, required_c_matrix_fields, CT.asym_line)
    errors += _no_strict_subset_missing(data, c_fields, CT.asym_line)
    errors += _not_all_missing(data, required_c_matrix_fields + c_fields, CT.asym_line)

    return errors


def validate_generic_branch(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, CT.generic_branch)
    errors += _all_greater_than_zero(data, CT.generic_branch, AT.k)
    errors += _all_greater_than_or_equal_to_zero(data, CT.generic_branch, AT.sn)
    return errors


def validate_transformer(data: SingleDataset) -> list[ValidationError]:
    errors = validate_branch(data, CT.transformer)
    errors += _all_greater_than_zero(data, CT.transformer, AT.u1)
    errors += _all_greater_than_zero(data, CT.transformer, AT.u2)
    errors += _all_greater_than_zero(data, CT.transformer, AT.sn)
    errors += _all_greater_or_equal(data, CT.transformer, AT.uk, f"{AT.pk}/{AT.sn}")
    errors += _all_between(data, CT.transformer, AT.uk, 0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, CT.transformer, AT.pk)
    errors += _all_greater_or_equal(data, CT.transformer, AT.i0, f"{AT.p0}/{AT.sn}")
    errors += _all_less_than(data, CT.transformer, AT.i0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, CT.transformer, AT.p0)
    errors += _all_valid_enum_values(data, CT.transformer, AT.winding_from, WindingType)
    errors += _all_valid_enum_values(data, CT.transformer, AT.winding_to, WindingType)
    errors += _all_between_or_at(data, CT.transformer, AT.clock, -12, 12)
    errors += _all_valid_clocks(
        data,
        CT.transformer,
        AT.clock,
        AT.winding_from,
        AT.winding_to,
    )
    errors += _all_valid_enum_values(data, CT.transformer, AT.tap_side, BranchSide)
    errors += _all_between_or_at(
        data,
        CT.transformer,
        AT.tap_pos,
        AT.tap_min,
        AT.tap_max,
        data[CT.transformer][AT.tap_nom],
        0,
    )
    errors += _all_between_or_at(
        data,
        CT.transformer,
        AT.tap_nom,
        AT.tap_min,
        AT.tap_max,
        0,
    )
    errors += _all_greater_than_or_equal_to_zero(data, CT.transformer, AT.tap_size)
    errors += _all_greater_or_equal(
        data,
        CT.transformer,
        AT.uk_min,
        f"{AT.pk_min}/{AT.sn}",
        data[CT.transformer][AT.uk],
    )
    errors += _all_between(
        data,
        CT.transformer,
        AT.uk_min,
        0,
        1,
        data[CT.transformer][AT.uk],
    )
    errors += _all_greater_or_equal(
        data,
        CT.transformer,
        AT.uk_max,
        f"{AT.pk_max}/{AT.sn}",
        data[CT.transformer][AT.uk],
    )
    errors += _all_between(
        data,
        CT.transformer,
        AT.uk_max,
        0,
        1,
        data[CT.transformer][AT.uk],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.transformer,
        AT.pk_min,
        data[CT.transformer][AT.pk],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.transformer,
        AT.pk_max,
        data[CT.transformer][AT.pk],
    )
    return errors


def validate_branch3(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(data, component, AT.node_1, CT.node)
    errors += _all_valid_ids(data, component, AT.node_2, CT.node)
    errors += _all_valid_ids(data, component, AT.node_3, CT.node)
    errors += _all_not_two_values_equal(data, component, AT.node_1, AT.node_2)
    errors += _all_not_two_values_equal(data, component, AT.node_1, AT.node_3)
    errors += _all_not_two_values_equal(data, component, AT.node_2, AT.node_3)
    errors += _all_boolean(data, component, AT.status_1)
    errors += _all_boolean(data, component, AT.status_2)
    errors += _all_boolean(data, component, AT.status_3)
    return errors


def validate_three_winding_transformer(data: SingleDataset) -> list[ValidationError]:  # noqa: PLR0915
    errors = validate_branch3(data, CT.three_winding_transformer)
    errors += _all_greater_than_zero(data, CT.three_winding_transformer, AT.u1)
    errors += _all_greater_than_zero(data, CT.three_winding_transformer, AT.u2)
    errors += _all_greater_than_zero(data, CT.three_winding_transformer, AT.u3)
    errors += _all_greater_than_zero(data, CT.three_winding_transformer, AT.sn_1)
    errors += _all_greater_than_zero(data, CT.three_winding_transformer, AT.sn_2)
    errors += _all_greater_than_zero(data, CT.three_winding_transformer, AT.sn_3)
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_12,
        f"{AT.pk_12}/{AT.sn_1}",
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_12,
        f"{AT.pk_12}/{AT.sn_2}",
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_13,
        f"{AT.pk_13}/{AT.sn_1}",
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_13,
        f"{AT.pk_13}/{AT.sn_3}",
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_23,
        f"{AT.pk_23}/{AT.sn_2}",
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_23,
        f"{AT.pk_23}/{AT.sn_3}",
    )
    errors += _all_between(data, CT.three_winding_transformer, AT.uk_12, 0, 1)
    errors += _all_between(data, CT.three_winding_transformer, AT.uk_13, 0, 1)
    errors += _all_between(data, CT.three_winding_transformer, AT.uk_23, 0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, CT.three_winding_transformer, AT.pk_12)
    errors += _all_greater_than_or_equal_to_zero(data, CT.three_winding_transformer, AT.pk_13)
    errors += _all_greater_than_or_equal_to_zero(data, CT.three_winding_transformer, AT.pk_23)
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.i0,
        f"{AT.p0}/{AT.sn_1}",
    )
    errors += _all_less_than(data, CT.three_winding_transformer, AT.i0, 1)
    errors += _all_greater_than_or_equal_to_zero(data, CT.three_winding_transformer, AT.p0)
    errors += _all_valid_enum_values(data, CT.three_winding_transformer, AT.winding_1, WindingType)
    errors += _all_valid_enum_values(data, CT.three_winding_transformer, AT.winding_2, WindingType)
    errors += _all_valid_enum_values(data, CT.three_winding_transformer, AT.winding_3, WindingType)
    errors += _all_between_or_at(data, CT.three_winding_transformer, AT.clock_12, -12, 12)
    errors += _all_between_or_at(data, CT.three_winding_transformer, AT.clock_13, -12, 12)
    errors += _all_valid_clocks(
        data,
        CT.three_winding_transformer,
        AT.clock_12,
        AT.winding_1,
        AT.winding_2,
    )
    errors += _all_valid_clocks(
        data,
        CT.three_winding_transformer,
        AT.clock_13,
        AT.winding_1,
        AT.winding_3,
    )
    errors += _all_valid_enum_values(data, CT.three_winding_transformer, AT.tap_side, Branch3Side)
    errors += _all_between_or_at(
        data,
        CT.three_winding_transformer,
        AT.tap_pos,
        AT.tap_min,
        AT.tap_max,
        data[CT.three_winding_transformer][AT.tap_nom],
        0,
    )
    errors += _all_between_or_at(
        data,
        CT.three_winding_transformer,
        AT.tap_nom,
        AT.tap_min,
        AT.tap_max,
        0,
    )
    errors += _all_greater_than_or_equal_to_zero(data, CT.three_winding_transformer, AT.tap_size)
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_12_min,
        f"{AT.pk_12_min}/{AT.sn_1}",
        data[CT.three_winding_transformer][AT.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_12_min,
        f"{AT.pk_12_min}/{AT.sn_2}",
        data[CT.three_winding_transformer][AT.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_13_min,
        f"{AT.pk_13_min}/{AT.sn_1}",
        data[CT.three_winding_transformer][AT.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_13_min,
        f"{AT.pk_13_min}/{AT.sn_3}",
        data[CT.three_winding_transformer][AT.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_23_min,
        f"{AT.pk_23_min}/{AT.sn_2}",
        data[CT.three_winding_transformer][AT.uk_23],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_23_min,
        f"{AT.pk_23_min}/{AT.sn_3}",
        data[CT.three_winding_transformer][AT.uk_23],
    )
    errors += _all_between(
        data,
        CT.three_winding_transformer,
        AT.uk_12_min,
        0,
        1,
        data[CT.three_winding_transformer][AT.uk_12],
    )
    errors += _all_between(
        data,
        CT.three_winding_transformer,
        AT.uk_13_min,
        0,
        1,
        data[CT.three_winding_transformer][AT.uk_13],
    )
    errors += _all_between(
        data,
        CT.three_winding_transformer,
        AT.uk_23_min,
        0,
        1,
        data[CT.three_winding_transformer][AT.uk_23],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_12_max,
        f"{AT.pk_12_max}/{AT.sn_1}",
        data[CT.three_winding_transformer][AT.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_12_max,
        f"{AT.pk_12_max}/{AT.sn_2}",
        data[CT.three_winding_transformer][AT.uk_12],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_13_max,
        f"{AT.pk_13_max}/{AT.sn_1}",
        data[CT.three_winding_transformer][AT.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_13_max,
        f"{AT.pk_13_max}/{AT.sn_3}",
        data[CT.three_winding_transformer][AT.uk_13],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_23_max,
        f"{AT.pk_23_max}/{AT.sn_2}",
        data[CT.three_winding_transformer][AT.uk_23],
    )
    errors += _all_greater_or_equal(
        data,
        CT.three_winding_transformer,
        AT.uk_23_max,
        f"{AT.pk_23_max}/{AT.sn_3}",
        data[CT.three_winding_transformer][AT.uk_23],
    )
    errors += _all_between(
        data,
        CT.three_winding_transformer,
        AT.uk_12_max,
        0,
        1,
        data[CT.three_winding_transformer][AT.uk_12],
    )
    errors += _all_between(
        data,
        CT.three_winding_transformer,
        AT.uk_13_max,
        0,
        1,
        data[CT.three_winding_transformer][AT.uk_13],
    )
    errors += _all_between(
        data,
        CT.three_winding_transformer,
        AT.uk_23_max,
        0,
        1,
        data[CT.three_winding_transformer][AT.uk_23],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.three_winding_transformer,
        AT.pk_12_min,
        data[CT.three_winding_transformer][AT.pk_12],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.three_winding_transformer,
        AT.pk_13_min,
        data[CT.three_winding_transformer][AT.pk_13],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.three_winding_transformer,
        AT.pk_23_min,
        data[CT.three_winding_transformer][AT.pk_23],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.three_winding_transformer,
        AT.pk_12_max,
        data[CT.three_winding_transformer][AT.pk_12],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.three_winding_transformer,
        AT.pk_13_max,
        data[CT.three_winding_transformer][AT.pk_13],
    )
    errors += _all_greater_than_or_equal_to_zero(
        data,
        CT.three_winding_transformer,
        AT.pk_23_max,
        data[CT.three_winding_transformer][AT.pk_23],
    )
    return errors


def validate_appliance(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_boolean(data, component, AT.status)
    errors += _all_valid_ids(data, component, AT.node, CT.node)
    return errors


def validate_source(data: SingleDataset) -> list[ValidationError]:
    errors = validate_appliance(data, CT.source)
    errors += _all_greater_than_zero(data, CT.source, AT.u_ref)
    errors += _all_greater_than_zero(data, CT.source, AT.sk)
    errors += _all_greater_than_or_equal_to_zero(data, CT.source, AT.rx_ratio)
    errors += _all_greater_than_zero(data, CT.source, AT.z01_ratio)
    return errors


def validate_generic_load_gen(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_appliance(data, component)
    errors += _all_valid_enum_values(data, component, AT.type, LoadGenType)
    return errors


def validate_shunt(data: SingleDataset) -> list[ValidationError]:
    return validate_appliance(data, CT.shunt)


def validate_voltage_regulator(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, CT.voltage_regulator)
    errors += _all_valid_ids(
        data,
        CT.voltage_regulator,
        AT.regulated_object,
        [CT.sym_gen, CT.asym_gen, CT.sym_load, CT.asym_load],
    )
    errors += _all_boolean(data, CT.voltage_regulator, AT.status)
    errors += _all_unique(data, CT.voltage_regulator, AT.regulated_object)
    errors += _all_greater_than_zero(data, CT.voltage_regulator, AT.u_ref)
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
    if CT.voltage_regulator in data:
        node_u_refs = _init_node_u_ref_from_sources(data)
        vr_data = data[CT.voltage_regulator]
        if vr_data.size != 0:
            appliance_to_node: dict[int, int] = _init_appliance_to_node_mapping(data)

            regulator_ids = vr_data[AT.id]
            regulator_status = vr_data[AT.status]
            appliance_ids = vr_data[AT.regulated_object]
            u_refs = vr_data[AT.u_ref]

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
                errors.append(InvalidVoltageRegulationError(CT.voltage_regulator, AT.u_ref, error_regulator_ids))

    return errors


def _init_node_u_ref_from_sources(data: SingleDataset):
    """Initialize a mapping of node IDs to u_ref values defined by sources connected to those nodes.
    Multiple sources connected to the same node are possible and the resulting u_ref is effectively
    determined relative to the sk value of those sources. In that case, a voltage regulator at the same node
    probably won't reference the same u_ref value as the sources, so we remove the u_ref of the sources again
    and only check the voltage regulators among each other.
    """
    node_u_refs: dict[int, set[float]] = {}
    if CT.source in data:
        source_data = data[CT.source]
        for idx, node_id in enumerate(source_data[AT.node]):
            status = source_data[AT.status][idx]
            if status != 0:
                u_ref = source_data[AT.u_ref][idx]
                node_u_refs.setdefault(node_id, set()).add(u_ref)

        # clear nodes with different source u_refs from consideration
        for node_id, u_refs in node_u_refs.items():
            if len(u_refs) > 1:
                u_refs.clear()

    return node_u_refs


def _init_appliance_to_node_mapping(data: SingleDataset):
    appliance_to_node: dict[int, int] = {}
    for component_type in [
        CT.sym_gen,
        CT.asym_gen,
        CT.sym_load,
        CT.asym_load,
    ]:
        if component_type in data:
            for appliance in data[component_type]:
                if appliance[AT.status] != 0:
                    appliance_to_node[appliance[AT.id]] = appliance[AT.node]

    return appliance_to_node


def validate_generic_voltage_sensor(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, AT.u_sigma)
    errors += _all_greater_than_zero(data, component, AT.u_measured)
    errors += _all_valid_ids(data, component, AT.measured_object, CT.node)
    return errors


def validate_generic_power_sensor(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, AT.power_sigma)
    errors += _all_valid_enum_values(data, component, AT.measured_terminal_type, MeasuredTerminalType)
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=[
            CT.node,
            CT.line,
            CT.asym_line,
            CT.generic_branch,
            CT.transformer,
            CT.three_winding_transformer,
            CT.source,
            CT.shunt,
            CT.sym_load,
            CT.asym_load,
            CT.sym_gen,
            CT.asym_gen,
        ],
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=[
            CT.line,
            CT.asym_line,
            CT.generic_branch,
            CT.transformer,
        ],
        measured_terminal_type=MeasuredTerminalType.branch_from,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=[
            CT.line,
            CT.asym_line,
            CT.generic_branch,
            CT.transformer,
        ],
        measured_terminal_type=MeasuredTerminalType.branch_to,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.source,
        measured_terminal_type=MeasuredTerminalType.source,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.shunt,
        measured_terminal_type=MeasuredTerminalType.shunt,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=[CT.sym_load, CT.asym_load],
        measured_terminal_type=MeasuredTerminalType.load,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=[CT.sym_gen, CT.asym_gen],
        measured_terminal_type=MeasuredTerminalType.generator,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_1,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_2,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_3,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.node,
        measured_terminal_type=MeasuredTerminalType.node,
    )
    if component in (CT.sym_power_sensor, CT.asym_power_sensor):
        errors += _valid_p_q_sigma(data, component)

    return errors


def validate_generic_current_sensor(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_greater_than_zero(data, component, AT.i_sigma)
    errors += _all_greater_than_zero(data, component, AT.i_angle_sigma)
    errors += _all_valid_enum_values(data, component, AT.measured_terminal_type, MeasuredTerminalType)
    errors += _all_valid_enum_values(data, component, AT.angle_measurement_type, AngleMeasurementType)
    errors += _all_in_valid_values(
        data,
        component,
        AT.measured_terminal_type,
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
        field=AT.measured_object,
        ref_components=[
            CT.line,
            CT.asym_line,
            CT.generic_branch,
            CT.transformer,
            CT.three_winding_transformer,
        ],
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=[
            CT.line,
            CT.asym_line,
            CT.generic_branch,
            CT.transformer,
        ],
        measured_terminal_type=MeasuredTerminalType.branch_from,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=[
            CT.line,
            CT.asym_line,
            CT.generic_branch,
            CT.transformer,
        ],
        measured_terminal_type=MeasuredTerminalType.branch_to,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_1,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_2,
    )
    errors += _all_valid_ids(
        data,
        component,
        field=AT.measured_object,
        ref_components=CT.three_winding_transformer,
        measured_terminal_type=MeasuredTerminalType.branch3_3,
    )
    errors += _all_same_current_angle_measurement_type_on_terminal(
        data,
        component,
        measured_object_field=AT.measured_object,
        measured_terminal_type_field=AT.measured_terminal_type,
        angle_measurement_type_field=AT.angle_measurement_type,
    )
    errors += _any_voltage_angle_measurement_if_global_current_measurement(
        data,
        component,
        angle_measurement_type_filter=(AT.angle_measurement_type, AngleMeasurementType.global_angle),
        voltage_sensor_u_angle_measured={
            CT.sym_voltage_sensor: AT.u_angle_measured,
            CT.asym_voltage_sensor: AT.u_angle_measured,
        },
    )

    return errors


def validate_fault(data: SingleDataset) -> list[ValidationError]:
    errors = validate_base(data, CT.fault)
    errors += _all_boolean(data, CT.fault, AT.status)
    errors += _all_valid_enum_values(data, CT.fault, AT.fault_type, FaultType)
    errors += _all_valid_enum_values(data, CT.fault, AT.fault_phase, FaultPhase)
    errors += _all_valid_fault_phases(data, CT.fault, AT.fault_type, AT.fault_phase)
    errors += _all_valid_ids(data, CT.fault, field=AT.fault_object, ref_components=CT.node)
    errors += _all_greater_than_or_equal_to_zero(data, CT.fault, AT.r_f)
    errors += _all_enabled_identical(data, CT.fault, AT.fault_type, AT.status)
    errors += _all_enabled_identical(data, CT.fault, AT.fault_phase, AT.status)
    return errors


def validate_regulator(data: SingleDataset, component: CT) -> list[ValidationError]:
    errors = validate_base(data, component)
    errors += _all_valid_ids(
        data,
        component,
        field=AT.regulated_object,
        ref_components=[CT.transformer, CT.three_winding_transformer],
    )
    return errors


def validate_transformer_tap_regulator(data: SingleDataset) -> list[ValidationError]:
    errors = validate_regulator(data, CT.transformer_tap_regulator)
    errors += _all_boolean(data, CT.transformer_tap_regulator, AT.status)
    errors += _all_unique(data, CT.transformer_tap_regulator, AT.regulated_object)
    errors += _all_valid_enum_values(data, CT.transformer_tap_regulator, AT.control_side, [BranchSide, Branch3Side])
    errors += _all_valid_associated_enum_values(
        data,
        CT.transformer_tap_regulator,
        AT.control_side,
        AT.regulated_object,
        [CT.transformer],
        [BranchSide],
    )
    errors += _all_valid_associated_enum_values(
        data,
        CT.transformer_tap_regulator,
        AT.control_side,
        AT.regulated_object,
        [CT.three_winding_transformer],
        [Branch3Side],
    )
    errors += _all_greater_than_or_equal_to_zero(data, CT.transformer_tap_regulator, AT.u_set)
    errors += _all_greater_than_zero(data, CT.transformer_tap_regulator, AT.u_band)
    errors += _all_greater_than_or_equal_to_zero(data, CT.transformer_tap_regulator, AT.line_drop_compensation_r, 0.0)
    errors += _all_greater_than_or_equal_to_zero(data, CT.transformer_tap_regulator, AT.line_drop_compensation_x, 0.0)
    return errors


def validate_no_mixed_sensors_on_same_terminal(data: SingleDataset) -> list[ValidationError]:
    errors: list[ValidationError] = []

    for power_sensor in [CT.sym_power_sensor, CT.asym_power_sensor]:
        for current_sensor in [CT.sym_current_sensor, CT.asym_current_sensor]:
            if power_sensor in data and current_sensor in data:
                errors += _all_same_sensor_type_on_same_terminal(
                    data,
                    power_sensor_type=power_sensor,
                    current_sensor_type=current_sensor,
                    measured_object_field=AT.measured_object,
                    measured_terminal_type_field=AT.measured_terminal_type,
                )

    return errors
