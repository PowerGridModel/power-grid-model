# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path

import numpy as np
import pytest

from power_grid_model import (
    AngleMeasurementType,
    AttributeType as AT,
    ComponentType as CT,
    DatasetType as DT,
    LoadGenType,
    MeasuredTerminalType,
    PowerGridModel,
    initialize_array,
)
from power_grid_model.utils import get_dataset_scenario, json_deserialize_from_file

THREE_WINDING_INPUT = (
    Path(__file__).parent.parent / "data" / "state_estimation" / "three_winding_transformer" / "input.json"
)


def _make_appliance_input():
    node = initialize_array(DT.input, CT.node, 1)
    node[AT.id] = 1
    node[AT.u_rated] = 10e3

    # Source 2 keeps the node energized when the measured source 3 is disabled.
    source = initialize_array(DT.input, CT.source, 2)
    source[AT.id] = [2, 3]
    source[AT.node] = 1
    source[AT.status] = 1
    source[AT.u_ref] = 1.0
    source[AT.sk] = 1e12

    shunt = initialize_array(DT.input, CT.shunt, 1)
    shunt[AT.id] = 4
    shunt[AT.node] = 1
    shunt[AT.status] = 1
    shunt[AT.g1] = 1e-4
    shunt[AT.b1] = 1e-4
    shunt[AT.g0] = 1e-4
    shunt[AT.b0] = 1e-4

    sym_load = initialize_array(DT.input, CT.sym_load, 1)
    sym_load[AT.id] = 5
    sym_load[AT.node] = 1
    sym_load[AT.status] = 1
    sym_load[AT.type] = LoadGenType.const_power
    sym_load[AT.p_specified] = 1e3
    sym_load[AT.q_specified] = 2e2

    sym_gen = initialize_array(DT.input, CT.sym_gen, 1)
    sym_gen[AT.id] = 6
    sym_gen[AT.node] = 1
    sym_gen[AT.status] = 1
    sym_gen[AT.type] = LoadGenType.const_power
    sym_gen[AT.p_specified] = 1e3
    sym_gen[AT.q_specified] = 2e2

    sym_voltage_sensor = initialize_array(DT.input, CT.sym_voltage_sensor, 1)
    sym_voltage_sensor[AT.id] = 10
    sym_voltage_sensor[AT.measured_object] = 1
    sym_voltage_sensor[AT.u_measured] = 10e3
    sym_voltage_sensor[AT.u_angle_measured] = 0.0
    sym_voltage_sensor[AT.u_sigma] = 1.0

    sym_power_sensor = initialize_array(DT.input, CT.sym_power_sensor, 4)
    sym_power_sensor[AT.id] = [20, 21, 22, 23]
    sym_power_sensor[AT.measured_object] = [3, 4, 5, 6]
    sym_power_sensor[AT.measured_terminal_type] = [
        MeasuredTerminalType.source,
        MeasuredTerminalType.shunt,
        MeasuredTerminalType.load,
        MeasuredTerminalType.generator,
    ]
    sym_power_sensor[AT.power_sigma] = 1.0
    sym_power_sensor[AT.p_measured] = [0.0, 1e4, 1e3, 1e3]
    sym_power_sensor[AT.q_measured] = [0.0, -1e4, 2e2, 2e2]

    return {
        CT.node: node,
        CT.source: source,
        CT.shunt: shunt,
        CT.sym_load: sym_load,
        CT.sym_gen: sym_gen,
        CT.sym_voltage_sensor: sym_voltage_sensor,
        CT.sym_power_sensor: sym_power_sensor,
    }


def _make_appliance_update():
    update = {}
    for component_type, component_id in (
        (CT.source, 3),
        (CT.shunt, 4),
        (CT.sym_load, 5),
        (CT.sym_gen, 6),
    ):
        component_update = initialize_array(DT.update, component_type, (3, 1))
        component_update[AT.id] = component_id
        component_update[AT.status] = [[1], [0], [1]]
        update[component_type] = component_update

    sensor_update = initialize_array(DT.update, CT.sym_power_sensor, (3, 4))
    sensor_update[AT.id] = [[20, 21, 22, 23], [20, 21, 22, 23], [20, 21, 22, 23]]
    sensor_update[AT.power_sigma] = [
        [1.0, 1.0, 1.0, 1.0],
        [np.inf, np.inf, np.inf, np.inf],
        [np.inf, np.inf, np.inf, np.inf],
    ]
    update[CT.sym_power_sensor] = sensor_update

    return update


def _make_branch_input():
    node = initialize_array(DT.input, CT.node, 2)
    node[AT.id] = [1, 2]
    node[AT.u_rated] = 10e3

    source = initialize_array(DT.input, CT.source, 2)
    source[AT.id] = [3, 4]
    source[AT.node] = [1, 2]
    source[AT.status] = 1
    source[AT.u_ref] = 1.0
    source[AT.sk] = 1e12

    line = initialize_array(DT.input, CT.line, 1)
    line[AT.id] = 5
    line[AT.from_node] = 1
    line[AT.to_node] = 2
    line[AT.from_status] = 1
    line[AT.to_status] = 1
    line[AT.r1] = 1.0
    line[AT.x1] = 1.0
    line[AT.c1] = 1e-5
    line[AT.tan1] = 0.1
    line[AT.r0] = 1.0
    line[AT.x0] = 1.0
    line[AT.c0] = 1e-5
    line[AT.tan0] = 0.1

    sym_voltage_sensor = initialize_array(DT.input, CT.sym_voltage_sensor, 2)
    sym_voltage_sensor[AT.id] = [10, 11]
    sym_voltage_sensor[AT.measured_object] = [1, 2]
    sym_voltage_sensor[AT.u_measured] = 10e3
    sym_voltage_sensor[AT.u_angle_measured] = 0.0
    sym_voltage_sensor[AT.u_sigma] = 1.0

    sym_power_sensor = initialize_array(DT.input, CT.sym_power_sensor, 2)
    sym_power_sensor[AT.id] = [20, 21]
    sym_power_sensor[AT.measured_object] = 5
    sym_power_sensor[AT.measured_terminal_type] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.branch_to,
    ]
    sym_power_sensor[AT.power_sigma] = 1.0
    sym_power_sensor[AT.p_measured] = 0.0
    sym_power_sensor[AT.q_measured] = 0.0

    sym_current_sensor = initialize_array(DT.input, CT.sym_current_sensor, 2)
    sym_current_sensor[AT.id] = [30, 31]
    sym_current_sensor[AT.measured_object] = 5
    sym_current_sensor[AT.measured_terminal_type] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.branch_to,
    ]
    sym_current_sensor[AT.angle_measurement_type] = AngleMeasurementType.local_angle
    sym_current_sensor[AT.i_sigma] = np.inf
    sym_current_sensor[AT.i_angle_sigma] = np.inf
    sym_current_sensor[AT.i_measured] = [1.0, 2.0]
    sym_current_sensor[AT.i_angle_measured] = 0.0

    return {
        CT.node: node,
        CT.source: source,
        CT.line: line,
        CT.sym_voltage_sensor: sym_voltage_sensor,
        CT.sym_power_sensor: sym_power_sensor,
        CT.sym_current_sensor: sym_current_sensor,
    }


def _make_branch_update():
    line_update = initialize_array(DT.update, CT.line, (4, 1))
    line_update[AT.id] = 5
    line_update[AT.from_status] = [[1], [0], [1], [0]]
    line_update[AT.to_status] = [[1], [0], [0], [1]]
    return {CT.line: line_update}


def _make_branch3_input(symmetric):
    input_data = json_deserialize_from_file(THREE_WINDING_INPUT)
    if symmetric:
        return input_data

    input_data.pop(CT.sym_voltage_sensor)
    asym_voltage_sensor = initialize_array(DT.input, CT.asym_voltage_sensor, 3)
    asym_voltage_sensor[AT.id] = [71, 72, 73]
    asym_voltage_sensor[AT.measured_object] = [1, 2, 3]
    asym_voltage_sensor[AT.u_measured] = np.repeat(
        (np.array([138e3, 69e3, 13.8e3]) / np.sqrt(3.0)).reshape(-1, 1), 3, axis=1
    )
    asym_voltage_sensor[AT.u_angle_measured] = np.repeat(
        np.array([[0.0, -2.0 * np.pi / 3.0, 2.0 * np.pi / 3.0]]), 3, axis=0
    )
    asym_voltage_sensor[AT.u_sigma] = 5e3
    input_data[CT.asym_voltage_sensor] = asym_voltage_sensor
    return input_data


def _make_branch3_update():
    transformer_update = initialize_array(DT.update, CT.three_winding_transformer, (4, 1))
    transformer_update[AT.id] = 4
    transformer_update[AT.status_1] = [[1], [0], [1], [1]]
    transformer_update[AT.status_2] = [[1], [0], [0], [1]]
    transformer_update[AT.status_3] = [[1], [0], [1], [0]]
    return {CT.three_winding_transformer: transformer_update}


def _assert_null_power_sensor_output(output):
    assert np.all(output[AT.energized] == 0)
    assert np.all(output[AT.p_residual] == 0.0)
    assert np.all(output[AT.q_residual] == 0.0)


@pytest.mark.parametrize("symmetric", [True, False])
def test_power_sensor_output_follows_appliance_status(symmetric):
    output = PowerGridModel(_make_appliance_input()).calculate_state_estimation(
        update_data=_make_appliance_update(),
        symmetric=symmetric,
        calculation_method="newton_raphson",
    )

    enabled_output = get_dataset_scenario(output, 0)[CT.sym_power_sensor]
    np.testing.assert_array_equal(enabled_output[AT.id], [20, 21, 22, 23])
    np.testing.assert_array_equal(enabled_output[AT.energized], [1, 1, 1, 1])

    disabled_output = get_dataset_scenario(output, 1)[CT.sym_power_sensor]
    np.testing.assert_array_equal(disabled_output[AT.id], [20, 21, 22, 23])
    _assert_null_power_sensor_output(disabled_output)

    excluded_measurements_output = get_dataset_scenario(output, 2)[CT.sym_power_sensor]
    np.testing.assert_array_equal(excluded_measurements_output[AT.id], [20, 21, 22, 23])
    np.testing.assert_array_equal(excluded_measurements_output[AT.energized], [1, 1, 1, 1])


@pytest.mark.parametrize("symmetric", [True, False])
def test_sensor_output_uses_branch_topology_group(symmetric):
    output = PowerGridModel(_make_branch_input()).calculate_state_estimation(
        update_data=_make_branch_update(),
        symmetric=symmetric,
        calculation_method="newton_raphson",
    )
    expected_energized = ([1, 1], [0, 0], [1, 1], [1, 1])

    for scenario, expected in enumerate(expected_energized):
        scenario_output = get_dataset_scenario(output, scenario)
        power_sensor_output = scenario_output[CT.sym_power_sensor]
        current_sensor_output = scenario_output[CT.sym_current_sensor]

        np.testing.assert_array_equal(power_sensor_output[AT.id], [20, 21])
        np.testing.assert_array_equal(current_sensor_output[AT.id], [30, 31])
        np.testing.assert_array_equal(power_sensor_output[AT.energized], expected)
        np.testing.assert_array_equal(current_sensor_output[AT.energized], expected)

    _assert_null_power_sensor_output(get_dataset_scenario(output, 1)[CT.sym_power_sensor])
    disconnected_current_sensor = get_dataset_scenario(output, 1)[CT.sym_current_sensor]
    assert np.all(disconnected_current_sensor[AT.i_residual] == 0.0)
    assert np.all(disconnected_current_sensor[AT.i_angle_residual] == 0.0)


@pytest.mark.parametrize("symmetric", [True, False])
def test_power_sensor_output_uses_branch3_topology_group(symmetric):
    output = PowerGridModel(_make_branch3_input(symmetric)).calculate_state_estimation(
        update_data=_make_branch3_update(),
        symmetric=symmetric,
        calculation_method="iterative_linear",
        max_iterations=100,
    )
    expected_energized = (
        [1, 1, 1],  # All three terminals connected.
        [0, 0, 0],  # The complete branch3 is disconnected.
        [1, 1, 1],  # Terminal 2 is open, but terminals 1 and 3 still contribute.
        [1, 1, 1],  # Terminal 3 is open, but terminals 1 and 2 still contribute.
    )

    for scenario, expected in enumerate(expected_energized):
        sensor_output = get_dataset_scenario(output, scenario)[CT.sym_power_sensor]
        np.testing.assert_array_equal(sensor_output[AT.id], [61, 62, 63])
        np.testing.assert_array_equal(sensor_output[AT.energized], expected)

    _assert_null_power_sensor_output(get_dataset_scenario(output, 1)[CT.sym_power_sensor])
