# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np
import pytest

from power_grid_model import (
    AttributeType as AT,
    ComponentType as CT,
    DatasetType as DT,
    MeasuredTerminalType,
    PowerGridModel,
    initialize_array,
)
from power_grid_model.utils import get_dataset_scenario


@pytest.fixture
def sensor_disabled_object_input():
    node = initialize_array(DT.input, CT.node, 1)
    node[AT.id] = [1]
    node[AT.u_rated] = [10e3]

    line = initialize_array(DT.input, CT.line, 1)
    line[AT.id] = [5]
    line[AT.from_node] = [1]
    line[AT.to_node] = [1]
    line[AT.from_status] = [0]
    line[AT.to_status] = [0]
    line[AT.r1] = [1e6]
    line[AT.x1] = [1e6]
    line[AT.c1] = [2e-5]
    line[AT.tan1] = [0.1]
    line[AT.r0] = [1e6]
    line[AT.x0] = [1e6]
    line[AT.c0] = [1e-6]
    line[AT.tan0] = [0.2]

    shunt = initialize_array(DT.input, CT.shunt, 1)
    shunt[AT.id] = [7]
    shunt[AT.node] = [1]
    shunt[AT.status] = [0]
    shunt[AT.g1] = [6.283185307179587e-4]
    shunt[AT.b1] = [6.283185307179587e-3]
    shunt[AT.g0] = [6.283185307179587e-5]
    shunt[AT.b0] = [3.141592653589793e-4]

    source = initialize_array(DT.input, CT.source, 1)
    source[AT.id] = [10]
    source[AT.node] = [1]
    source[AT.status] = [1]
    source[AT.u_ref] = [1.05]
    source[AT.sk] = [1e12]

    sym_voltage_sensor = initialize_array(DT.input, CT.sym_voltage_sensor, 1)
    sym_voltage_sensor[AT.id] = [13]
    sym_voltage_sensor[AT.measured_object] = [1]
    sym_voltage_sensor[AT.u_measured] = [6062.193194211956]
    sym_voltage_sensor[AT.u_sigma] = [0.1]
    sym_voltage_sensor[AT.u_angle_measured] = [-1.4832085023115637e-06]
    sym_voltage_sensor[AT.u_angle_sigma] = [0.1]

    asym_power_sensor = initialize_array(DT.input, CT.asym_power_sensor, 2)
    asym_power_sensor[AT.id] = [16, 17]
    asym_power_sensor[AT.measured_object] = [5, 7]
    asym_power_sensor[AT.measured_terminal_type] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.shunt,
    ]
    asym_power_sensor[AT.power_sigma] = [5.0, 10.0]
    asym_power_sensor[AT.p_measured] = [[1154.0, 1153.0, 1155.0], [2308.0, 2306.0, 2310.0]]
    asym_power_sensor[AT.q_measured] = [[-11540.0, -11550.0, -11550.0], [-23080.0, -23100.0, -23100.0]]

    return {
        CT.node: node,
        CT.line: line,
        CT.shunt: shunt,
        CT.source: source,
        CT.sym_voltage_sensor: sym_voltage_sensor,
        CT.asym_power_sensor: asym_power_sensor,
    }


@pytest.fixture
def sensor_disabled_object_update():
    line = initialize_array(DT.update, CT.line, (2, 1))
    line[AT.id] = [[5], [5]]
    line[AT.from_status] = [[0], [1]]
    line[AT.to_status] = [[0], [1]]

    shunt = initialize_array(DT.update, CT.shunt, (2, 1))
    shunt[AT.id] = [[7], [7]]
    shunt[AT.status] = [[1], [0]]

    asym_power_sensor = initialize_array(DT.update, CT.asym_power_sensor, (2, 2))
    asym_power_sensor[AT.id] = [[16, 17], [16, 17]]
    asym_power_sensor[AT.power_sigma] = [[np.inf, np.nan], [np.nan, np.inf]]

    return {
        CT.line: line,
        CT.shunt: shunt,
        CT.asym_power_sensor: asym_power_sensor,
    }


def test_sensor_output_on_disabled_branch(sensor_disabled_object_input, sensor_disabled_object_update):
    """
    Regression test for issue #1464:
    Power/current sensor output on a disabled shunt/load_gen incorrectly showed
    energized=1 with non-zero residuals, while sensors on disabled branches
    correctly showed energized=0. The root cause was that the sensor output
    checked only the topological status (which is cached for appliances) instead
    of the measured object's actual status.
    """
    model = PowerGridModel(sensor_disabled_object_input)
    output = model.calculate_state_estimation(
        update_data=sensor_disabled_object_update, calculation_method="newton_raphson"
    )

    # Scenario 0: line disabled (from_status=0, to_status=0), shunt active (status=1)
    s0 = get_dataset_scenario(output, 0)
    s0_sensors = s0[CT.asym_power_sensor]

    # Sensor 16 on disabled line should have energized=0
    assert s0_sensors[0][AT.id] == 16
    assert s0_sensors[0][AT.energized] == 0
    assert s0_sensors[0][AT.p_residual][0] == 0.0
    assert s0_sensors[0][AT.q_residual][0] == 0.0

    # Sensor 17 on active shunt should have energized=1
    assert s0_sensors[1][AT.id] == 17
    assert s0_sensors[1][AT.energized] == 1

    # Scenario 1: line active (from_status=1, to_status=1), shunt disabled (status=0)
    s1 = get_dataset_scenario(output, 1)
    s1_sensors = s1[CT.asym_power_sensor]

    # Sensor 16 on active line should have energized=1
    assert s1_sensors[0][AT.id] == 16
    assert s1_sensors[0][AT.energized] == 1

    # Sensor 17 on disabled shunt should have energized=0
    assert s1_sensors[1][AT.id] == 17
    assert s1_sensors[1][AT.energized] == 0
    assert s1_sensors[1][AT.p_residual][0] == 0.0
    assert s1_sensors[1][AT.q_residual][0] == 0.0
