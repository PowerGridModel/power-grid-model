# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import itertools

import numpy as np
import pytest

from power_grid_model import (
    AttributeType as AT,
    Branch3Side,
    BranchSide,
    ComponentType as CT,
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
def original_data() -> dict[CT, np.ndarray]:
    node = initialize_array(DatasetType.input, CT.node, 4)
    node[AT.id] = [0, 2, 1, 2]
    node[AT.u_rated] = [10.5e3, 10.5e3, 0, 10.5e3]

    line = initialize_array(DatasetType.input, CT.line, 3)
    line[AT.id] = [3, 4, 5]
    line[AT.from_node] = [0, -1, 2]
    line[AT.to_node] = [2, 1, 8]
    line[AT.from_status] = [0, 1, -2]
    line[AT.to_status] = [0, 4, 1]
    line[AT.r1] = [0, 100, 1]
    line[AT.x1] = [0, 5, 0]
    line[AT.r0] = [10, 0, 0]
    line[AT.x0] = [0, 0, 50]
    line[AT.i_n] = [-3, 0, 50]

    asym_line = initialize_array(DatasetType.input, CT.asym_line, 4)
    asym_line[AT.id] = [55, 56, 57, 58]
    asym_line[AT.from_node] = [0, 1, 0, 0]
    asym_line[AT.to_node] = [1, 2, 2, 1]
    asym_line[AT.from_status] = [1, 1, 1, 1]
    asym_line[AT.to_status] = [1, 1, 1, 1]
    asym_line[AT.r_aa] = [-1, 2, 2, 2]
    asym_line[AT.r_ba] = [-1, 2, 2, 2]
    asym_line[AT.r_bb] = [-1, 2, 2, 2]
    asym_line[AT.r_ca] = [-1, 2, 2, 2]
    asym_line[AT.r_cb] = [-1, 2, 2, 2]
    asym_line[AT.r_cc] = [-1, 2, 2, 2]
    asym_line[AT.r_na] = [-1, 2, 2, 2]
    asym_line[AT.r_nb] = [-1, 2, 2, 2]
    asym_line[AT.r_nc] = [-1, 2, 2, 2]
    asym_line[AT.r_nn] = [-1, np.nan, 2, 2]
    asym_line[AT.x_aa] = [-1, 2, 2, 2]
    asym_line[AT.x_ba] = [-1, 2, 2, 2]
    asym_line[AT.x_bb] = [-1, 2, 2, 2]
    asym_line[AT.x_ca] = [-1, 2, 2, 2]
    asym_line[AT.x_cb] = [-1, 2, 2, 2]
    asym_line[AT.x_cc] = [-1, 2, 2, 2]
    asym_line[AT.x_na] = [-1, 2, np.nan, 2]
    asym_line[AT.x_nb] = [-1, 2, 2, 2]
    asym_line[AT.x_nc] = [-1, 2, 2, 2]
    asym_line[AT.x_nn] = [-1, 2, 2, 2]
    asym_line[AT.c_aa] = [-1, np.nan, 2, np.nan]
    asym_line[AT.c_ba] = [-1, np.nan, 2, np.nan]
    asym_line[AT.c_bb] = [-1, np.nan, 2, np.nan]
    asym_line[AT.c_ca] = [-1, np.nan, 2, np.nan]
    asym_line[AT.c_cb] = [-1, np.nan, 2, np.nan]
    asym_line[AT.c_cc] = [-1, np.nan, 2, 2]
    asym_line[AT.c0] = [-1, np.nan, np.nan, np.nan]
    asym_line[AT.c1] = [-1, np.nan, np.nan, np.nan]
    asym_line[AT.i_n] = [50, 50, 50, 50]

    generic_branch = initialize_array(DatasetType.input, CT.generic_branch, 1)
    generic_branch[AT.id] = [6]
    generic_branch[AT.from_node] = [1]
    generic_branch[AT.to_node] = [2]
    generic_branch[AT.from_status] = [1]
    generic_branch[AT.to_status] = [1]
    generic_branch[AT.r1] = [0.129059]
    generic_branch[AT.x1] = [16.385859]
    generic_branch[AT.g1] = [8.692e-7]
    generic_branch[AT.b1] = [-2.336e-7]
    generic_branch[AT.k] = [0.0]
    generic_branch[AT.theta] = [0.0]
    generic_branch[AT.sn] = [-10.0]

    link = initialize_array(DatasetType.input, CT.link, 2)
    link[AT.id] = [12, 13]
    link[AT.from_node] = [0, -1]
    link[AT.to_node] = [8, 1]
    link[AT.from_status] = [3, 1]
    link[AT.to_status] = [0, 4]

    transformer = initialize_array(DatasetType.input, CT.transformer, 3)
    transformer[AT.id] = [1, 14, 15]
    transformer[AT.from_node] = [1, 7, 2]  # TODO check from node 1 to node 1
    transformer[AT.to_node] = [1, 8, 1]
    transformer[AT.from_status] = [1, 1, -1]
    transformer[AT.to_status] = [2, 0, 1]
    transformer[AT.u1] = [10500.0, 0.0, -1.0]
    transformer[AT.u2] = [400.0, -3.0, 0.0]
    transformer[AT.sn] = [630000.0, 0.0, -20.0]
    transformer[AT.uk] = [-1.0, 1.1, 0.9]
    transformer[AT.pk] = [63000.0, 0.0, -10.0]
    transformer[AT.i0] = [0.0368, 10.0, 0.9]
    transformer[AT.p0] = [63000.0, 0.0, -10.0]
    transformer[AT.winding_from] = [8, 0, 2]
    transformer[AT.winding_to] = [5, 1, 2]
    transformer[AT.clock] = [13, -13, 7]
    transformer[AT.tap_side] = [-1, 0, 1]
    transformer[AT.tap_pos] = [-1, 6, -4]
    transformer[AT.tap_min] = [-2, 4, 3]
    transformer[AT.tap_max] = [2, -4, -3]
    transformer[AT.tap_nom] = [-3, _nan_type(CT.transformer, AT.tap_nom), 4]
    transformer[AT.tap_size] = [262.5, 0.0, -10.0]
    transformer[AT.uk_min] = [0.0000000005, _nan_type(CT.transformer, AT.uk_min), 0.9]
    transformer[AT.uk_max] = [0.0000000005, _nan_type(CT.transformer, AT.uk_max), 0.8]
    transformer[AT.pk_min] = [300.0, 0.0, _nan_type(CT.transformer, AT.pk_min)]
    transformer[AT.pk_max] = [400.0, -0.1, _nan_type(CT.transformer, AT.pk_max)]

    three_winding_transformer = initialize_array(DatasetType.input, CT.three_winding_transformer, 4)
    three_winding_transformer[AT.id] = [1, 28, 29, 30]
    three_winding_transformer[AT.node_1] = [0, 1, 9, 2]
    three_winding_transformer[AT.node_2] = [1, 15, 1, 0]
    three_winding_transformer[AT.node_3] = [1, 2, 12, 1]
    three_winding_transformer[AT.status_1] = [1, 5, 1, 1]
    three_winding_transformer[AT.status_2] = [2, 1, 1, 1]
    three_winding_transformer[AT.status_3] = [1, 0, -1, 0]
    three_winding_transformer[AT.u1] = [-100, 0, 200, 100]
    three_winding_transformer[AT.u2] = [0, -200, 100, 200]
    three_winding_transformer[AT.u3] = [-100, 0, 400, 300]
    three_winding_transformer[AT.sn_1] = [0, -1200, 100, 300]
    three_winding_transformer[AT.sn_2] = [-1000, 0, 200, 200]
    three_winding_transformer[AT.sn_3] = [0, -2300, 300, 100]
    three_winding_transformer[AT.uk_12] = [-1, 1.1, 0.05, 0.1]
    three_winding_transformer[AT.uk_13] = [-2, 1.2, 0.3, 0.2]
    three_winding_transformer[AT.uk_23] = [-1.5, 1, 0.15, 0.2]
    three_winding_transformer[AT.pk_12] = [-450, 100, 10, 40]
    three_winding_transformer[AT.pk_13] = [-40, 50, 40, 50]
    three_winding_transformer[AT.pk_23] = [-120, 1, 40, 30]
    three_winding_transformer[AT.i0] = [-0.5, 1.8, 0.3, 0.6]
    three_winding_transformer[AT.p0] = [-100, 410, 60, 40]
    three_winding_transformer[AT.winding_1] = [15, -1, 0, 2]
    three_winding_transformer[AT.winding_2] = [19, -2, 1, 3]
    three_winding_transformer[AT.winding_3] = [-2, 13, 2, 2]
    three_winding_transformer[AT.clock_12] = [-12, 24, 4, 3]
    three_winding_transformer[AT.clock_13] = [-30, 40, 3, 4]
    three_winding_transformer[AT.tap_side] = [-1, 9, 1, 0]
    three_winding_transformer[AT.tap_pos] = [50, -24, 5, 3]
    three_winding_transformer[AT.tap_min] = [-10, -10, -10, -10]
    three_winding_transformer[AT.tap_max] = [10, 10, 10, 10]
    three_winding_transformer[AT.tap_size] = [-12, 0, 3, 130]
    three_winding_transformer[AT.tap_nom] = [
        -12,
        41,
        _nan_type(CT.three_winding_transformer, AT.tap_nom),
        0,
    ]
    three_winding_transformer[AT.uk_12_min] = [
        _nan_type(CT.three_winding_transformer, AT.uk_12_min),
        1.1,
        0.05,
        _nan_type(CT.three_winding_transformer, AT.uk_12_min),
    ]
    three_winding_transformer[AT.uk_13_min] = [
        _nan_type(CT.three_winding_transformer, AT.uk_13_min),
        1.2,
        0.3,
        _nan_type(CT.three_winding_transformer, AT.uk_13_min),
    ]
    three_winding_transformer[AT.uk_23_min] = [
        _nan_type(CT.three_winding_transformer, AT.uk_23_min),
        1,
        0.15,
        _nan_type(CT.three_winding_transformer, AT.uk_23_min),
    ]
    three_winding_transformer[AT.pk_12_min] = [
        -450,
        _nan_type(CT.three_winding_transformer, AT.pk_12_min),
        10,
        40,
    ]
    three_winding_transformer[AT.pk_13_min] = [
        -40,
        _nan_type(CT.three_winding_transformer, AT.pk_13_min),
        40,
        50,
    ]
    three_winding_transformer[AT.pk_23_min] = [
        -120,
        _nan_type(CT.three_winding_transformer, AT.pk_23_min),
        40,
        30,
    ]
    three_winding_transformer[AT.uk_12_max] = [
        _nan_type(CT.three_winding_transformer, AT.uk_12_max),
        1.1,
        0.05,
        _nan_type(CT.three_winding_transformer, AT.uk_12_max),
    ]
    three_winding_transformer[AT.uk_13_max] = [
        _nan_type(CT.three_winding_transformer, AT.uk_13_max),
        1.2,
        0.3,
        _nan_type(CT.three_winding_transformer, AT.uk_13_max),
    ]
    three_winding_transformer[AT.uk_23_max] = [
        _nan_type(CT.three_winding_transformer, AT.uk_23_max),
        1,
        0.15,
        _nan_type(CT.three_winding_transformer, AT.uk_23_max),
    ]
    three_winding_transformer[AT.pk_12_max] = [
        -450,
        _nan_type(CT.three_winding_transformer, AT.pk_12_max),
        10,
        40,
    ]
    three_winding_transformer[AT.pk_13_max] = [
        -40,
        _nan_type(CT.three_winding_transformer, AT.pk_12_max),
        40,
        50,
    ]
    three_winding_transformer[AT.pk_23_max] = [
        -120,
        _nan_type(CT.three_winding_transformer, AT.pk_12_max),
        40,
        30,
    ]

    transformer_tap_regulator = initialize_array(DatasetType.input, CT.transformer_tap_regulator, 5)
    transformer_tap_regulator[AT.id] = [51, 52, 53, 54, 1]
    transformer_tap_regulator[AT.status] = [0, -1, 2, 1, 5]
    transformer_tap_regulator[AT.regulated_object] = [14, 15, 28, 14, 2]
    transformer_tap_regulator[AT.control_side] = [1, 2, 0, 0, 60]
    transformer_tap_regulator[AT.u_set] = [100, -100, 100, 100, 100]
    transformer_tap_regulator[AT.u_band] = [100, -4, 100, 100, 0]
    transformer_tap_regulator[AT.line_drop_compensation_r] = [0.0, -1.0, 1.0, 0.0, 2.0]
    transformer_tap_regulator[AT.line_drop_compensation_x] = [0.0, 4.0, 2.0, 0.0, -4.0]

    source = initialize_array(DatasetType.input, CT.source, 3)
    source[AT.id] = [16, 17, 1]
    source[AT.node] = [10, 1, 2]
    source[AT.status] = [0, -1, 2]
    source[AT.u_ref] = [-10.0, 0.0, 100.0]
    source[AT.sk] = [0.0, 100.0, -20.0]
    source[AT.rx_ratio] = [0.0, -30.0, 300.0]
    source[AT.z01_ratio] = [-1.0, 0.0, 200.0]

    shunt = initialize_array(DatasetType.input, CT.shunt, 3)
    shunt[AT.id] = [18, 19, 1]
    shunt[AT.node] = [10, 1, 2]
    shunt[AT.status] = [0, -1, 2]

    sym_load = initialize_array(DatasetType.input, CT.sym_load, 3)
    sym_load[AT.id] = [1, 20, 21]
    sym_load[AT.type] = [1, 0, 5]
    sym_load[AT.node] = [10, 1, 2]
    sym_load[AT.status] = [0, -1, 2]

    sym_gen = initialize_array(DatasetType.input, CT.sym_gen, 3)
    sym_gen[AT.id] = [1, 22, 23]
    sym_gen[AT.type] = [2, -1, 1]
    sym_gen[AT.node] = [10, 1, 2]
    sym_gen[AT.status] = [0, -1, 2]

    asym_load = initialize_array(DatasetType.input, CT.asym_load, 3)
    asym_load[AT.id] = [1, 24, 25]
    asym_load[AT.type] = [5, 0, 2]
    asym_load[AT.node] = [10, 1, 2]
    asym_load[AT.status] = [0, -1, 2]

    asym_gen = initialize_array(DatasetType.input, CT.asym_gen, 3)
    asym_gen[AT.id] = [1, 26, 27]
    asym_gen[AT.type] = [-1, 5, 2]
    asym_gen[AT.node] = [10, 1, 2]
    asym_gen[AT.status] = [0, -1, 2]

    sym_voltage_sensor = initialize_array(DatasetType.input, CT.sym_voltage_sensor, 5)
    sym_voltage_sensor[AT.id] = range(107, 107 + 5)
    sym_voltage_sensor[AT.measured_object] = [2, 3, 1, 200, 2]
    sym_voltage_sensor[AT.u_measured] = [0.0, 10.4e3, 10.6e3, -20.0, 1e4]
    sym_voltage_sensor[AT.u_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]

    asym_voltage_sensor = initialize_array(DatasetType.input, CT.asym_voltage_sensor, 5)
    asym_voltage_sensor[AT.id] = range(107, 107 + 5)
    asym_voltage_sensor[AT.measured_object] = [2, 3, 1, 200, 2]
    asym_voltage_sensor[AT.u_measured] = np.array(
        [
            [10.5e3, 10.4e3, 10.6e3],
            [np.nan, np.nan, np.nan],
            [0, 0, 0],
            [-1e4, 1e4, 1e4],
            [1.0e4, 1.0e4, 1.0e4],
        ]
    )
    asym_voltage_sensor[AT.u_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]

    sym_power_sensor = initialize_array(DatasetType.input, CT.sym_power_sensor, 15)
    sym_power_sensor[AT.id] = range(107, 107 + 15)
    sym_power_sensor[AT.measured_object] = [12, 3, 13, 200, 3] + [3] * 10
    sym_power_sensor[AT.power_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf] * 2 + [np.nan] * 5
    sym_power_sensor[AT.measured_terminal_type] = [1, 1, 10, 1, 1] + [1] * 10
    sym_power_sensor[AT.p_sigma] = [np.nan] * 5 + [1.0, 1.0, 0.0, -1.0, np.inf] * 2
    sym_power_sensor[AT.q_sigma] = [np.nan] * 5 + [1.0, np.inf, 0.0, -1.0, np.inf] * 2

    asym_power_sensor = initialize_array(DatasetType.input, CT.asym_power_sensor, 15)
    asym_power_sensor[AT.id] = range(107, 107 + 15)
    asym_power_sensor[AT.measured_object] = [12, 3, 13, 200, 3] + [3] * 10
    asym_power_sensor[AT.power_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf] * 2 + [np.nan] * 5
    asym_power_sensor[AT.measured_terminal_type] = [1, 1, 10, 1, 1] + [1] * 10
    asym_power_sensor[AT.p_sigma] = [[np.nan] * 3] * 5 + [
        [1.0, 1.0, 1.0],
        [np.inf, np.inf, np.inf],
        [1.0, np.nan, 1.0],
        [1.0, 1.0, -1.0],
        [np.inf, 1.0, 1.0],
    ] * 2
    asym_power_sensor[AT.q_sigma] = [[np.nan] * 3] * 5 + [
        [1.0, 1.0, 1.0],
        [np.inf, np.inf, np.inf],
        [np.inf, np.nan, 1.0],
        [1.0, 1.0, -1.0],
        [np.inf, 1.0, 1.0],
    ] * 2

    sym_current_sensor = initialize_array(DatasetType.input, CT.sym_current_sensor, 5)
    sym_current_sensor[AT.id] = range(107, 107 + 5)
    sym_current_sensor[AT.measured_object] = [12, 3, 13, 200, 3]
    sym_current_sensor[AT.i_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    sym_current_sensor[AT.i_angle_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    sym_current_sensor[AT.measured_terminal_type] = [1, 1, 10, 2, 1]
    sym_current_sensor[AT.angle_measurement_type] = [0, 1, 10, 2, 0]

    asym_current_sensor = initialize_array(DatasetType.input, CT.asym_current_sensor, 5)
    asym_current_sensor[AT.id] = range(107, 107 + 5)
    asym_current_sensor[AT.measured_object] = [12, 3, 13, 200, 3]
    asym_current_sensor[AT.i_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    asym_current_sensor[AT.i_angle_sigma] = [1.0, np.nan, 0.0, -1.0, np.inf]
    asym_current_sensor[AT.measured_terminal_type] = [1, 1, 10, 2, 1]
    asym_current_sensor[AT.angle_measurement_type] = [0, 1, 10, 2, 0]

    fault = initialize_array(DatasetType.input, CT.fault, 20)
    fault[AT.id] = [1, *list(range(32, 51))]
    fault[AT.status] = [0, -1, 2] + 17 * [1]
    fault[AT.fault_type] = 6 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [_nan_type(CT.fault, AT.fault_type), 4]
    fault[AT.fault_phase] = (
        list(range(1, 7)) + [0, 4, 5, 6] + 2 * list(range(4)) + [_nan_type(CT.fault, AT.fault_phase), 7]
    )
    fault[AT.fault_object] = [200, 3] + list(range(10, 28, 2)) + 9 * [0]
    fault[AT.r_f] = [-1.0, 0.0, 1.0] + 17 * [_nan_type(CT.fault, AT.r_f)]
    fault[AT.x_f] = [-1.0, 0.0, 1.0] + 17 * [_nan_type(CT.fault, AT.x_f)]

    voltage_regulator = initialize_array(DatasetType.input, CT.voltage_regulator, 8)
    voltage_regulator[AT.id] = [60, 61, 62, 63, 64, 65, 66, 67]
    # 20-27 are gen/load IDs, 200 is invalid, 16,18 are shunts/sources, 20 is duplicate with different u_ref
    voltage_regulator[AT.regulated_object] = [20, 23, 25, 27, 200, 16, 18, 20]
    voltage_regulator[AT.status] = [1, 0, -1, 1, 1, 5, 0, 1]  # -1 and 5 are invalid boolean values
    voltage_regulator[AT.u_ref] = [1.02, 100, 100.0, 100.0, 1.0, np.inf, 0.0, 1.03]

    data = {
        CT.node: node,
        CT.line: line,
        CT.asym_line: asym_line,
        CT.generic_branch: generic_branch,
        CT.link: link,
        CT.transformer: transformer,
        CT.three_winding_transformer: three_winding_transformer,
        CT.transformer_tap_regulator: transformer_tap_regulator,
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
                (CT.asym_gen, AT.id),
                (CT.asym_load, AT.id),
                (CT.asym_current_sensor, AT.id),
                (CT.asym_power_sensor, AT.id),
                (CT.asym_voltage_sensor, AT.id),
                (CT.node, AT.id),
                (CT.shunt, AT.id),
                (CT.source, AT.id),
                (CT.sym_gen, AT.id),
                (CT.sym_load, AT.id),
                (CT.sym_current_sensor, AT.id),
                (CT.sym_power_sensor, AT.id),
                (CT.sym_voltage_sensor, AT.id),
                (CT.transformer, AT.id),
                (CT.three_winding_transformer, AT.id),
                (CT.fault, AT.id),
                (CT.transformer_tap_regulator, AT.id),
            ],
            [
                (CT.asym_gen, 1),
                (CT.asym_load, 1),
                *[(CT.asym_current_sensor, i) for i in range(107, 107 + 5)],
                *[(CT.asym_power_sensor, i) for i in range(107, 107 + 15)],
                *[(CT.asym_voltage_sensor, i) for i in range(107, 107 + 5)],
                (CT.node, 1),
                (CT.shunt, 1),
                (CT.source, 1),
                (CT.sym_gen, 1),
                (CT.sym_load, 1),
                *[(CT.sym_current_sensor, i) for i in range(107, 107 + 5)],
                *[(CT.sym_power_sensor, i) for i in range(107, 107 + 15)],
                *[(CT.sym_voltage_sensor, i) for i in range(107, 107 + 5)],
                (CT.transformer, 1),
                (CT.three_winding_transformer, 1),
                (CT.fault, 1),
                (CT.transformer_tap_regulator, 1),
            ],
        )
        in validation_errors
    )

    assert NotGreaterThanError(CT.node, AT.u_rated, [1], 0) in validation_errors
    assert NotUniqueError(CT.node, AT.id, [2, 2]) in validation_errors

    assert NotBooleanError(CT.line, AT.from_status, [5]) in validation_errors
    assert NotBooleanError(CT.line, AT.to_status, [4]) in validation_errors
    assert InvalidIdError(CT.line, AT.from_node, [4], CT.node) in validation_errors
    assert InvalidIdError(CT.line, AT.to_node, [5], CT.node) in validation_errors
    assert TwoValuesZeroError(CT.line, [AT.r1, AT.x1], [3]) in validation_errors
    assert TwoValuesZeroError(CT.line, [AT.r0, AT.x0], [4]) in validation_errors
    assert NotGreaterThanError(CT.line, AT.i_n, [3, 4], 0) in validation_errors

    assert NotBooleanError(CT.link, AT.from_status, [12]) in validation_errors
    assert NotBooleanError(CT.link, AT.to_status, [13]) in validation_errors
    assert InvalidIdError(CT.link, AT.from_node, [13], CT.node) in validation_errors
    assert InvalidIdError(CT.link, AT.to_node, [12], CT.node) in validation_errors

    assert NotBooleanError(CT.transformer, AT.from_status, [15]) in validation_errors
    assert NotBooleanError(CT.transformer, AT.to_status, [1]) in validation_errors
    assert InvalidIdError(CT.transformer, AT.from_node, [14], CT.node) in validation_errors
    assert InvalidIdError(CT.transformer, AT.to_node, [14], CT.node) in validation_errors
    assert NotGreaterThanError(CT.transformer, AT.u1, [14, 15], 0) in validation_errors
    assert NotGreaterThanError(CT.transformer, AT.u2, [14, 15], 0) in validation_errors
    assert NotGreaterThanError(CT.transformer, AT.sn, [14, 15], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.uk, [1], f"{AT.pk}/{AT.sn}") in validation_errors
    assert NotBetweenError(CT.transformer, AT.uk, [1, 14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.pk, [15], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.i0, [1], f"{AT.p0}/{AT.sn}") in validation_errors
    assert NotLessThanError(CT.transformer, AT.i0, [14], 1) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.p0, [15], 0) in validation_errors
    assert NotBetweenOrAtError(CT.transformer, AT.clock, [1, 14], (-12, 12)) in validation_errors
    assert NotBetweenOrAtError(CT.transformer, AT.tap_pos, [14, 15], (AT.tap_min, AT.tap_max)) in validation_errors
    assert NotBetweenOrAtError(CT.transformer, AT.tap_nom, [1, 15], (AT.tap_min, AT.tap_max)) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.tap_size, [15], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.uk_min, [1], f"{AT.pk_min}/{AT.sn}") in validation_errors
    assert NotBetweenError(CT.transformer, AT.uk_min, [14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.uk_max, [1], f"{AT.pk_max}/{AT.sn}") in validation_errors
    assert NotBetweenError(CT.transformer, AT.uk_max, [14], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.pk_min, [15], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.transformer, AT.pk_max, [14, 15], 0) in validation_errors
    assert InvalidEnumValueError(CT.transformer, AT.winding_from, [1], WindingType) in validation_errors
    assert InvalidEnumValueError(CT.transformer, AT.winding_to, [1], WindingType) in validation_errors
    assert InvalidEnumValueError(CT.transformer, AT.tap_side, [1], BranchSide) in validation_errors

    assert InvalidIdError(CT.source, AT.node, [16], CT.node) in validation_errors
    assert NotBooleanError(CT.source, AT.status, [17, 1]) in validation_errors
    assert NotGreaterThanError(CT.source, AT.u_ref, [16, 17], 0) in validation_errors
    assert NotGreaterThanError(CT.source, AT.sk, [16, 1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.source, AT.rx_ratio, [17], 0) in validation_errors
    assert NotGreaterThanError(CT.source, AT.z01_ratio, [16, 17], 0) in validation_errors

    assert InvalidIdError(CT.shunt, AT.node, [18], CT.node) in validation_errors
    assert NotBooleanError(CT.shunt, AT.status, [19, 1]) in validation_errors

    assert InvalidIdError(CT.sym_load, AT.node, [1], CT.node) in validation_errors
    assert NotBooleanError(CT.sym_load, AT.status, [20, 21]) in validation_errors
    assert InvalidEnumValueError(CT.sym_load, AT.type, [21], LoadGenType) in validation_errors

    assert InvalidIdError(CT.sym_gen, AT.node, [1], CT.node) in validation_errors
    assert NotBooleanError(CT.sym_gen, AT.status, [22, 23]) in validation_errors
    assert InvalidEnumValueError(CT.sym_gen, AT.type, [22], LoadGenType) in validation_errors

    assert InvalidIdError(CT.asym_load, AT.node, [1], CT.node) in validation_errors
    assert NotBooleanError(CT.asym_load, AT.status, [24, 25]) in validation_errors
    assert InvalidEnumValueError(CT.asym_load, AT.type, [1], LoadGenType) in validation_errors

    assert InvalidIdError(CT.asym_gen, AT.node, [1], CT.node) in validation_errors
    assert NotBooleanError(CT.asym_gen, AT.status, [26, 27]) in validation_errors
    assert InvalidEnumValueError(CT.asym_gen, AT.type, [1, 26], LoadGenType) in validation_errors

    assert InvalidIdError(CT.sym_voltage_sensor, AT.measured_object, [108, 110], AT.node) in validation_errors
    assert NotGreaterThanError(CT.sym_voltage_sensor, AT.u_measured, [107, 110], 0) in validation_errors
    assert NotGreaterThanError(CT.sym_voltage_sensor, AT.u_sigma, [109, 110], 0) in validation_errors
    # TODO check if u_sigma = np.nan is detected with missing values

    assert InvalidIdError(CT.asym_voltage_sensor, AT.measured_object, [108, 110], AT.node) in validation_errors
    assert NotGreaterThanError(CT.asym_voltage_sensor, AT.u_measured, [109, 110], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_voltage_sensor, AT.u_sigma, [109, 110], 0) in validation_errors

    assert (
        InvalidIdError(
            CT.sym_power_sensor,
            AT.measured_object,
            [107, 109, 110],
            [
                CT.node,
                CT.line,
                CT.asym_line,
                CT.generic_branch,
                CT.transformer,
                CT.three_winding_transformer,
                CT.source,
                CT.shunt,
                CT.sym_load,
                CT.asym_load,
                CT.sym_gen,
                CT.asym_gen,
            ],
        )
        in validation_errors
    )
    assert NotGreaterThanError(CT.sym_power_sensor, AT.power_sigma, [109, 110, 114, 115], 0) in validation_errors
    assert (
        InvalidIdError(
            CT.sym_power_sensor,
            AT.measured_object,
            [107, 110],
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            {AT.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(CT.sym_power_sensor, AT.measured_terminal_type, [109], MeasuredTerminalType)
        in validation_errors
    )

    assert (
        InvalidIdError(
            CT.asym_power_sensor,
            AT.measured_object,
            [107, 109, 110],
            [
                CT.node,
                CT.line,
                CT.asym_line,
                CT.generic_branch,
                CT.transformer,
                CT.three_winding_transformer,
                CT.source,
                CT.shunt,
                CT.sym_load,
                CT.asym_load,
                CT.sym_gen,
                CT.asym_gen,
            ],
        )
        in validation_errors
    )
    assert NotGreaterThanError(CT.asym_power_sensor, AT.power_sigma, [109, 110, 114, 115], 0) in validation_errors
    assert (
        InvalidIdError(
            CT.asym_power_sensor,
            AT.measured_object,
            [107, 110],
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            {AT.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(CT.asym_power_sensor, AT.measured_terminal_type, [109], MeasuredTerminalType)
        in validation_errors
    )

    assert (
        InvalidIdError(
            CT.sym_current_sensor,
            AT.measured_object,
            [107, 109, 110],
            [
                CT.line,
                CT.asym_line,
                CT.generic_branch,
                CT.transformer,
                CT.three_winding_transformer,
            ],
        )
        in validation_errors
    )
    assert NotGreaterThanError(CT.sym_current_sensor, AT.i_sigma, [109, 110], 0) in validation_errors
    assert NotGreaterThanError(CT.sym_current_sensor, AT.i_angle_sigma, [109, 110], 0) in validation_errors
    assert (
        InvalidIdError(
            CT.sym_current_sensor,
            AT.measured_object,
            [107],
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            {AT.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(CT.sym_current_sensor, AT.measured_terminal_type, [109], MeasuredTerminalType)
        in validation_errors
    )
    assert (
        UnsupportedMeasuredTerminalType(
            CT.sym_current_sensor,
            AT.measured_terminal_type,
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
        InvalidEnumValueError(CT.sym_current_sensor, AT.angle_measurement_type, [109, 110], AngleMeasurementType)
        in validation_errors
    )

    assert (
        InvalidIdError(
            CT.asym_current_sensor,
            AT.measured_object,
            [107, 109, 110],
            [
                CT.line,
                CT.asym_line,
                CT.generic_branch,
                CT.transformer,
                CT.three_winding_transformer,
            ],
        )
        in validation_errors
    )
    assert NotGreaterThanError(CT.asym_current_sensor, AT.i_sigma, [109, 110], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_current_sensor, AT.i_angle_sigma, [109, 110], 0) in validation_errors
    assert (
        InvalidIdError(
            CT.asym_current_sensor,
            AT.measured_object,
            [107],
            [CT.line, CT.asym_line, CT.generic_branch, CT.transformer],
            {AT.measured_terminal_type: MeasuredTerminalType.branch_to},
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(CT.asym_current_sensor, AT.measured_terminal_type, [109], MeasuredTerminalType)
        in validation_errors
    )
    assert (
        UnsupportedMeasuredTerminalType(
            CT.asym_current_sensor,
            AT.measured_terminal_type,
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
        InvalidEnumValueError(CT.asym_current_sensor, AT.angle_measurement_type, [109, 110], AngleMeasurementType)
        in validation_errors
    )
    for power_sensor_type, current_sensor_type in itertools.product(
        [CT.sym_power_sensor, CT.asym_power_sensor],
        [CT.sym_current_sensor, CT.asym_current_sensor],
    ):
        assert (
            MixedPowerCurrentSensorError(
                [
                    (power_sensor_type, AT.measured_object),
                    (power_sensor_type, AT.measured_terminal_type),
                    (current_sensor_type, AT.measured_object),
                    (current_sensor_type, AT.measured_terminal_type),
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

    assert NotGreaterOrEqualError(CT.transformer, AT.uk_max, [15], AT.uk_min) not in validation_errors

    assert NotBooleanError(CT.fault, AT.status, [32, 33]) in validation_errors
    assert InvalidIdError(CT.fault, AT.fault_object, [1, *list(range(32, 42))], [CT.node]) in validation_errors


def test_validate_three_winding_transformer(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert validation_errors is not None
    assert NotBooleanError(CT.three_winding_transformer, AT.status_1, [28]) in validation_errors
    assert NotBooleanError(CT.three_winding_transformer, AT.status_2, [1]) in validation_errors
    assert NotBooleanError(CT.three_winding_transformer, AT.status_3, [29]) in validation_errors
    assert InvalidIdError(CT.three_winding_transformer, AT.node_1, [29], CT.node) in validation_errors
    assert InvalidIdError(CT.three_winding_transformer, AT.node_2, [28], CT.node) in validation_errors
    assert InvalidIdError(CT.three_winding_transformer, AT.node_3, [29], CT.node) in validation_errors
    assert NotGreaterThanError(CT.three_winding_transformer, AT.u1, [1, 28], 0) in validation_errors
    assert NotGreaterThanError(CT.three_winding_transformer, AT.u2, [1, 28], 0) in validation_errors
    assert NotGreaterThanError(CT.three_winding_transformer, AT.u3, [1, 28], 0) in validation_errors
    assert NotGreaterThanError(CT.three_winding_transformer, AT.sn_1, [1, 28], 0) in validation_errors
    assert NotGreaterThanError(CT.three_winding_transformer, AT.sn_2, [1, 28], 0) in validation_errors
    assert NotGreaterThanError(CT.three_winding_transformer, AT.sn_3, [1, 28], 0) in validation_errors
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_12,
            [29, 30],
            f"{AT.pk_12}/{AT.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_12,
            [1, 30],
            f"{AT.pk_12}/{AT.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_13,
            [29],
            f"{AT.pk_13}/{AT.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_13,
            [30],
            f"{AT.pk_13}/{AT.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_23,
            [1, 29],
            f"{AT.pk_23}/{AT.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_23,
            [30],
            f"{AT.pk_23}/{AT.sn_3}",
        )
        in validation_errors
    )
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_12, [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_13, [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_23, [1, 28], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_12, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_13, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_23, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.i0, [29], f"{AT.p0}/{AT.sn_1}") in validation_errors
    assert NotLessThanError(CT.three_winding_transformer, AT.i0, [28], 1) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.p0, [1], 0) in validation_errors
    assert NotBetweenOrAtError(CT.three_winding_transformer, AT.clock_13, [1, 28], (-12, 12)) in validation_errors
    assert (
        NotBetweenOrAtError(
            CT.three_winding_transformer,
            AT.tap_pos,
            [1, 28],
            (AT.tap_min, AT.tap_max),
        )
        in validation_errors
    )
    assert (
        NotBetweenOrAtError(
            CT.three_winding_transformer,
            AT.tap_nom,
            [1, 28],
            (AT.tap_min, AT.tap_max),
        )
        in validation_errors
    )
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.tap_size, [1], 0) in validation_errors
    assert InvalidEnumValueError(CT.three_winding_transformer, AT.winding_1, [1, 28], WindingType) in validation_errors
    assert InvalidEnumValueError(CT.three_winding_transformer, AT.winding_2, [1, 28], WindingType) in validation_errors
    assert InvalidEnumValueError(CT.three_winding_transformer, AT.winding_3, [1, 28], WindingType) in validation_errors
    assert InvalidEnumValueError(CT.three_winding_transformer, AT.tap_side, [1, 28], Branch3Side) in validation_errors


def test_validate_three_winding_transformer_ukpkminmax(input_data):
    validation_errors = validate_input_data(input_data, symmetric=False)
    assert validation_errors is not None
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_12_min,
            [29, 30],
            f"{AT.pk_12_min}/{AT.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_12_min,
            [1, 30],
            f"{AT.pk_12_min}/{AT.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_13_min,
            [29],
            f"{AT.pk_13_min}/{AT.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_13_min,
            [30],
            f"{AT.pk_13_min}/{AT.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_23_min,
            [1, 29],
            f"{AT.pk_23_min}/{AT.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_23_min,
            [30],
            f"{AT.pk_23_min}/{AT.sn_3}",
        )
        in validation_errors
    )
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_12_min, [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_13_min, [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_23_min, [1, 28], (0, 1)) in validation_errors
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_12_max,
            [29, 30],
            f"{AT.pk_12_max}/{AT.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_12_max,
            [1, 30],
            f"{AT.pk_12_max}/{AT.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_13_max,
            [29],
            f"{AT.pk_13_max}/{AT.sn_1}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_13_max,
            [30],
            f"{AT.pk_13_max}/{AT.sn_3}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_23_max,
            [1, 29],
            f"{AT.pk_23_max}/{AT.sn_2}",
        )
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(
            CT.three_winding_transformer,
            AT.uk_23_max,
            [30],
            f"{AT.pk_23_max}/{AT.sn_3}",
        )
        in validation_errors
    )
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_12_max, [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_13_max, [1, 28], (0, 1)) in validation_errors
    assert NotBetweenError(CT.three_winding_transformer, AT.uk_23_max, [1, 28], (0, 1)) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_12_min, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_13_min, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_23_min, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_12_max, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_13_max, [1], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.three_winding_transformer, AT.pk_23_max, [1], 0) in validation_errors


def test_validate_input_data_transformer_tap_regulator(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.power_flow)
    assert validation_errors is not None
    assert NotBooleanError(CT.transformer_tap_regulator, AT.status, [52, 1, 53]) in validation_errors
    assert (
        InvalidIdError(
            CT.transformer_tap_regulator,
            AT.regulated_object,
            [1],
            [CT.transformer, CT.three_winding_transformer],
        )
        in validation_errors
    )
    assert (
        InvalidEnumValueError(CT.transformer_tap_regulator, AT.control_side, [1], [BranchSide, Branch3Side])
        in validation_errors
    )
    assert (
        InvalidAssociatedEnumValueError(
            CT.transformer_tap_regulator,
            [AT.control_side, AT.regulated_object],
            [52],
            [BranchSide],
        )
        in validation_errors
    )
    assert NotGreaterOrEqualError(CT.transformer_tap_regulator, AT.u_set, [52], 0.0) in validation_errors
    assert NotGreaterThanError(CT.transformer_tap_regulator, AT.u_band, [52, 1], 0.0) in validation_errors
    assert (
        NotGreaterOrEqualError(CT.transformer_tap_regulator, AT.line_drop_compensation_r, [52], 0.0)
        in validation_errors
    )
    assert (
        NotGreaterOrEqualError(CT.transformer_tap_regulator, AT.line_drop_compensation_x, [1], 0.0) in validation_errors
    )
    assert NotUniqueError(CT.transformer_tap_regulator, AT.regulated_object, [51, 54]) in validation_errors


def test_validate_input_data_voltage_regulator(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.power_flow)
    assert validation_errors is not None

    assert NotBooleanError(CT.voltage_regulator, AT.status, [62, 65]) in validation_errors
    assert NotUniqueError(CT.voltage_regulator, AT.regulated_object, [60, 67]) in validation_errors
    assert NotGreaterThanError(CT.voltage_regulator, AT.u_ref, [66], 0.0) in validation_errors
    assert InfinityError(CT.voltage_regulator, AT.u_ref, [65]) in validation_errors
    assert (
        InvalidIdError(
            CT.voltage_regulator,
            AT.regulated_object,
            [64, 65, 66],
            [CT.sym_gen, CT.asym_gen, CT.sym_load, CT.asym_load],
        )
        in validation_errors
    )
    assert InvalidVoltageRegulationError(CT.voltage_regulator, AT.u_ref, [60, 67]) in validation_errors


def test_fault(input_data):
    validation_errors = validate_input_data(input_data, calculation_type=CalculationType.short_circuit)
    assert validation_errors is not None
    assert InvalidEnumValueError(CT.fault, AT.fault_type, [50], FaultType) in validation_errors
    assert InvalidEnumValueError(CT.fault, AT.fault_phase, [50], FaultPhase) in validation_errors
    assert FaultPhaseError(CT.fault, [AT.fault_type, AT.fault_phase], [1, *list(range(32, 51))])
    assert NotGreaterOrEqualError(CT.fault, AT.r_f, [1], 0) in validation_errors
    assert (
        NotIdenticalError(
            CT.fault,
            AT.fault_type,
            list(range(32, 51)),
            5 * [0] + 4 * [1] + 4 * [2] + 4 * [3] + [_nan_type(CT.fault, AT.fault_type), 4],
        )
        in validation_errors
    )
    assert (
        NotIdenticalError(
            CT.fault,
            AT.fault_phase,
            list(range(32, 51)),
            list(range(2, 7)) + [0, 4, 5, 6] + 2 * list(range(4)) + [_nan_type(CT.fault, AT.fault_phase), 7],
        )
        in validation_errors
    )


def test_validate_input_data_asym_calculation(input_data):
    validation_errors = validate_input_data(input_data, symmetric=False)
    assert validation_errors is not None
    assert NotGreaterThanError(CT.node, AT.u_rated, [1], 0) in validation_errors
    assert NotUniqueError(CT.node, AT.id, [2, 2]) in validation_errors
    assert NotBooleanError(CT.line, AT.from_status, [5]) in validation_errors
    assert NotBooleanError(CT.line, AT.to_status, [4]) in validation_errors
    assert InvalidIdError(CT.line, AT.from_node, [4], CT.node) in validation_errors
    assert InvalidIdError(CT.line, AT.to_node, [5], CT.node) in validation_errors


def test_validate_input_data_invalid_structure():
    with pytest.raises(TypeError, match=r"should be a Numpy structured array"):
        validate_input_data({CT.node: np.array([[1, 10500.0], [2, 10500.0]])}, symmetric=True)


def test_generic_branch_input_data(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert validation_errors is not None
    assert NotGreaterThanError(CT.generic_branch, AT.k, [6], 0) in validation_errors
    assert NotGreaterOrEqualError(CT.generic_branch, AT.sn, [6], 0) in validation_errors


def test_asym_line_input_data(input_data):
    validation_errors = validate_input_data(input_data, symmetric=True)
    assert validation_errors is not None
    assert NotGreaterThanError(CT.asym_line, AT.r_aa, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_ba, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_bb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_ca, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_cb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_cc, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_na, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_nb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_nc, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.r_nn, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_aa, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_ba, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_bb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_ca, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_cb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_cc, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_na, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_nb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_nc, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.x_nn, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c_aa, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c_ba, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c_bb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c_ca, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c_cb, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c_cc, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c0, [55], 0) in validation_errors
    assert NotGreaterThanError(CT.asym_line, AT.c1, [55], 0) in validation_errors
    assert (
        MultiFieldValidationError(
            CT.asym_line,
            [
                AT.r_na,
                AT.r_nb,
                AT.r_nc,
                AT.r_nn,
                AT.x_na,
                AT.x_nb,
                AT.x_nc,
                AT.x_nn,
            ],
            [56, 57],
        )
        in validation_errors
    )
    assert (
        MultiFieldValidationError(
            CT.asym_line,
            [
                AT.c_aa,
                AT.c_ba,
                AT.c_bb,
                AT.c_ca,
                AT.c_cb,
                AT.c_cc,
                AT.c0,
                AT.c1,
            ],
            [56],
        )
        in validation_errors
    )
    assert (
        MultiFieldValidationError(
            CT.asym_line,
            [
                AT.c_aa,
                AT.c_ba,
                AT.c_bb,
                AT.c_ca,
                AT.c_cb,
                AT.c_cc,
            ],
            [58],
        )
        in validation_errors
    )


@pytest.mark.parametrize("component_type", CT)
def test_validate_input_data__no_components(component_type: CT):
    input_data = {component_type: initialize_array(DatasetType.input, component_type, 0)}
    assert validate_input_data(input_data) is None
    assert_valid_input_data(input_data)
