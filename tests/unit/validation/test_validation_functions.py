# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import copy
from itertools import product
from typing import cast
from unittest.mock import ANY, MagicMock, patch

import numpy as np
import pytest

from power_grid_model import CalculationType, LoadGenType, MeasuredTerminalType, initialize_array, power_grid_meta_data
from power_grid_model._core.dataset_definitions import ComponentType, DatasetType
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

NaN = cast(int, power_grid_meta_data[DatasetType.input][ComponentType.node].nans["id"])


def test_assert_valid_data_structure():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 3)
    line_input = initialize_array(DatasetType.input, ComponentType.line, 3)
    node_update = initialize_array(DatasetType.update, ComponentType.node, 3)
    line_update = initialize_array(DatasetType.update, ComponentType.line, 3)

    # Input data: Assertion ok
    assert_valid_data_structure({ComponentType.node: node_input, ComponentType.line: line_input}, DatasetType.input)

    # Update data: Assertion ok
    assert_valid_data_structure({ComponentType.node: node_update, ComponentType.line: line_update}, DatasetType.update)

    # There is no such thing as 'output' data
    with pytest.raises(KeyError, match=r"output"):
        assert_valid_data_structure({ComponentType.node: node_input, ComponentType.line: line_input}, "output")

    # Input data is not valid update data
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure(
            {ComponentType.node: node_input, ComponentType.line: line_input}, DatasetType.update
        )

    # Update data is not valid input data
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure(
            {ComponentType.node: node_update, ComponentType.line: line_update}, DatasetType.input
        )

    # A normal numpy array is not valid input data
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)])
    with pytest.raises(TypeError, match=r"Unexpected Numpy array"):
        assert_valid_data_structure({ComponentType.node: node_dummy, ComponentType.line: line_input}, DatasetType.input)

    # A structured numpy array, with wrong data type for u_rated f4 != f8, is not valid input data
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)], dtype=[("id", "i4"), ("u_rated", "f4")])
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({ComponentType.node: node_dummy, ComponentType.line: line_input}, DatasetType.input)

    # A structured numpy array, with correct data types is not a valid, is still not valid input data.
    # N.B. It is not 'aligned'
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)], dtype=[("id", "i4"), ("u_rated", "f8")])
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({ComponentType.node: node_dummy, ComponentType.line: line_input}, DatasetType.input)

    # Invalid component type
    input_with_wrong_component = {ComponentType.node: node_input, "some_random_component": line_input}
    with pytest.raises(KeyError, match="Unknown component 'some_random_component' in DatasetType.input data."):
        assert_valid_data_structure(input_with_wrong_component, DatasetType.input)

    input_with_wrong_data_type = {ComponentType.node: node_input, ComponentType.line: [1, 2, 3]}
    with pytest.raises(TypeError, match="Unexpected data type list for 'ComponentType.line' DatasetType.input data "):
        assert_valid_data_structure(input_with_wrong_data_type, DatasetType.input)


def test_validate_unique_ids_across_components():
    node = initialize_array(DatasetType.input, ComponentType.node, 3)
    node["id"] = [1, 2, 3]

    line = initialize_array(DatasetType.input, ComponentType.line, 3)
    line["id"] = [4, 5, 3]

    transformer = initialize_array(DatasetType.input, ComponentType.transformer, 3)
    transformer["id"] = [1, 6, 7]

    source = initialize_array(DatasetType.input, ComponentType.source, 3)
    source["id"] = [8, 9, 10]

    input_data = {
        ComponentType.node: node,
        ComponentType.line: line,
        ComponentType.transformer: transformer,
        ComponentType.source: source,
    }

    unique_id_errors = validate_unique_ids_across_components(input_data)

    assert (
        MultiComponentNotUniqueError(
            [(ComponentType.node, "id"), (ComponentType.line, "id"), (ComponentType.transformer, "id")],
            [(ComponentType.node, 1), (ComponentType.node, 3), (ComponentType.line, 3), (ComponentType.transformer, 1)],
        )
        in unique_id_errors
    )
    n_non_unique_ids = 4
    assert len(unique_id_errors[0].ids) == n_non_unique_ids


def test_validate_ids():
    source = initialize_array(DatasetType.input, ComponentType.source, 3)
    source["id"] = [1, 2, 3]

    sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 3)
    sym_load["id"] = [4, 5, 6]

    input_data = {
        ComponentType.source: source,
        ComponentType.sym_load: sym_load,
    }

    source_update = initialize_array(DatasetType.update, ComponentType.source, 3)
    source_update["id"] = [1, 2, 4]
    source_update["u_ref"] = [1.0, 2.0, 3.0]

    sym_load_update = initialize_array(DatasetType.update, ComponentType.sym_load, 3)
    sym_load_update["id"] = [4, 5, 7]
    sym_load_update["p_specified"] = [4.0, 5.0, 6.0]

    update_data = {ComponentType.source: source_update, ComponentType.sym_load: sym_load_update}

    invalid_ids = validate_ids(update_data, input_data)

    assert IdNotInDatasetError(ComponentType.source, [4], DatasetType.update) in invalid_ids
    assert IdNotInDatasetError(ComponentType.sym_load, [7], DatasetType.update) in invalid_ids

    source_update_no_id = initialize_array(DatasetType.update, ComponentType.source, 3)
    source_update_no_id["u_ref"] = [1.0, 2.0, 3.0]

    update_data_col = compatibility_convert_row_columnar_dataset(
        data={ComponentType.source: source_update_no_id, ComponentType.sym_load: sym_load_update},
        data_filter=ComponentAttributeFilterOptions.relevant,
        dataset_type=DatasetType.update,
    )
    invalid_ids = validate_ids(update_data_col, input_data)
    assert len(invalid_ids) == 1
    assert IdNotInDatasetError(ComponentType.sym_load, [7], DatasetType.update) in invalid_ids

    source_update_less_no_id = initialize_array(DatasetType.update, ComponentType.source, 2)
    source_update_less_no_id["u_ref"] = [1.0, 2.0]

    update_data_col_less_no_id = compatibility_convert_row_columnar_dataset(
        data={ComponentType.source: source_update_less_no_id, ComponentType.sym_load: sym_load_update},
        data_filter=ComponentAttributeFilterOptions.relevant,
        dataset_type=DatasetType.update,
    )
    invalid_ids = validate_ids(update_data_col_less_no_id, input_data)
    n_invalid_ids_update_source_without_id = 2
    assert len(invalid_ids) == n_invalid_ids_update_source_without_id
    assert IdNotInDatasetError(ComponentType.sym_load, [7], DatasetType.update) in invalid_ids

    source_update_part_nan_id = initialize_array(DatasetType.update, ComponentType.source, 3)
    source_update_part_nan_id["id"] = [1, np.iinfo(np.int32).min, 4]
    source_update_part_nan_id["u_ref"] = [1.0, 2.0, 3.0]

    update_data_col_part_nan_id = compatibility_convert_row_columnar_dataset(
        data={ComponentType.source: source_update_part_nan_id, ComponentType.sym_load: sym_load_update},
        data_filter=ComponentAttributeFilterOptions.relevant,
        dataset_type=DatasetType.update,
    )
    invalid_ids = validate_ids(update_data_col_part_nan_id, input_data)
    n_invalid_ids_source_update = 2
    assert len(invalid_ids) == n_invalid_ids_source_update
    assert IdNotInDatasetError(ComponentType.sym_load, [7], DatasetType.update) in invalid_ids


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
    node = initialize_array(DatasetType.input, ComponentType.node, 1)
    line = initialize_array(DatasetType.input, ComponentType.line, 1)
    link = initialize_array(DatasetType.input, ComponentType.link, 1)
    transformer = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    three_winding_transformer = initialize_array(DatasetType.input, ComponentType.three_winding_transformer, 1)
    source = initialize_array(DatasetType.input, ComponentType.source, 1)
    shunt = initialize_array(DatasetType.input, ComponentType.shunt, 1)
    sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 1)
    sym_gen = initialize_array(DatasetType.input, ComponentType.sym_gen, 1)
    asym_load = initialize_array(DatasetType.input, ComponentType.asym_load, 1)
    asym_gen = initialize_array(DatasetType.input, ComponentType.asym_gen, 1)
    sym_voltage_sensor = initialize_array(DatasetType.input, ComponentType.sym_voltage_sensor, 1)

    asym_voltage_sensor = initialize_array(DatasetType.input, ComponentType.asym_voltage_sensor, 1)
    asym_voltage_sensor["u_measured"] = [[1.0, np.nan, 2.0]]

    sym_power_sensor = initialize_array(DatasetType.input, ComponentType.sym_power_sensor, 1)

    asym_power_sensor = initialize_array(DatasetType.input, ComponentType.asym_power_sensor, 1)
    asym_power_sensor["p_measured"] = [[np.nan, 2.0, 1.0]]
    asym_power_sensor["q_measured"] = [[2.0, 1.0, np.nan]]

    sym_current_sensor = initialize_array(DatasetType.input, ComponentType.sym_current_sensor, 1)

    asym_current_sensor = initialize_array(DatasetType.input, ComponentType.asym_current_sensor, 1)
    asym_current_sensor["i_measured"] = [[np.nan, 2.0, 1.0]]
    asym_current_sensor["i_angle_measured"] = [[2.0, 1.0, np.nan]]

    fault = initialize_array(DatasetType.input, ComponentType.fault, 1)

    data = {
        ComponentType.node: node,
        ComponentType.line: line,
        ComponentType.link: link,
        ComponentType.transformer: transformer,
        ComponentType.three_winding_transformer: three_winding_transformer,
        ComponentType.source: source,
        ComponentType.shunt: shunt,
        ComponentType.sym_load: sym_load,
        ComponentType.sym_gen: sym_gen,
        ComponentType.asym_load: asym_load,
        ComponentType.asym_gen: asym_gen,
        ComponentType.sym_voltage_sensor: sym_voltage_sensor,
        ComponentType.asym_voltage_sensor: asym_voltage_sensor,
        ComponentType.sym_power_sensor: sym_power_sensor,
        ComponentType.asym_power_sensor: asym_power_sensor,
        ComponentType.sym_current_sensor: sym_current_sensor,
        ComponentType.asym_current_sensor: asym_current_sensor,
        ComponentType.fault: fault,
    }
    required_values_errors = validate_required_values(data=data, calculation_type=calculation_type, symmetric=symmetric)

    pf_dependent = calculation_type == CalculationType.power_flow or calculation_type is None
    se_dependent = calculation_type == CalculationType.state_estimation or calculation_type is None
    sc_dependent = calculation_type == CalculationType.short_circuit or calculation_type is None
    asym_dependent = not symmetric

    assert MissingValueError(ComponentType.node, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.node, "u_rated", [NaN]) in required_values_errors

    assert MissingValueError(ComponentType.line, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "from_node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "to_node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "from_status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "to_status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "r1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "x1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "c1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "tan1", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.line, "r0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(ComponentType.line, "x0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(ComponentType.line, "c0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(ComponentType.line, "tan0", [NaN]) in required_values_errors) == asym_dependent

    # i_n made optional later in lines
    assert MissingValueError(ComponentType.line, "i_n", [NaN]) not in required_values_errors

    assert MissingValueError(ComponentType.link, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.link, "from_node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.link, "to_node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.link, "from_status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.link, "to_status", [NaN]) in required_values_errors

    assert MissingValueError(ComponentType.transformer, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "from_node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "to_node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "from_status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "to_status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "u1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "u2", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "sn", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "uk", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "pk", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "i0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "p0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "winding_from", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "winding_to", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "clock", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "tap_side", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "tap_min", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "tap_max", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.transformer, "tap_size", [NaN]) in required_values_errors

    assert MissingValueError(ComponentType.three_winding_transformer, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "node_1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "node_2", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "node_3", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "status_1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "status_2", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "status_3", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "u1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "u2", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "u3", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "sn_1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "sn_2", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "sn_3", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "uk_12", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "uk_13", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "uk_23", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "pk_12", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "pk_13", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "pk_23", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "i0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "p0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "winding_1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "winding_2", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "winding_3", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "clock_12", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "clock_13", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "tap_side", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "tap_min", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "tap_max", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.three_winding_transformer, "tap_size", [NaN]) in required_values_errors

    assert MissingValueError(ComponentType.source, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.source, "node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.source, "status", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.source, "u_ref", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(ComponentType.shunt, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.shunt, "node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.shunt, "status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.shunt, "g1", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.shunt, "b1", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.shunt, "g0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError(ComponentType.shunt, "b0", [NaN]) in required_values_errors) == asym_dependent

    assert MissingValueError(ComponentType.sym_load, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_load, "node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_load, "status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_load, "type", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.sym_load, "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(ComponentType.sym_load, "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(ComponentType.sym_gen, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_gen, "node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_gen, "status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_gen, "type", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.sym_gen, "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(ComponentType.sym_gen, "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(ComponentType.asym_load, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_load, "node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_load, "status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_load, "type", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.asym_load, "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(ComponentType.asym_load, "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(ComponentType.asym_gen, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_gen, "node", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_gen, "status", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_gen, "type", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.asym_gen, "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError(ComponentType.asym_gen, "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError(ComponentType.sym_voltage_sensor, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_voltage_sensor, "measured_object", [NaN]) in required_values_errors
    assert (
        MissingValueError(ComponentType.sym_voltage_sensor, "u_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.sym_voltage_sensor, "u_measured", [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(ComponentType.asym_voltage_sensor, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_voltage_sensor, "measured_object", [NaN]) in required_values_errors
    assert (
        MissingValueError(ComponentType.asym_voltage_sensor, "u_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.asym_voltage_sensor, "u_measured", [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(ComponentType.sym_power_sensor, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_power_sensor, "measured_object", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_power_sensor, "measured_terminal_type", [NaN]) in required_values_errors
    assert (
        MissingValueError(ComponentType.sym_power_sensor, "power_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.sym_power_sensor, "p_measured", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.sym_power_sensor, "q_measured", [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(ComponentType.asym_power_sensor, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_power_sensor, "measured_object", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_power_sensor, "measured_terminal_type", [NaN]) in required_values_errors
    assert (
        MissingValueError(ComponentType.asym_power_sensor, "power_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.asym_power_sensor, "p_measured", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.asym_power_sensor, "q_measured", [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(ComponentType.sym_current_sensor, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.sym_current_sensor, "measured_object", [NaN]) in required_values_errors
    assert (
        MissingValueError(ComponentType.sym_current_sensor, "measured_terminal_type", [NaN]) in required_values_errors
    )
    assert (
        MissingValueError(ComponentType.sym_current_sensor, "angle_measurement_type", [NaN]) in required_values_errors
    )
    assert (
        MissingValueError(ComponentType.sym_current_sensor, "i_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.sym_current_sensor, "i_angle_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.sym_current_sensor, "i_measured", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.sym_current_sensor, "i_angle_measured", [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(ComponentType.asym_current_sensor, "id", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.asym_current_sensor, "measured_object", [NaN]) in required_values_errors
    assert (
        MissingValueError(ComponentType.asym_current_sensor, "measured_terminal_type", [NaN]) in required_values_errors
    )
    assert (
        MissingValueError(ComponentType.asym_current_sensor, "angle_measurement_type", [NaN]) in required_values_errors
    )
    assert (
        MissingValueError(ComponentType.asym_current_sensor, "i_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.asym_current_sensor, "i_angle_sigma", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.asym_current_sensor, "i_measured", [NaN]) in required_values_errors
    ) == se_dependent
    assert (
        MissingValueError(ComponentType.asym_current_sensor, "i_angle_measured", [NaN]) in required_values_errors
    ) == se_dependent

    assert MissingValueError(ComponentType.fault, "id", [NaN]) in required_values_errors
    assert (MissingValueError(ComponentType.fault, "status", [NaN]) in required_values_errors) == sc_dependent
    assert (MissingValueError(ComponentType.fault, "fault_type", [NaN]) in required_values_errors) == sc_dependent


def test_validate_required_values_asym_calculation():
    line = initialize_array(DatasetType.input, ComponentType.line, 1)
    shunt = initialize_array(DatasetType.input, ComponentType.shunt, 1)

    data = {ComponentType.line: line, ComponentType.shunt: shunt}
    required_values_errors = validate_required_values(data=data, symmetric=False)

    assert MissingValueError(ComponentType.line, "r0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "x0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "c0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.line, "tan0", [NaN]) in required_values_errors

    assert MissingValueError(ComponentType.shunt, "g0", [NaN]) in required_values_errors
    assert MissingValueError(ComponentType.shunt, "b0", [NaN]) in required_values_errors


@pytest.mark.parametrize("fault_types", product(list(FaultType), list(FaultType)))
def test_validate_fault_sc_calculation(fault_types):
    line = initialize_array(DatasetType.input, ComponentType.line, 1)
    shunt = initialize_array(DatasetType.input, ComponentType.shunt, 1)
    fault = initialize_array(DatasetType.input, ComponentType.fault, 2)
    fault["fault_type"] = fault_types

    data = {ComponentType.line: line, ComponentType.shunt: shunt, ComponentType.fault: fault}
    required_values_errors = validate_required_values(data=data, calculation_type=CalculationType.short_circuit)

    asym_sc_calculation = np.any(
        list(fault_type not in (FaultType.three_phase, FaultType.nan) for fault_type in fault_types)
    )

    assert (MissingValueError(ComponentType.line, "r0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(ComponentType.line, "x0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(ComponentType.line, "c0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(ComponentType.line, "tan0", [NaN]) in required_values_errors) == asym_sc_calculation

    assert (MissingValueError(ComponentType.shunt, "g0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError(ComponentType.shunt, "b0", [NaN]) in required_values_errors) == asym_sc_calculation


def test_validate_values():
    # Create invalid nodes and lines
    node = initialize_array(DatasetType.input, ComponentType.node, 3)
    line = initialize_array(DatasetType.input, ComponentType.line, 3)

    # Validate nodes and lines individually
    node_errors = validate_values({ComponentType.node: node})
    line_errors = validate_values({ComponentType.line: line})

    # Validate nodes and lines combined
    both_errors = validate_values({ComponentType.node: node, ComponentType.line: line})

    # The errors should add up (in this simple case)
    assert both_errors == node_errors + line_errors


def test_validate_values__calculation_types():
    # Create invalid sensor
    sym_voltage_sensor = initialize_array(DatasetType.input, ComponentType.sym_voltage_sensor, 3)
    all_errors = validate_values({ComponentType.sym_voltage_sensor: sym_voltage_sensor})
    power_flow_errors = validate_values(
        {ComponentType.sym_voltage_sensor: sym_voltage_sensor}, calculation_type=CalculationType.power_flow
    )
    state_estimation_errors = validate_values(
        {ComponentType.sym_voltage_sensor: sym_voltage_sensor}, calculation_type=CalculationType.state_estimation
    )

    assert not power_flow_errors
    assert all_errors == state_estimation_errors


@pytest.mark.parametrize(
    ("sensor_type", "parameter"),
    [
        (ComponentType.sym_voltage_sensor, "u_sigma"),
        (ComponentType.asym_voltage_sensor, "u_sigma"),
        (ComponentType.sym_power_sensor, "power_sigma"),
        (ComponentType.sym_power_sensor, "p_sigma"),
        (ComponentType.sym_power_sensor, "q_sigma"),
        (ComponentType.asym_power_sensor, "power_sigma"),
        (ComponentType.asym_power_sensor, "p_sigma"),
        (ComponentType.asym_power_sensor, "q_sigma"),
        (ComponentType.sym_current_sensor, "i_sigma"),
        (ComponentType.sym_current_sensor, "i_angle_sigma"),
        (ComponentType.asym_current_sensor, "i_sigma"),
        (ComponentType.asym_current_sensor, "i_angle_sigma"),
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
        (
            ComponentType.sym_power_sensor,
            [[np.nan, np.nan], [], []],
            [InvalidIdError, NotUniqueError],
        ),
        (
            ComponentType.sym_power_sensor,
            [[0.1, np.nan], [], []],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.sym_power_sensor,
            [[np.nan, 0.1], [], []],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.sym_power_sensor,
            [[0.1, 0.1], [], []],
            [InvalidIdError, NotUniqueError],
        ),
        (
            ComponentType.asym_power_sensor,
            [[], [[np.nan, np.nan, np.nan]] * 3, [[np.nan, np.nan, np.nan]] * 3],
            [InvalidIdError, NotUniqueError],
        ),
        (
            ComponentType.asym_power_sensor,
            [
                [],
                [[0.1, np.nan, 0.1], [0.1, np.nan, 0.1], [0.1, 0.1, 0.1]],
                [[np.nan, 0.1, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [[], [[0.1, np.nan, np.nan]] * 3, [[np.nan, np.nan, np.nan]] * 3],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [[], [[np.nan, np.nan, np.nan]] * 3, [[0.1, np.nan, np.nan]] * 3],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [
                [],
                [[0.1, 0.1, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[np.nan, np.nan, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [
                [],
                [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[np.nan, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [
                [],
                [[np.nan, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [
                [],
                [[np.nan, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [
                [],
                [[0.1, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
                [[0.1, np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            ],
            [InvalidIdError, NotUniqueError, PQSigmaPairError],
        ),
        (
            ComponentType.asym_power_sensor,
            [[], [[0.1, 0.1, 0.1]] * 3, [[0.1, 0.1, 0.1]] * 3],
            [InvalidIdError, NotUniqueError],
        ),
    ],
)
def test_validate_values__bad_p_q_sigma(sensor_type, values, error_types):
    def arbitrary_fill(array, sensor_type, values):
        if sensor_type == ComponentType.sym_power_sensor:
            array["p_sigma"] = values[0][0]
            array["q_sigma"] = values[0][1]
        else:
            array["p_sigma"][0] = values[1][0]
            array["p_sigma"][1] = values[1][1]
            array["p_sigma"][2] = values[1][2]
            array["q_sigma"][0] = values[2][0]
            array["q_sigma"][1] = values[2][1]
            array["q_sigma"][2] = values[2][2]
        array["id"] = [123, 234, 345]

    sensor_array = initialize_array(DatasetType.input, sensor_type, 3)
    arbitrary_fill(sensor_array, sensor_type, values)
    all_errors = validate_values({sensor_type: sensor_array})

    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in error_types)
        assert set(error.ids).issubset(set(sensor_array["id"]))


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
        node = initialize_array(DatasetType.input, ComponentType.node, 1)
        node["id"] = 123
        sym_power_sensor = initialize_array(DatasetType.input, ComponentType.sym_power_sensor, 1)
        sym_power_sensor["p_sigma"] = values[0][0]
        sym_power_sensor["q_sigma"] = values[0][1]
        sym_power_sensor["id"] = 456
        asym_power_sensor = initialize_array(DatasetType.input, ComponentType.asym_power_sensor, 1)
        asym_power_sensor["p_measured"] = values[1][0]
        asym_power_sensor["q_measured"] = values[1][1]
        asym_power_sensor["id"] = 789

        return {
            ComponentType.node: node,
            ComponentType.sym_power_sensor: sym_power_sensor,
            ComponentType.asym_power_sensor: asym_power_sensor,
        }

    data = two_component_data(values)
    all_errors = validate_values(data)
    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in error_types)
        assert (data[error.component]["id"] == error.ids).all()


def test_validate_values__bad_p_q_sigma_single_component_twice():
    def single_component_twice_data():
        node = initialize_array(DatasetType.input, ComponentType.node, 1)
        node["id"] = 123
        sym_power_sensor = initialize_array(DatasetType.input, ComponentType.sym_power_sensor, 2)
        sym_power_sensor["p_sigma"] = [np.nan, 0.1]
        sym_power_sensor["q_sigma"] = [np.nan, np.nan]
        sym_power_sensor["id"] = [456, 789]

        return {
            ComponentType.node: node,
            ComponentType.sym_power_sensor: sym_power_sensor,
        }

    data = single_component_twice_data()
    all_errors = validate_values(data)
    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in [InvalidIdError, PQSigmaPairError])
        if isinstance(error, PQSigmaPairError):
            assert error.ids[0] == data["sym_power_sensor"]["id"][1]


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
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            MeasuredTerminalType.branch_from,
        ),
        (
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            MeasuredTerminalType.branch_to,
        ),
        (ComponentType.source, MeasuredTerminalType.source),
        (ComponentType.shunt, MeasuredTerminalType.shunt),
        ([ComponentType.sym_load, ComponentType.asym_load], MeasuredTerminalType.load),
        ([ComponentType.sym_gen, ComponentType.asym_gen], MeasuredTerminalType.generator),
        (ComponentType.three_winding_transformer, MeasuredTerminalType.branch3_1),
        (ComponentType.three_winding_transformer, MeasuredTerminalType.branch3_2),
        (ComponentType.three_winding_transformer, MeasuredTerminalType.branch3_3),
        (ComponentType.node, MeasuredTerminalType.node),
    ],
)
@patch("power_grid_model.validation._validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation._validation._all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation._validation._all_valid_ids")
def test_validate_generic_power_sensor__terminal_types(
    _all_valid_ids: MagicMock,
    ref_component: ComponentType | list[ComponentType],
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


@pytest.mark.parametrize("current_sensor_type", [ComponentType.sym_current_sensor, ComponentType.asym_current_sensor])
@pytest.mark.parametrize(
    "measured_terminal_type, supported",
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
    current_sensor_type: ComponentType, measured_terminal_type: MeasuredTerminalType, supported: bool
):
    current_sensor_data = initialize_array(DatasetType.input, current_sensor_type, 1)
    current_sensor_data["id"] = 1
    current_sensor_data["measured_terminal_type"] = measured_terminal_type

    result = validate_generic_current_sensor(
        data={current_sensor_type: current_sensor_data}, component=current_sensor_type
    )

    if supported:
        assert not result
    else:
        assert result == [
            UnsupportedMeasuredTerminalType(
                current_sensor_type,
                "measured_terminal_type",
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
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            MeasuredTerminalType.branch_from,
        ),
        (
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            MeasuredTerminalType.branch_to,
        ),
        (ComponentType.three_winding_transformer, MeasuredTerminalType.branch3_1),
        (ComponentType.three_winding_transformer, MeasuredTerminalType.branch3_2),
        (ComponentType.three_winding_transformer, MeasuredTerminalType.branch3_3),
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
    ref_component: ComponentType | list[ComponentType],
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
        measured_object_field="measured_object",
        measured_terminal_type_field="measured_terminal_type",
        angle_measurement_type_field="angle_measurement_type",
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
        angle_measurement_type_filter=("angle_measurement_type", AngleMeasurementType.global_angle),
        voltage_sensor_u_angle_measured={
            ComponentType.sym_voltage_sensor: "u_angle_measured",
            ComponentType.asym_voltage_sensor: "u_angle_measured",
        },
    )


@pytest.mark.parametrize("power_sensor_type", [ComponentType.sym_power_sensor, ComponentType.asym_power_sensor])
@pytest.mark.parametrize("current_sensor_type", [ComponentType.sym_current_sensor, ComponentType.asym_current_sensor])
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
    power_sensor_type: ComponentType,
    current_sensor_type: ComponentType,
):
    # Act
    validate_values({power_sensor_type: None, current_sensor_type: None})

    # Assert
    _all_same_sensor_type_on_same_terminal.assert_any_call(
        ANY,
        power_sensor_type=power_sensor_type,
        current_sensor_type=current_sensor_type,
        measured_object_field="measured_object",
        measured_terminal_type_field="measured_terminal_type",
    )


def test_power_sigma_or_p_q_sigma():
    # node
    node = initialize_array(DatasetType.input, ComponentType.node, 2)
    node["id"] = np.array([0, 3])
    node["u_rated"] = [10.5e3, 10.5e3]

    # line
    line = initialize_array(DatasetType.input, ComponentType.line, 1)
    line["id"] = [2]
    line["from_node"] = [0]
    line["to_node"] = [3]
    line["from_status"] = [1]
    line["to_status"] = [1]
    line["r1"] = [0.001]
    line["x1"] = [0.02]
    line["c1"] = [0.0]
    line["tan1"] = [0.0]
    line["i_n"] = [1000.0]

    # load
    sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 2)
    sym_load["id"] = [4, 9]
    sym_load["node"] = [3, 0]
    sym_load["status"] = [1, 1]
    sym_load["type"] = [LoadGenType.const_power, LoadGenType.const_power]
    sym_load["p_specified"] = [1e6, 1e6]
    sym_load["q_specified"] = [-1e6, -1e6]

    # source
    source = initialize_array(DatasetType.input, ComponentType.source, 1)
    source["id"] = [1]
    source["node"] = [0]
    source["status"] = [1]
    source["u_ref"] = [1.0]

    # voltage sensor
    voltage_sensor = initialize_array(DatasetType.input, ComponentType.sym_voltage_sensor, 1)
    voltage_sensor["id"] = 5
    voltage_sensor["measured_object"] = 0
    voltage_sensor["u_sigma"] = [100.0]
    voltage_sensor["u_measured"] = [10.5e3]

    # power sensor
    sym_power_sensor = initialize_array(DatasetType.input, ComponentType.sym_power_sensor, 3)
    sym_power_sensor["id"] = [6, 7, 8]
    sym_power_sensor["measured_object"] = [2, 4, 9]
    sym_power_sensor["measured_terminal_type"] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.load,
        MeasuredTerminalType.load,
    ]
    sym_power_sensor["p_measured"] = [1e6, -1e6, -1e6]
    sym_power_sensor["q_measured"] = [1e6, -1e6, -1e6]
    sym_power_sensor["power_sigma"] = [np.nan, 1e9, 1e9]
    sym_power_sensor["p_sigma"] = [1e4, np.nan, 1e4]
    sym_power_sensor["q_sigma"] = [1e9, np.nan, 1e9]

    # power sensor
    asym_power_sensor = initialize_array(DatasetType.input, ComponentType.asym_power_sensor, 4)
    asym_power_sensor["id"] = [66, 77, 88, 99]
    asym_power_sensor["measured_object"] = [2, 4, 9, 9]
    asym_power_sensor["measured_terminal_type"] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.load,
        MeasuredTerminalType.load,
        MeasuredTerminalType.load,
    ]
    asym_power_sensor["p_measured"] = [[1e6, 1e6, 1e6], [-1e6, -1e6, -1e6], [-1e6, -1e6, -1e6], [-1e6, -1e6, -1e6]]
    asym_power_sensor["q_measured"] = [[1e6, 1e6, 1e6], [-1e6, -1e6, -1e6], [-1e6, -1e6, -1e6], [-1e6, -1e6, -1e6]]
    asym_power_sensor["power_sigma"] = [np.nan, 1e9, 1e9, 1e9]
    asym_power_sensor["p_sigma"] = [[1e4, 1e4, 1e4], [np.nan, np.nan, np.nan], [1e4, 1e4, 1e4], [1e4, 1e4, 1e4]]
    asym_power_sensor["q_sigma"] = [[1e9, 1e9, 1e9], [np.nan, np.nan, np.nan], [1e9, 1e4, 1e4], [1e9, 1e4, 1e4]]

    # all
    input_data = {
        ComponentType.node: node,
        ComponentType.line: line,
        ComponentType.sym_load: sym_load,
        ComponentType.source: source,
        ComponentType.sym_voltage_sensor: voltage_sensor,
        ComponentType.sym_power_sensor: sym_power_sensor,
        ComponentType.asym_power_sensor: asym_power_sensor,
    }

    assert_valid_input_data(input_data=input_data, calculation_type=CalculationType.state_estimation)

    np.testing.assert_array_equal(sym_power_sensor["power_sigma"], [np.nan, 1e9, 1e9])
    np.testing.assert_array_equal(sym_power_sensor["p_sigma"], [1e4, np.nan, 1e4])
    np.testing.assert_array_equal(sym_power_sensor["q_sigma"], [1e9, np.nan, 1e9])
    np.testing.assert_array_equal(asym_power_sensor["power_sigma"], [np.nan, 1e9, 1e9, 1e9])
    np.testing.assert_array_equal(
        asym_power_sensor["p_sigma"], [[1e4, 1e4, 1e4], [np.nan, np.nan, np.nan], [1e4, 1e4, 1e4], [1e4, 1e4, 1e4]]
    )
    np.testing.assert_array_equal(
        asym_power_sensor["q_sigma"], [[1e9, 1e9, 1e9], [np.nan, np.nan, np.nan], [1e9, 1e4, 1e4], [1e9, 1e4, 1e4]]
    )

    # bad weather
    bad_input_data = copy.deepcopy(input_data)
    bad_sym_power_sensor = bad_input_data[ComponentType.sym_power_sensor]
    bad_sym_power_sensor["power_sigma"] = [np.nan, np.nan, 1e9]
    bad_sym_power_sensor["p_sigma"] = [np.nan, np.nan, 1e4]
    bad_sym_power_sensor["q_sigma"] = [np.nan, 1e9, np.nan]
    errors = validate_input_data(input_data=bad_input_data, calculation_type=CalculationType.state_estimation)
    n_sym_input_validation_errors = 4
    assert len(errors) == n_sym_input_validation_errors
    assert errors == [
        MissingValueError(ComponentType.sym_power_sensor, "power_sigma", [6]),
        MissingValueError(ComponentType.sym_power_sensor, "p_sigma", [7]),
        MissingValueError(ComponentType.sym_power_sensor, "q_sigma", [8]),
        PQSigmaPairError(ComponentType.sym_power_sensor, ("p_sigma", "q_sigma"), [7, 8]),
    ]

    np.testing.assert_array_equal(bad_sym_power_sensor["power_sigma"], [np.nan, np.nan, 1e9])
    np.testing.assert_array_equal(bad_sym_power_sensor["p_sigma"], [np.nan, np.nan, 1e4])
    np.testing.assert_array_equal(bad_sym_power_sensor["q_sigma"], [np.nan, 1e9, np.nan])

    # bad weather
    bad_input_data = copy.deepcopy(input_data)
    bad_asym_power_sensor = bad_input_data[ComponentType.asym_power_sensor]
    bad_asym_power_sensor["power_sigma"] = [np.nan, np.nan, 1e9, np.nan]
    bad_asym_power_sensor["p_sigma"] = [
        [np.nan, np.nan, np.nan],
        [np.nan, np.nan, np.nan],
        [1e4, np.nan, np.nan],
        [1e4, np.nan, np.nan],
    ]
    bad_asym_power_sensor["q_sigma"] = [
        [np.nan, np.nan, np.nan],
        [1e9, 1e9, 1e9],
        [np.nan, 1e4, 1e4],
        [np.nan, 1e4, 1e4],
    ]
    errors = validate_input_data(input_data=bad_input_data, calculation_type=CalculationType.state_estimation)
    n_asym_input_validation_errors = 4
    assert len(errors) == n_asym_input_validation_errors
    assert errors == [
        MissingValueError(ComponentType.asym_power_sensor, "power_sigma", [66]),
        MissingValueError(ComponentType.asym_power_sensor, "p_sigma", [77, 88, 99]),
        MissingValueError(ComponentType.asym_power_sensor, "q_sigma", [88, 99]),
        PQSigmaPairError(ComponentType.asym_power_sensor, ("p_sigma", "q_sigma"), [77, 88, 99]),
    ]

    np.testing.assert_array_equal(bad_asym_power_sensor["power_sigma"], [np.nan, np.nan, 1e9, np.nan])
    np.testing.assert_array_equal(
        bad_asym_power_sensor["p_sigma"],
        [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan], [1e4, np.nan, np.nan], [1e4, np.nan, np.nan]],
    )
    np.testing.assert_array_equal(
        bad_asym_power_sensor["q_sigma"],
        [[np.nan, np.nan, np.nan], [1e9, 1e9, 1e9], [np.nan, 1e4, 1e4], [np.nan, 1e4, 1e4]],
    )


def test_all_default_values():
    """
    Initialize all components that have attributes that have default values, without setting values for
    those attributes.
    """
    node = initialize_array(DatasetType.input, ComponentType.node, 3)
    node["id"] = [0, 1, 2]
    node["u_rated"] = [50.0e3, 20.0e3, 10.5e3]

    source = initialize_array(DatasetType.input, ComponentType.source, 1)
    source["id"] = [3]
    source["node"] = [2]
    source["status"] = [1]
    source["u_ref"] = [1.0]

    transformer = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer["id"] = [4]
    transformer["from_node"] = [0]
    transformer["to_node"] = [2]
    transformer["from_status"] = [1]
    transformer["to_status"] = [1]
    transformer["u1"] = [50e3]
    transformer["u2"] = [10.5e3]
    transformer["sn"] = [1e5]
    transformer["uk"] = [0.1]
    transformer["pk"] = [1e3]
    transformer["i0"] = [1.0e-6]
    transformer["p0"] = [0.1]
    transformer["winding_from"] = [2]
    transformer["winding_to"] = [1]
    transformer["clock"] = [5]
    transformer["tap_side"] = [0]
    transformer["tap_min"] = [-11]
    transformer["tap_max"] = [9]
    transformer["tap_size"] = [100]

    three_winding_transformer = initialize_array(DatasetType.input, ComponentType.three_winding_transformer, 1)
    three_winding_transformer["id"] = [6]
    three_winding_transformer["node_1"] = [0]
    three_winding_transformer["node_2"] = [1]
    three_winding_transformer["node_3"] = [2]
    three_winding_transformer["status_1"] = [1]
    three_winding_transformer["status_2"] = [1]
    three_winding_transformer["status_3"] = [1]
    three_winding_transformer["u1"] = [50.0e3]
    three_winding_transformer["u2"] = [20.0e3]
    three_winding_transformer["u3"] = [10.5e3]
    three_winding_transformer["sn_1"] = [1e5]
    three_winding_transformer["sn_2"] = [1e5]
    three_winding_transformer["sn_3"] = [1e5]
    three_winding_transformer["uk_12"] = [0.09]
    three_winding_transformer["uk_13"] = [0.06]
    three_winding_transformer["uk_23"] = [0.06]
    three_winding_transformer["pk_12"] = [1e3]
    three_winding_transformer["pk_13"] = [1e3]
    three_winding_transformer["pk_23"] = [1e3]
    three_winding_transformer["i0"] = [0]
    three_winding_transformer["p0"] = [0]
    three_winding_transformer["winding_1"] = [2]
    three_winding_transformer["winding_2"] = [1]
    three_winding_transformer["winding_3"] = [1]
    three_winding_transformer["clock_12"] = [5]
    three_winding_transformer["clock_13"] = [5]
    three_winding_transformer["tap_side"] = [0]
    three_winding_transformer["tap_min"] = [-10]
    three_winding_transformer["tap_max"] = [10]
    three_winding_transformer["tap_size"] = [1380]

    fault = initialize_array(DatasetType.input, ComponentType.fault, 1)
    fault["id"] = [5]
    fault["status"] = [1]
    fault["fault_object"] = [0]

    input_data = {
        ComponentType.node: node,
        ComponentType.transformer: transformer,
        ComponentType.three_winding_transformer: three_winding_transformer,
        ComponentType.source: source,
        ComponentType.fault: fault,
    }

    assert_valid_input_data(input_data=input_data, calculation_type=CalculationType.power_flow)


@patch("power_grid_model.validation._validation.validate_transformer", new=MagicMock(return_value=[]))
@patch("power_grid_model.validation._validation.validate_three_winding_transformer", new=MagicMock(return_value=[]))
def test_validate_values__tap_regulator_control_side():
    # Create valid transformer
    transformer = initialize_array(DatasetType.input, ComponentType.transformer, 4)
    transformer["id"] = [0, 1, 2, 3]
    transformer["tap_side"] = [BranchSide.from_side, BranchSide.from_side, BranchSide.from_side, BranchSide.from_side]

    # Create valid three winding transformer
    three_winding_transformer = initialize_array(DatasetType.input, ComponentType.three_winding_transformer, 3)
    three_winding_transformer["id"] = [4, 5, 6]
    three_winding_transformer["tap_side"] = [Branch3Side.side_1, Branch3Side.side_1, Branch3Side.side_1]

    # Create invalid regulator
    transformer_tap_regulator = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 7)
    transformer_tap_regulator["id"] = np.arange(7, 14)
    transformer_tap_regulator["status"] = 1
    transformer_tap_regulator["regulated_object"] = np.arange(7)
    transformer_tap_regulator["control_side"] = [
        BranchSide.to_side,  # OK
        BranchSide.from_side,  # OK
        Branch3Side.side_3,  # branch3 provided but it is a 2-winding transformer (invalid)
        10,  # control side entirely out of range (invalid)
        Branch3Side.side_3,  # OK
        Branch3Side.side_1,  # OK
        10,  # control side entirely out of range (invalid)
    ]

    input_data = {
        ComponentType.transformer: transformer,
        ComponentType.three_winding_transformer: three_winding_transformer,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator,
    }
    all_errors = validate_values(input_data)
    power_flow_errors = validate_values(input_data, calculation_type=CalculationType.power_flow)
    state_estimation_errors = validate_values(input_data, calculation_type=CalculationType.state_estimation)

    assert power_flow_errors == all_errors
    assert not state_estimation_errors

    n_input_validation_errors = 3

    assert len(all_errors) == n_input_validation_errors
    assert (
        InvalidEnumValueError(
            ComponentType.transformer_tap_regulator, "control_side", [10, 13], [BranchSide, Branch3Side]
        )
        in all_errors
    )
    assert (
        InvalidAssociatedEnumValueError(
            ComponentType.transformer_tap_regulator,
            ["control_side", "regulated_object"],
            [9, 10],
            [BranchSide],
        )
        in all_errors
    )
    assert (
        InvalidAssociatedEnumValueError(
            ComponentType.transformer_tap_regulator,
            ["control_side", "regulated_object"],
            [13],
            [Branch3Side],
        )
        in all_errors
    )
