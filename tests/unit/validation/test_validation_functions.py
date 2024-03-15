# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from itertools import product
from typing import List, Union
from unittest.mock import ANY, MagicMock, patch

import numpy as np
import pytest

from power_grid_model import CalculationType, LoadGenType, MeasuredTerminalType, initialize_array, power_grid_meta_data
from power_grid_model.enum import CalculationType, FaultPhase, FaultType
from power_grid_model.validation import assert_valid_input_data
from power_grid_model.validation.errors import (
    IdNotInDatasetError,
    InfinityError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    MultiFieldValidationError,
    NotUniqueError,
)
from power_grid_model.validation.validation import (
    assert_valid_data_structure,
    validate_generic_power_sensor,
    validate_ids_exist,
    validate_required_values,
    validate_unique_ids_across_components,
    validate_values,
)

NaN = power_grid_meta_data["input"]["node"].nans["id"]


def test_assert_valid_data_structure():
    node_input = initialize_array("input", "node", 3)
    line_input = initialize_array("input", "line", 3)
    node_update = initialize_array("update", "node", 3)
    line_update = initialize_array("update", "line", 3)

    # Input data: Assertion ok
    assert_valid_data_structure({"node": node_input, "line": line_input}, "input")

    # Update data: Assertion ok
    assert_valid_data_structure({"node": node_update, "line": line_update}, "update")

    # There is no such thing as 'output' data
    with pytest.raises(KeyError, match=r"output"):
        assert_valid_data_structure({"node": node_input, "line": line_input}, "output")

    # Input data is not valid update data
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({"node": node_input, "line": line_input}, "update")

    # Update data is not valid input data
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({"node": node_update, "line": line_update}, "input")

    # A normal numpy array is not valid input data
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)])
    with pytest.raises(TypeError, match=r"Unexpected Numpy array"):
        assert_valid_data_structure({"node": node_dummy, "line": line_input}, "input")

    # A structured numpy array, with wrong data type for u_rated f4 != f8, is not valid input data
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)], dtype=[("id", "i4"), ("u_rated", "f4")])
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({"node": node_dummy, "line": line_input}, "input")

    # A structured numpy array, with correct data types is not a valid, is still not valid input data.
    # N.B. It is not 'aligned'
    node_dummy = np.array([(1, 10.5e3), (2, 10.5e3), (3, 10.5e3)], dtype=[("id", "i4"), ("u_rated", "f8")])
    with pytest.raises(TypeError, match=r"Unexpected Numpy structured array"):
        assert_valid_data_structure({"node": node_dummy, "line": line_input}, "input")

    # Invalid component type
    input_with_wrong_component = {"node": node_input, "some_random_component": line_input}
    with pytest.raises(KeyError, match="Unknown component 'some_random_component' in input_data."):
        assert_valid_data_structure(input_with_wrong_component, "input")

    input_with_wrong_data_type = {"node": node_input, "line": [1, 2, 3]}
    with pytest.raises(TypeError, match="Unexpected data type list for 'line' input_data "):
        assert_valid_data_structure(input_with_wrong_data_type, "input")


def test_validate_unique_ids_across_components():
    node = initialize_array("input", "node", 3)
    node["id"] = [1, 2, 3]

    line = initialize_array("input", "line", 3)
    line["id"] = [4, 5, 3]

    transformer = initialize_array("input", "transformer", 3)
    transformer["id"] = [1, 6, 7]

    source = initialize_array("input", "source", 3)
    source["id"] = [8, 9, 10]

    input_data = {"node": node, "line": line, "transformer": transformer, "source": source}

    unique_id_errors = validate_unique_ids_across_components(input_data)

    assert (
        MultiComponentNotUniqueError(
            [("node", "id"), ("line", "id"), ("transformer", "id")],
            [("node", 1), ("node", 3), ("line", 3), ("transformer", 1)],
        )
        in unique_id_errors
    )
    assert len(unique_id_errors[0].ids) == 4


def test_validate_ids_exist():
    source = initialize_array("input", "source", 3)
    source["id"] = [1, 2, 3]

    sym_load = initialize_array("input", "sym_load", 3)
    sym_load["id"] = [4, 5, 6]

    input_data = {
        "source": source,
        "sym_load": sym_load,
    }

    source_update = initialize_array("update", "source", 3)
    source_update["id"] = [1, 2, 4]
    source_update["u_ref"] = [1.0, 2.0, 3.0]

    sym_load_update = initialize_array("update", "sym_load", 3)
    sym_load_update["id"] = [4, 5, 7]
    sym_load_update["p_specified"] = [4.0, 5.0, 6.0]

    update_data = {"source": source_update, "sym_load": sym_load_update}

    invalid_ids = validate_ids_exist(update_data, input_data)

    assert IdNotInDatasetError("source", [4], "input_data") in invalid_ids
    assert IdNotInDatasetError("sym_load", [7], "input_data") in invalid_ids


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
    node = initialize_array("input", "node", 1)
    line = initialize_array("input", "line", 1)
    link = initialize_array("input", "link", 1)
    transformer = initialize_array("input", "transformer", 1)
    three_winding_transformer = initialize_array("input", "three_winding_transformer", 1)
    source = initialize_array("input", "source", 1)
    shunt = initialize_array("input", "shunt", 1)
    sym_load = initialize_array("input", "sym_load", 1)
    sym_gen = initialize_array("input", "sym_gen", 1)
    asym_load = initialize_array("input", "asym_load", 1)
    asym_gen = initialize_array("input", "asym_gen", 1)
    sym_voltage_sensor = initialize_array("input", "sym_voltage_sensor", 1)

    asym_voltage_sensor = initialize_array("input", "asym_voltage_sensor", 1)
    asym_voltage_sensor["u_measured"] = [[1.0, np.nan, 2.0]]

    sym_power_sensor = initialize_array("input", "sym_power_sensor", 1)

    asym_power_sensor = initialize_array("input", "asym_power_sensor", 1)
    asym_power_sensor["p_measured"] = [[np.nan, 2.0, 1.0]]
    asym_power_sensor["q_measured"] = [[2.0, 1.0, np.nan]]

    fault = initialize_array("input", "fault", 1)

    data = {
        "node": node,
        "line": line,
        "link": link,
        "transformer": transformer,
        "three_winding_transformer": three_winding_transformer,
        "source": source,
        "shunt": shunt,
        "sym_load": sym_load,
        "sym_gen": sym_gen,
        "asym_load": asym_load,
        "asym_gen": asym_gen,
        "sym_voltage_sensor": sym_voltage_sensor,
        "asym_voltage_sensor": asym_voltage_sensor,
        "sym_power_sensor": sym_power_sensor,
        "asym_power_sensor": asym_power_sensor,
        "fault": fault,
    }
    required_values_errors = validate_required_values(data=data, calculation_type=calculation_type, symmetric=symmetric)

    pf_dependent = calculation_type == CalculationType.power_flow or calculation_type is None
    se_dependent = calculation_type == CalculationType.state_estimation or calculation_type is None
    sc_dependent = calculation_type == CalculationType.short_circuit or calculation_type is None
    asym_dependent = not symmetric

    assert MissingValueError("node", "id", [NaN]) in required_values_errors
    assert MissingValueError("node", "u_rated", [NaN]) in required_values_errors

    assert MissingValueError("line", "id", [NaN]) in required_values_errors
    assert MissingValueError("line", "from_node", [NaN]) in required_values_errors
    assert MissingValueError("line", "to_node", [NaN]) in required_values_errors
    assert MissingValueError("line", "from_status", [NaN]) in required_values_errors
    assert MissingValueError("line", "to_status", [NaN]) in required_values_errors
    assert MissingValueError("line", "r1", [NaN]) in required_values_errors
    assert MissingValueError("line", "x1", [NaN]) in required_values_errors
    assert MissingValueError("line", "c1", [NaN]) in required_values_errors
    assert MissingValueError("line", "tan1", [NaN]) in required_values_errors
    assert (MissingValueError("line", "r0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError("line", "x0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError("line", "c0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError("line", "tan0", [NaN]) in required_values_errors) == asym_dependent

    # i_n made optional later in lines
    assert MissingValueError("line", "i_n", [NaN]) not in required_values_errors

    assert MissingValueError("link", "id", [NaN]) in required_values_errors
    assert MissingValueError("link", "from_node", [NaN]) in required_values_errors
    assert MissingValueError("link", "to_node", [NaN]) in required_values_errors
    assert MissingValueError("link", "from_status", [NaN]) in required_values_errors
    assert MissingValueError("link", "to_status", [NaN]) in required_values_errors

    assert MissingValueError("transformer", "id", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "from_node", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "to_node", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "from_status", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "to_status", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "u1", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "u2", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "sn", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "uk", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "pk", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "i0", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "p0", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "winding_from", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "winding_to", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "clock", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "tap_side", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "tap_pos", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "tap_min", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "tap_max", [NaN]) in required_values_errors
    assert MissingValueError("transformer", "tap_size", [NaN]) in required_values_errors

    assert MissingValueError("three_winding_transformer", "id", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "node_1", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "node_2", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "node_3", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "status_1", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "status_2", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "status_3", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "u1", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "u2", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "u3", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "sn_1", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "sn_2", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "sn_3", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "uk_12", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "uk_13", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "uk_23", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "pk_12", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "pk_13", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "pk_23", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "i0", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "p0", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "winding_1", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "winding_2", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "winding_3", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "clock_12", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "clock_13", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "tap_side", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "tap_pos", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "tap_min", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "tap_max", [NaN]) in required_values_errors
    assert MissingValueError("three_winding_transformer", "tap_size", [NaN]) in required_values_errors

    assert MissingValueError("source", "id", [NaN]) in required_values_errors
    assert MissingValueError("source", "node", [NaN]) in required_values_errors
    assert MissingValueError("source", "status", [NaN]) in required_values_errors
    assert (MissingValueError("source", "u_ref", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError("shunt", "id", [NaN]) in required_values_errors
    assert MissingValueError("shunt", "node", [NaN]) in required_values_errors
    assert MissingValueError("shunt", "status", [NaN]) in required_values_errors
    assert MissingValueError("shunt", "g1", [NaN]) in required_values_errors
    assert MissingValueError("shunt", "b1", [NaN]) in required_values_errors
    assert (MissingValueError("shunt", "g0", [NaN]) in required_values_errors) == asym_dependent
    assert (MissingValueError("shunt", "b0", [NaN]) in required_values_errors) == asym_dependent

    assert MissingValueError("sym_load", "id", [NaN]) in required_values_errors
    assert MissingValueError("sym_load", "node", [NaN]) in required_values_errors
    assert MissingValueError("sym_load", "status", [NaN]) in required_values_errors
    assert MissingValueError("sym_load", "type", [NaN]) in required_values_errors
    assert (MissingValueError("sym_load", "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError("sym_load", "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError("sym_gen", "id", [NaN]) in required_values_errors
    assert MissingValueError("sym_gen", "node", [NaN]) in required_values_errors
    assert MissingValueError("sym_gen", "status", [NaN]) in required_values_errors
    assert MissingValueError("sym_gen", "type", [NaN]) in required_values_errors
    assert (MissingValueError("sym_gen", "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError("sym_gen", "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError("asym_load", "id", [NaN]) in required_values_errors
    assert MissingValueError("asym_load", "node", [NaN]) in required_values_errors
    assert MissingValueError("asym_load", "status", [NaN]) in required_values_errors
    assert MissingValueError("asym_load", "type", [NaN]) in required_values_errors
    assert (MissingValueError("asym_load", "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError("asym_load", "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError("asym_gen", "id", [NaN]) in required_values_errors
    assert MissingValueError("asym_gen", "node", [NaN]) in required_values_errors
    assert MissingValueError("asym_gen", "status", [NaN]) in required_values_errors
    assert MissingValueError("asym_gen", "type", [NaN]) in required_values_errors
    assert (MissingValueError("asym_gen", "p_specified", [NaN]) in required_values_errors) == pf_dependent
    assert (MissingValueError("asym_gen", "q_specified", [NaN]) in required_values_errors) == pf_dependent

    assert MissingValueError("sym_voltage_sensor", "id", [NaN]) in required_values_errors
    assert MissingValueError("sym_voltage_sensor", "measured_object", [NaN]) in required_values_errors
    assert (MissingValueError("sym_voltage_sensor", "u_sigma", [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError("sym_voltage_sensor", "u_measured", [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError("asym_voltage_sensor", "id", [NaN]) in required_values_errors
    assert MissingValueError("asym_voltage_sensor", "measured_object", [NaN]) in required_values_errors
    assert (MissingValueError("asym_voltage_sensor", "u_sigma", [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError("asym_voltage_sensor", "u_measured", [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError("sym_power_sensor", "id", [NaN]) in required_values_errors
    assert MissingValueError("sym_power_sensor", "measured_object", [NaN]) in required_values_errors
    assert MissingValueError("sym_power_sensor", "measured_terminal_type", [NaN]) in required_values_errors
    assert (MissingValueError("sym_power_sensor", "power_sigma", [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError("sym_power_sensor", "p_measured", [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError("sym_power_sensor", "q_measured", [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError("asym_power_sensor", "id", [NaN]) in required_values_errors
    assert MissingValueError("asym_power_sensor", "measured_object", [NaN]) in required_values_errors
    assert MissingValueError("asym_power_sensor", "measured_terminal_type", [NaN]) in required_values_errors
    assert (MissingValueError("asym_power_sensor", "power_sigma", [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError("asym_power_sensor", "p_measured", [NaN]) in required_values_errors) == se_dependent
    assert (MissingValueError("asym_power_sensor", "q_measured", [NaN]) in required_values_errors) == se_dependent

    assert MissingValueError("fault", "id", [NaN]) in required_values_errors
    assert (MissingValueError("fault", "status", [NaN]) in required_values_errors) == sc_dependent
    assert (MissingValueError("fault", "fault_type", [NaN]) in required_values_errors) == sc_dependent


def test_validate_required_values_asym_calculation():
    line = initialize_array("input", "line", 1)
    shunt = initialize_array("input", "shunt", 1)

    data = {"line": line, "shunt": shunt}
    required_values_errors = validate_required_values(data=data, symmetric=False)

    assert MissingValueError("line", "r0", [NaN]) in required_values_errors
    assert MissingValueError("line", "x0", [NaN]) in required_values_errors
    assert MissingValueError("line", "c0", [NaN]) in required_values_errors
    assert MissingValueError("line", "tan0", [NaN]) in required_values_errors

    assert MissingValueError("shunt", "g0", [NaN]) in required_values_errors
    assert MissingValueError("shunt", "b0", [NaN]) in required_values_errors


@pytest.mark.parametrize("fault_types", product(list(FaultType), list(FaultType)))
def test_validate_fault_sc_calculation(fault_types):
    line = initialize_array("input", "line", 1)
    shunt = initialize_array("input", "shunt", 1)
    fault = initialize_array("input", "fault", 2)
    fault["fault_type"] = fault_types

    data = {"line": line, "shunt": shunt, "fault": fault}
    required_values_errors = validate_required_values(data=data, calculation_type=CalculationType.short_circuit)

    asym_sc_calculation = np.any(
        list(fault_type not in (FaultType.three_phase, FaultType.nan) for fault_type in fault_types)
    )

    assert (MissingValueError("line", "r0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError("line", "x0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError("line", "c0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError("line", "tan0", [NaN]) in required_values_errors) == asym_sc_calculation

    assert (MissingValueError("shunt", "g0", [NaN]) in required_values_errors) == asym_sc_calculation
    assert (MissingValueError("shunt", "b0", [NaN]) in required_values_errors) == asym_sc_calculation


def test_validate_values():
    # Create invalid nodes and lines
    node = initialize_array("input", "node", 3)
    line = initialize_array("input", "line", 3)

    # Validate nodes and lines individually
    node_errors = validate_values({"node": node})
    line_errors = validate_values({"line": line})

    # Validate nodes and lines combined
    both_errors = validate_values({"node": node, "line": line})

    # The errors should add up (in this simple case)
    assert both_errors == node_errors + line_errors


def test_validate_values__calculation_types():
    # Create invalid sensor
    sym_voltage_sensor = initialize_array("input", "sym_voltage_sensor", 3)
    all_errors = validate_values({"sym_voltage_sensor": sym_voltage_sensor})
    power_flow_errors = validate_values(
        {"sym_voltage_sensor": sym_voltage_sensor}, calculation_type=CalculationType.power_flow
    )
    state_estimation_errors = validate_values(
        {"sym_voltage_sensor": sym_voltage_sensor}, calculation_type=CalculationType.state_estimation
    )

    assert not power_flow_errors
    assert all_errors == state_estimation_errors


@pytest.mark.parametrize(
    ("sensor_type", "parameter"),
    [
        ("sym_voltage_sensor", "u_sigma"),
        ("asym_voltage_sensor", "u_sigma"),
        ("sym_power_sensor", "power_sigma"),
        ("asym_power_sensor", "power_sigma"),
    ],
)
def test_validate_values__infinite_sigmas(sensor_type, parameter):
    sensor_array = initialize_array("input", sensor_type, 3)
    sensor_array[parameter] = np.inf
    all_errors = validate_values({sensor_type: sensor_array})

    for error in all_errors:
        assert not isinstance(error, InfinityError)


@pytest.mark.parametrize(
    ("sensor_type", "values", "error_types"),
    [
        (
            "sym_power_sensor",
            [[np.nan, np.nan], [np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError],
        ),
        (
            "sym_power_sensor",
            [[0.1, np.nan], [np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "sym_power_sensor",
            [[np.nan, 0.1], [np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "sym_power_sensor",
            [[0.1, 0.1], [np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [0.1, np.nan, 0.1], [np.nan, 0.1, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [0.1, np.nan, np.nan], [np.nan, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [np.nan, np.nan, np.nan], [0.1, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [0.1, 0.1, 0.1], [np.nan, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [np.nan, np.nan, np.nan], [0.1, 0.1, 0.1]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [np.nan, np.nan, np.nan], [0.1, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [0.1, np.nan, np.nan], [0.1, np.nan, np.nan]],
            [InvalidIdError, NotUniqueError, MultiFieldValidationError],
        ),
        (
            "asym_power_sensor",
            [[np.nan, np.nan], [0.1, 0.1, 0.1], [0.1, 0.1, 0.1]],
            [InvalidIdError, NotUniqueError],
        ),
    ],
)
def test_validate_values__bad_p_q_sigma(sensor_type, values, error_types):
    def arbitrary_fill(array, sensor_type, values):
        if sensor_type == "sym_power_sensor":
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

    sensor_array = initialize_array("input", sensor_type, 3)
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
            [InvalidIdError, MultiFieldValidationError],
        ),
        (
            [[np.nan, np.nan], [[np.nan, 0.1, np.nan], [np.nan, np.nan, np.nan]]],
            [InvalidIdError, MultiFieldValidationError],
        ),
        (
            [[np.nan, np.nan], [[np.nan, 0.1, np.nan], [np.nan, 0.1, np.nan]]],
            [InvalidIdError, MultiFieldValidationError],
        ),
        (
            [[0.1, 0.1], [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]]],
            [InvalidIdError, MultiFieldValidationError],
        ),
        ([[0.1, 0.1], [[np.nan, np.nan, np.nan], [np.nan, np.nan, np.nan]]], [InvalidIdError]),
        ([[np.nan, np.nan], [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1]]], [InvalidIdError]),
        ([[0.1, 0.1], [[0.1, 0.1, 0.1], [0.1, 0.1, 0.1]]], [InvalidIdError]),
    ],
)
def test_validate_values__bad_p_q_sigma_both_components(values, error_types):
    def two_component_data(values):
        node = initialize_array("input", "node", 1)
        node["id"] = 123
        sym_power_sensor = initialize_array("input", "sym_power_sensor", 1)
        sym_power_sensor["p_sigma"] = values[0][0]
        sym_power_sensor["q_sigma"] = values[0][1]
        sym_power_sensor["id"] = 456
        asym_power_sensor = initialize_array("input", "asym_power_sensor", 1)
        asym_power_sensor["p_measured"] = values[1][0]
        asym_power_sensor["q_measured"] = values[1][1]
        asym_power_sensor["id"] = 789

        return {
            "node": node,
            "sym_power_sensor": sym_power_sensor,
            "asym_power_sensor": asym_power_sensor,
        }

    data = two_component_data(values)
    all_errors = validate_values(data)
    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in error_types)
        assert (data[error.component]["id"] == error.ids).all()


def test_validate_values__bad_p_q_sigma_single_component_twice():
    def single_component_twice_data():
        node = initialize_array("input", "node", 1)
        node["id"] = 123
        sym_power_sensor = initialize_array("input", "sym_power_sensor", 2)
        sym_power_sensor["p_sigma"] = [np.nan, 0.1]
        sym_power_sensor["q_sigma"] = [np.nan, np.nan]
        sym_power_sensor["id"] = [456, 789]

        return {
            "node": node,
            "sym_power_sensor": sym_power_sensor,
        }

    data = single_component_twice_data()
    all_errors = validate_values(data)
    for error in all_errors:
        assert any(isinstance(error, error_type) for error_type in [InvalidIdError, MultiFieldValidationError])
        if isinstance(error, MultiFieldValidationError):
            assert error.ids[0] == 789


@pytest.mark.parametrize("measured_terminal_type", MeasuredTerminalType)
@patch("power_grid_model.validation.validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation.validation.all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation.validation.all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation.validation.all_valid_ids")
def test_validate_generic_power_sensor__all_terminal_types(
    all_valid_ids: MagicMock, measured_terminal_type: MeasuredTerminalType
):
    # Act
    validate_generic_power_sensor(data={}, component="")

    # Assert
    all_valid_ids.assert_any_call(
        ANY, ANY, field=ANY, ref_components=ANY, measured_terminal_type=measured_terminal_type
    )


@pytest.mark.parametrize(
    ("ref_component", "measured_terminal_type"),
    [
        (["line", "transformer"], MeasuredTerminalType.branch_from),
        (["line", "transformer"], MeasuredTerminalType.branch_to),
        ("source", MeasuredTerminalType.source),
        ("shunt", MeasuredTerminalType.shunt),
        (["sym_load", "asym_load"], MeasuredTerminalType.load),
        (["sym_gen", "asym_gen"], MeasuredTerminalType.generator),
        ("three_winding_transformer", MeasuredTerminalType.branch3_1),
        ("three_winding_transformer", MeasuredTerminalType.branch3_2),
        ("three_winding_transformer", MeasuredTerminalType.branch3_3),
        ("node", MeasuredTerminalType.node),
    ],
)
@patch("power_grid_model.validation.validation.validate_base", new=MagicMock())
@patch("power_grid_model.validation.validation.all_greater_than_zero", new=MagicMock())
@patch("power_grid_model.validation.validation.all_valid_enum_values", new=MagicMock())
@patch("power_grid_model.validation.validation.all_valid_ids")
def test_validate_generic_power_sensor__terminal_types(
    all_valid_ids: MagicMock, ref_component: Union[str, List[str]], measured_terminal_type: MeasuredTerminalType
):
    # Act
    validate_generic_power_sensor(data={}, component="")

    # Assert
    all_valid_ids.assert_any_call(
        ANY, ANY, field=ANY, ref_components=ref_component, measured_terminal_type=measured_terminal_type
    )


def test_power_sigma_or_p_q_sigma():
    # node
    node = initialize_array("input", "node", 2)
    node["id"] = np.array([0, 3])
    node["u_rated"] = [10.5e3, 10.5e3]

    # line
    line = initialize_array("input", "line", 1)
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
    sym_load = initialize_array("input", "sym_load", 2)
    sym_load["id"] = [4, 9]
    sym_load["node"] = [3, 0]
    sym_load["status"] = [1, 1]
    sym_load["type"] = [LoadGenType.const_power, LoadGenType.const_power]
    sym_load["p_specified"] = [1e6, 1e6]
    sym_load["q_specified"] = [-1e6, -1e6]

    # source
    source = initialize_array("input", "source", 1)
    source["id"] = [1]
    source["node"] = [0]
    source["status"] = [1]
    source["u_ref"] = [1.0]

    # voltage sensor
    voltage_sensor = initialize_array("input", "sym_voltage_sensor", 1)
    voltage_sensor["id"] = 5
    voltage_sensor["measured_object"] = 0
    voltage_sensor["u_sigma"] = [100.0]
    voltage_sensor["u_measured"] = [10.5e3]

    # power sensor
    sym_power_sensor = initialize_array("input", "sym_power_sensor", 3)
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

    # all
    input_data = {
        "node": node,
        "line": line,
        "sym_load": sym_load,
        "source": source,
        "sym_voltage_sensor": voltage_sensor,
        "sym_power_sensor": sym_power_sensor,
    }

    assert_valid_input_data(input_data=input_data, calculation_type=CalculationType.state_estimation)
