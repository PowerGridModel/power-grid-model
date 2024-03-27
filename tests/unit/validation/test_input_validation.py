# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from typing import Dict

import numpy as np
import pytest

from power_grid_model import Branch3Side, BranchSide, LoadGenType, MeasuredTerminalType, WindingType, initialize_array
from power_grid_model.enum import CalculationType, FaultPhase, FaultType
from power_grid_model.validation import validate_input_data
from power_grid_model.validation.errors import (
    FaultPhaseError,
    InvalidEnumValueError,
    InvalidIdError,
    MultiComponentNotUniqueError,
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
from power_grid_model.validation.utils import nan_type


@pytest.fixture
def input_data() -> Dict[str, np.ndarray]:
    node = initialize_array("input", "node", 4)
    node["id"] = [0, 2, 1, 2]
    node["u_rated"] = [10.5e3, 10.5e3, 0, 10.5e3]

    line = initialize_array("input", "line", 3)
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

    link = initialize_array("input", "link", 2)
    link["id"] = [12, 13]
    link["from_node"] = [0, -1]
    link["to_node"] = [8, 1]
    link["from_status"] = [3, 1]
    link["to_status"] = [0, 4]

    transformer = initialize_array("input", "transformer", 3)
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
    transformer["tap_nom"] = [-3, nan_type("transformer", "tap_nom"), 4]
    transformer["tap_size"] = [262.5, 0.0, -10.0]
    transformer["uk_min"] = [0.0000000005, nan_type("transformer", "uk_min"), 0.9]
    transformer["uk_max"] = [0.0000000005, nan_type("transformer", "uk_max"), 0.8]
    transformer["pk_min"] = [300.0, 0.0, nan_type("transformer", "pk_min")]
    transformer["pk_max"] = [400.0, -0.1, nan_type("transformer", "pk_max")]

    three_winding_transformer = initialize_array("input", "three_winding_transformer", 4)
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
    three_winding_transformer["tap_nom"] = [-12, 41, nan_type("three_winding_transformer", "tap_nom"), 0]
    three_winding_transformer["uk_12_min"] = [
        nan_type("three_winding_transformer", "uk_12_min"),
        1.1,
        0.05,
        nan_type("three_winding_transformer", "uk_12_min"),
    ]
    three_winding_transformer["uk_13_min"] = [
        nan_type("three_winding_transformer", "uk_13_min"),
        1.2,
        0.3,
        nan_type("three_winding_transformer", "uk_13_min"),
    ]
    three_winding_transformer["uk_23_min"] = [
        nan_type("three_winding_transformer", "uk_23_min"),
        1,
        0.15,
        nan_type("three_winding_transformer", "uk_23_min"),
    ]
    three_winding_transformer["pk_12_min"] = [-450, nan_type("three_winding_transformer", "pk_12_min"), 10, 40]
    three_winding_transformer["pk_13_min"] = [-40, nan_type("three_winding_transformer", "pk_13_min"), 40, 50]
    three_winding_transformer["pk_23_min"] = [-120, nan_type("three_winding_transformer", "pk_23_min"), 40, 30]
    three_winding_transformer["uk_12_max"] = [
        nan_type("three_winding_transformer", "uk_12_max"),
        1.1,
        0.05,
        nan_type("three_winding_transformer", "uk_12_max"),
    ]
    three_winding_transformer["uk_13_max"] = [
        nan_type("three_winding_transformer", "uk_13_max"),
        1.2,
        0.3,
        nan_type("three_winding_transformer", "uk_13_max"),
    ]
    three_winding_transformer["uk_23_max"] = [
        nan_type("three_winding_transformer", "uk_23_max"),
        1,
        0.15,
        nan_type("three_winding_transformer", "uk_23_max"),
    ]
    three_winding_transformer["pk_12_max"] = [-450, nan_type("three_winding_transformer", "pk_12_max"), 10, 40]
    three_winding_transformer["pk_13_max"] = [-40, nan_type("three_winding_transformer", "pk_12_max"), 40, 50]
    three_winding_transformer["pk_23_max"] = [-120, nan_type("three_winding_transformer", "pk_12_max"), 40, 30]

    source = initialize_array("input", "source", 3)
    source["id"] = [16, 17, 1]
    source["node"] = [10, 1, 2]
    source["status"] = [0, -1, 2]
    source["u_ref"] = [-10.0, 0.0, 100.0]
    source["sk"] = [0.0, 100.0, -20.0]
    source["rx_ratio"] = [0.0, -30.0, 300.0]
    source["z01_ratio"] = [-1.0, 0.0, 200.0]

    shunt = initialize_array("input", "shunt", 3)
    shunt["id"] = [18, 19, 1]
    shunt["node"] = [10, 1, 2]
    shunt["status"] = [0, -1, 2]

    sym_load = initialize_array("input", "sym_load", 3)
    sym_load["id"] = [1, 20, 21]
    sym_load["type"] = [1, 0, 5]
    sym_load["node"] = [10, 1, 2]
    sym_load["status"] = [0, -1, 2]

    sym_gen = initialize_array("input", "sym_gen", 3)
    sym_gen["id"] = [1, 22, 23]
    sym_gen["type"] = [2, -1, 1]
    sym_gen["node"] = [10, 1, 2]
    sym_gen["status"] = [0, -1, 2]

    asym_load = initialize_array("input", "asym_load", 3)
    asym_load["id"] = [1, 24, 25]
    asym_load["type"] = [5, 0, 2]
    asym_load["node"] = [10, 1, 2]
    asym_load["status"] = [0, -1, 2]

    asym_gen = initialize_array("input", "asym_gen", 3)
    asym_gen["id"] = [1, 26, 27]
    asym_gen["type"] = [-1, 5, 2]
    asym_gen["node"] = [10, 1, 2]
    asym_gen["status"] = [0, -1, 2]

    sym_voltage_sensor = initialize_array("input", "sym_voltage_sensor", 4)
    sym_voltage_sensor["id"] = [7, 8, 9, 10]
    sym_voltage_sensor["measured_object"] = [2, 3, 1, 200]
    sym_voltage_sensor["u_measured"] = [0.0, 10.4e3, 10.6e3, -20.0]
    sym_voltage_sensor["u_sigma"] = [1.0, np.nan, 0.0, -1.0]

    asym_voltage_sensor = initialize_array("input", "asym_voltage_sensor", 4)
    asym_voltage_sensor["id"] = [7, 8, 9, 10]
    asym_voltage_sensor["measured_object"] = [2, 3, 1, 200]
    asym_voltage_sensor["u_measured"] = [
        [10.5e3, 10.4e3, 10.6e3],
        [np.nan, np.nan, np.nan],
        [0, 0, 0],
        [-1e4, 1e4, 1e4],
    ]
    asym_voltage_sensor["u_sigma"] = [1.0, np.nan, 0.0, -1.0]

    sym_power_sensor = initialize_array("input", "sym_power_sensor", 4)
    sym_power_sensor["id"] = [7, 8, 9, 10]
    sym_power_sensor["measured_object"] = [12, 3, 13, 200]
    sym_power_sensor["power_sigma"] = [1.0, np.nan, 0.0, -1.0]
    sym_power_sensor["measured_terminal_type"] = [1, 1, 10, 1]

    asym_power_sensor = initialize_array("input", "asym_power_sensor", 4)
    asym_power_sensor["id"] = [7, 8, 9, 10]
    asym_power_sensor["measured_object"] = [12, 3, 13, 200]
    asym_power_sensor["power_sigma"] = [1.0, np.nan, 0.0, -1.0]
    asym_power_sensor["measured_terminal_type"] = [1, 1, 10, 1]

    fault = initialize_array("input", "fault", 20)
    fault["id"] = [1] + list(range(32, 51))
    fault["status"] = [0, -1, 2] + 17 * [1]
    fault["fault_type"] = 6 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [nan_type("fault", "fault_type"), 4]
    fault["fault_phase"] = list(range(1, 7)) + [0, 4, 5, 6] + 2 * list(range(4)) + [nan_type("fault", "fault_phase"), 7]
    fault["fault_object"] = [200, 3] + list(range(10, 28, 2)) + 9 * [0]
    fault["r_f"] = [-1.0, 0.0, 1.0] + 17 * [nan_type("fault", "r_f")]
    fault["x_f"] = [-1.0, 0.0, 1.0] + 17 * [nan_type("fault", "x_f")]

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
    return data


def test_validate_input_data_sym_calculation(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)

    assert (
        MultiComponentNotUniqueError(
            [
                ("asym_gen", "id"),
                ("asym_load", "id"),
                ("asym_power_sensor", "id"),
                ("asym_voltage_sensor", "id"),
                ("node", "id"),
                ("shunt", "id"),
                ("source", "id"),
                ("sym_gen", "id"),
                ("sym_load", "id"),
                ("sym_power_sensor", "id"),
                ("sym_voltage_sensor", "id"),
                ("transformer", "id"),
                ("three_winding_transformer", "id"),
                ("fault", "id"),
            ],
            [
                ("asym_gen", 1),
                ("asym_load", 1),
                ("asym_power_sensor", 7),
                ("asym_power_sensor", 8),
                ("asym_power_sensor", 9),
                ("asym_power_sensor", 10),
                ("asym_voltage_sensor", 7),
                ("asym_voltage_sensor", 8),
                ("asym_voltage_sensor", 9),
                ("asym_voltage_sensor", 10),
                ("node", 1),
                ("shunt", 1),
                ("source", 1),
                ("sym_gen", 1),
                ("sym_load", 1),
                ("sym_power_sensor", 7),
                ("sym_power_sensor", 8),
                ("sym_power_sensor", 9),
                ("sym_power_sensor", 10),
                ("sym_voltage_sensor", 7),
                ("sym_voltage_sensor", 8),
                ("sym_voltage_sensor", 9),
                ("sym_voltage_sensor", 10),
                ("transformer", 1),
                ("three_winding_transformer", 1),
                ("fault", 1),
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
            ["line", "transformer"],
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
            ["line", "transformer"],
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


def test_fault(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.short_circuit)
    assert InvalidEnumValueError("fault", "fault_type", [50], FaultType) in validation_errors
    assert InvalidEnumValueError("fault", "fault_phase", [50], FaultPhase) in validation_errors
    assert FaultPhaseError("fault", ("fault_type", "fault_phase"), [1] + list(range(32, 51)))
    assert NotGreaterOrEqualError("fault", "r_f", [1], 0) in validation_errors
    assert (
        NotIdenticalError(
            "fault",
            "fault_type",
            list(range(32, 51)),
            5 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [nan_type("fault", "fault_type"), 4],
        )
        in validation_errors
    )
    assert (
        NotIdenticalError(
            "fault",
            "fault_phase",
            list(range(32, 51)),
            list(range(2, 7)) + [0, 4, 5, 6] + 2 * list(range(4)) + [nan_type("fault", "fault_phase"), 7],
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
