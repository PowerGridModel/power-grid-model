# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from copy import copy

import pytest

from power_grid_model import PowerGridModel
from power_grid_model.core.power_grid_meta import initialize_array
from power_grid_model.enum import CalculationMethod, LoadGenType, MeasuredTerminalType
from power_grid_model.errors import (
    AutomaticTapCalculationError,
    ConflictID,
    ConflictVoltage,
    IDWrongType,
    InvalidBranch,
    InvalidBranch3,
    InvalidCalculationMethod,
    InvalidMeasuredObject,
    InvalidTransformerClock,
    NotObservableError,
)


def test_empty_model():
    model = PowerGridModel.__new__(PowerGridModel)
    with pytest.raises(TypeError):
        model.calculate_power_flow()
    with pytest.raises(TypeError):
        model.copy()
    with pytest.raises(TypeError):
        n = model.all_component_count
    with pytest.raises(TypeError):
        copy(model)


def test_unknown_component_types():
    model = PowerGridModel(input_data={})
    with pytest.raises(KeyError, match=r"fake_type"):
        model.calculate_power_flow(output_component_types={"fake_type"})


def test_handle_conflict_voltage_error():
    node_input = initialize_array("input", "node", 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [10.0e3, 20.0e3]

    line_input = initialize_array("input", "line", 1)
    line_input["id"] = [2]
    line_input["from_node"] = [0]
    line_input["to_node"] = [1]

    with pytest.raises(ConflictVoltage):
        PowerGridModel(input_data={"node": node_input, "line": line_input})


@pytest.mark.skip(reason="TODO")
def test_handle_missing_case_for_enum_error():
    pass


def test_handle_invalid_branch_error():
    node_input = initialize_array("input", "node", 1)
    node_input["id"] = [0]
    node_input["u_rated"] = [0.0]

    line_input = initialize_array("input", "line", 1)
    line_input["id"] = [1]
    line_input["from_node"] = [0]
    line_input["to_node"] = [0]

    with pytest.raises(InvalidBranch):
        PowerGridModel(input_data={"node": node_input, "line": line_input})


def test_handle_invalid_branch3_error():
    node_input = initialize_array("input", "node", 1)
    node_input["id"] = [0]
    node_input["u_rated"] = [0.0]

    three_winding_transformer_input = initialize_array("input", "three_winding_transformer", 1)
    three_winding_transformer_input["id"] = [1]
    three_winding_transformer_input["node_1"] = [0]
    three_winding_transformer_input["node_2"] = [0]
    three_winding_transformer_input["node_3"] = [0]

    with pytest.raises(InvalidBranch3):
        PowerGridModel(input_data={"node": node_input, "three_winding_transformer": three_winding_transformer_input})


def test_handle_invalid_transformer_clock_error():
    node_input = initialize_array("input", "node", 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [0.0, 0.0]

    transformer_input = initialize_array("input", "transformer", 1)
    transformer_input["id"] = [2]
    transformer_input["from_node"] = [0]
    transformer_input["to_node"] = [1]
    transformer_input["clock"] = [-1]

    with pytest.raises(InvalidTransformerClock):
        PowerGridModel(input_data={"node": node_input, "transformer": transformer_input})


@pytest.mark.skip(reason="TODO")
def test_handle_sparse_matrix_error():
    pass


def test_handle_not_observable_error():
    node_input = initialize_array("input", "node", 1)
    node_input["id"] = [0]
    node_input["u_rated"] = [10.0e3]

    source_input = initialize_array("input", "source", 1)
    source_input["id"] = [1]
    source_input["node"] = [0]
    source_input["status"] = [1]

    sym_load_input = initialize_array("input", "sym_load", 1)
    sym_load_input["id"] = [2]
    sym_load_input["node"] = [0]
    sym_load_input["status"] = [1]
    sym_load_input["type"] = [LoadGenType.const_power]

    model = PowerGridModel(input_data={"node": node_input, "source": source_input, "sym_load": sym_load_input})
    with pytest.raises(NotObservableError):
        model.calculate_state_estimation(calculation_method=CalculationMethod.iterative_linear)


@pytest.mark.skip(reason="TODO")
def test_handle_iteration_diverge_error():
    pass


def test_handle_conflict_id_error():
    node_input = initialize_array("input", "node", 2)
    node_input["id"] = [0, 0]
    node_input["u_rated"] = [0.0, 0.0]

    with pytest.raises(ConflictID):
        PowerGridModel(input_data={"node": node_input})


def test_handle_invalid_measured_object_error():
    node_input = initialize_array("input", "node", 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [0.0, 0.0]

    link_input = initialize_array("input", "link", 1)
    link_input["id"] = [2]
    link_input["from_node"] = [0]
    link_input["to_node"] = [1]

    sym_power_sensor_input = initialize_array("input", "sym_power_sensor", 1)
    sym_power_sensor_input["id"] = [3]
    sym_power_sensor_input["measured_object"] = [2]
    sym_power_sensor_input["measured_terminal_type"] = [MeasuredTerminalType.branch_from]

    with pytest.raises(InvalidMeasuredObject):
        PowerGridModel(input_data={"node": node_input, "link": link_input, "sym_power_sensor": sym_power_sensor_input})


@pytest.mark.skip(reason="TODO")
def test_handle_invalid_regulated_object_error():
    pass


def test_handle_id_wrong_type_error():
    node_input = initialize_array("input", "node", 1)
    node_input["id"] = [0]
    node_input["u_rated"] = [0.0]

    sym_power_sensor_input = initialize_array("input", "sym_power_sensor", 1)
    sym_power_sensor_input["id"] = [1]
    sym_power_sensor_input["measured_object"] = [0]
    sym_power_sensor_input["measured_terminal_type"] = [MeasuredTerminalType.branch_from]

    with pytest.raises(IDWrongType):
        PowerGridModel(input_data={"node": node_input, "sym_power_sensor": sym_power_sensor_input})


def test_handle_invalid_calculation_method_error():
    node_input = initialize_array("input", "node", 1)
    node_input["id"] = [0]
    node_input["u_rated"] = [10.0e3]

    source_input = initialize_array("input", "source", 1)
    source_input["id"] = [1]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [10.0e3]

    sym_load_input = initialize_array("input", "sym_load", 1)
    sym_load_input["id"] = [2]
    sym_load_input["node"] = [0]
    sym_load_input["status"] = [1]
    sym_load_input["type"] = [LoadGenType.const_power]
    sym_load_input["p_specified"] = [10.0e3]
    sym_load_input["q_specified"] = [1.0e3]

    model = PowerGridModel(input_data={"node": node_input, "source": source_input, "sym_load": sym_load_input})
    with pytest.raises(InvalidCalculationMethod):
        model.calculate_power_flow(calculation_method=CalculationMethod.iec60909)


@pytest.mark.xfail(reason="TODO: Automatic tap changer")
def test_transformer_tap_regulator_at_lv_tap_side():
    node_input = initialize_array("input", "node", 2)
    node_input["id"] = [0, 1]
    node_input["u_rated"] = [1e4, 4e2]

    source_input = initialize_array("input", "source", 1)
    source_input["id"] = [2]
    source_input["node"] = [0]
    source_input["status"] = [1]
    source_input["u_ref"] = [10.0e3]

    transformer_input = initialize_array("input", "transformer", 1)
    transformer_input["id"] = [3]
    transformer_input["from_node"] = [0]
    transformer_input["to_node"] = [1]
    transformer_input["from_status"] = [1]
    transformer_input["to_status"] = [1]
    transformer_input["u1"] = [1e4]
    transformer_input["u2"] = [4e2]
    transformer_input["sn"] = [1e5]
    transformer_input["uk"] = [0.1]
    transformer_input["pk"] = [1e3]
    transformer_input["i0"] = [1.0e-6]
    transformer_input["p0"] = [0.1]
    transformer_input["winding_from"] = [2]
    transformer_input["winding_to"] = [1]
    transformer_input["clock"] = [5]
    transformer_input["tap_side"] = [1]
    transformer_input["tap_pos"] = [3]
    transformer_input["tap_min"] = [-11]
    transformer_input["tap_max"] = [9]
    transformer_input["tap_size"] = [100]

    model = PowerGridModel(input_data={"node": node_input, "transformer": transformer_input, "source": source_input})
    with pytest.raises(AutomaticTapCalculationError):
        model.calculate_power_flow()


@pytest.mark.skip(reason="TODO")
def test_handle_power_grid_dataset_error():
    pass


@pytest.mark.skip(reason="TODO")
def test_handle_power_grid_unreachable_error():
    pass
