# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import copy
from itertools import product
from typing import cast
from unittest.mock import ANY, MagicMock, patch

import numpy as np
import pytest

from power_grid_model import (
    AttributeType as AT,
    CalculationType,
    ComponentType as CT,
    DatasetType,
    LoadGenType,
    MeasuredTerminalType,
    initialize_array,
    power_grid_meta_data,
)
from power_grid_model._core.enum import AngleMeasurementType
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset
from power_grid_model.enum import Branch3Side, BranchSide, ComponentAttributeFilterOptions, FaultType
from power_grid_model.validation import assert_valid_input_data
from power_grid_model.validation._validation import (
    assert_valid_data_structure,
    validate_generic_current_sensor,
    validate_generic_power_sensor,
    validate_ids,
    validate_input_data,
    validate_required_values,
    validate_unique_ids_across_components,
    validate_values,
)
from power_grid_model.validation.errors import (
    IdNotInDatasetError,
    InfinityError,
    InvalidAssociatedEnumValueError,
    InvalidEnumValueError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    NotUniqueError,
    PQSigmaPairError,
    UnsupportedMeasuredTerminalType,
)

NaN = cast(int, power_grid_meta_data[DatasetType.input][CT.node].nans[AT.id])


def test_assert_valid_data_structure():
    node_input = initialize_array(DatasetType.input, CT.node, 3)
    line_input = initialize_array(DatasetType.input, CT.line, 3)
    node_update = initialize_array(DatasetType.update, CT.node, 3)
    line_update = initialize_array(DatasetType.update, CT.line, 3)

    # Input data: Assertion ok
    assert_valid_data_structure({CT.node: node_input, CT.line: line_input}, DatasetType.input)

    # Update data: Assertion ok
    assert_valid_data_structure({CT.node: node_update, CT.line: line_update}, DatasetType.update)

    # There is no such thing as 'output' data
    with pytest.raises(KeyError, match=r"output"):
        assert_valid_data_structure({CT.node: node_input, CT.line: line_input}, "output")

    # Input data is not valid update data
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({CT.node: node_input, CT.line: line_input}, DatasetType.update)

    # Update data is not valid input data
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({CT.node: node_update, CT.line: line_update}, DatasetType.input)

    # A normal numpy array is not valid input data
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)])
    with pytest.raises(TypeError, match=r"Unexpected Numpy array"):
        assert_valid_data_structure({CT.node: node_dummy, CT.line: line_input}, DatasetType.input)

    # A structured numpy array, with wrong data type for u_rated f4 != f8, is not valid input data
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)], dtype=[(AT.id, "i4"), (AT.u_rated, "f4")])
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({CT.node: node_dummy, CT.line: line_input}, DatasetType.input)

    # A structured numpy array, with correct data types is not a valid, is still not valid input data.
    # N.B. It is not 'aligned'
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)], dtype=[(AT.id, "i4"), (AT.u_rated, "f8")])
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({CT.node: node_dummy, CT.line: line_input}, DatasetType.input)

    # Invalid component type
    input_with_wrong_component = {CT.node: node_input, "some_random_component": line_input}
    with pytest.raises(KeyError, match=r"Unknown component \'some_random_component\' in input data\."):
        assert_valid_data_structure(input_with_wrong_component, DatasetType.input)

    input_with_wrong_data_type = {CT.node: node_input, CT.line: [1, 2, 3]}
    with pytest.raises(TypeError, match=r"Unexpected data type list for \'line\' input data"):
        assert_valid_data_structure(input_with_wrong_data_type, DatasetType.input)


def test_validate_unique_ids_across_components():
    node = initialize_array(DatasetType.input, CT.node, 3)
    node[AT.id] = [1, 2, 3]

    line = initialize_array(DatasetType.input, CT.line, 3)
    line[AT.id] = [4, 5, 3]

    transformer = initialize_array(DatasetType.input, CT.transformer, 3)
    transformer[AT.id] = [1, 6, 7]

    source = initialize_array(DatasetType.input, CT.source, 3)
    source[AT.id] = [8, 9, 10]

    input_data = {
        CT.node: node,
        CT.line: line,
        CT.transformer: transformer,
        CT.source: source,
    }

    unique_id_errors = validate_unique_ids_across_components(input_data)

    assert (
        MultiComponentNotUniqueError(
            [
                (CT.node, AT.id),
                (CT.line, AT.id),
                (CT.transformer, AT.id),
            ],
            [(CT.node, 1), (CT.node, 3), (CT.line, 3), (CT.transformer, 1)],
        )
        in unique_id_errors
    )
    n_non_unique_ids = 4
    assert len(unique_id_errors[0].ids) == n_non_unique_ids


def test_validate_ids():
    source = initialize_array(DatasetType.input, CT.source, 3)
    source[AT.id] = [1, 2, 3]

    sym_load = initialize_array(DatasetType.input, CT.sym_load, 3)
    sym_load[AT.id] = [4, 5, 6]

    input_data = {
        CT.source: source,
        CT.sym_load: sym_load,
    }

    source_update = initialize_array(DatasetType.update, CT.source, 3)
    source_update[AT.id] = [1, 2, 4]
    source_update[AT.u_ref] = [1.0, 2.0, 3.0]

    sym_load_update = initialize_array(DatasetType.update, CT.sym_load, 3)
    sym_load_update[AT.id] = [4, 5, 7]
    sym_load_update[AT.p_specified] = [4.0, 5.0, 6.0]

    update_data = {CT.source: source_update, CT.sym_load: sym_load_update}

    invalid_ids = validate_ids(update_data, input_data)

    assert IdNotInDatasetError(CT.source, [4], DatasetType.update) in invalid_ids
    assert IdNotInDatasetError(CT.sym_load, [7], DatasetType.update) in invalid_ids

    source_update_no_id = initialize_array(DatasetType.update, CT.source, 3)
    source_update_no_id[AT.u_ref] = [1.0, 2.0, 3.0]

    update_data_col = compatibility_convert_row_columnar_dataset(
        data={CT.source: source_update_no_id, CT.sym_load: sym_load_update},
        data_filter=ComponentAttributeFilterOptions.relevant,
        dataset_type=DatasetType.update,
    )
    invalid_ids = validate_ids(update_data_col, input_data)
    assert len(invalid_ids) == 1
    assert IdNotInDatasetError(CT.sym_load, [7], DatasetType.update) in invalid_ids

    source_update_less_no_id = initialize_array(DatasetType.update, CT.source, 2)
    source_update_less_no_id[AT.u_ref] = [1.0, 2.0]

    update_data_col_less_no_id = compatibility_convert_row_columnar_dataset(
        data={CT.source: source_update_less_no_id, CT.sym_load: sym_load_update},
        data_filter=ComponentAttributeFilterOptions.relevant,
        dataset_type=DatasetType.update,
    )
    invalid_ids = validate_ids(update_data_col_less_no_id, input_data)
    n_invalid_ids_update_source_without_id = 2
    assert len(invalid_ids) == n_invalid_ids_update_source_without_id
    assert IdNotInDatasetError(CT.sym_load, [7], DatasetType.update) in invalid_ids

    source_update_part_nan_id = initialize_array(DatasetType.update, CT.source, 3)
    source_update_part_nan_id[AT.id] = [1, np.iinfo(np.int32).min, 4]
    source_update_part_nan_id[AT.u_ref] = [1.0, 2.0, 3.0]

    update_data_col_part_nan_id = compatibility_convert_row_columnar_dataset(
        data={CT.source: source_update_part_nan_id, CT.sym_load: sym_load_update},
        data_filter=ComponentAttributeFilterOptions.relevant,
        dataset_type=DatasetType.update,
    )
    invalid_ids = validate_ids(update_data_col_part_nan_id, input_data)
    n_invalid_ids_source_update = 2
    assert len(invalid_ids) == n_invalid_ids_source_update
    assert IdNotInDatasetError(CT.sym_load, [7], DatasetType.update) in invalid_ids


@pytest.mark.parametrize(
    "calculation_type",
    [
        pytest.param(None, id="no calculation type specified"),
        pytest.param(CalculationType.power_flow, id="power_flow"),
        pytest.param(CalculationType.state_estimation, id="state_estimation"),
        pytest.param(CalculationType.short_circuit, id="short_circuit"),
    ],
)
@pytest.mark.parametrize("symmetric", [pytest.param(True, id="symmetric"), pytest.param(False, id="asymmetric")])
def test_validate_required_values_sym_calculation(calculation_type, symmetric):
    node = initialize_array(DatasetType.input, CT.node, 1)
    line = initialize_array(DatasetType.input, CT.line, 1)
    link = initialize_array(DatasetType.input, CT.link, 1)
    transformer = initialize_array(DatasetType.input, CT.transformer, 1)
    three_winding_transformer = initialize_array(DatasetType.input, CT.three_winding_transformer, 1)
    source = initialize_array(DatasetType.input, CT.source, 1)
    shunt = initialize_array(DatasetType.input, CT.shunt, 1)
    sym_load = initialize_array(DatasetType.input, CT.sym_load, 1)
    sym_gen = initialize_array(DatasetType.input, CT.sym_gen, 1)
    asym_load = initialize_array(DatasetType.input, CT.asym_load, 1)
    asym_gen = initialize_array(DatasetType.input, CT.asym_gen, 1)
    sym_voltage_sensor = initialize_array(DatasetType.input, CT.sym_voltage_sensor, 1)

    asym_voltage_sensor = initialize_array(DatasetType.input, CT.asym_voltage_sensor, 1)
    asym_voltage_sensor[AT.u_measured] = [[1.0, np.nan, 2.0]]

    sym_power_sensor = initialize_array(DatasetType.input, CT.sym_power_sensor, 1)

    asym_power_sensor = initialize_array(DatasetType.input, CT.asym_power_sensor, 1)
    asym_power_sensor[AT.p_measured] = [[np.nan, 2.0, 1.0]]
    asym_power_sensor[AT.q_measured] = [[2.0, 1.0, np.nan]]

    sym_current_sensor = initialize_array(DatasetType.input, CT.sym_current_sensor, 1)

    asym_current_sensor = initialize_array(DatasetType.input, CT.asym_current_sensor, 1)
    asym_current_sensor[AT.i_measured] = [[np.nan, 2.0, 1.0]]
    asym_current_sensor[AT.i_angle_measured] = [[2.0, 1.0, np.nan]]

    fault = initialize_array(DatasetType.input, CT.fault, 1)

    voltage_regulator = initialize_array(DatasetType.input, CT.voltage_regulator, 1)

    data = {
        CT.node: node,
        CT.line: line,
        CT.link: link,
        CT.transformer: transformer,
        CT.three_winding_transformer: three_winding_transformer,
        CT.source: source,
        CT.shunt: shunt,
        CT.sym_load: sym_load,
        CT.sym_gen: sym_gen,
        CT.asym_load: asym_load,
        CT.asym_gen: asym_gen,
        CT.sym_voltage_sensor: sym_voltage_sensor,
        CT.asym_voltage_sensor: asym_voltage_sensor,
        CT.sym_power_sensor: sym_power_sensor,
        CT.asym_power_sensor: asym_power_sensor,
        CT.sym_current_sensor: sym_current_sensor,
        CT.asym_current_sensor: asym_current_sensor,
        CT.fault: fault,
        CT.voltage_regulator: voltage_regulator,
    }
    required_values_errors = validate_required_values(data=data, calculation_type=calculation_type, symmetric=symmetric)

    pf_dependent = calculation_type == CalculationType.power_flow or calculation_type is None
    se_dependent = calculation_type == CalculationType.state_estimation or calculation_type is None
    sc_dependent = calculation_type == CalculationType.short_circuit or calculation_type is None
    asym_dependent = not symmetric

    assert MissingValueError(CT.node, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.node, AT.u_rated, [NaN]) in required_values_errors

    assert MissingValueError(CT.line, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.from_node, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.to_node, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.from_status, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.to_status, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.r1, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.x1, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.c1, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.tan1, [NaN]) in required_values_errors
    assert (MissingValueError(CT.line, AT.r0, [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(CT.line, AT.x0, [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(CT.line, AT.c0, [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(CT.line, AT.tan0, [NaN]) in required_values_errors) == asym_dependent

    # i_n made optional later in lines
    assert MissingValueError(CT.line, AT.i_n, [NaN]) not in required_values_errors

    assert MissingValueError(CT.link, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.link, AT.from_node, [NaN]) in required_values_errors
    assert MissingValueError(CT.link, AT.to_node, [NaN]) in required_values_errors
    assert MissingValueError(CT.link, AT.from_status, [NaN]) in required_values_errors
    assert MissingValueError(CT.link, AT.to_status, [NaN]) in required_values_errors

    assert MissingValueError(CT.transformer, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.from_node, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.to_node, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.from_status, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.to_status, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.u1, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.u2, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.sn, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.uk, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.pk, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.i0, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.p0, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.winding_from, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.winding_to, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.clock, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.tap_side, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.tap_min, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.tap_max, [NaN]) in required_values_errors
    assert MissingValueError(CT.transformer, AT.tap_size, [NaN]) in required_values_errors

    assert MissingValueError(CT.three_winding_transformer, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.node_1, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.node_2, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.node_3, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.status_1, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.status_2, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.status_3, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.u1, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.u2, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.u3, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.sn_1, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.sn_2, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.sn_3, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.uk_12, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.uk_13, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.uk_23, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.pk_12, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.pk_13, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.pk_23, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.i0, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.p0, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.winding_1, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.winding_2, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.winding_3, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.clock_12, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.clock_13, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.tap_side, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.tap_min, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.tap_max, [NaN]) in required_values_errors
    assert MissingValueError(CT.three_winding_transformer, AT.tap_size, [NaN]) in required_values_errors

    assert MissingValueError(CT.source, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.source, AT.node, [NaN]) in required_values_errors
    assert MissingValueError(CT.source, AT.status, [NaN]) in required_values_errors
    assert (MissingValueError(CT.source, AT.u_ref, [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(CT.shunt, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.shunt, AT.node, [NaN]) in required_values_errors
    assert MissingValueError(CT.shunt, AT.status, [NaN]) in required_values_errors
    assert MissingValueError(CT.shunt, AT.g1, [NaN]) in required_values_errors
    assert MissingValueError(CT.shunt, AT.b1, [NaN]) in required_values_errors
    assert (MissingValueError(CT.shunt, AT.g0, [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(CT.shunt, AT.b0, [NaN]) in required_values_errors) == asym_dependent

    assert MissingValueError(CT.sym_load, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_load, AT.node, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_load, AT.status, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_load, AT.type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.sym_load, AT.p_specified, [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(CT.sym_load, AT.q_specified, [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(CT.sym_gen, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_gen, AT.node, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_gen, AT.status, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_gen, AT.type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.sym_gen, AT.p_specified, [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(CT.sym_gen, AT.q_specified, [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(CT.asym_load, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_load, AT.node, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_load, AT.status, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_load, AT.type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.asym_load, AT.p_specified, [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(CT.asym_load, AT.q_specified, [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(CT.asym_gen, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_gen, AT.node, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_gen, AT.status, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_gen, AT.type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.asym_gen, AT.p_specified, [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(CT.asym_gen, AT.q_specified, [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(CT.sym_voltage_sensor, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_voltage_sensor, AT.measured_object, [NaN]) in required_values_errors
    assert (MissingValueError(CT.sym_voltage_sensor, AT.u_sigma, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.sym_voltage_sensor, AT.u_measured, [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError(CT.asym_voltage_sensor, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_voltage_sensor, AT.measured_object, [NaN]) in required_values_errors
    assert (MissingValueError(CT.asym_voltage_sensor, AT.u_sigma, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.asym_voltage_sensor, AT.u_measured, [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError(CT.sym_power_sensor, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_power_sensor, AT.measured_object, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_power_sensor, AT.measured_terminal_type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.sym_power_sensor, AT.power_sigma, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.sym_power_sensor, AT.p_measured, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.sym_power_sensor, AT.q_measured, [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError(CT.asym_power_sensor, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_power_sensor, AT.measured_object, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_power_sensor, AT.measured_terminal_type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.asym_power_sensor, AT.power_sigma, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.asym_power_sensor, AT.p_measured, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.asym_power_sensor, AT.q_measured, [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError(CT.sym_current_sensor, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_current_sensor, AT.measured_object, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_current_sensor, AT.measured_terminal_type, [NaN]) in required_values_errors
    assert MissingValueError(CT.sym_current_sensor, AT.angle_measurement_type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.sym_current_sensor, AT.i_sigma, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.sym_current_sensor, AT.i_angle_sigma, [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError(CT.sym_current_sensor, AT.i_measured, [NaN]) in required_values_errors) == se_dependent
    assert (
        MissingValueError(CT.sym_current_sensor, AT.i_angle_measured, [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(CT.asym_current_sensor, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_current_sensor, AT.measured_object, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_current_sensor, AT.measured_terminal_type, [NaN]) in required_values_errors
    assert MissingValueError(CT.asym_current_sensor, AT.angle_measurement_type, [NaN]) in required_values_errors
    assert (MissingValueError(CT.asym_current_sensor, AT.i_sigma, [NaN]) in required_values_errors) == se_dependent
    assert (
        MissingValueError(CT.asym_current_sensor, AT.i_angle_sigma, [NaN]) in required_values_errors
    ) == se_dependent
    assert (MissingValueError(CT.asym_current_sensor, AT.i_measured, [NaN]) in required_values_errors) == se_dependent
    assert (
        MissingValueError(CT.asym_current_sensor, AT.i_angle_measured, [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(CT.fault, AT.id, [NaN]) in required_values_errors
    assert (MissingValueError(CT.fault, AT.status, [NaN]) in required_values_errors) == sc_dependent
    assert (MissingValueError(CT.fault, AT.fault_type, [NaN]) in required_values_errors) == sc_dependent

    assert MissingValueError(CT.voltage_regulator, AT.id, [NaN]) in required_values_errors
    assert MissingValueError(CT.voltage_regulator, AT.status, [NaN]) in required_values_errors
    assert MissingValueError(CT.voltage_regulator, AT.regulated_object, [NaN]) in required_values_errors
    assert (MissingValueError(CT.voltage_regulator, AT.u_ref, [NaN]) in required_values_errors) == pf_dependent


def test_validate_required_values_asym_calculation():
    line = initialize_array(DatasetType.input, CT.line, 1)
    shunt = initialize_array(DatasetType.input, CT.shunt, 1)

    data = {CT.line: line, CT.shunt: shunt}
    required_values_errors = validate_required_values(data=data, symmetric=False)

    assert MissingValueError(CT.line, AT.r0, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.x0, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.c0, [NaN]) in required_values_errors
    assert MissingValueError(CT.line, AT.tan0, [NaN]) in required_values_errors

    assert MissingValueError(CT.shunt, AT.g0, [NaN]) in required_values_errors
    assert MissingValueError(CT.shunt, AT.b0, [NaN]) in required_values_errors


@pytest.mark.parametrize("fault_types", product(list(FaultType), list(FaultType)))
def test_validate_fault_sc_calculation(fault_types):
    line = initialize_array(DatasetType.input, CT.line, 1)
    shunt = initialize_array(DatasetType.input, CT.shunt, 1)
    fault = initialize_array(DatasetType.input, CT.fault, 2)
    fault[AT.fault_type] = fault_types

    data = {CT.line: line, CT.shunt: shunt, CT.fault: fault}
    required_values_errors = validate_required_values(data=data, calculation_type=CalculationType.short_circuit)

    asym_sc_calculation = np.any(
        list(fault_type not in (FaultType.three_phase, FaultType.nan) for fault_type in fault_types)
    )

    assert (MissingValueError(CT.line, AT.r0, [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(CT.line, AT.x0, [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(CT.line, AT.c0, [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(CT.line, AT.tan0, [NaN]) in required_values_errors) == asym_sc_calculation

    assert (MissingValueError(CT.shunt, AT.g0, [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(CT.shunt, AT.b0, [NaN]) in required_values_errors) == asym_sc_calculation


def test_validate_values():
    # Create invalid nodes and lines
    node = initialize_array(DatasetType.input, CT.node, 3)
    line = initialize_array(DatasetType.input, CT.line, 3)

    # Validate nodes and lines individually
    node_errors = validate_values({CT.node: node})
    line_errors = validate_values({CT.line: line})

    # Validate nodes and lines combined
    both_errors = validate_values({CT.node: node, CT.line: line})

    # The errors should add up (in this simple case)
    assert both_errors == node_errors + line_errors


def test_validate_values__calculation_types():
    # Create invalid sensor
    sym_voltage_sensor = initialize_array(DatasetType.input, CT.sym_voltage_sensor, 3)
    all_errors = validate_values({CT.sym_voltage_sensor: sym_voltage_sensor})
    power_flow_errors = validate_values(
        {CT.sym_voltage_sensor: sym_voltage_sensor}, calculation_type=CalculationType.power_flow
    )
    state_estimation_errors = validate_values(
        {CT.sym_voltage_sensor: sym_voltage_sensor}, calculation_type=CalculationType.state_estimation
    )

    assert not power_flow_errors
    assert all_errors == state_estimation_errors


@pytest.mark.parametrize(
    ("sensor_type", "parameter"),
    [
        (CT.sym_voltage_sensor, AT.u_sigma),
        (CT.asym_voltage_sensor, AT.u_sigma),
        (CT.sym_power_sensor, AT.power_sigma),
        (CT.sym_power_sensor, AT.p_sigma),
        (CT.sym_power_sensor, AT.q_sigma),
        (CT.asym_power_sensor, AT.power_sigma),
        (CT.asym_power_sensor, AT.p_sigma),
        (CT.asym_power_sensor, AT.q_sigma),
        (CT.sym_current_sensor, AT.i_sigma),
        (CT.sym_current_sensor, AT.i_angle_sigma),
        (CT.asym_current_sensor, AT.i_sigma),
        (CT.asym_current_sensor, AT.i_angle_sigma),
    ],
)
def test_validate_values__infinite_sigmas(sensor_type, parameter):
    sensor_array = initialize_array(DatasetType.input, sensor_type, 3)
    sensor_array[parameter] = np.inf
    all_errors = validate_values({sensor_type: sensor_array})

    for error in all_errors:
        assert not isinstance(error, InfinityError)


@pytest.mark.parametrize(
    ("sensor_type", "values", "error_types"),
    [
        pytest.param(
            CT.sym_power_sensor,
            [[np.nan, np.nan], [], []],
            [InvalidIdError, NotUniqueError],
            id="both nan - sym_power_sensor",
        ),
        pytest.param(
            CT.sym_power_sensor,
            [[np.inf, np.inf], [], []],
            [InvalidIdError, NotUniqueError],
            id="both inf - sym_power_sensor",
        ),
        pytest.param(
            CT.sym_power_sensor,
            [[0.1, np.nan], [], []],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="q_sigma nan - sym_power_sensor",
        ),
        pytest.param(
            CT.sym_power_sensor,
            [[0.1, np.inf], [], []],
            [InvalidIdError, NotUniqueError],
            id="q_sigma inf - sym_power_sensor",
        ),
        pytest.param(
            CT.sym_power_sensor,
            [[np.nan, 0.1], [], []],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="p_sigma nan - sym_power_sensor",
        ),
        pytest.param(
            CT.sym_power_sensor,
            [[np.inf, 0.1], [], []],
            [InvalidIdError, NotUniqueError],
            id="p_sigma inf - sym_power_sensor",
        ),
        pytest.param(
            CT.sym_power_sensor,
            [[0.1, 0.1], [], []],
            [InvalidIdError, NotUniqueError],
            id="no nan or inf - sym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [[], [[np.nan, np.nan, np.nan]] * 3, [[np.nan, np.nan, np.nan]] * 3],
            [InvalidIdError, NotUniqueError],
            id="all nan - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [[], [[np.inf, np.inf, np.inf]] * 3, [[np.inf, np.inf, np.inf]] * 3],
            [InvalidIdError, NotUniqueError],
            id="all inf - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, np.nan, 0.1], [0.1, np.nan, 0.1], [0.1, 0.1, 0.1]],
                [[np.nan, 0.1, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="mixed nans for some sensor - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, np.inf, 0.1], [0.1, np.inf, 0.1], [0.1, 0.1, 0.1]],
                [[np.inf, 0.1, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError],
            id="mixed infs for some sensor - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [[], [[0.1, np.nan, np.nan]] * 3, [[np.nan, np.nan, np.nan]] * 3],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="all nan except one phase p_sigma - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [[], [[0.1, np.inf, np.inf]] * 3, [[np.inf, np.inf, np.inf]] * 3],
            [InvalidIdError, NotUniqueError],
            id="all inf except one phase p_sigma - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [[], [[np.nan, np.nan, np.nan]] * 3, [[0.1, np.nan, np.nan]] * 3],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="all nan except one phase q_sigma - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [[], [[np.inf, np.inf, np.inf]] * 3, [[0.1, np.inf, np.inf]] * 3],
            [InvalidIdError, NotUniqueError],
            id="all inf except one phase q_sigma - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, 0.1, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[np.nan, np.nan, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="one sensor mixed nans - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, 0.1, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[np.inf, np.inf, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError],
            id="one sensor mixed infs - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[np.nan, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="q_sigma of one sensor nan - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[np.inf, np.inf, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError],
            id="q_sigma of one sensor inf - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[np.nan, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="p_sigma of one sensor nan - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[np.inf, np.inf, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError],
            id="p_sigma of one sensor inf - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[np.nan, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="all nan except one attribute for one phase for one sensor - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[np.inf, np.inf, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, np.inf, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError],
            id="all inf except one attribute for one phase for one sensor - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
            id="all nan except one phase for one sensor - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [
                [],
                [[0.1, np.inf, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, np.inf, np.inf], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError],
            id="all inf except one phase for one sensor - asym_power_sensor",
        ),
        pytest.param(
            CT.asym_power_sensor,
            [[], [[0.1, 0.1, 0.1]] * 3, [[0.1, 0.1, 0.1]] * 3],
            [InvalidIdError, NotUniqueError],
            id="no nan or inf - asym_power_sensor",
        ),
    ],
)
def test_validate_values__bad_p_q_sigma(sensor_type, values, error_types):
    def arbitrary_fill(array, sensor_type, values):
        if sensor_type == CT.sym_power_sensor:
            array[AT.p_sigma] = values[0][0]
            array[AT.q_sigma] = values[0][1]
        else:
            array[AT.p_sigma][0] = values[1][0]
            array[AT.p_sigma][1] = values[1][1]
            array[AT.p_sigma][2] = values[1][2]
            array[AT.q_sigma][0] = values[2][0]
            array[AT.q_sigma][1] = values[2][1]
            array[AT.q_sigma][2] = values[2][2]
        array[AT.id] = [123, 234, 345]

    sensor_array = initialize_array(DatasetType.input, sensor_type, 3)
    arbitrary_fill(sensor_array, sensor_type, values)
    all_errors = validate_values({sensor_type: sensor_array})

    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in error_types)
        assert set(error.ids).issubset(set(sensor_array[AT.id]))


@pytest.mark.parametrize(
    ("values", "error_types"),
    [
        ([[np.nan, np.nan], [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]]], [InvalidIdError]),
        (
            [[0.1, np.nan], [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]]],
            [InvalidIdError, PQSigmaPairError],
        ),
        (
            [[np.nan, np.nan], [[np.nan, 0.1, np.nan], [np.nan, np.nan, np.nan]]],
            [InvalidIdError, PQSigmaPairError],
        ),
        (
            [[np.nan, np.nan], [[np.nan, 0.1, np.nan], [np.nan, 0.1, np.nan]]],
            [InvalidIdError, PQSigmaPairError],
        ),
        (
            [[0.1, 0.1], [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]]],
            [InvalidIdError, PQSigmaPairError],
        ),
        ([[0.1, 0.1], [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]]], [InvalidIdError]),
        ([[np.nan, np.nan], [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1]]], [InvalidIdError]),
        ([[0.1, 0.1], [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1]]], [InvalidIdError]),
    ],
)
def test_validate_values__bad_p_q_sigma_both_components(values, error_types):
    def two_component_data(values):
        node = initialize_array(DatasetType.input, CT.node, 1)
        node[AT.id] = 123
        sym_power_sensor = initialize_array(DatasetType.input, CT.sym_power_sensor, 1)
        sym_power_sensor[AT.p_sigma] = values[0][0]
        sym_power_sensor[AT.q_sigma] = values[0][1]
        sym_power_sensor[AT.id] = 456
        asym_power_sensor = initialize_array(DatasetType.input, CT.asym_power_sensor, 1)
        asym_power_sensor[AT.p_measured] = values[1][0]
        asym_power_sensor[AT.q_measured] = values[1][1]
        asym_power_sensor[AT.id] = 789

        return {
            CT.node: node,
            CT.sym_power_sensor: sym_power_sensor,
            CT.asym_power_sensor: asym_power_sensor,
        }

    data = two_component_data(values)
    all_errors = validate_values(data)
    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in error_types)
        assert (data[error.component][AT.id] == error.ids).all()


def test_validate_values__bad_p_q_sigma_single_component_twice():
    def single_component_twice_data():
        node = initialize_array(DatasetType.input, CT.node, 1)
        node[AT.id] = 123
        sym_power_sensor = initialize_array(DatasetType.input, CT.sym_power_sensor, 2)
        sym_power_sensor[AT.p_sigma] = [np.nan, 0.1]
        sym_power_sensor[AT.q_sigma] = [np.nan, np.nan]
        sym_power_sensor[AT.id] = [456, 789]

        return {
            CT.node: node,
            CT.sym_power_sensor: sym_power_sensor,
        }

    data = single_component_twice_data()
    all_errors = validate_values(data)
    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in [InvalidIdError, PQSigmaPairError])
        if isinstance(error, PQSigmaPairError):
            assert error.ids[0] == data["sym_power_sensor"][AT.id][1]


@pytest.mark.parametrize("measured_terminal_type", MeasuredTerminalType)
@patch("power_grid_model.validation._validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_ids")
def test_validate_generic_power_sensor__all_terminal_types(
    _all_valid_ids: MagicMock, measured_terminal_type: MeasuredTerminalType
):
    # Act
    validate_generic_power_sensor(data={}, component="")  # type: ignore

    # Assert
    _all_valid_ids.assert_any_call(
        ANY, ANY, field=ANY, ref_components=ANY, measured_terminal_type=measured_terminal_type
    )


@pytest.mark.parametrize(
    ("ref_component", "measured_terminal_type"),
    [
        (
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            MeasuredTerminalType.branch_from,
        ),
        (
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            MeasuredTerminalType.branch_to,
        ),
        (CT.source, MeasuredTerminalType.source),
        (CT.shunt, MeasuredTerminalType.shunt),
        ([CT.sym_load, CT.asym_load], MeasuredTerminalType.load),
        ([CT.sym_gen, CT.asym_gen], MeasuredTerminalType.generator),
        (CT.three_winding_transformer, MeasuredTerminalType.branch3_1),
        (CT.three_winding_transformer, MeasuredTerminalType.branch3_2),
        (CT.three_winding_transformer, MeasuredTerminalType.branch3_3),
        (CT.node, MeasuredTerminalType.node),
    ],
)
@patch("power_grid_model.validation._validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_ids")
def test_validate_generic_power_sensor__terminal_types(
    _all_valid_ids: MagicMock,
    ref_component: CT | list[CT],
    measured_terminal_type: MeasuredTerminalType,
):
    # Act
    validate_generic_power_sensor(data={}, component="")  # type: ignore

    # Assert
    _all_valid_ids.assert_any_call(
        ANY, ANY, field=ANY, ref_components=ref_component, measured_terminal_type=measured_terminal_type
    )


@pytest.mark.parametrize(
    "measured_terminal_type",
    [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.branch_to,
        MeasuredTerminalType.branch3_1,
        MeasuredTerminalType.branch3_2,
        MeasuredTerminalType.branch3_3,
    ],
)
@patch("power_grid_model.validation._validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_in_valid_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_same_current_angle_measurement_type_on_terminal", new=MagicMock())
@patch(
    "power_grid_model.validation._validation._any_voltage_angle_measurement_if_global_current_measurement",
    new=MagicMock(),
)
@patch("power_grid_model.validation._validation._all_valid_ids")
def test_validate_generic_current_sensor__all_terminal_types(
    _all_valid_ids: MagicMock, measured_terminal_type: MeasuredTerminalType
):
    # Act
    validate_generic_current_sensor(data={}, component="")  # type: ignore

    # Assert
    _all_valid_ids.assert_any_call(
        ANY,
        ANY,
        field=ANY,
        ref_components=ANY,
        measured_terminal_type=measured_terminal_type,
    )


@pytest.mark.parametrize("current_sensor_type", [CT.sym_current_sensor, CT.asym_current_sensor])
@pytest.mark.parametrize(
    ("measured_terminal_type", "supported"),
    [
        (MeasuredTerminalType.branch_from, True),
        (MeasuredTerminalType.branch_to, True),
        (MeasuredTerminalType.branch3_1, True),
        (MeasuredTerminalType.branch3_2, True),
        (MeasuredTerminalType.branch3_3, True),
        (MeasuredTerminalType.source, False),
        (MeasuredTerminalType.shunt, False),
        (MeasuredTerminalType.load, False),
        (MeasuredTerminalType.generator, False),
        (MeasuredTerminalType.node, False),
    ],
)
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock(return_value=[]))
@patch("power_grid_model.validation._validation._all_valid_ids", new=MagicMock(return_value=[]))
@patch(
    "power_grid_model.validation._validation._all_same_current_angle_measurement_type_on_terminal",
    new=MagicMock(return_value=[]),
)
@patch(
    "power_grid_model.validation._validation._any_voltage_angle_measurement_if_global_current_measurement",
    new=MagicMock(return_value=[]),
)
@patch(
    "power_grid_model.validation._validation._all_same_current_angle_measurement_type_on_terminal",
    new=MagicMock(return_value=[]),
)
def test_validate_generic_current_sensor__only_branches_supported(
    current_sensor_type: CT, measured_terminal_type: MeasuredTerminalType, supported: bool
):
    current_sensor_data = initialize_array(DatasetType.input, current_sensor_type, 1)
    current_sensor_data[AT.id] = 1
    current_sensor_data[AT.measured_terminal_type] = measured_terminal_type

    result = validate_generic_current_sensor(
        data={current_sensor_type: current_sensor_data}, component=current_sensor_type
    )

    if supported:
        assert not result
    else:
        assert result == [
            UnsupportedMeasuredTerminalType(
                current_sensor_type,
                AT.measured_terminal_type,
                [1],
                [
                    MeasuredTerminalType.branch_from,
                    MeasuredTerminalType.branch_to,
                    MeasuredTerminalType.branch3_1,
                    MeasuredTerminalType.branch3_2,
                    MeasuredTerminalType.branch3_3,
                ],
            )
        ]


@pytest.mark.parametrize(
    ("ref_component", "measured_terminal_type"),
    [
        (
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            MeasuredTerminalType.branch_from,
        ),
        (
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            MeasuredTerminalType.branch_to,
        ),
        (CT.three_winding_transformer, MeasuredTerminalType.branch3_1),
        (CT.three_winding_transformer, MeasuredTerminalType.branch3_2),
        (CT.three_winding_transformer, MeasuredTerminalType.branch3_3),
    ],
)
@patch("power_grid_model.validation._validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation._validation._all_in_valid_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_same_current_angle_measurement_type_on_terminal", new=MagicMock())
@patch(
    "power_grid_model.validation._validation._any_voltage_angle_measurement_if_global_current_measurement",
    new=MagicMock(),
)
@patch("power_grid_model.validation._validation._all_valid_ids")
def test_validate_generic_current_sensor__terminal_types(
    _all_valid_ids: MagicMock,
    ref_component: CT | list[CT],
    measured_terminal_type: MeasuredTerminalType,
):
    # Act
    validate_generic_current_sensor(data={}, component="")  # type: ignore

    # Assert
    _all_valid_ids.assert_any_call(
        ANY,
        ANY,
        field=ANY,
        ref_components=ref_component,
        measured_terminal_type=measured_terminal_type,
    )


@patch("power_grid_model.validation._validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation._validation._all_in_valid_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_ids", new=MagicMock())
@patch(
    "power_grid_model.validation._validation._any_voltage_angle_measurement_if_global_current_measurement",
    new=MagicMock(),
)
@patch("power_grid_model.validation._validation._all_same_current_angle_measurement_type_on_terminal")
def test_validate_generic_current_sensor__angle_measurement_type_mixing(
    _all_same_current_angle_measurement_type_on_terminal,
):
    # Act
    validate_generic_current_sensor(data={}, component="")  # type: ignore

    # Assert
    _all_same_current_angle_measurement_type_on_terminal.assert_any_call(
        ANY,
        ANY,
        measured_object_field=AT.measured_object,
        measured_terminal_type_field=AT.measured_terminal_type,
        angle_measurement_type_field=AT.angle_measurement_type,
    )


@patch("power_grid_model.validation._validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation._validation._all_in_valid_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_ids", new=MagicMock())
@patch("power_grid_model.validation._validation._all_same_current_angle_measurement_type_on_terminal", new=MagicMock())
@patch("power_grid_model.validation._validation._any_voltage_angle_measurement_if_global_current_measurement")
def test_any_voltage_angle_measurement_if_global_current_measurement(
    any_voltage_angle_measurement_if_global_current_measurement,
):
    # Act
    validate_generic_current_sensor(data={}, component="")

    # Assert
    any_voltage_angle_measurement_if_global_current_measurement.assert_any_call(
        ANY,
        ANY,
        angle_measurement_type_filter=(AT.angle_measurement_type, AngleMeasurementType.global_angle),
        voltage_sensor_u_angle_measured={
            CT.sym_voltage_sensor: AT.u_angle_measured,
            CT.asym_voltage_sensor: AT.u_angle_measured,
        },
    )


@pytest.mark.parametrize("power_sensor_type", [CT.sym_power_sensor, CT.asym_power_sensor])
@pytest.mark.parametrize("current_sensor_type", [CT.sym_current_sensor, CT.asym_current_sensor])
@patch("power_grid_model.validation._validation._all_finite", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_node", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_line", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_asym_line", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_branch", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_branch", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_transformer", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_three_winding_transformer", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_source", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_load_gen", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_load_gen", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_load_gen", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_load_gen", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_shunt", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_voltage_sensor", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_voltage_sensor", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_power_sensor", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_power_sensor", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_current_sensor", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_generic_current_sensor", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_fault", new=MagicMock())
@patch("power_grid_model.validation._validation.validate_transformer_tap_regulator", new=MagicMock())
@patch("power_grid_model.validation._validation._all_same_sensor_type_on_same_terminal")
def test_no_power_and_current_sensor_on_same_terminal(
    _all_same_sensor_type_on_same_terminal,
    power_sensor_type: CT,
    current_sensor_type: CT,
):
    # Act
    validate_values({power_sensor_type: None, current_sensor_type: None})

    # Assert
    _all_same_sensor_type_on_same_terminal.assert_any_call(
        ANY,
        power_sensor_type=power_sensor_type,
        current_sensor_type=current_sensor_type,
        measured_object_field=AT.measured_object,
        measured_terminal_type_field=AT.measured_terminal_type,
    )


def test_power_sigma_or_p_q_sigma():
    # node
    node = initialize_array(DatasetType.input, CT.node, 2)
    node[AT.id] = np.array([0, 3])
    node[AT.u_rated] = [10.5e3, 10.5e3]

    # line
    line = initialize_array(DatasetType.input, CT.line, 1)
    line[AT.id] = [2]
    line[AT.from_node] = [0]
    line[AT.to_node] = [3]
    line[AT.from_status] = [1]
    line[AT.to_status] = [1]
    line[AT.r1] = [0.001]
    line[AT.x1] = [0.02]
    line[AT.c1] = [0.0]
    line[AT.tan1] = [0.0]
    line[AT.i_n] = [1000.0]

    # load
    sym_load = initialize_array(DatasetType.input, CT.sym_load, 2)
    sym_load[AT.id] = [4, 9]
    sym_load[AT.node] = [3, 0]
    sym_load[AT.status] = [1, 1]
    sym_load[AT.type] = [LoadGenType.const_power, LoadGenType.const_power]
    sym_load[AT.p_specified] = [1e6, 1e6]
    sym_load[AT.q_specified] = [-1e6, -1e6]

    # source
    source = initialize_array(DatasetType.input, CT.source, 1)
    source[AT.id] = [1]
    source[AT.node] = [0]
    source[AT.status] = [1]
    source[AT.u_ref] = [1.0]

    # voltage sensor
    voltage_sensor = initialize_array(DatasetType.input, CT.sym_voltage_sensor, 1)
    voltage_sensor[AT.id] = 5
    voltage_sensor[AT.measured_object] = 0
    voltage_sensor[AT.u_sigma] = [100.0]
    voltage_sensor[AT.u_measured] = [10.5e3]

    # power sensor
    sym_power_sensor = initialize_array(DatasetType.input, CT.sym_power_sensor, 3)
    sym_power_sensor[AT.id] = [6, 7, 8]
    sym_power_sensor[AT.measured_object] = [2, 4, 9]
    sym_power_sensor[AT.measured_terminal_type] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.load,
        MeasuredTerminalType.load,
    ]
    sym_power_sensor[AT.p_measured] = [1e6, -1e6, -1e6]
    sym_power_sensor[AT.q_measured] = [1e6, -1e6, -1e6]
    sym_power_sensor[AT.power_sigma] = [np.nan, 1e9, 1e9]
    sym_power_sensor[AT.p_sigma] = [1e4, np.nan, 1e4]
    sym_power_sensor[AT.q_sigma] = [1e9, np.nan, 1e9]

    # power sensor
    asym_power_sensor = initialize_array(DatasetType.input, CT.asym_power_sensor, 4)
    asym_power_sensor[AT.id] = [66, 77, 88, 99]
    asym_power_sensor[AT.measured_object] = [2, 4, 9, 9]
    asym_power_sensor[AT.measured_terminal_type] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.load,
        MeasuredTerminalType.load,
        MeasuredTerminalType.load,
    ]
    asym_power_sensor[AT.p_measured] = [
        [1e6, 1e6, 1e6],
        [-1e6, -1e6, -1e6],
        [-1e6, -1e6, -1e6],
        [-1e6, -1e6, -1e6],
    ]
    asym_power_sensor[AT.q_measured] = [
        [1e6, 1e6, 1e6],
        [-1e6, -1e6, -1e6],
        [-1e6, -1e6, -1e6],
        [-1e6, -1e6, -1e6],
    ]
    asym_power_sensor[AT.power_sigma] = [np.nan, 1e9, 1e9, 1e9]
    asym_power_sensor[AT.p_sigma] = [
        [1e4, 1e4, 1e4],
        [np.nan, np.nan, np.nan],
        [1e4, 1e4, 1e4],
        [1e4, 1e4, 1e4],
    ]
    asym_power_sensor[AT.q_sigma] = [
        [1e9, 1e9, 1e9],
        [np.nan, np.nan, np.nan],
        [1e9, 1e4, 1e4],
        [1e9, 1e4, 1e4],
    ]

    # all
    input_data = {
        CT.node: node,
        CT.line: line,
        CT.sym_load: sym_load,
        CT.source: source,
        CT.sym_voltage_sensor: voltage_sensor,
        CT.sym_power_sensor: sym_power_sensor,
        CT.asym_power_sensor: asym_power_sensor,
    }

    assert_valid_input_data(input_data=input_data, calculation_type=CalculationType.state_estimation)

    np.testing.assert_array_equal(sym_power_sensor[AT.power_sigma], [np.nan, 1e9, 1e9])
    np.testing.assert_array_equal(sym_power_sensor[AT.p_sigma], [1e4, np.nan, 1e4])
    np.testing.assert_array_equal(sym_power_sensor[AT.q_sigma], [1e9, np.nan, 1e9])
    np.testing.assert_array_equal(asym_power_sensor[AT.power_sigma], [np.nan, 1e9, 1e9, 1e9])
    np.testing.assert_array_equal(
        asym_power_sensor[AT.p_sigma],
        [[1e4, 1e4, 1e4], [np.nan, np.nan, np.nan], [1e4, 1e4, 1e4], [1e4, 1e4, 1e4]],
    )
    np.testing.assert_array_equal(
        asym_power_sensor[AT.q_sigma],
        [[1e9, 1e9, 1e9], [np.nan, np.nan, np.nan], [1e9, 1e4, 1e4], [1e9, 1e4, 1e4]],
    )

    # bad weather
    bad_input_data = copy.deepcopy(input_data)
    bad_sym_power_sensor = bad_input_data[CT.sym_power_sensor]
    bad_sym_power_sensor[AT.power_sigma] = [np.nan, np.nan, 1e9]
    bad_sym_power_sensor[AT.p_sigma] = [np.nan, np.nan, 1e4]
    bad_sym_power_sensor[AT.q_sigma] = [np.nan, 1e9, np.nan]
    errors = validate_input_data(input_data=bad_input_data, calculation_type=CalculationType.state_estimation)
    n_sym_input_validation_errors = 4
    assert len(errors) == n_sym_input_validation_errors
    assert errors == [
        MissingValueError(CT.sym_power_sensor, AT.power_sigma, [6]),
        MissingValueError(CT.sym_power_sensor, AT.p_sigma, [7]),
        MissingValueError(CT.sym_power_sensor, AT.q_sigma, [8]),
        PQSigmaPairError(CT.sym_power_sensor, (AT.p_sigma, AT.q_sigma), [7, 8]),
    ]

    np.testing.assert_array_equal(bad_sym_power_sensor[AT.power_sigma], [np.nan, np.nan, 1e9])
    np.testing.assert_array_equal(bad_sym_power_sensor[AT.p_sigma], [np.nan, np.nan, 1e4])
    np.testing.assert_array_equal(bad_sym_power_sensor[AT.q_sigma], [np.nan, 1e9, np.nan])

    # bad weather
    bad_input_data = copy.deepcopy(input_data)
    bad_asym_power_sensor = bad_input_data[CT.asym_power_sensor]
    bad_asym_power_sensor[AT.power_sigma] = [np.nan, np.nan, 1e9, np.nan]
    bad_asym_power_sensor[AT.p_sigma] = [
        [np.nan, np.nan, np.nan],
        [np.nan, np.nan, np.nan],
        [1e4, np.nan, np.nan],
        [1e4, np.nan, np.nan],
    ]
    bad_asym_power_sensor[AT.q_sigma] = [
        [np.nan, np.nan, np.nan],
        [1e9, 1e9, 1e9],
        [np.nan, 1e4, 1e4],
        [np.nan, 1e4, 1e4],
    ]
    errors = validate_input_data(input_data=bad_input_data, calculation_type=CalculationType.state_estimation)
    n_asym_input_validation_errors = 4
    assert len(errors) == n_asym_input_validation_errors
    assert errors == [
        MissingValueError(CT.asym_power_sensor, AT.power_sigma, [66]),
        MissingValueError(CT.asym_power_sensor, AT.p_sigma, [77, 88, 99]),
        MissingValueError(CT.asym_power_sensor, AT.q_sigma, [88, 99]),
        PQSigmaPairError(CT.asym_power_sensor, (AT.p_sigma, AT.q_sigma), [77, 88, 99]),
    ]

    np.testing.assert_array_equal(bad_asym_power_sensor[AT.power_sigma], [np.nan, np.nan, 1e9, np.nan])
    np.testing.assert_array_equal(
        bad_asym_power_sensor[AT.p_sigma],
        [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan], [1e4, np.nan, np.nan], [1e4, np.nan, np.nan]],
    )
    np.testing.assert_array_equal(
        bad_asym_power_sensor[AT.q_sigma],
        [[np.nan, np.nan, np.nan], [1e9, 1e9, 1e9], [np.nan, 1e4, 1e4], [np.nan, 1e4, 1e4]],
    )


def test_all_default_values():
    """
    Initialize all components that have attributes that have default values, without setting values for
    those attributes.
    """
    node = initialize_array(DatasetType.input, CT.node, 3)
    node[AT.id] = [0, 1, 2]
    node[AT.u_rated] = [50.0e3, 20.0e3, 10.5e3]

    source = initialize_array(DatasetType.input, CT.source, 1)
    source[AT.id] = [3]
    source[AT.node] = [2]
    source[AT.status] = [1]
    source[AT.u_ref] = [1.0]

    transformer = initialize_array(DatasetType.input, CT.transformer, 1)
    transformer[AT.id] = [4]
    transformer[AT.from_node] = [0]
    transformer[AT.to_node] = [2]
    transformer[AT.from_status] = [1]
    transformer[AT.to_status] = [1]
    transformer[AT.u1] = [50e3]
    transformer[AT.u2] = [10.5e3]
    transformer[AT.sn] = [1e5]
    transformer[AT.uk] = [0.1]
    transformer[AT.pk] = [1e3]
    transformer[AT.i0] = [1.0e-6]
    transformer[AT.p0] = [0.1]
    transformer[AT.winding_from] = [2]
    transformer[AT.winding_to] = [1]
    transformer[AT.clock] = [5]
    transformer[AT.tap_side] = [0]
    transformer[AT.tap_min] = [-11]
    transformer[AT.tap_max] = [9]
    transformer[AT.tap_size] = [100]

    three_winding_transformer = initialize_array(DatasetType.input, CT.three_winding_transformer, 1)
    three_winding_transformer[AT.id] = [6]
    three_winding_transformer[AT.node_1] = [0]
    three_winding_transformer[AT.node_2] = [1]
    three_winding_transformer[AT.node_3] = [2]
    three_winding_transformer[AT.status_1] = [1]
    three_winding_transformer[AT.status_2] = [1]
    three_winding_transformer[AT.status_3] = [1]
    three_winding_transformer[AT.u1] = [50.0e3]
    three_winding_transformer[AT.u2] = [20.0e3]
    three_winding_transformer[AT.u3] = [10.5e3]
    three_winding_transformer[AT.sn_1] = [1e5]
    three_winding_transformer[AT.sn_2] = [1e5]
    three_winding_transformer[AT.sn_3] = [1e5]
    three_winding_transformer[AT.uk_12] = [0.09]
    three_winding_transformer[AT.uk_13] = [0.06]
    three_winding_transformer[AT.uk_23] = [0.06]
    three_winding_transformer[AT.pk_12] = [1e3]
    three_winding_transformer[AT.pk_13] = [1e3]
    three_winding_transformer[AT.pk_23] = [1e3]
    three_winding_transformer[AT.i0] = [0]
    three_winding_transformer[AT.p0] = [0]
    three_winding_transformer[AT.winding_1] = [2]
    three_winding_transformer[AT.winding_2] = [1]
    three_winding_transformer[AT.winding_3] = [1]
    three_winding_transformer[AT.clock_12] = [5]
    three_winding_transformer[AT.clock_13] = [-7]  # supports periodic clock input
    three_winding_transformer[AT.tap_side] = [0]
    three_winding_transformer[AT.tap_min] = [-10]
    three_winding_transformer[AT.tap_max] = [10]
    three_winding_transformer[AT.tap_size] = [1380]

    fault = initialize_array(DatasetType.input, CT.fault, 1)
    fault[AT.id] = [5]
    fault[AT.status] = [1]
    fault[AT.fault_object] = [0]

    input_data = {
        CT.node: node,
        CT.transformer: transformer,
        CT.three_winding_transformer: three_winding_transformer,
        CT.source: source,
        CT.fault: fault,
    }

    assert_valid_input_data(input_data=input_data, calculation_type=CalculationType.power_flow)


@patch("power_grid_model.validation._validation.validate_transformer", new=MagicMock(return_value=[]))
@patch("power_grid_model.validation._validation.validate_three_winding_transformer", new=MagicMock(return_value=[]))
def test_validate_values__tap_regulator_control_side():
    # Create valid transformer
    transformer = initialize_array(DatasetType.input, CT.transformer, 4)
    transformer[AT.id] = [0, 1, 2, 3]
    transformer[AT.tap_side] = [
        BranchSide.from_side,
        BranchSide.from_side,
        BranchSide.from_side,
        BranchSide.from_side,
    ]

    # Create valid three winding transformer
    three_winding_transformer = initialize_array(DatasetType.input, CT.three_winding_transformer, 3)
    three_winding_transformer[AT.id] = [4, 5, 6]
    three_winding_transformer[AT.tap_side] = [Branch3Side.side_1, Branch3Side.side_1, Branch3Side.side_1]

    # Create invalid regulator
    transformer_tap_regulator = initialize_array(DatasetType.input, CT.transformer_tap_regulator, 7)
    transformer_tap_regulator[AT.id] = np.arange(7, 14)
    transformer_tap_regulator[AT.status] = 1
    transformer_tap_regulator[AT.regulated_object] = np.arange(7)
    transformer_tap_regulator[AT.control_side] = [
        BranchSide.to_side,  # OK
        BranchSide.from_side,  # OK
        Branch3Side.side_3,  # branch3 provided but it is a 2-winding transformer (invalid)
        10,  # control side entirely out of range (invalid)
        Branch3Side.side_3,  # OK
        Branch3Side.side_1,  # OK
        10,  # control side entirely out of range (invalid)
    ]

    input_data = {
        CT.transformer: transformer,
        CT.three_winding_transformer: three_winding_transformer,
        CT.transformer_tap_regulator: transformer_tap_regulator,
    }
    all_errors = validate_values(input_data)
    power_flow_errors = validate_values(input_data, calculation_type=CalculationType.power_flow)
    state_estimation_errors = validate_values(input_data, calculation_type=CalculationType.state_estimation)

    assert power_flow_errors == all_errors
    assert not state_estimation_errors

    n_input_validation_errors = 3

    assert len(all_errors) == n_input_validation_errors
    assert (
        InvalidEnumValueError(CT.transformer_tap_regulator, AT.control_side, [10, 13], [BranchSide, Branch3Side])
        in all_errors
    )
    assert (
        InvalidAssociatedEnumValueError(
            CT.transformer_tap_regulator,
            [AT.control_side, AT.regulated_object],
            [9, 10],
            [BranchSide],
        )
        in all_errors
    )
    assert (
        InvalidAssociatedEnumValueError(
            CT.transformer_tap_regulator,
            [AT.control_side, AT.regulated_object],
            [13],
            [Branch3Side],
        )
        in all_errors
    )
