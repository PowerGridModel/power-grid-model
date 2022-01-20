# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from typing import Dict

import numpy as np
import pytest
from power_grid_model import initialize_array, MeasuredTerminalType, WindingType, LoadGenType
from power_grid_model.validation import validate_input_data
from power_grid_model.validation.errors import (
    NotGreaterThanError,
    NotUniqueError,
    NotBooleanError,
    InvalidIdError,
    TwoValuesZeroError,
    NotGreaterOrEqualError,
    NotLessThanError,
    NotBetweenError,
    NotBetweenOrAtError,
    InvalidEnumValueError,
    MultiComponentNotUniqueError,
)


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
    transformer["winding_from"] = [4, 0, 2]
    transformer["winding_to"] = [5, 1, 2]
    transformer["clock"] = [13, -1, 7]
    transformer["tap_side"] = [-1, 0, 1]
    transformer["tap_pos"] = [-1, 6, -4]
    transformer["tap_min"] = [-2, 4, 3]
    transformer["tap_max"] = [2, -4, -3]
    transformer["tap_nom"] = [-3, 3, 4]
    transformer["tap_size"] = [262.5, 0.0, -10.0]
    transformer["uk_min"] = [0.0000000005, 0.0, 0.9]
    transformer["uk_max"] = [0.0000000005, 0.0, 0.9]
    transformer["pk_min"] = [300.0, 0.0, -10.0]
    transformer["pk_max"] = [400.0, -0.1, -10.0]

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
    sym_power_sensor["measured_object"] = [2, 3, 0, 200]
    sym_power_sensor["power_sigma"] = [1.0, np.nan, 0.0, -1.0]
    sym_power_sensor["measured_terminal_type"] = [1, 1, 10, 1]

    asym_power_sensor = initialize_array("input", "asym_power_sensor", 4)
    asym_power_sensor["id"] = [7, 8, 9, 10]
    asym_power_sensor["measured_object"] = [2, 3, 0, 200]
    asym_power_sensor["power_sigma"] = [1.0, np.nan, 0.0, -1.0]
    asym_power_sensor["measured_terminal_type"] = [1, 1, 10, 1]

    data = {
        "node": node,
        "line": line,
        "link": link,
        "transformer": transformer,
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
    assert NotGreaterThanError("transformer", "tap_size", [14, 15], 0) in validation_errors
    assert NotGreaterOrEqualError("transformer", "uk_min", [1], "pk_min/sn") in validation_errors
    assert NotBetweenError("transformer", "uk_min", [14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError("transformer", "uk_max", [1], "pk_max/sn") in validation_errors
    assert NotBetweenError("transformer", "uk_max", [14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError("transformer", "pk_min", [15], 0) in validation_errors
    assert NotGreaterOrEqualError("transformer", "pk_max", [14, 15], 0) in validation_errors
    assert InvalidEnumValueError("transformer", "winding_from", [1], WindingType)
    assert InvalidEnumValueError("transformer", "winding_to", [1], WindingType)
    assert InvalidEnumValueError("transformer", "tap_side", [1], WindingType)

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
    assert InvalidEnumValueError("sym_load", "type", [20], LoadGenType)

    assert InvalidIdError("sym_gen", "node", [1], "node") in validation_errors
    assert NotBooleanError("sym_gen", "status", [22, 23]) in validation_errors
    assert InvalidEnumValueError("sym_gen", "type", [22], LoadGenType)

    assert InvalidIdError("asym_load", "node", [1], "node") in validation_errors
    assert NotBooleanError("asym_load", "status", [24, 25]) in validation_errors
    assert InvalidEnumValueError("asym_load", "type", [24], LoadGenType)

    assert InvalidIdError("asym_gen", "node", [1], "node") in validation_errors
    assert NotBooleanError("asym_gen", "status", [26, 27]) in validation_errors
    assert InvalidEnumValueError("sym_load", "type", [1, 26], LoadGenType)

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
            ["line", "transformer", "source", "shunt", "sym_load", "asym_load", "sym_gen", "asym_gen"],
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
    assert InvalidEnumValueError("sym_power_sensor", "measured_terminal_type", [10], MeasuredTerminalType)

    assert (
        InvalidIdError(
            "asym_power_sensor",
            "measured_object",
            [7, 9, 10],
            ["line", "transformer", "source", "shunt", "sym_load", "asym_load", "sym_gen", "asym_gen"],
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
    assert InvalidEnumValueError("asym_power_sensor", "measured_terminal_type", [10], MeasuredTerminalType)


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
