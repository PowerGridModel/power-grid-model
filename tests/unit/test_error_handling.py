# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from copy import copy

import pytest

from power_grid_model import PowerGridModel
from power_grid_model._core.dataset_definitions import AttributeType, ComponentType, DatasetType
from power_grid_model._core.power_grid_meta import initialize_array
from power_grid_model.enum import (
    AngleMeasurementType,
    CalculationMethod,
    LoadGenType,
    MeasuredTerminalType,
    TapChangingStrategy,
)
from power_grid_model.errors import (
    AutomaticTapInputError,
    ConflictID,
    ConflictingAngleMeasurementType,
    ConflictVoltage,
    IDNotFound,
    IDWrongType,
    InvalidBranch,
    InvalidBranch3,
    InvalidCalculationMethod,
    InvalidMeasuredObject,
    InvalidRegulatedObject,
    InvalidTransformerClock,
    IterationDiverge,
    MissingCaseForEnumError,
    NotObservableError,
)

from .utils import PowerGridModelWithExt


def test_empty_model():
    model = PowerGridModel.__new__(PowerGridModel)
    with pytest.raises(TypeError):
        model.calculate_power_flow()
    with pytest.raises(TypeError):
        model.copy()
    with pytest.raises(TypeError):
        _n = model.all_component_count
    with pytest.raises(TypeError):
        copy(model)


def test_unknown_component_types():
    model = PowerGridModel(input_data={})
    with pytest.raises(KeyError, match=r"fake_type"):
        model.calculate_power_flow(output_component_types={"fake_type"})


def test_handle_conflict_voltage_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [10.0e3, 20.0e3]

    line_input = initialize_array(DatasetType.input, ComponentType.line, 1)
    line_input[AttributeType.id] = [2]
    line_input[AttributeType.from_node] = [0]
    line_input[AttributeType.to_node] = [1]

    with pytest.raises(ConflictVoltage):
        PowerGridModel(input_data={ComponentType.node: node_input, ComponentType.line: line_input})


def test_handle_missing_case_for_enum_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [1e4, 4e2]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [2]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [10.0e3]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input[AttributeType.id] = [3]
    transformer_input[AttributeType.from_node] = [0]
    transformer_input[AttributeType.to_node] = [1]
    transformer_input[AttributeType.from_status] = [1]
    transformer_input[AttributeType.to_status] = [1]
    transformer_input[AttributeType.winding_from] = [2]
    transformer_input[AttributeType.winding_to] = [1]
    transformer_input[AttributeType.clock] = [5]
    transformer_input[AttributeType.tap_side] = [0]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input[AttributeType.id] = [4]
    transformer_tap_regulator_input[AttributeType.regulated_object] = [3]
    transformer_tap_regulator_input[AttributeType.status] = [1]
    transformer_tap_regulator_input[AttributeType.control_side] = [127]

    with pytest.raises(MissingCaseForEnumError):
        PowerGridModelWithExt(
            input_data={
                ComponentType.node: node_input,
                ComponentType.transformer: transformer_input,
                ComponentType.source: source_input,
                ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
            }
        )


def test_handle_invalid_branch_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 1)
    node_input[AttributeType.id] = [0]
    node_input[AttributeType.u_rated] = [0.0]

    line_input = initialize_array(DatasetType.input, ComponentType.line, 1)
    line_input[AttributeType.id] = [1]
    line_input[AttributeType.from_node] = [0]
    line_input[AttributeType.to_node] = [0]

    with pytest.raises(InvalidBranch):
        PowerGridModel(input_data={ComponentType.node: node_input, ComponentType.line: line_input})


def test_handle_invalid_branch3_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 1)
    node_input[AttributeType.id] = [0]
    node_input[AttributeType.u_rated] = [0.0]

    three_winding_transformer_input = initialize_array(DatasetType.input, ComponentType.three_winding_transformer, 1)
    three_winding_transformer_input[AttributeType.id] = [1]
    three_winding_transformer_input[AttributeType.node_1] = [0]
    three_winding_transformer_input[AttributeType.node_2] = [0]
    three_winding_transformer_input[AttributeType.node_3] = [0]

    with pytest.raises(InvalidBranch3):
        PowerGridModel(
            input_data={
                ComponentType.node: node_input,
                ComponentType.three_winding_transformer: three_winding_transformer_input,
            }
        )


def test_handle_invalid_transformer_clock_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [0.0, 0.0]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input[AttributeType.id] = [2]
    transformer_input[AttributeType.from_node] = [0]
    transformer_input[AttributeType.to_node] = [1]
    transformer_input[AttributeType.clock] = [-1]

    with pytest.raises(InvalidTransformerClock):
        PowerGridModel(input_data={ComponentType.node: node_input, ComponentType.transformer: transformer_input})


@pytest.mark.skip(reason="TODO")
def test_handle_sparse_matrix_error():
    pass


def test_handle_not_observable_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 1)
    node_input[AttributeType.id] = [0]
    node_input[AttributeType.u_rated] = [10.0e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [1]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]

    sym_load_input = initialize_array(DatasetType.input, ComponentType.sym_load, 1)
    sym_load_input[AttributeType.id] = [2]
    sym_load_input[AttributeType.node] = [0]
    sym_load_input[AttributeType.status] = [1]
    sym_load_input[AttributeType.type] = [LoadGenType.const_power]

    model = PowerGridModel(
        input_data={
            ComponentType.node: node_input,
            ComponentType.source: source_input,
            ComponentType.sym_load: sym_load_input,
        }
    )
    with pytest.raises(NotObservableError):
        model.calculate_state_estimation(calculation_method=CalculationMethod.iterative_linear)


def test_handle_iteration_diverge_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 1)
    node_input[AttributeType.id] = [0]
    node_input[AttributeType.u_rated] = [100.0]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [1]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [1.0]
    source_input[AttributeType.sk] = [1000.0]
    source_input[AttributeType.rx_ratio] = [0.0]

    sym_load_input = initialize_array(DatasetType.input, ComponentType.sym_load, 1)
    sym_load_input[AttributeType.id] = [2]
    sym_load_input[AttributeType.node] = [0]
    sym_load_input[AttributeType.status] = [1]
    sym_load_input[AttributeType.type] = [LoadGenType.const_current]
    sym_load_input[AttributeType.p_specified] = [0.0]
    sym_load_input[AttributeType.q_specified] = [500.0]

    model = PowerGridModel(
        input_data={
            ComponentType.node: node_input,
            ComponentType.source: source_input,
            ComponentType.sym_load: sym_load_input,
        }
    )
    with pytest.raises(IterationDiverge):
        model.calculate_power_flow(max_iterations=1, error_tolerance=1.0e-100)


def test_handle_conflict_id_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 0]
    node_input[AttributeType.u_rated] = [0.0, 0.0]

    with pytest.raises(ConflictID):
        PowerGridModel(input_data={ComponentType.node: node_input})


def test_handle_id_not_found_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 1)
    node_input[AttributeType.id] = [0]
    node_input[AttributeType.u_rated] = [0.0]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [1]
    source_input[AttributeType.node] = [99]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [0.0]

    with pytest.raises(IDNotFound):
        PowerGridModel(input_data={ComponentType.node: node_input, ComponentType.source: source_input})


@pytest.mark.parametrize("sensor_type", [ComponentType.sym_power_sensor, ComponentType.sym_current_sensor])
def test_handle_invalid_measured_object_error(sensor_type):
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [0.0, 0.0]

    link_input = initialize_array(DatasetType.input, ComponentType.link, 1)
    link_input[AttributeType.id] = [2]
    link_input[AttributeType.from_node] = [0]
    link_input[AttributeType.to_node] = [1]

    sensor_input = initialize_array(DatasetType.input, sensor_type, 1)
    sensor_input[AttributeType.id] = [3]
    sensor_input[AttributeType.measured_object] = [2]
    sensor_input[AttributeType.measured_terminal_type] = [MeasuredTerminalType.branch_from]

    with pytest.raises(InvalidMeasuredObject):
        PowerGridModel(
            input_data={ComponentType.node: node_input, ComponentType.link: link_input, sensor_type: sensor_input}
        )


def test_handle_invalid_regulated_object_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [1e4, 4e2]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [2]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [10.0e3]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input[AttributeType.id] = [3]
    transformer_tap_regulator_input[AttributeType.regulated_object] = [2]

    with pytest.raises(InvalidRegulatedObject):
        PowerGridModel(
            input_data={
                ComponentType.node: node_input,
                ComponentType.source: source_input,
                ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
            }
        )


def test_handle_id_wrong_type_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 1)
    node_input[AttributeType.id] = [0]
    node_input[AttributeType.u_rated] = [0.0]

    sym_power_sensor_input = initialize_array(DatasetType.input, ComponentType.sym_power_sensor, 1)
    sym_power_sensor_input[AttributeType.id] = [1]
    sym_power_sensor_input[AttributeType.measured_object] = [0]
    sym_power_sensor_input[AttributeType.measured_terminal_type] = [MeasuredTerminalType.branch_from]

    with pytest.raises(IDWrongType):
        PowerGridModel(
            input_data={ComponentType.node: node_input, ComponentType.sym_power_sensor: sym_power_sensor_input}
        )


def test_handle_invalid_calculation_method_error():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 1)
    node_input[AttributeType.id] = [0]
    node_input[AttributeType.u_rated] = [10.0e3]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [1]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [10.0e3]

    sym_load_input = initialize_array(DatasetType.input, ComponentType.sym_load, 1)
    sym_load_input[AttributeType.id] = [2]
    sym_load_input[AttributeType.node] = [0]
    sym_load_input[AttributeType.status] = [1]
    sym_load_input[AttributeType.type] = [LoadGenType.const_power]
    sym_load_input[AttributeType.p_specified] = [10.0e3]
    sym_load_input[AttributeType.q_specified] = [1.0e3]

    model = PowerGridModel(
        input_data={
            ComponentType.node: node_input,
            ComponentType.source: source_input,
            ComponentType.sym_load: sym_load_input,
        }
    )
    with pytest.raises(InvalidCalculationMethod):
        model.calculate_power_flow(calculation_method=CalculationMethod.iec60909)


def test_transformer_tap_regulator_control_side_not_closer_to_source():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [150e3, 10e3]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input[AttributeType.id] = [2]
    transformer_input[AttributeType.from_node] = [0]
    transformer_input[AttributeType.to_node] = [1]
    transformer_input[AttributeType.from_status] = [1]
    transformer_input[AttributeType.to_status] = [1]
    transformer_input[AttributeType.u1] = [1e4]
    transformer_input[AttributeType.u2] = [4e2]
    transformer_input[AttributeType.sn] = [1e5]
    transformer_input[AttributeType.uk] = [0.1]
    transformer_input[AttributeType.pk] = [1e3]
    transformer_input[AttributeType.i0] = [1.0e-6]
    transformer_input[AttributeType.p0] = [0.1]
    transformer_input[AttributeType.winding_from] = [1]
    transformer_input[AttributeType.winding_to] = [1]
    transformer_input[AttributeType.clock] = [0]
    transformer_input[AttributeType.tap_side] = [1]
    transformer_input[AttributeType.tap_pos] = [0]
    transformer_input[AttributeType.tap_nom] = [0]
    transformer_input[AttributeType.tap_min] = [-1]
    transformer_input[AttributeType.tap_max] = [1]
    transformer_input[AttributeType.tap_size] = [100]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [3]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [1.0]

    transformer_tap_regulator_input = initialize_array(DatasetType.input, ComponentType.transformer_tap_regulator, 1)
    transformer_tap_regulator_input[AttributeType.id] = [4]
    transformer_tap_regulator_input[AttributeType.regulated_object] = [2]
    transformer_tap_regulator_input[AttributeType.status] = [1]
    transformer_tap_regulator_input[AttributeType.u_set] = [10]
    transformer_tap_regulator_input[AttributeType.u_band] = [200]
    transformer_tap_regulator_input[AttributeType.control_side] = [0]

    model = PowerGridModel(
        input_data={
            ComponentType.node: node_input,
            ComponentType.transformer: transformer_input,
            ComponentType.source: source_input,
            ComponentType.transformer_tap_regulator: transformer_tap_regulator_input,
        }
    )
    with pytest.raises(AutomaticTapInputError):
        model.calculate_power_flow(tap_changing_strategy=TapChangingStrategy.min_voltage_tap, decode_error=True)


def test_automatic_tap_changing():
    model = PowerGridModel(input_data={})
    model.calculate_power_flow(tap_changing_strategy=TapChangingStrategy.min_voltage_tap)


def test_conflicting_angle_measurement_type() -> None:
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [10e3, 10e3]

    line_input = initialize_array(DatasetType.input, ComponentType.line, 1)
    line_input[AttributeType.id] = [2]
    line_input[AttributeType.from_node] = [0]
    line_input[AttributeType.to_node] = [1]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [3]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [1.0]

    sym_voltage_sensor_input = initialize_array(DatasetType.input, ComponentType.sym_voltage_sensor, 1)
    sym_voltage_sensor_input[AttributeType.id] = [4]
    sym_voltage_sensor_input[AttributeType.measured_object] = [0]
    sym_voltage_sensor_input[AttributeType.u_sigma] = [1.0]
    sym_voltage_sensor_input[AttributeType.u_measured] = [1.0]
    sym_voltage_sensor_input[AttributeType.u_angle_measured] = [0.0]

    sym_current_sensor_input = initialize_array(DatasetType.input, ComponentType.sym_current_sensor, 2)
    sym_current_sensor_input[AttributeType.id] = [5, 6]
    sym_current_sensor_input[AttributeType.measured_object] = [2, 2]
    sym_current_sensor_input[AttributeType.measured_terminal_type] = [
        MeasuredTerminalType.branch_from,
        MeasuredTerminalType.branch_from,
    ]
    sym_current_sensor_input[AttributeType.angle_measurement_type] = [
        AngleMeasurementType.local_angle,
        AngleMeasurementType.global_angle,
    ]
    sym_current_sensor_input[AttributeType.i_sigma] = [1.0, 1.0]
    sym_current_sensor_input[AttributeType.i_angle_sigma] = [1.0, 1.0]
    sym_current_sensor_input[AttributeType.i_measured] = [0.0, 0.0]
    sym_current_sensor_input[AttributeType.i_angle_measured] = [0.0, 0.0]

    model = PowerGridModel(
        input_data={
            ComponentType.node: node_input,
            ComponentType.line: line_input,
            ComponentType.source: source_input,
            ComponentType.sym_voltage_sensor: sym_voltage_sensor_input,
            ComponentType.sym_current_sensor: sym_current_sensor_input,
        }
    )

    with pytest.raises(ConflictingAngleMeasurementType):
        model._calculate_state_estimation(decode_error=True)


def test_global_current_measurement_without_voltage_angle() -> None:
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [0, 1]
    node_input[AttributeType.u_rated] = [10e3, 10e3]

    line_input = initialize_array(DatasetType.input, ComponentType.line, 1)
    line_input[AttributeType.id] = [2]
    line_input[AttributeType.from_node] = [0]
    line_input[AttributeType.to_node] = [1]

    source_input = initialize_array(DatasetType.input, ComponentType.source, 1)
    source_input[AttributeType.id] = [3]
    source_input[AttributeType.node] = [0]
    source_input[AttributeType.status] = [1]
    source_input[AttributeType.u_ref] = [1.0]

    sym_voltage_sensor_input = initialize_array(DatasetType.input, ComponentType.sym_voltage_sensor, 1)
    sym_voltage_sensor_input[AttributeType.id] = [4]
    sym_voltage_sensor_input[AttributeType.measured_object] = [0]
    sym_voltage_sensor_input[AttributeType.u_sigma] = [1.0]
    sym_voltage_sensor_input[AttributeType.u_measured] = [0.0]

    sym_current_sensor_input = initialize_array(DatasetType.input, ComponentType.sym_current_sensor, 1)
    sym_current_sensor_input[AttributeType.id] = [5]
    sym_current_sensor_input[AttributeType.measured_object] = [2]
    sym_current_sensor_input[AttributeType.measured_terminal_type] = [
        MeasuredTerminalType.branch_from,
    ]
    sym_current_sensor_input[AttributeType.angle_measurement_type] = [
        AngleMeasurementType.global_angle,
    ]
    sym_current_sensor_input[AttributeType.i_sigma] = [1.0]
    sym_current_sensor_input[AttributeType.i_measured] = [0.0]

    model = PowerGridModel(
        input_data={
            ComponentType.node: node_input,
            ComponentType.line: line_input,
            ComponentType.source: source_input,
            ComponentType.sym_voltage_sensor: sym_voltage_sensor_input,
            ComponentType.sym_current_sensor: sym_current_sensor_input,
        }
    )

    with pytest.raises(NotObservableError):
        model._calculate_state_estimation(decode_error=True)


@pytest.mark.skip(reason="TODO")
def test_handle_power_grid_dataset_error():
    pass


@pytest.mark.skip(reason="TODO")
def test_handle_power_grid_unreachable_error():
    pass
