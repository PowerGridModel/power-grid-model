# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import numpy as np
import pytest

from power_grid_model import (
    Branch3Side,
    BranchSide,
    ComponentType,
    DatasetType,
    LoadGenType,
    MeasuredTerminalType,
    WindingType,
    initialize_array,
)
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset
from power_grid_model.enum import CalculationType, ComponentAttributeFilterOptions, FaultPhase, FaultType
from power_grid_model.validation import validate_input_data
from power_grid_model.validation.errors import (
    FaultPhaseError,
    InvalidAssociatedEnumValueError,
    InvalidEnumValueError,
    InvalidIdError,
    MultiComponentNotUniqueError,
    MultiFieldValidationError,
    NotBetweenError,
    NotBetweenOrAtError,
    NotBooleanError,
    NotGreaterOrEqualError,
    NotGreaterThanError,
    NotIdenticalError,
    NotLessThanError,
    NotUniqueError,
    TwoValuesZeroError,
)
from power_grid_model.validation.utils import _nan_type


@pytest.fixture
def original_data() -> dict[ComponentType, np.ndarray]:
    node = initialize_array(DatasetType.input, ComponentType.node, 4)
    node["id"] = [0, 2, 1, 2]
    node["u_rated"] = [10.5e3, 10.5e3, 0, 10.5e3]

    line = initialize_array(DatasetType.input, ComponentType.line, 3)
    line["id"] = [3, 4, 5]
    line["from_node"] = [0, -1, 2]
    line["to_node"] = [2, 1, 8]
    line["from_status"] = [0, 1, -2]
    line["to_status"] = [0, 4, 1]
    line["r1"] = [0, 100, 1]
    line["x1"] = [0, 5, 0]
    line["r0"] = [10, 0, 0]
    line["x0"] = [0, 0, 50]
    line["i_n"] = [-3, 0, 50]

    asym_line = initialize_array(DatasetType.input, ComponentType.asym_line, 4)
    asym_line["id"] = [55, 56, 57, 58]
    asym_line["from_node"] = [0, 1, 0, 0]
    asym_line["to_node"] = [1, 2, 2, 1]
    asym_line["from_status"] = [1, 1, 1, 1]
    asym_line["to_status"] = [1, 1, 1, 1]
    asym_line["r_aa"] = [-1, 2, 2, 2]
    asym_line["r_ba"] = [-1, 2, 2, 2]
    asym_line["r_bb"] = [-1, 2, 2, 2]
    asym_line["r_ca"] = [-1, 2, 2, 2]
    asym_line["r_cb"] = [-1, 2, 2, 2]
    asym_line["r_cc"] = [-1, 2, 2, 2]
    asym_line["r_na"] = [-1, 2, 2, 2]
    asym_line["r_nb"] = [-1, 2, 2, 2]
    asym_line["r_nc"] = [-1, 2, 2, 2]
    asym_line["r_nn"] = [-1, np.nan, 2, 2]
    asym_line["x_aa"] = [-1, 2, 2, 2]
    asym_line["x_ba"] = [-1, 2, 2, 2]
    asym_line["x_bb"] = [-1, 2, 2, 2]
    asym_line["x_ca"] = [-1, 2, 2, 2]
    asym_line["x_cb"] = [-1, 2, 2, 2]
    asym_line["x_cc"] = [-1, 2, 2, 2]
    asym_line["x_na"] = [-1, 2, np.nan, 2]
    asym_line["x_nb"] = [-1, 2, 2, 2]
    asym_line["x_nc"] = [-1, 2, 2, 2]
    asym_line["x_nn"] = [-1, 2, 2, 2]
    asym_line["c_aa"] = [-1, np.nan, 2, np.nan]
    asym_line["c_ba"] = [-1, np.nan, 2, np.nan]
    asym_line["c_bb"] = [-1, np.nan, 2, np.nan]
    asym_line["c_ca"] = [-1, np.nan, 2, np.nan]
    asym_line["c_cb"] = [-1, np.nan, 2, np.nan]
    asym_line["c_cc"] = [-1, np.nan, 2, 2]
    asym_line["c0"] = [-1, np.nan, np.nan, np.nan]
    asym_line["c1"] = [-1, np.nan, np.nan, np.nan]
    asym_line["i_n"] = [50, 50, 50, 50]

    generic_branch = initialize_array(DatasetType.input, ComponentType.generic_branch, 1)
    generic_branch["id"] = [6]
    generic_branch["from_node"] = [1]
    generic_branch["to_node"] = [2]
    generic_branch["from_status"] = [1]
    generic_branch["to_status"] = [1]
    generic_branch["r1"] = [0.129059]
    generic_branch["x1"] = [16.385859]
    generic_branch["g1"] = [8.692e-7]
    generic_branch["b1"] = [-2.336e-7]
    generic_branch["k"] = [0.0]
    generic_branch["theta"] = [0.0]
    generic_branch["sn"] = [-10.0]

    link = initialize_array(DatasetType.input, ComponentType.link, 2)
    link["id"] = [12, 13]
    link["from_node"] = [0, -1]
    link["to_node"] = [8, 1]
    link["from_status"] = [3, 1]
    link["to_status"] = [0, 4]

    transformer = initialize_array(DatasetType.input, ComponentType.transformer, 3)
    transformer["id"] = [1, 14, 15]
    transformer["from_node"] = [1, 7, 2]  # TODO check from node 1 to node 1
    transformer["to_node"] = [1, 8, 1]
    transformer["from_status"] = [1, 1, -1]
    transformer["to_status"] = [2, 0, 1]
    transformer["u1"] = [10500.0, 0.0, -1.0]
    transformer["u2"] = [400.0, -3.0, 0.0]
    transformer["sn"] = [630000.0, 0.0, -20.0]
    transformer["uk"] = [-1.0, 1.1, 0.9]
    transformer["pk"] = [63000.0, 0.0, -10.0]
    transformer["i0"] = [0.0368, 10.0, 0.9]
    transformer["p0"] = [63000.0, 0.0, -10.0]
    transformer["winding_from"] = [8, 0, 2]
    transformer["winding_to"] = [5, 1, 2]
    transformer["clock"] = [13, -1, 7]
    transformer["tap_side"] = [-1, 0, 1]
    transformer["tap_pos"] = [-1, 6, -4]
    transformer["tap_min"] = [-2, 4, 3]
    transformer["tap_max"] = [2, -4, -3]
    transformer["tap_nom"] = [-3, _nan_type("transformer", "tap_nom"), 4]
    transformer["tap_size"] = [262.5, 0.0, -10.0]
    transformer["uk_min"] = [0.0000000005, _nan_type("transformer", "uk_min"), 0.9]
    transformer["uk_max"] = [0.0000000005, _nan_type("transformer", "uk_max"), 0.8]
    transformer["pk_min"] = [300.0, 0.0, _nan_type("transformer", "pk_min")]
    transformer["pk_max"] = [400.0, -0.1, _nan_type("transformer", "pk_max")]

    three_winding_transformer = initialize_array(DatasetType.input, ComponentType.three_winding_transformer, 4)
    three_winding_transformer["id"] = [1, 28, 29, 30]
    three_winding_transformer["node_1"] = [0, 1, 9, 2]
    three_winding_transformer["node_2"] = [1, 15, 1, 0]
    three_winding_transformer["node_3"] = [1, 2, 12, 1]
    three_winding_transformer["status_1"] = [1, 5, 1, 1]
    three_winding_transformer["status_2"] = [2, 1, 1, 1]
    three_winding_transformer["status_3"] = [1, 0, -1, 0]
    three_winding_transformer["u1"] = [-100, 0, 200, 100]
    three_winding_transformer["u2"] = [0, -200, 100, 200]
    three_winding_transformer["u3"] = [-100, 0, 400, 300]
    three_winding_transformer["sn_1"] = [0, -1200, 100, 300]
    three_winding_transformer["sn_2"] = [-1000, 0, 200, 200]
    three_winding_transformer["sn_3"] = [0, -2300, 300, 100]
    three_winding_transformer["uk_12"] = [-1, 1.1, 0.05, 0.1]
    three_winding_transformer["uk_13"] = [-2, 1.2, 0.3, 0.2]
    three_winding_transformer["uk_23"] = [-1.5, 1, 0.15, 0.2]
    three_winding_transformer["pk_12"] = [-450, 100, 10, 40]
    three_winding_transformer["pk_13"] = [-40, 50, 40, 50]
    three_winding_transformer["pk_23"] = [-120, 1, 40, 30]
    three_winding_transformer["i0"] = [-0.5, 1.8, 0.3, 0.6]
    three_winding_transformer["p0"] = [-100, 410, 60, 40]
    three_winding_transformer["winding_1"] = [15, -1, 0, 2]
    three_winding_transformer["winding_2"] = [19, -2, 1, 3]
    three_winding_transformer["winding_3"] = [-2, 13, 2, 2]
    three_winding_transformer["clock_12"] = [-12, 24, 4, 3]
    three_winding_transformer["clock_13"] = [-30, 40, 3, 4]
    three_winding_transformer["tap_side"] = [-1, 9, 1, 0]
    three_winding_transformer["tap_pos"] = [50, -24, 5, 3]
    three_winding_transformer["tap_min"] = [-10, -10, -10, -10]
    three_winding_transformer["tap_max"] = [10, 10, 10, 10]
    three_winding_transformer["tap_size"] = [-12, 0, 3, 130]
    three_winding_transformer["tap_nom"] = [-12, 41, _nan_type("three_winding_transformer", "tap_nom"), 0]
    three_winding_transformer["uk_12_min"] = [
        _nan_type("three_winding_transformer", "uk_12_min"),
        1.1,
        0.05,
        _nan_type("three_winding_transformer", "uk_12_min"),
    ]
    three_winding_transformer["uk_13_min"] = [
        _nan_type("three_winding_transformer", "uk_13_min"),
        1.2,
        0.3,
        _nan_type("three_winding_transformer", "uk_13_min"),
    ]
    three_winding_transformer["uk_23_min"] = [
        _nan_type("three_winding_transformer", "uk_23_min"),
        1,
        0.15,
        _nan_type("three_winding_transformer", "uk_23_min"),
    ]
    three_winding_transformer["pk_12_min"] = [-450, _nan_type("three_winding_transformer", "pk_12_min"), 10, 40]
    three_winding_transformer["pk_13_min"] = [-40, _nan_type("three_winding_transformer", "pk_13_min"), 40, 50]
    three_winding_transformer["pk_23_min"] = [-120, _nan_type("three_winding_transformer", "pk_23_min"), 40, 30]
    three_winding_transformer["uk_12_max"] = [
        _nan_type("three_winding_transformer", "uk_12_max"),
        1.1,
        0.05,
        _nan_type("three_winding_transformer", "uk_12_max"),
    ]
    three_winding_transformer["uk_13_max"] = [
        _nan_type("three_winding_transformer", "uk_13_max"),
        1.2,
        0.3,
        _nan_type("three_winding_transformer", "uk_13_max"),
    ]
    three_winding_transformer["uk_23_max"] = [
        _nan_type("three_winding_transformer", "uk_23_max"),
        1,
        0.15,
        _nan_type("three_winding_transformer", "uk_23_max"),
    ]
    three_winding_transformer["pk_12_max"] = [-450, _nan_type("three_winding_transformer", "pk_12_max"), 10, 40]
    three_winding_transformer["pk_13_max"] = [-40, _nan_type("three_winding_transformer", "pk_12_max"), 40, 50]
    three_winding_transformer["pk_23_max"] = [-120, _nan_type("three_winding_transformer", "pk_12_max"), 40, 30]

    transformer_tap_regulator = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 5)
    transformer_tap_regulator["id"] = [51, 52, 53, 54, 1]
    transformer_tap_regulator["status"] = [0, -1, 2, 1, 5]
    transformer_tap_regulator["regulated_object"] = [14, 15, 28, 14, 2]
    transformer_tap_regulator["control_side"] = [1, 2, 0, 0, 60]
    transformer_tap_regulator["u_set"] = [100, -100, 100, 100, 100]
    transformer_tap_regulator["u_band"] = [100, -4, 100, 100, 0]
    transformer_tap_regulator["line_drop_compensation_r"] = [0.0, -1.0, 1.0, 0.0, 2.0]
    transformer_tap_regulator["line_drop_compensation_x"] = [0.0, 4.0, 2.0, 0.0, -4.0]

    source = initialize_array(DatasetType.input, ComponentType.source, 3)
    source["id"] = [16, 17, 1]
    source["node"] = [10, 1, 2]
    source["status"] = [0, -1, 2]
    source["u_ref"] = [-10.0, 0.0, 100.0]
    source["sk"] = [0.0, 100.0, -20.0]
    source["rx_ratio"] = [0.0, -30.0, 300.0]
    source["z01_ratio"] = [-1.0, 0.0, 200.0]

    shunt = initialize_array(DatasetType.input, ComponentType.shunt, 3)
    shunt["id"] = [18, 19, 1]
    shunt["node"] = [10, 1, 2]
    shunt["status"] = [0, -1, 2]

    sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 3)
    sym_load["id"] = [1, 20, 21]
    sym_load["type"] = [1, 0, 5]
    sym_load["node"] = [10, 1, 2]
    sym_load["status"] = [0, -1, 2]

    sym_gen = initialize_array(DatasetType.input, ComponentType.sym_gen, 3)
    sym_gen["id"] = [1, 22, 23]
    sym_gen["type"] = [2, -1, 1]
    sym_gen["node"] = [10, 1, 2]
    sym_gen["status"] = [0, -1, 2]

    asym_load = initialize_array(DatasetType.input, ComponentType.asym_load, 3)
    asym_load["id"] = [1, 24, 25]
    asym_load["type"] = [5, 0, 2]
    asym_load["node"] = [10, 1, 2]
    asym_load["status"] = [0, -1, 2]

    asym_gen = initialize_array(DatasetType.input, ComponentType.asym_gen, 3)
    asym_gen["id"] = [1, 26, 27]
    asym_gen["type"] = [-1, 5, 2]
    asym_gen["node"] = [10, 1, 2]
    asym_gen["status"] = [0, -1, 2]

    sym_voltage_sensor = initialize_array(DatasetType.input, ComponentType.sym_voltage_sensor, 4)
    sym_voltage_sensor["id"] = [7, 8, 9, 10]
    sym_voltage_sensor["measured_object"] = [2, 3, 1, 200]
    sym_voltage_sensor["u_measured"] = [0.0, 10.4e3, 10.6e3, -20.0]
    sym_voltage_sensor["u_sigma"] = [1.0, np.nan, 0.0, -1.0]

    asym_voltage_sensor = initialize_array(DatasetType.input, ComponentType.asym_voltage_sensor, 4)
    asym_voltage_sensor["id"] = [7, 8, 9, 10]
    asym_voltage_sensor["measured_object"] = [2, 3, 1, 200]
    asym_voltage_sensor["u_measured"] = np.array(
        [
            [10.5e3, 10.4e3, 10.6e3],
            [np.nan, np.nan, np.nan],
            [0, 0, 0],
            [-1e4, 1e4, 1e4],
        ]
    )
    asym_voltage_sensor["u_sigma"] = [1.0, np.nan, 0.0, -1.0]

    sym_power_sensor = initialize_array(DatasetType.input, ComponentType.sym_power_sensor, 4)
    sym_power_sensor["id"] = [7, 8, 9, 10]
    sym_power_sensor["measured_object"] = [12, 3, 13, 200]
    sym_power_sensor["power_sigma"] = [1.0, np.nan, 0.0, -1.0]
    sym_power_sensor["measured_terminal_type"] = [1, 1, 10, 1]

    asym_power_sensor = initialize_array(DatasetType.input, ComponentType.asym_power_sensor, 4)
    asym_power_sensor["id"] = [7, 8, 9, 10]
    asym_power_sensor["measured_object"] = [12, 3, 13, 200]
    asym_power_sensor["power_sigma"] = [1.0, np.nan, 0.0, -1.0]
    asym_power_sensor["measured_terminal_type"] = [1, 1, 10, 1]

    fault = initialize_array(DatasetType.input, ComponentType.fault, 20)
    fault["id"] = [1] + list(range(32, 51))
    fault["status"] = [0, -1, 2] + 17 * [1]
    fault["fault_type"] = 6 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [_nan_type("fault", "fault_type"), 4]
    fault["fault_phase"] = (
        list(range(1, 7)) + [0, 4, 5, 6] + 2 * list(range(4)) + [_nan_type("fault", "fault_phase"), 7]
    )
    fault["fault_object"] = [200, 3] + list(range(10, 28, 2)) + 9 * [0]
    fault["r_f"] = [-1.0, 0.0, 1.0] + 17 * [_nan_type("fault", "r_f")]
    fault["x_f"] = [-1.0, 0.0, 1.0] + 17 * [_nan_type("fault", "x_f")]
    data = {
        ComponentType.node: node,
        ComponentType.line: line,
        ComponentType.asym_line: asym_line,
        ComponentType.generic_branch: generic_branch,
        ComponentType.link: link,
        ComponentType.transformer: transformer,
        ComponentType.three_winding_transformer: three_winding_transformer,
        ComponentType.transformer_tap_regulator: transformer_tap_regulator,
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
        ComponentType.fault: fault,
    }
    return data


@pytest.fixture
def original_data_columnar_all(original_data):
    return compatibility_convert_row_columnar_dataset(
        original_data, ComponentAttributeFilterOptions.everything, DatasetType.input
    )


@pytest.fixture
def original_data_columnar_relevant(original_data):
    return compatibility_convert_row_columnar_dataset(
        original_data, ComponentAttributeFilterOptions.relevant, DatasetType.input
    )


@pytest.fixture(params=["original_data", "original_data_columnar_all", "original_data_columnar_relevant"])
def input_data(request):
    return request.getfixturevalue(request.param)


def test_validate_input_data_sym_calculation(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)

    assert (
        MultiComponentNotUniqueError(
            [
                (ComponentType.asym_gen, "id"),
                (ComponentType.asym_load, "id"),
                (ComponentType.asym_power_sensor, "id"),
                (ComponentType.asym_voltage_sensor, "id"),
                (ComponentType.node, "id"),
                (ComponentType.shunt, "id"),
                (ComponentType.source, "id"),
                (ComponentType.sym_gen, "id"),
                (ComponentType.sym_load, "id"),
                (ComponentType.sym_power_sensor, "id"),
                (ComponentType.sym_voltage_sensor, "id"),
                (ComponentType.transformer, "id"),
                (ComponentType.three_winding_transformer, "id"),
                (ComponentType.fault, "id"),
                (ComponentType.transformer_tap_regulator, "id"),
            ],
            [
                (ComponentType.asym_gen, 1),
                (ComponentType.asym_load, 1),
                (ComponentType.asym_power_sensor, 7),
                (ComponentType.asym_power_sensor, 8),
                (ComponentType.asym_power_sensor, 9),
                (ComponentType.asym_power_sensor, 10),
                (ComponentType.asym_voltage_sensor, 7),
                (ComponentType.asym_voltage_sensor, 8),
                (ComponentType.asym_voltage_sensor, 9),
                (ComponentType.asym_voltage_sensor, 10),
                (ComponentType.node, 1),
                (ComponentType.shunt, 1),
                (ComponentType.source, 1),
                (ComponentType.sym_gen, 1),
                (ComponentType.sym_load, 1),
                (ComponentType.sym_power_sensor, 7),
                (ComponentType.sym_power_sensor, 8),
                (ComponentType.sym_power_sensor, 9),
                (ComponentType.sym_power_sensor, 10),
                (ComponentType.sym_voltage_sensor, 7),
                (ComponentType.sym_voltage_sensor, 8),
                (ComponentType.sym_voltage_sensor, 9),
                (ComponentType.sym_voltage_sensor, 10),
                (ComponentType.transformer, 1),
                (ComponentType.three_winding_transformer, 1),
                (ComponentType.fault, 1),
                (ComponentType.transformer_tap_regulator, 1),
            ],
        )
        in validation_errors
    )

    assert NotGreaterThanError("node", "u_rated", [1], 0) in validation_errors
    assert NotUniqueError("node", "id", [2, 2]) in validation_errors

    assert NotBooleanError("line", "from_status", [5]) in validation_errors
    assert NotBooleanError("line", "to_status", [4]) in validation_errors
    assert InvalidIdError("line", "from_node", [4], "node") in validation_errors
    assert InvalidIdError("line", "to_node", [5], "node") in validation_errors
    assert TwoValuesZeroError("line", ["r1", "x1"], [3]) in validation_errors
    assert TwoValuesZeroError("line", ["r0", "x0"], [4]) in validation_errors
    assert NotGreaterThanError("line", "i_n", [3, 4], 0) in validation_errors

    assert NotBooleanError("link", "from_status", [12]) in validation_errors
    assert NotBooleanError("link", "to_status", [13]) in validation_errors
    assert InvalidIdError("link", "from_node", [13], "node") in validation_errors
    assert InvalidIdError("link", "to_node", [12], "node") in validation_errors

    assert NotBooleanError("transformer", "from_status", [15]) in validation_errors
    assert NotBooleanError("transformer", "to_status", [1]) in validation_errors
    assert InvalidIdError("transformer", "from_node", [14], "node") in validation_errors
    assert InvalidIdError("transformer", "to_node", [14], "node") in validation_errors
    assert NotGreaterThanError("transformer", "u1", [14, 15], 0) in validation_errors
    assert NotGreaterThanError("transformer", "u2", [14, 15], 0) in validation_errors
    assert NotGreaterThanError("transformer", "sn", [14, 15], 0) in validation_errors
    assert NotGreaterOrEqualError("transformer", "uk", [1], "pk/sn") in validation_errors
    assert NotBetweenError("transformer", "uk", [1, 14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError("transformer", "pk", [15], 0) in validation_errors
    assert NotGreaterOrEqualError("transformer", "i0", [1], "p0/sn") in validation_errors
    assert NotLessThanError("transformer", "i0", [14], 1) in validation_errors
    assert NotGreaterOrEqualError("transformer", "p0", [15], 0) in validation_errors
    assert NotBetweenOrAtError("transformer", "clock", [1, 14], (0, 12)) in validation_errors
    assert NotBetweenOrAtError("transformer", "tap_pos", [14, 15], ("tap_min", "tap_max")) in validation_errors
    assert NotBetweenOrAtError("transformer", "tap_nom", [1, 15], ("tap_min", "tap_max")) in validation_errors
    assert NotGreaterOrEqualError("transformer", "tap_size", [15], 0) in validation_errors
    assert NotGreaterOrEqualError("transformer", "uk_min", [1], "pk_min/sn") in validation_errors
    assert NotBetweenError("transformer", "uk_min", [14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError("transformer", "uk_max", [1], "pk_max/sn") in validation_errors
    assert NotBetweenError("transformer", "uk_max", [14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError("transformer", "pk_min", [15], 0) in validation_errors
    assert NotGreaterOrEqualError("transformer", "pk_max", [14, 15], 0) in validation_errors
    assert InvalidEnumValueError("transformer", "winding_from", [1], WindingType) in validation_errors
    assert InvalidEnumValueError("transformer", "winding_to", [1], WindingType) in validation_errors
    assert InvalidEnumValueError("transformer", "tap_side", [1], BranchSide) in validation_errors

    assert InvalidIdError("source", "node", [16], "node") in validation_errors
    assert NotBooleanError("source", "status", [17, 1]) in validation_errors
    assert NotGreaterThanError("source", "u_ref", [16, 17], 0) in validation_errors
    assert NotGreaterThanError("source", "sk", [16, 1], 0) in validation_errors
    assert NotGreaterOrEqualError("source", "rx_ratio", [17], 0) in validation_errors
    assert NotGreaterThanError("source", "z01_ratio", [16, 17], 0) in validation_errors

    assert InvalidIdError("shunt", "node", [18], "node") in validation_errors
    assert NotBooleanError("shunt", "status", [19, 1]) in validation_errors

    assert InvalidIdError("sym_load", "node", [1], "node") in validation_errors
    assert NotBooleanError("sym_load", "status", [20, 21]) in validation_errors
    assert InvalidEnumValueError("sym_load", "type", [21], LoadGenType) in validation_errors

    assert InvalidIdError("sym_gen", "node", [1], "node") in validation_errors
    assert NotBooleanError("sym_gen", "status", [22, 23]) in validation_errors
    assert InvalidEnumValueError("sym_gen", "type", [22], LoadGenType) in validation_errors

    assert InvalidIdError("asym_load", "node", [1], "node") in validation_errors
    assert NotBooleanError("asym_load", "status", [24, 25]) in validation_errors
    assert InvalidEnumValueError("asym_load", "type", [1], LoadGenType) in validation_errors

    assert InvalidIdError("asym_gen", "node", [1], "node") in validation_errors
    assert NotBooleanError("asym_gen", "status", [26, 27]) in validation_errors
    assert InvalidEnumValueError("asym_gen", "type", [1, 26], LoadGenType) in validation_errors

    assert InvalidIdError("sym_voltage_sensor", "measured_object", [8, 10], "node") in validation_errors
    assert NotGreaterThanError("sym_voltage_sensor", "u_measured", [7, 10], 0) in validation_errors
    assert NotGreaterThanError("sym_voltage_sensor", "u_sigma", [9, 10], 0) in validation_errors
    # TODO check if u_sigma = np.nan is detected with missing values

    assert InvalidIdError("asym_voltage_sensor", "measured_object", [8, 10], "node") in validation_errors
    assert NotGreaterThanError("asym_voltage_sensor", "u_measured", [9, 10], 0) in validation_errors
    assert NotGreaterThanError("asym_voltage_sensor", "u_sigma", [9, 10], 0) in validation_errors

    assert (
        InvalidIdError(
            "sym_power_sensor",
            "measured_object",
            [7, 9, 10],
            [
                "node",
                "line",
                "generic_branch",
                "transformer",
                "three_winding_transformer",
                "source",
                "shunt",
                "sym_load",
                "asym_load",
                "sym_gen",
                "asym_gen",
            ],
        )
        in validation_errors
    )
    assert NotGreaterThanError("sym_power_sensor", "power_sigma", [9, 10], 0) in validation_errors
    assert (
        InvalidIdError(
            "sym_power_sensor",
            "measured_object",
            [7, 10],
            ["line", "generic_branch", "transformer"],
            {"measured_terminal_type": MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError("sym_power_sensor", "measured_terminal_type", [9], MeasuredTerminalType)
        in validation_errors
    )

    assert (
        InvalidIdError(
            "asym_power_sensor",
            "measured_object",
            [7, 9, 10],
            [
                "node",
                "line",
                "generic_branch",
                "transformer",
                "three_winding_transformer",
                "source",
                "shunt",
                "sym_load",
                "asym_load",
                "sym_gen",
                "asym_gen",
            ],
        )
        in validation_errors
    )
    assert NotGreaterThanError("asym_power_sensor", "power_sigma", [9, 10], 0) in validation_errors
    assert (
        InvalidIdError(
            "asym_power_sensor",
            "measured_object",
            [7, 10],
            ["line", "generic_branch", "transformer"],
            {"measured_terminal_type": MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError("asym_power_sensor", "measured_terminal_type", [9], MeasuredTerminalType)
        in validation_errors
    )

    assert NotGreaterOrEqualError("transformer", "uk_max", [15], "uk_min") not in validation_errors

    assert NotBooleanError("fault", "status", [32, 33]) in validation_errors
    assert InvalidIdError("fault", "fault_object", [1] + list(range(32, 42)), ["node"]) in validation_errors


def test_validate_three_winding_transformer(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert NotBooleanError("three_winding_transformer", "status_1", [28]) in validation_errors
    assert NotBooleanError("three_winding_transformer", "status_2", [1]) in validation_errors
    assert NotBooleanError("three_winding_transformer", "status_3", [29]) in validation_errors
    assert InvalidIdError("three_winding_transformer", "node_1", [29], "node") in validation_errors
    assert InvalidIdError("three_winding_transformer", "node_2", [28], "node") in validation_errors
    assert InvalidIdError("three_winding_transformer", "node_3", [29], "node") in validation_errors
    assert NotGreaterThanError("three_winding_transformer", "u1", [1, 28], 0) in validation_errors
    assert NotGreaterThanError("three_winding_transformer", "u2", [1, 28], 0) in validation_errors
    assert NotGreaterThanError("three_winding_transformer", "u3", [1, 28], 0) in validation_errors
    assert NotGreaterThanError("three_winding_transformer", "sn_1", [1, 28], 0) in validation_errors
    assert NotGreaterThanError("three_winding_transformer", "sn_2", [1, 28], 0) in validation_errors
    assert NotGreaterThanError("three_winding_transformer", "sn_3", [1, 28], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_12", [29, 30], "pk_12/sn_1") in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_12", [1, 30], "pk_12/sn_2") in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_13", [29], "pk_13/sn_1") in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_13", [30], "pk_13/sn_3") in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_23", [1, 29], "pk_23/sn_2") in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_23", [30], "pk_23/sn_3") in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_12", [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_13", [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_23", [1, 28], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_12", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_13", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_23", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "i0", [29], "p0/sn_1") in validation_errors
    assert NotLessThanError("three_winding_transformer", "i0", [28], 1) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "p0", [1], 0) in validation_errors
    assert NotBetweenOrAtError("three_winding_transformer", "clock_12", [1, 28], (0, 12)) in validation_errors
    assert NotBetweenOrAtError("three_winding_transformer", "clock_13", [1, 28], (0, 12)) in validation_errors
    assert (
        NotBetweenOrAtError("three_winding_transformer", "tap_pos", [1, 28], ("tap_min", "tap_max"))
        in validation_errors
    )
    assert (
        NotBetweenOrAtError("three_winding_transformer", "tap_nom", [1, 28], ("tap_min", "tap_max"))
        in validation_errors
    )
    assert NotGreaterOrEqualError("three_winding_transformer", "tap_size", [1], 0) in validation_errors
    assert InvalidEnumValueError("three_winding_transformer", "winding_1", [1, 28], WindingType) in validation_errors
    assert InvalidEnumValueError("three_winding_transformer", "winding_2", [1, 28], WindingType) in validation_errors
    assert InvalidEnumValueError("three_winding_transformer", "winding_3", [1, 28], WindingType) in validation_errors
    assert InvalidEnumValueError("three_winding_transformer", "tap_side", [1, 28], Branch3Side) in validation_errors


def test_validate_three_winding_transformer_ukpkminmax(input_data):
    validation_errors = validate_input_data(input_data, symmetric=False)
    assert (
        NotGreaterOrEqualError("three_winding_transformer", "uk_12_min", [29, 30], "pk_12_min/sn_1")
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError("three_winding_transformer", "uk_12_min", [1, 30], "pk_12_min/sn_2") in validation_errors
    )
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_13_min", [29], "pk_13_min/sn_1") in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_13_min", [30], "pk_13_min/sn_3") in validation_errors
    assert (
        NotGreaterOrEqualError("three_winding_transformer", "uk_23_min", [1, 29], "pk_23_min/sn_2") in validation_errors
    )
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_23_min", [30], "pk_23_min/sn_3") in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_12_min", [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_13_min", [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_23_min", [1, 28], (0, 1)) in validation_errors
    assert (
        NotGreaterOrEqualError("three_winding_transformer", "uk_12_max", [29, 30], "pk_12_max/sn_1")
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError("three_winding_transformer", "uk_12_max", [1, 30], "pk_12_max/sn_2") in validation_errors
    )
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_13_max", [29], "pk_13_max/sn_1") in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_13_max", [30], "pk_13_max/sn_3") in validation_errors
    assert (
        NotGreaterOrEqualError("three_winding_transformer", "uk_23_max", [1, 29], "pk_23_max/sn_2") in validation_errors
    )
    assert NotGreaterOrEqualError("three_winding_transformer", "uk_23_max", [30], "pk_23_max/sn_3") in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_12_max", [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_13_max", [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError("three_winding_transformer", "uk_23_max", [1, 28], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_12_min", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_13_min", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_23_min", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_12_max", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_13_max", [1], 0) in validation_errors
    assert NotGreaterOrEqualError("three_winding_transformer", "pk_23_max", [1], 0) in validation_errors


def test_validate_input_data_transformer_tap_regulator(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.power_flow)
    assert NotBooleanError("transformer_tap_regulator", "status", [52, 1, 53]) in validation_errors
    assert (
        InvalidIdError(
            "transformer_tap_regulator", "regulated_object", [1], ["transformer", "three_winding_transformer"]
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError("transformer_tap_regulator", "control_side", [1], [BranchSide, Branch3Side])
        in validation_errors
    )
    assert (
        InvalidAssociatedEnumValueError(
            "transformer_tap_regulator", ["control_side", "regulated_object"], [52], [BranchSide]
        )
        in validation_errors
    )
    assert NotGreaterOrEqualError("transformer_tap_regulator", "u_set", [52], 0.0) in validation_errors
    assert NotGreaterThanError("transformer_tap_regulator", "u_band", [52, 1], 0.0) in validation_errors
    assert (
        NotGreaterOrEqualError("transformer_tap_regulator", "line_drop_compensation_r", [52], 0.0) in validation_errors
    )
    assert (
        NotGreaterOrEqualError("transformer_tap_regulator", "line_drop_compensation_x", [1], 0.0) in validation_errors
    )
    assert NotUniqueError("transformer_tap_regulator", "regulated_object", [51, 54]) in validation_errors


def test_fault(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.short_circuit)
    assert InvalidEnumValueError(ComponentType.fault, "fault_type", [50], FaultType) in validation_errors
    assert InvalidEnumValueError(ComponentType.fault, "fault_phase", [50], FaultPhase) in validation_errors
    assert FaultPhaseError(ComponentType.fault, ("fault_type", "fault_phase"), [1] + list(range(32, 51)))
    assert NotGreaterOrEqualError(ComponentType.fault, "r_f", [1], 0) in validation_errors
    assert (
        NotIdenticalError(
            ComponentType.fault,
            "fault_type",
            list(range(32, 51)),
            5 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [_nan_type("fault", "fault_type"), 4],
        )
        in validation_errors
    )
    assert (
        NotIdenticalError(
            ComponentType.fault,
            "fault_phase",
            list(range(32, 51)),
            list(range(2, 7)) + [0, 4, 5, 6] + 2 * list(range(4)) + [_nan_type("fault", "fault_phase"), 7],
        )
        in validation_errors
    )


def test_validate_input_data_asym_calculation(input_data):
    validation_errors = validate_input_data(input_data, symmetric=False)
    assert NotGreaterThanError("node", "u_rated", [1], 0) in validation_errors
    assert NotUniqueError("node", "id", [2, 2]) in validation_errors
    assert NotBooleanError("line", "from_status", [5]) in validation_errors
    assert NotBooleanError("line", "to_status", [4]) in validation_errors
    assert InvalidIdError("line", "from_node", [4], "node") in validation_errors
    assert InvalidIdError("line", "to_node", [5], "node") in validation_errors


def test_validate_input_data_invalid_structure():
    with pytest.raises(TypeError, match=r"should be a Numpy structured array"):
        validate_input_data({"node": np.array([[1, 10500.0], [2, 10500.0]])}, symmetric=True)


def test_generic_branch_input_data(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert NotGreaterThanError("generic_branch", "k", [6], 0) in validation_errors
    assert NotGreaterOrEqualError("generic_branch", "sn", [6], 0) in validation_errors


def test_asym_line_input_data(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert NotGreaterThanError(ComponentType.asym_line, "r_aa", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_ba", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_bb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_ca", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_cb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_cc", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_na", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_nb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_nc", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "r_nn", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_aa", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_ba", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_bb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_ca", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_cb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_cc", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_na", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_nb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_nc", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "x_nn", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c_aa", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c_ba", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c_bb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c_ca", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c_cb", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c_cc", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c0", [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, "c1", [55], 0) in validation_errors
    assert (
        MultiFieldValidationError(
            ComponentType.asym_line, ["r_na", "r_nb", "r_nc", "r_nn", "x_na", "x_nb", "x_nc", "x_nn"], [56, 57]
        )
        in validation_errors
    )
    assert (
        MultiFieldValidationError(
            ComponentType.asym_line, ["c_aa", "c_ba", "c_bb", "c_ca", "c_cb", "c_cc", "c0", "c1"], [56]
        )
        in validation_errors
    )
    assert (
        MultiFieldValidationError(ComponentType.asym_line, ["c_aa", "c_ba", "c_bb", "c_ca", "c_cb", "c_cc"], [58])
        in validation_errors
    )
