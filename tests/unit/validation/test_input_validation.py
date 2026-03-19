# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import itertools

import numpy as np
import pytest

from power_grid_model import (
    AttributeType,
    Branch3Side,
    BranchSide,
    ComponentType,
    DatasetType,
    LoadGenType,
    MeasuredTerminalType,
    WindingType,
    initialize_array,
)
from power_grid_model._core.enum import AngleMeasurementType
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset
from power_grid_model.enum import CalculationType, ComponentAttributeFilterOptions, FaultPhase, FaultType
from power_grid_model.validation import validate_input_data
from power_grid_model.validation.assertions import assert_valid_input_data
from power_grid_model.validation.errors import (
    FaultPhaseError,
    InfinityError,
    InvalidAssociatedEnumValueError,
    InvalidEnumValueError,
    InvalidIdError,
    InvalidVoltageRegulationError,
    MixedPowerCurrentSensorError,
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
    UnsupportedMeasuredTerminalType,
)
from power_grid_model.validation.utils import _nan_type


@pytest.fixture
def original_data() -> dict[ComponentType, np.ndarray]:
    node = initialize_array(DatasetType.input, ComponentType.node, 4)
    node[AttributeType.id] = [0, 2, 1, 2]
    node[AttributeType.u_rated] = [10.5e3, 10.5e3, 0, 10.5e3]

    line = initialize_array(DatasetType.input, ComponentType.line, 3)
    line[AttributeType.id] = [3, 4, 5]
    line[AttributeType.from_node] = [0, -1, 2]
    line[AttributeType.to_node] = [2, 1, 8]
    line[AttributeType.from_status] = [0, 1, -2]
    line[AttributeType.to_status] = [0, 4, 1]
    line[AttributeType.r1] = [0, 100, 1]
    line[AttributeType.x1] = [0, 5, 0]
    line[AttributeType.r0] = [10, 0, 0]
    line[AttributeType.x0] = [0, 0, 50]
    line[AttributeType.i_n] = [-3, 0, 50]

    asym_line = initialize_array(DatasetType.input, ComponentType.asym_line, 4)
    asym_line[AttributeType.id] = [55, 56, 57, 58]
    asym_line[AttributeType.from_node] = [0, 1, 0, 0]
    asym_line[AttributeType.to_node] = [1, 2, 2, 1]
    asym_line[AttributeType.from_status] = [1, 1, 1, 1]
    asym_line[AttributeType.to_status] = [1, 1, 1, 1]
    asym_line[AttributeType.r_aa] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_ba] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_bb] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_ca] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_cb] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_cc] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_na] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_nb] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_nc] = [-1, 2, 2, 2]
    asym_line[AttributeType.r_nn] = [-1, np.nan, 2, 2]
    asym_line[AttributeType.x_aa] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_ba] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_bb] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_ca] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_cb] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_cc] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_na] = [-1, 2, np.nan, 2]
    asym_line[AttributeType.x_nb] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_nc] = [-1, 2, 2, 2]
    asym_line[AttributeType.x_nn] = [-1, 2, 2, 2]
    asym_line[AttributeType.c_aa] = [-1, np.nan, 2, np.nan]
    asym_line[AttributeType.c_ba] = [-1, np.nan, 2, np.nan]
    asym_line[AttributeType.c_bb] = [-1, np.nan, 2, np.nan]
    asym_line[AttributeType.c_ca] = [-1, np.nan, 2, np.nan]
    asym_line[AttributeType.c_cb] = [-1, np.nan, 2, np.nan]
    asym_line[AttributeType.c_cc] = [-1, np.nan, 2, 2]
    asym_line[AttributeType.c0] = [-1, np.nan, np.nan, np.nan]
    asym_line[AttributeType.c1] = [-1, np.nan, np.nan, np.nan]
    asym_line[AttributeType.i_n] = [50, 50, 50, 50]

    generic_branch = initialize_array(DatasetType.input, ComponentType.generic_branch, 1)
    generic_branch[AttributeType.id] = [6]
    generic_branch[AttributeType.from_node] = [1]
    generic_branch[AttributeType.to_node] = [2]
    generic_branch[AttributeType.from_status] = [1]
    generic_branch[AttributeType.to_status] = [1]
    generic_branch[AttributeType.r1] = [0.129059]
    generic_branch[AttributeType.x1] = [16.385859]
    generic_branch[AttributeType.g1] = [8.692e-7]
    generic_branch[AttributeType.b1] = [-2.336e-7]
    generic_branch[AttributeType.k] = [0.0]
    generic_branch[AttributeType.theta] = [0.0]
    generic_branch[AttributeType.sn] = [-10.0]

    link = initialize_array(DatasetType.input, ComponentType.link, 2)
    link[AttributeType.id] = [12, 13]
    link[AttributeType.from_node] = [0, -1]
    link[AttributeType.to_node] = [8, 1]
    link[AttributeType.from_status] = [3, 1]
    link[AttributeType.to_status] = [0, 4]

    transformer = initialize_array(DatasetType.input, ComponentType.transformer, 3)
    transformer[AttributeType.id] = [1, 14, 15]
    transformer[AttributeType.from_node] = [1, 7, 2]  # TODO check from node 1 to node 1
    transformer[AttributeType.to_node] = [1, 8, 1]
    transformer[AttributeType.from_status] = [1, 1, -1]
    transformer[AttributeType.to_status] = [2, 0, 1]
    transformer[AttributeType.u1] = [10500.0, 0.0, -1.0]
    transformer[AttributeType.u2] = [400.0, -3.0, 0.0]
    transformer[AttributeType.sn] = [630000.0, 0.0, -20.0]
    transformer[AttributeType.uk] = [-1.0, 1.1, 0.9]
    transformer[AttributeType.pk] = [63000.0, 0.0, -10.0]
    transformer[AttributeType.i0] = [0.0368, 10.0, 0.9]
    transformer[AttributeType.p0] = [63000.0, 0.0, -10.0]
    transformer[AttributeType.winding_from] = [8, 0, 2]
    transformer[AttributeType.winding_to] = [5, 1, 2]
    transformer[AttributeType.clock] = [13, -13, 7]
    transformer[AttributeType.tap_side] = [-1, 0, 1]
    transformer[AttributeType.tap_pos] = [-1, 6, -4]
    transformer[AttributeType.tap_min] = [-2, 4, 3]
    transformer[AttributeType.tap_max] = [2, -4, -3]
    transformer[AttributeType.tap_nom] = [-3, _nan_type(ComponentType.transformer, AttributeType.tap_nom), 4]
    transformer[AttributeType.tap_size] = [262.5, 0.0, -10.0]
    transformer[AttributeType.uk_min] = [0.0000000005, _nan_type(ComponentType.transformer, AttributeType.uk_min), 0.9]
    transformer[AttributeType.uk_max] = [0.0000000005, _nan_type(ComponentType.transformer, AttributeType.uk_max), 0.8]
    transformer[AttributeType.pk_min] = [300.0, 0.0, _nan_type(ComponentType.transformer, AttributeType.pk_min)]
    transformer[AttributeType.pk_max] = [400.0, -0.1, _nan_type(ComponentType.transformer, AttributeType.pk_max)]

    three_winding_transformer = initialize_array(DatasetType.input, ComponentType.three_winding_transformer, 4)
    three_winding_transformer[AttributeType.id] = [1, 28, 29, 30]
    three_winding_transformer[AttributeType.node_1] = [0, 1, 9, 2]
    three_winding_transformer[AttributeType.node_2] = [1, 15, 1, 0]
    three_winding_transformer[AttributeType.node_3] = [1, 2, 12, 1]
    three_winding_transformer[AttributeType.status_1] = [1, 5, 1, 1]
    three_winding_transformer[AttributeType.status_2] = [2, 1, 1, 1]
    three_winding_transformer[AttributeType.status_3] = [1, 0, -1, 0]
    three_winding_transformer[AttributeType.u1] = [-100, 0, 200, 100]
    three_winding_transformer[AttributeType.u2] = [0, -200, 100, 200]
    three_winding_transformer[AttributeType.u3] = [-100, 0, 400, 300]
    three_winding_transformer[AttributeType.sn_1] = [0, -1200, 100, 300]
    three_winding_transformer[AttributeType.sn_2] = [-1000, 0, 200, 200]
    three_winding_transformer[AttributeType.sn_3] = [0, -2300, 300, 100]
    three_winding_transformer[AttributeType.uk_12] = [-1, 1.1, 0.05, 0.1]
    three_winding_transformer[AttributeType.uk_13] = [-2, 1.2, 0.3, 0.2]
    three_winding_transformer[AttributeType.uk_23] = [-1.5, 1, 0.15, 0.2]
    three_winding_transformer[AttributeType.pk_12] = [-450, 100, 10, 40]
    three_winding_transformer[AttributeType.pk_13] = [-40, 50, 40, 50]
    three_winding_transformer[AttributeType.pk_23] = [-120, 1, 40, 30]
    three_winding_transformer[AttributeType.i0] = [-0.5, 1.8, 0.3, 0.6]
    three_winding_transformer[AttributeType.p0] = [-100, 410, 60, 40]
    three_winding_transformer[AttributeType.winding_1] = [15, -1, 0, 2]
    three_winding_transformer[AttributeType.winding_2] = [19, -2, 1, 3]
    three_winding_transformer[AttributeType.winding_3] = [-2, 13, 2, 2]
    three_winding_transformer[AttributeType.clock_12] = [-12, 24, 4, 3]
    three_winding_transformer[AttributeType.clock_13] = [-30, 40, 3, 4]
    three_winding_transformer[AttributeType.tap_side] = [-1, 9, 1, 0]
    three_winding_transformer[AttributeType.tap_pos] = [50, -24, 5, 3]
    three_winding_transformer[AttributeType.tap_min] = [-10, -10, -10, -10]
    three_winding_transformer[AttributeType.tap_max] = [10, 10, 10, 10]
    three_winding_transformer[AttributeType.tap_size] = [-12, 0, 3, 130]
    three_winding_transformer[AttributeType.tap_nom] = [
        -12,
        41,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.tap_nom),
        0,
    ]
    three_winding_transformer[AttributeType.uk_12_min] = [
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_12_min),
        1.1,
        0.05,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_12_min),
    ]
    three_winding_transformer[AttributeType.uk_13_min] = [
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_13_min),
        1.2,
        0.3,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_13_min),
    ]
    three_winding_transformer[AttributeType.uk_23_min] = [
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_23_min),
        1,
        0.15,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_23_min),
    ]
    three_winding_transformer[AttributeType.pk_12_min] = [
        -450,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.pk_12_min),
        10,
        40,
    ]
    three_winding_transformer[AttributeType.pk_13_min] = [
        -40,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.pk_13_min),
        40,
        50,
    ]
    three_winding_transformer[AttributeType.pk_23_min] = [
        -120,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.pk_23_min),
        40,
        30,
    ]
    three_winding_transformer[AttributeType.uk_12_max] = [
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_12_max),
        1.1,
        0.05,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_12_max),
    ]
    three_winding_transformer[AttributeType.uk_13_max] = [
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_13_max),
        1.2,
        0.3,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_13_max),
    ]
    three_winding_transformer[AttributeType.uk_23_max] = [
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_23_max),
        1,
        0.15,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.uk_23_max),
    ]
    three_winding_transformer[AttributeType.pk_12_max] = [
        -450,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.pk_12_max),
        10,
        40,
    ]
    three_winding_transformer[AttributeType.pk_13_max] = [
        -40,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.pk_12_max),
        40,
        50,
    ]
    three_winding_transformer[AttributeType.pk_23_max] = [
        -120,
        _nan_type(ComponentType.three_winding_transformer, AttributeType.pk_12_max),
        40,
        30,
    ]

    transformer_tap_regulator = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 5)
    transformer_tap_regulator[AttributeType.id] = [51, 52, 53, 54, 1]
    transformer_tap_regulator[AttributeType.status] = [0, -1, 2, 1, 5]
    transformer_tap_regulator[AttributeType.regulated_object] = [14, 15, 28, 14, 2]
    transformer_tap_regulator[AttributeType.control_side] = [1, 2, 0, 0, 60]
    transformer_tap_regulator[AttributeType.u_set] = [100, -100, 100, 100, 100]
    transformer_tap_regulator[AttributeType.u_band] = [100, -4, 100, 100, 0]
    transformer_tap_regulator[AttributeType.line_drop_compensation_r] = [0.0, -1.0, 1.0, 0.0, 2.0]
    transformer_tap_regulator[AttributeType.line_drop_compensation_x] = [0.0, 4.0, 2.0, 0.0, -4.0]

    source = initialize_array(DatasetType.input, ComponentType.source, 3)
    source[AttributeType.id] = [16, 17, 1]
    source[AttributeType.node] = [10, 1, 2]
    source[AttributeType.status] = [0, -1, 2]
    source[AttributeType.u_ref] = [-10.0, 0.0, 100.0]
    source[AttributeType.sk] = [0.0, 100.0, -20.0]
    source[AttributeType.rx_ratio] = [0.0, -30.0, 300.0]
    source[AttributeType.z01_ratio] = [-1.0, 0.0, 200.0]

    shunt = initialize_array(DatasetType.input, ComponentType.shunt, 3)
    shunt[AttributeType.id] = [18, 19, 1]
    shunt[AttributeType.node] = [10, 1, 2]
    shunt[AttributeType.status] = [0, -1, 2]

    sym_load = initialize_array(DatasetType.input, ComponentType.sym_load, 3)
    sym_load[AttributeType.id] = [1, 20, 21]
    sym_load[AttributeType.type] = [1, 0, 5]
    sym_load[AttributeType.node] = [10, 1, 2]
    sym_load[AttributeType.status] = [0, -1, 2]

    sym_gen = initialize_array(DatasetType.input, ComponentType.sym_gen, 3)
    sym_gen[AttributeType.id] = [1, 22, 23]
    sym_gen[AttributeType.type] = [2, -1, 1]
    sym_gen[AttributeType.node] = [10, 1, 2]
    sym_gen[AttributeType.status] = [0, -1, 2]

    asym_load = initialize_array(DatasetType.input, ComponentType.asym_load, 3)
    asym_load[AttributeType.id] = [1, 24, 25]
    asym_load[AttributeType.type] = [5, 0, 2]
    asym_load[AttributeType.node] = [10, 1, 2]
    asym_load[AttributeType.status] = [0, -1, 2]

    asym_gen = initialize_array(DatasetType.input, ComponentType.asym_gen, 3)
    asym_gen[AttributeType.id] = [1, 26, 27]
    asym_gen[AttributeType.type] = [-1, 5, 2]
    asym_gen[AttributeType.node] = [10, 1, 2]
    asym_gen[AttributeType.status] = [0, -1, 2]

    sym_voltage_sensor = initialize_array(DatasetType.input, ComponentType.sym_voltage_sensor, 5)
    sym_voltage_sensor[AttributeType.id] = range(107, 107 + 5)
    sym_voltage_sensor[AttributeType.measured_object] = [2, 3, 1, 200, 2]
    sym_voltage_sensor[AttributeType.u_measured] = [0.0, 10.4e3, 10.6e3, -20.0, 1e4]
    sym_voltage_sensor[AttributeType.u_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]

    asym_voltage_sensor = initialize_array(DatasetType.input, ComponentType.asym_voltage_sensor, 5)
    asym_voltage_sensor[AttributeType.id] = range(107, 107 + 5)
    asym_voltage_sensor[AttributeType.measured_object] = [2, 3, 1, 200, 2]
    asym_voltage_sensor[AttributeType.u_measured] = np.array(
        [
            [10.5e3, 10.4e3, 10.6e3],
            [np.nan, np.nan, np.nan],
            [0, 0, 0],
            [-1e4, 1e4, 1e4],
            [1.0e4, 1.0e4, 1.0e4],
        ]
    )
    asym_voltage_sensor[AttributeType.u_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]

    sym_power_sensor = initialize_array(DatasetType.input, ComponentType.sym_power_sensor, 15)
    sym_power_sensor[AttributeType.id] = range(107, 107 + 15)
    sym_power_sensor[AttributeType.measured_object] = [12, 3, 13, 200, 3] + [3] * 10
    sym_power_sensor[AttributeType.power_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf] * 2 + [np.nan] * 5
    sym_power_sensor[AttributeType.measured_terminal_type] = [1, 1, 10, 1, 1] + [1] * 10
    sym_power_sensor[AttributeType.p_sigma] = [np.nan] * 5 + [1.0, 1.0, 0.0, -1.0, np.inf] * 2
    sym_power_sensor[AttributeType.q_sigma] = [np.nan] * 5 + [1.0, np.inf, 0.0, -1.0, np.inf] * 2

    asym_power_sensor = initialize_array(DatasetType.input, ComponentType.asym_power_sensor, 15)
    asym_power_sensor[AttributeType.id] = range(107, 107 + 15)
    asym_power_sensor[AttributeType.measured_object] = [12, 3, 13, 200, 3] + [3] * 10
    asym_power_sensor[AttributeType.power_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf] * 2 + [np.nan] * 5
    asym_power_sensor[AttributeType.measured_terminal_type] = [1, 1, 10, 1, 1] + [1] * 10
    asym_power_sensor[AttributeType.p_sigma] = [[np.nan] * 3] * 5 + [
        [1.0, 1.0, 1.0],
        [np.inf, np.inf, np.inf],
        [1.0, np.nan, 1.0],
        [1.0, 1.0, -1.0],
        [np.inf, 1.0, 1.0],
    ] * 2
    asym_power_sensor[AttributeType.q_sigma] = [[np.nan] * 3] * 5 + [
        [1.0, 1.0, 1.0],
        [np.inf, np.inf, np.inf],
        [np.inf, np.nan, 1.0],
        [1.0, 1.0, -1.0],
        [np.inf, 1.0, 1.0],
    ] * 2

    sym_current_sensor = initialize_array(DatasetType.input, ComponentType.sym_current_sensor, 5)
    sym_current_sensor[AttributeType.id] = range(107, 107 + 5)
    sym_current_sensor[AttributeType.measured_object] = [12, 3, 13, 200, 3]
    sym_current_sensor[AttributeType.i_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    sym_current_sensor[AttributeType.i_angle_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    sym_current_sensor[AttributeType.measured_terminal_type] = [1, 1, 10, 2, 1]
    sym_current_sensor[AttributeType.angle_measurement_type] = [0, 1, 10, 2, 0]

    asym_current_sensor = initialize_array(DatasetType.input, ComponentType.asym_current_sensor, 5)
    asym_current_sensor[AttributeType.id] = range(107, 107 + 5)
    asym_current_sensor[AttributeType.measured_object] = [12, 3, 13, 200, 3]
    asym_current_sensor[AttributeType.i_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    asym_current_sensor[AttributeType.i_angle_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    asym_current_sensor[AttributeType.measured_terminal_type] = [1, 1, 10, 2, 1]
    asym_current_sensor[AttributeType.angle_measurement_type] = [0, 1, 10, 2, 0]

    fault = initialize_array(DatasetType.input, ComponentType.fault, 20)
    fault[AttributeType.id] = [1, *list(range(32, 51))]
    fault[AttributeType.status] = [0, -1, 2] + 17 * [1]
    fault[AttributeType.fault_type] = (
        6 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [_nan_type(ComponentType.fault, AttributeType.fault_type), 4]
    )
    fault[AttributeType.fault_phase] = (
        list(range(1, 7))
        + [0, 4, 5, 6]
        + 2 * list(range(4))
        + [_nan_type(ComponentType.fault, AttributeType.fault_phase), 7]
    )
    fault[AttributeType.fault_object] = [200, 3] + list(range(10, 28, 2)) + 9 * [0]
    fault[AttributeType.r_f] = [-1.0, 0.0, 1.0] + 17 * [_nan_type(ComponentType.fault, AttributeType.r_f)]
    fault[AttributeType.x_f] = [-1.0, 0.0, 1.0] + 17 * [_nan_type(ComponentType.fault, AttributeType.x_f)]

    voltage_regulator = initialize_array(DatasetType.input, ComponentType.voltage_regulator, 8)
    voltage_regulator[AttributeType.id] = [60, 61, 62, 63, 64, 65, 66, 67]
    # 20-27 are gen/load IDs, 200 is invalid, 16,18 are shunts/sources, 20 is duplicate with different u_ref
    voltage_regulator[AttributeType.regulated_object] = [20, 23, 25, 27, 200, 16, 18, 20]
    voltage_regulator[AttributeType.status] = [1, 0, -1, 1, 1, 5, 0, 1]  # -1 and 5 are invalid boolean values
    voltage_regulator[AttributeType.u_ref] = [1.02, 100, 100.0, 100.0, 1.0, np.inf, 0.0, 1.03]

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
        ComponentType.sym_current_sensor: sym_current_sensor,
        ComponentType.asym_current_sensor: asym_current_sensor,
        ComponentType.fault: fault,
        ComponentType.voltage_regulator: voltage_regulator,
    }
    return data  # noqa: RET504


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

    assert validation_errors is not None
    assert (
        MultiComponentNotUniqueError(
            [
                (ComponentType.asym_gen, AttributeType.id),
                (ComponentType.asym_load, AttributeType.id),
                (ComponentType.asym_current_sensor, AttributeType.id),
                (ComponentType.asym_power_sensor, AttributeType.id),
                (ComponentType.asym_voltage_sensor, AttributeType.id),
                (ComponentType.node, AttributeType.id),
                (ComponentType.shunt, AttributeType.id),
                (ComponentType.source, AttributeType.id),
                (ComponentType.sym_gen, AttributeType.id),
                (ComponentType.sym_load, AttributeType.id),
                (ComponentType.sym_current_sensor, AttributeType.id),
                (ComponentType.sym_power_sensor, AttributeType.id),
                (ComponentType.sym_voltage_sensor, AttributeType.id),
                (ComponentType.transformer, AttributeType.id),
                (ComponentType.three_winding_transformer, AttributeType.id),
                (ComponentType.fault, AttributeType.id),
                (ComponentType.transformer_tap_regulator, AttributeType.id),
            ],
            [
                (ComponentType.asym_gen, 1),
                (ComponentType.asym_load, 1),
                *[(ComponentType.asym_current_sensor, i) for i in range(107, 107 + 5)],
                *[(ComponentType.asym_power_sensor, i) for i in range(107, 107 + 15)],
                *[(ComponentType.asym_voltage_sensor, i) for i in range(107, 107 + 5)],
                (ComponentType.node, 1),
                (ComponentType.shunt, 1),
                (ComponentType.source, 1),
                (ComponentType.sym_gen, 1),
                (ComponentType.sym_load, 1),
                *[(ComponentType.sym_current_sensor, i) for i in range(107, 107 + 5)],
                *[(ComponentType.sym_power_sensor, i) for i in range(107, 107 + 15)],
                *[(ComponentType.sym_voltage_sensor, i) for i in range(107, 107 + 5)],
                (ComponentType.transformer, 1),
                (ComponentType.three_winding_transformer, 1),
                (ComponentType.fault, 1),
                (ComponentType.transformer_tap_regulator, 1),
            ],
        )
        in validation_errors
    )

    assert NotGreaterThanError(ComponentType.node, AttributeType.u_rated, [1], 0) in validation_errors
    assert NotUniqueError(ComponentType.node, AttributeType.id, [2, 2]) in validation_errors

    assert NotBooleanError(ComponentType.line, AttributeType.from_status, [5]) in validation_errors
    assert NotBooleanError(ComponentType.line, AttributeType.to_status, [4]) in validation_errors
    assert InvalidIdError(ComponentType.line, AttributeType.from_node, [4], ComponentType.node) in validation_errors
    assert InvalidIdError(ComponentType.line, AttributeType.to_node, [5], ComponentType.node) in validation_errors
    assert TwoValuesZeroError(ComponentType.line, [AttributeType.r1, AttributeType.x1], [3]) in validation_errors
    assert TwoValuesZeroError(ComponentType.line, [AttributeType.r0, AttributeType.x0], [4]) in validation_errors
    assert NotGreaterThanError(ComponentType.line, AttributeType.i_n, [3, 4], 0) in validation_errors

    assert NotBooleanError(ComponentType.link, AttributeType.from_status, [12]) in validation_errors
    assert NotBooleanError(ComponentType.link, AttributeType.to_status, [13]) in validation_errors
    assert InvalidIdError(ComponentType.link, AttributeType.from_node, [13], ComponentType.node) in validation_errors
    assert InvalidIdError(ComponentType.link, AttributeType.to_node, [12], ComponentType.node) in validation_errors

    assert NotBooleanError(ComponentType.transformer, AttributeType.from_status, [15]) in validation_errors
    assert NotBooleanError(ComponentType.transformer, AttributeType.to_status, [1]) in validation_errors
    assert (
        InvalidIdError(ComponentType.transformer, AttributeType.from_node, [14], ComponentType.node)
        in validation_errors
    )
    assert (
        InvalidIdError(ComponentType.transformer, AttributeType.to_node, [14], ComponentType.node) in validation_errors
    )
    assert NotGreaterThanError(ComponentType.transformer, AttributeType.u1, [14, 15], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.transformer, AttributeType.u2, [14, 15], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.transformer, AttributeType.sn, [14, 15], 0) in validation_errors
    assert (
        NotGreaterOrEqualError(
            ComponentType.transformer, AttributeType.uk, [1], f"{AttributeType.pk}/{AttributeType.sn}"
        )
        in validation_errors
    )
    assert NotBetweenError(ComponentType.transformer, AttributeType.uk, [1, 14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError(ComponentType.transformer, AttributeType.pk, [15], 0) in validation_errors
    assert (
        NotGreaterOrEqualError(
            ComponentType.transformer, AttributeType.i0, [1], f"{AttributeType.p0}/{AttributeType.sn}"
        )
        in validation_errors
    )
    assert NotLessThanError(ComponentType.transformer, AttributeType.i0, [14], 1) in validation_errors
    assert NotGreaterOrEqualError(ComponentType.transformer, AttributeType.p0, [15], 0) in validation_errors
    assert NotBetweenOrAtError(ComponentType.transformer, AttributeType.clock, [1, 14], (-12, 12)) in validation_errors
    assert (
        NotBetweenOrAtError(
            ComponentType.transformer, AttributeType.tap_pos, [14, 15], (AttributeType.tap_min, AttributeType.tap_max)
        )
        in validation_errors
    )
    assert (
        NotBetweenOrAtError(
            ComponentType.transformer, AttributeType.tap_nom, [1, 15], (AttributeType.tap_min, AttributeType.tap_max)
        )
        in validation_errors
    )
    assert NotGreaterOrEqualError(ComponentType.transformer, AttributeType.tap_size, [15], 0) in validation_errors
    assert (
        NotGreaterOrEqualError(
            ComponentType.transformer, AttributeType.uk_min, [1], f"{AttributeType.pk_min}/{AttributeType.sn}"
        )
        in validation_errors
    )
    assert NotBetweenError(ComponentType.transformer, AttributeType.uk_min, [14], (0, 1)) in validation_errors
    assert (
        NotGreaterOrEqualError(
            ComponentType.transformer, AttributeType.uk_max, [1], f"{AttributeType.pk_max}/{AttributeType.sn}"
        )
        in validation_errors
    )
    assert NotBetweenError(ComponentType.transformer, AttributeType.uk_max, [14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError(ComponentType.transformer, AttributeType.pk_min, [15], 0) in validation_errors
    assert NotGreaterOrEqualError(ComponentType.transformer, AttributeType.pk_max, [14, 15], 0) in validation_errors
    assert (
        InvalidEnumValueError(ComponentType.transformer, AttributeType.winding_from, [1], WindingType)
        in validation_errors
    )
    assert (
        InvalidEnumValueError(ComponentType.transformer, AttributeType.winding_to, [1], WindingType)
        in validation_errors
    )
    assert (
        InvalidEnumValueError(ComponentType.transformer, AttributeType.tap_side, [1], BranchSide) in validation_errors
    )

    assert InvalidIdError(ComponentType.source, AttributeType.node, [16], ComponentType.node) in validation_errors
    assert NotBooleanError(ComponentType.source, AttributeType.status, [17, 1]) in validation_errors
    assert NotGreaterThanError(ComponentType.source, AttributeType.u_ref, [16, 17], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.source, AttributeType.sk, [16, 1], 0) in validation_errors
    assert NotGreaterOrEqualError(ComponentType.source, AttributeType.rx_ratio, [17], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.source, AttributeType.z01_ratio, [16, 17], 0) in validation_errors

    assert InvalidIdError(ComponentType.shunt, AttributeType.node, [18], ComponentType.node) in validation_errors
    assert NotBooleanError(ComponentType.shunt, AttributeType.status, [19, 1]) in validation_errors

    assert InvalidIdError(ComponentType.sym_load, AttributeType.node, [1], ComponentType.node) in validation_errors
    assert NotBooleanError(ComponentType.sym_load, AttributeType.status, [20, 21]) in validation_errors
    assert InvalidEnumValueError(ComponentType.sym_load, AttributeType.type, [21], LoadGenType) in validation_errors

    assert InvalidIdError(ComponentType.sym_gen, AttributeType.node, [1], ComponentType.node) in validation_errors
    assert NotBooleanError(ComponentType.sym_gen, AttributeType.status, [22, 23]) in validation_errors
    assert InvalidEnumValueError(ComponentType.sym_gen, AttributeType.type, [22], LoadGenType) in validation_errors

    assert InvalidIdError(ComponentType.asym_load, AttributeType.node, [1], ComponentType.node) in validation_errors
    assert NotBooleanError(ComponentType.asym_load, AttributeType.status, [24, 25]) in validation_errors
    assert InvalidEnumValueError(ComponentType.asym_load, AttributeType.type, [1], LoadGenType) in validation_errors

    assert InvalidIdError(ComponentType.asym_gen, AttributeType.node, [1], ComponentType.node) in validation_errors
    assert NotBooleanError(ComponentType.asym_gen, AttributeType.status, [26, 27]) in validation_errors
    assert InvalidEnumValueError(ComponentType.asym_gen, AttributeType.type, [1, 26], LoadGenType) in validation_errors

    assert (
        InvalidIdError(ComponentType.sym_voltage_sensor, AttributeType.measured_object, [108, 110], AttributeType.node)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.sym_voltage_sensor, AttributeType.u_measured, [107, 110], 0)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.sym_voltage_sensor, AttributeType.u_sigma, [109, 110], 0) in validation_errors
    )
    # TODO check if u_sigma = np.nan is detected with missing values

    assert (
        InvalidIdError(ComponentType.asym_voltage_sensor, AttributeType.measured_object, [108, 110], AttributeType.node)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.asym_voltage_sensor, AttributeType.u_measured, [109, 110], 0)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.asym_voltage_sensor, AttributeType.u_sigma, [109, 110], 0)
        in validation_errors
    )

    assert (
        InvalidIdError(
            ComponentType.sym_power_sensor,
            AttributeType.measured_object,
            [107, 109, 110],
            [
                ComponentType.node,
                ComponentType.line,
                ComponentType.asym_line,
                ComponentType.generic_branch,
                ComponentType.transformer,
                ComponentType.three_winding_transformer,
                ComponentType.source,
                ComponentType.shunt,
                ComponentType.sym_load,
                ComponentType.asym_load,
                ComponentType.sym_gen,
                ComponentType.asym_gen,
            ],
        )
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.sym_power_sensor, AttributeType.power_sigma, [109, 110, 114, 115], 0)
        in validation_errors
    )
    assert (
        InvalidIdError(
            ComponentType.sym_power_sensor,
            AttributeType.measured_object,
            [107, 110],
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            {AttributeType.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(
            ComponentType.sym_power_sensor, AttributeType.measured_terminal_type, [109], MeasuredTerminalType
        )
        in validation_errors
    )

    assert (
        InvalidIdError(
            ComponentType.asym_power_sensor,
            AttributeType.measured_object,
            [107, 109, 110],
            [
                ComponentType.node,
                ComponentType.line,
                ComponentType.asym_line,
                ComponentType.generic_branch,
                ComponentType.transformer,
                ComponentType.three_winding_transformer,
                ComponentType.source,
                ComponentType.shunt,
                ComponentType.sym_load,
                ComponentType.asym_load,
                ComponentType.sym_gen,
                ComponentType.asym_gen,
            ],
        )
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.asym_power_sensor, AttributeType.power_sigma, [109, 110, 114, 115], 0)
        in validation_errors
    )
    assert (
        InvalidIdError(
            ComponentType.asym_power_sensor,
            AttributeType.measured_object,
            [107, 110],
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            {AttributeType.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(
            ComponentType.asym_power_sensor, AttributeType.measured_terminal_type, [109], MeasuredTerminalType
        )
        in validation_errors
    )

    assert (
        InvalidIdError(
            ComponentType.sym_current_sensor,
            AttributeType.measured_object,
            [107, 109, 110],
            [
                ComponentType.line,
                ComponentType.asym_line,
                ComponentType.generic_branch,
                ComponentType.transformer,
                ComponentType.three_winding_transformer,
            ],
        )
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.sym_current_sensor, AttributeType.i_sigma, [109, 110], 0) in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.sym_current_sensor, AttributeType.i_angle_sigma, [109, 110], 0)
        in validation_errors
    )
    assert (
        InvalidIdError(
            ComponentType.sym_current_sensor,
            AttributeType.measured_object,
            [107],
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            {AttributeType.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(
            ComponentType.sym_current_sensor, AttributeType.measured_terminal_type, [109], MeasuredTerminalType
        )
        in validation_errors
    )
    assert (
        UnsupportedMeasuredTerminalType(
            ComponentType.sym_current_sensor,
            AttributeType.measured_terminal_type,
            [109, 110],
            [
                MeasuredTerminalType.branch_from,
                MeasuredTerminalType.branch_to,
                MeasuredTerminalType.branch3_1,
                MeasuredTerminalType.branch3_2,
                MeasuredTerminalType.branch3_3,
            ],
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(
            ComponentType.sym_current_sensor, AttributeType.angle_measurement_type, [109, 110], AngleMeasurementType
        )
        in validation_errors
    )

    assert (
        InvalidIdError(
            ComponentType.asym_current_sensor,
            AttributeType.measured_object,
            [107, 109, 110],
            [
                ComponentType.line,
                ComponentType.asym_line,
                ComponentType.generic_branch,
                ComponentType.transformer,
                ComponentType.three_winding_transformer,
            ],
        )
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.asym_current_sensor, AttributeType.i_sigma, [109, 110], 0)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.asym_current_sensor, AttributeType.i_angle_sigma, [109, 110], 0)
        in validation_errors
    )
    assert (
        InvalidIdError(
            ComponentType.asym_current_sensor,
            AttributeType.measured_object,
            [107],
            [ComponentType.line, ComponentType.asym_line, ComponentType.generic_branch, ComponentType.transformer],
            {AttributeType.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(
            ComponentType.asym_current_sensor, AttributeType.measured_terminal_type, [109], MeasuredTerminalType
        )
        in validation_errors
    )
    assert (
        UnsupportedMeasuredTerminalType(
            ComponentType.asym_current_sensor,
            AttributeType.measured_terminal_type,
            [109, 110],
            [
                MeasuredTerminalType.branch_from,
                MeasuredTerminalType.branch_to,
                MeasuredTerminalType.branch3_1,
                MeasuredTerminalType.branch3_2,
                MeasuredTerminalType.branch3_3,
            ],
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(
            ComponentType.asym_current_sensor, AttributeType.angle_measurement_type, [109, 110], AngleMeasurementType
        )
        in validation_errors
    )
    for power_sensor_type, current_sensor_type in itertools.product(
        [ComponentType.sym_power_sensor, ComponentType.asym_power_sensor],
        [ComponentType.sym_current_sensor, ComponentType.asym_current_sensor],
    ):
        assert (
            MixedPowerCurrentSensorError(
                [
                    (power_sensor_type, AttributeType.measured_object),
                    (power_sensor_type, AttributeType.measured_terminal_type),
                    (current_sensor_type, AttributeType.measured_object),
                    (current_sensor_type, AttributeType.measured_terminal_type),
                ],
                [
                    *[(power_sensor_type, i) for i in range(107, 107 + 3)],
                    # 110 is skipped because it points to an invalid measured terminal type
                    *[(power_sensor_type, i) for i in range(111, 111 + 11)],
                    *[(current_sensor_type, i) for i in range(107, 107 + 3)],
                    # 110 is skipped because it points to an invalid measured terminal type
                    (current_sensor_type, 111),
                ],
            )
            in validation_errors
        )

    assert (
        NotGreaterOrEqualError(ComponentType.transformer, AttributeType.uk_max, [15], AttributeType.uk_min)
        not in validation_errors
    )

    assert NotBooleanError(ComponentType.fault, AttributeType.status, [32, 33]) in validation_errors
    assert (
        InvalidIdError(ComponentType.fault, AttributeType.fault_object, [1, *list(range(32, 42))], [ComponentType.node])
        in validation_errors
    )


def test_validate_three_winding_transformer(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert validation_errors is not None
    assert NotBooleanError(ComponentType.three_winding_transformer, AttributeType.status_1, [28]) in validation_errors
    assert NotBooleanError(ComponentType.three_winding_transformer, AttributeType.status_2, [1]) in validation_errors
    assert NotBooleanError(ComponentType.three_winding_transformer, AttributeType.status_3, [29]) in validation_errors
    assert (
        InvalidIdError(ComponentType.three_winding_transformer, AttributeType.node_1, [29], ComponentType.node)
        in validation_errors
    )
    assert (
        InvalidIdError(ComponentType.three_winding_transformer, AttributeType.node_2, [28], ComponentType.node)
        in validation_errors
    )
    assert (
        InvalidIdError(ComponentType.three_winding_transformer, AttributeType.node_3, [29], ComponentType.node)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.three_winding_transformer, AttributeType.u1, [1, 28], 0) in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.three_winding_transformer, AttributeType.u2, [1, 28], 0) in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.three_winding_transformer, AttributeType.u3, [1, 28], 0) in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.three_winding_transformer, AttributeType.sn_1, [1, 28], 0)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.three_winding_transformer, AttributeType.sn_2, [1, 28], 0)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.three_winding_transformer, AttributeType.sn_3, [1, 28], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_12,
            [29, 30],
            f"{AttributeType.pk_12}/{AttributeType.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_12,
            [1, 30],
            f"{AttributeType.pk_12}/{AttributeType.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_13,
            [29],
            f"{AttributeType.pk_13}/{AttributeType.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_13,
            [30],
            f"{AttributeType.pk_13}/{AttributeType.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_23,
            [1, 29],
            f"{AttributeType.pk_23}/{AttributeType.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_23,
            [30],
            f"{AttributeType.pk_23}/{AttributeType.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_12, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_13, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_23, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_12, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_13, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_23, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer, AttributeType.i0, [29], f"{AttributeType.p0}/{AttributeType.sn_1}"
        )
        in validation_errors
    )
    assert NotLessThanError(ComponentType.three_winding_transformer, AttributeType.i0, [28], 1) in validation_errors
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.p0, [1], 0) in validation_errors
    )
    assert (
        NotBetweenOrAtError(ComponentType.three_winding_transformer, AttributeType.clock_13, [1, 28], (-12, 12))
        in validation_errors
    )
    assert (
        NotBetweenOrAtError(
            ComponentType.three_winding_transformer,
            AttributeType.tap_pos,
            [1, 28],
            (AttributeType.tap_min, AttributeType.tap_max),
        )
        in validation_errors
    )
    assert (
        NotBetweenOrAtError(
            ComponentType.three_winding_transformer,
            AttributeType.tap_nom,
            [1, 28],
            (AttributeType.tap_min, AttributeType.tap_max),
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.tap_size, [1], 0)
        in validation_errors
    )
    assert (
        InvalidEnumValueError(ComponentType.three_winding_transformer, AttributeType.winding_1, [1, 28], WindingType)
        in validation_errors
    )
    assert (
        InvalidEnumValueError(ComponentType.three_winding_transformer, AttributeType.winding_2, [1, 28], WindingType)
        in validation_errors
    )
    assert (
        InvalidEnumValueError(ComponentType.three_winding_transformer, AttributeType.winding_3, [1, 28], WindingType)
        in validation_errors
    )
    assert (
        InvalidEnumValueError(ComponentType.three_winding_transformer, AttributeType.tap_side, [1, 28], Branch3Side)
        in validation_errors
    )


def test_validate_three_winding_transformer_ukpkminmax(input_data):
    validation_errors = validate_input_data(input_data, symmetric=False)
    assert validation_errors is not None
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_12_min,
            [29, 30],
            f"{AttributeType.pk_12_min}/{AttributeType.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_12_min,
            [1, 30],
            f"{AttributeType.pk_12_min}/{AttributeType.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_13_min,
            [29],
            f"{AttributeType.pk_13_min}/{AttributeType.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_13_min,
            [30],
            f"{AttributeType.pk_13_min}/{AttributeType.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_23_min,
            [1, 29],
            f"{AttributeType.pk_23_min}/{AttributeType.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_23_min,
            [30],
            f"{AttributeType.pk_23_min}/{AttributeType.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_12_min, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_13_min, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_23_min, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_12_max,
            [29, 30],
            f"{AttributeType.pk_12_max}/{AttributeType.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_12_max,
            [1, 30],
            f"{AttributeType.pk_12_max}/{AttributeType.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_13_max,
            [29],
            f"{AttributeType.pk_13_max}/{AttributeType.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_13_max,
            [30],
            f"{AttributeType.pk_13_max}/{AttributeType.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_23_max,
            [1, 29],
            f"{AttributeType.pk_23_max}/{AttributeType.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.three_winding_transformer,
            AttributeType.uk_23_max,
            [30],
            f"{AttributeType.pk_23_max}/{AttributeType.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_12_max, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_13_max, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotBetweenError(ComponentType.three_winding_transformer, AttributeType.uk_23_max, [1, 28], (0, 1))
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_12_min, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_13_min, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_23_min, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_12_max, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_13_max, [1], 0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.three_winding_transformer, AttributeType.pk_23_max, [1], 0)
        in validation_errors
    )


def test_validate_input_data_transformer_tap_regulator(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.power_flow)
    assert validation_errors is not None
    assert (
        NotBooleanError(ComponentType.transformer_tap_regulator, AttributeType.status, [52, 1, 53]) in validation_errors
    )
    assert (
        InvalidIdError(
            ComponentType.transformer_tap_regulator,
            AttributeType.regulated_object,
            [1],
            [ComponentType.transformer, ComponentType.three_winding_transformer],
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(
            ComponentType.transformer_tap_regulator, AttributeType.control_side, [1], [BranchSide, Branch3Side]
        )
        in validation_errors
    )
    assert (
        InvalidAssociatedEnumValueError(
            ComponentType.transformer_tap_regulator,
            [AttributeType.control_side, AttributeType.regulated_object],
            [52],
            [BranchSide],
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(ComponentType.transformer_tap_regulator, AttributeType.u_set, [52], 0.0)
        in validation_errors
    )
    assert (
        NotGreaterThanError(ComponentType.transformer_tap_regulator, AttributeType.u_band, [52, 1], 0.0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.transformer_tap_regulator, AttributeType.line_drop_compensation_r, [52], 0.0
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            ComponentType.transformer_tap_regulator, AttributeType.line_drop_compensation_x, [1], 0.0
        )
        in validation_errors
    )
    assert (
        NotUniqueError(ComponentType.transformer_tap_regulator, AttributeType.regulated_object, [51, 54])
        in validation_errors
    )


def test_validate_input_data_voltage_regulator(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.power_flow)
    assert validation_errors is not None

    assert NotBooleanError(ComponentType.voltage_regulator, AttributeType.status, [62, 65]) in validation_errors
    assert (
        NotUniqueError(ComponentType.voltage_regulator, AttributeType.regulated_object, [60, 67]) in validation_errors
    )
    assert NotGreaterThanError(ComponentType.voltage_regulator, AttributeType.u_ref, [66], 0.0) in validation_errors
    assert InfinityError(ComponentType.voltage_regulator, AttributeType.u_ref, [65]) in validation_errors
    assert (
        InvalidIdError(
            ComponentType.voltage_regulator,
            AttributeType.regulated_object,
            [64, 65, 66],
            [ComponentType.sym_gen, ComponentType.asym_gen, ComponentType.sym_load, ComponentType.asym_load],
        )
        in validation_errors
    )
    assert (
        InvalidVoltageRegulationError(ComponentType.voltage_regulator, AttributeType.u_ref, [60, 67])
        in validation_errors
    )


def test_fault(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.short_circuit)
    assert validation_errors is not None
    assert InvalidEnumValueError(ComponentType.fault, AttributeType.fault_type, [50], FaultType) in validation_errors
    assert InvalidEnumValueError(ComponentType.fault, AttributeType.fault_phase, [50], FaultPhase) in validation_errors
    assert FaultPhaseError(
        ComponentType.fault, [AttributeType.fault_type, AttributeType.fault_phase], [1, *list(range(32, 51))]
    )
    assert NotGreaterOrEqualError(ComponentType.fault, AttributeType.r_f, [1], 0) in validation_errors
    assert (
        NotIdenticalError(
            ComponentType.fault,
            AttributeType.fault_type,
            list(range(32, 51)),
            5 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [_nan_type(ComponentType.fault, AttributeType.fault_type), 4],
        )
        in validation_errors
    )
    assert (
        NotIdenticalError(
            ComponentType.fault,
            AttributeType.fault_phase,
            list(range(32, 51)),
            list(range(2, 7))
            + [0, 4, 5, 6]
            + 2 * list(range(4))
            + [_nan_type(ComponentType.fault, AttributeType.fault_phase), 7],
        )
        in validation_errors
    )


def test_validate_input_data_asym_calculation(input_data):
    validation_errors = validate_input_data(input_data, symmetric=False)
    assert validation_errors is not None
    assert NotGreaterThanError(ComponentType.node, AttributeType.u_rated, [1], 0) in validation_errors
    assert NotUniqueError(ComponentType.node, AttributeType.id, [2, 2]) in validation_errors
    assert NotBooleanError(ComponentType.line, AttributeType.from_status, [5]) in validation_errors
    assert NotBooleanError(ComponentType.line, AttributeType.to_status, [4]) in validation_errors
    assert InvalidIdError(ComponentType.line, AttributeType.from_node, [4], ComponentType.node) in validation_errors
    assert InvalidIdError(ComponentType.line, AttributeType.to_node, [5], ComponentType.node) in validation_errors


def test_validate_input_data_invalid_structure():
    with pytest.raises(TypeError, match=r"should be a Numpy structured array"):
        validate_input_data({ComponentType.node: np.array([[1, 10500.0], [2, 10500.0]])}, symmetric=True)


def test_generic_branch_input_data(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert validation_errors is not None
    assert NotGreaterThanError(ComponentType.generic_branch, AttributeType.k, [6], 0) in validation_errors
    assert NotGreaterOrEqualError(ComponentType.generic_branch, AttributeType.sn, [6], 0) in validation_errors


def test_asym_line_input_data(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert validation_errors is not None
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_aa, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_ba, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_bb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_ca, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_cb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_cc, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_na, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_nb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_nc, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.r_nn, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_aa, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_ba, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_bb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_ca, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_cb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_cc, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_na, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_nb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_nc, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.x_nn, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c_aa, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c_ba, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c_bb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c_ca, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c_cb, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c_cc, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c0, [55], 0) in validation_errors
    assert NotGreaterThanError(ComponentType.asym_line, AttributeType.c1, [55], 0) in validation_errors
    assert (
        MultiFieldValidationError(
            ComponentType.asym_line,
            [
                AttributeType.r_na,
                AttributeType.r_nb,
                AttributeType.r_nc,
                AttributeType.r_nn,
                AttributeType.x_na,
                AttributeType.x_nb,
                AttributeType.x_nc,
                AttributeType.x_nn,
            ],
            [56, 57],
        )
        in validation_errors
    )
    assert (
        MultiFieldValidationError(
            ComponentType.asym_line,
            [
                AttributeType.c_aa,
                AttributeType.c_ba,
                AttributeType.c_bb,
                AttributeType.c_ca,
                AttributeType.c_cb,
                AttributeType.c_cc,
                AttributeType.c0,
                AttributeType.c1,
            ],
            [56],
        )
        in validation_errors
    )
    assert (
        MultiFieldValidationError(
            ComponentType.asym_line,
            [
                AttributeType.c_aa,
                AttributeType.c_ba,
                AttributeType.c_bb,
                AttributeType.c_ca,
                AttributeType.c_cb,
                AttributeType.c_cc,
            ],
            [58],
        )
        in validation_errors
    )


@pytest.mark.parametrize("component_type", ComponentType)
def test_validate_input_data__no_components(component_type: ComponentType):
    input_data = {component_type: initialize_array(DatasetType.input, component_type, 0)}
    assert validate_input_data(input_data) is None
    assert_valid_input_data(input_data)
