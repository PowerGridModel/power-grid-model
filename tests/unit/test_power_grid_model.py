# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from copy import copy, deepcopy

import numpy as np
import pytest

from power_grid_model import (
    AngleMeasurementType,
    AttributeType as AT,
    BranchSide,
    ComponentAttributeFilterOptions,
    ComponentType as CT,
    DatasetType as DT,
    LoadGenType,
    MeasuredTerminalType,
    PowerGridModel,
    initialize_array,
)
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset
from power_grid_model.data_types import BatchDataset
from power_grid_model.errors import InvalidCalculationMethod, IterationDiverge, PowerGridBatchError, PowerGridError
from power_grid_model.utils import get_dataset_scenario
from power_grid_model.validation import assert_valid_input_data

from .utils import compare_result

"""
Testing network

source_1(1.0 p.u., 100.0 V) --internal_impedance(j10.0 ohm, sk=1000.0 VA, rx_ratio=0.0)--
-- node_0 (100.0 V) --load_2(const_i, -j5.0A, 0.0 W, 500.0 var)

u0 = 100.0 V - (j10.0 ohm * -j5.0 A) = 50.0 V

update_0:
    u_ref = 0.5 p.u. (50.0 V)
    q_specified = 100 var (-j1.0A)
u0 = 50.0 V - (j10.0 ohm * -j1.0 A) = 40.0 V

update_1:
    q_specified = 300 var (-j3.0A)
u0 = 100.0 V - (j10.0 ohm * -j3.0 A) = 70.0 V
"""


@pytest.fixture
def input_row():
    node = initialize_array(DT.input, CT.node, 1)
    node[AT.id] = 0
    node[AT.u_rated] = 100.0

    source = initialize_array(DT.input, CT.source, 1)
    source[AT.id] = 1
    source[AT.node] = 0
    source[AT.status] = 1
    source[AT.u_ref] = 1.0
    source[AT.sk] = 1000.0
    source[AT.rx_ratio] = 0.0

    sym_load = initialize_array(DT.input, CT.sym_load, 1)
    sym_load[AT.id] = 2
    sym_load[AT.node] = 0
    sym_load[AT.status] = 1
    sym_load[AT.type] = 2
    sym_load[AT.p_specified] = 0.0
    sym_load[AT.q_specified] = 500.0

    return {
        CT.node: node,
        CT.source: source,
        CT.sym_load: sym_load,
    }


@pytest.fixture
def input_col(input_row):
    return compatibility_convert_row_columnar_dataset(input_row, ComponentAttributeFilterOptions.relevant, DT.input)


@pytest.fixture(params=["input_row", "input_col"])
def input(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def sym_output():
    node = initialize_array(DT.sym_output, CT.node, 1)
    node[AT.id] = 0
    node[AT.u] = 50.0
    node[AT.u_pu] = 0.5
    node[AT.u_angle] = 0.0

    return {CT.node: node}


@pytest.fixture
def update_batch_row():
    source = initialize_array(DT.update, CT.source, 1)
    source[AT.id] = 1
    source[AT.u_ref] = 0.5

    sym_load = initialize_array(DT.update, CT.sym_load, 2)
    sym_load[AT.id] = [2, 2]
    sym_load[AT.q_specified] = [100.0, 300.0]

    return {
        CT.source: {
            "data": source,
            "indptr": np.array([0, 1, 1]),
        },
        CT.sym_load: {
            "data": sym_load,
            "indptr": np.array([0, 1, 2]),
        },
    }


@pytest.fixture
def update_batch_col(update_batch_row):
    return compatibility_convert_row_columnar_dataset(
        update_batch_row, ComponentAttributeFilterOptions.relevant, DT.update
    )


@pytest.fixture(params=["update_batch_row", "update_batch_col"])
def update_batch(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def sym_output_batch():
    node = initialize_array(DT.sym_output, CT.node, (2, 1))
    node[AT.id] = [[0], [0]]
    node[AT.u] = [[40.0], [70.0]]
    node[AT.u_pu] = [[0.4], [0.7]]
    node[AT.u_angle] = [[0.0], [0.0]]

    return {
        CT.node: node,
    }


@pytest.fixture
def model(input):
    return PowerGridModel(input)


@pytest.fixture
def empty_model():
    return PowerGridModel({})


def test_simple_power_flow(model: PowerGridModel, sym_output):
    result = model.calculate_power_flow()
    compare_result(result, sym_output, rtol=0.0, atol=1e-8)


def test_simple_permanent_update(model: PowerGridModel, update_batch, sym_output_batch):
    model.update(update_data=get_dataset_scenario(update_batch, 0))  # single permanent model update
    result = model.calculate_power_flow()
    expected_result = get_dataset_scenario(sym_output_batch, 0)
    compare_result(result, expected_result, rtol=0.0, atol=1e-8)


def test_update_error(model: PowerGridModel):
    load_update = initialize_array(DT.update, CT.sym_load, 1)
    load_update[AT.id] = 5
    update_data = {CT.sym_load: load_update}
    with pytest.raises(PowerGridError, match="The id cannot be found:"):
        model.update(update_data=update_data)
    update_data_col = compatibility_convert_row_columnar_dataset(
        update_data, ComponentAttributeFilterOptions.relevant, DT.update
    )
    with pytest.raises(PowerGridError, match="The id cannot be found:"):
        model.update(update_data=update_data_col)


def test_copy_model(model: PowerGridModel, sym_output):
    model_2 = copy(model)
    result = model_2.calculate_power_flow()
    compare_result(result, sym_output, rtol=0.0, atol=1e-8)


def test_deepcopy_model(model: PowerGridModel, empty_model: PowerGridModel, sym_output, update_batch, sym_output_batch):
    # list containing different models twice
    model_list = [model, empty_model, model, empty_model]

    new_model_list = deepcopy(model_list)

    # check if identities are as expected
    assert id(new_model_list[0]) != id(model_list[0])
    assert id(new_model_list[1]) != id(model_list[1])
    assert id(new_model_list[0]) != id(new_model_list[1])
    assert id(new_model_list[0]) == id(new_model_list[2])
    assert id(new_model_list[1]) == id(new_model_list[3])

    # check if the deepcopied objects are really independent from the original ones
    # by modifying the copies and seeing if the original one is impacted by this change
    new_model_list[0].update(update_data=get_dataset_scenario(update_batch, 0))

    new_expected_result = get_dataset_scenario(sym_output_batch, 0)
    new_result_0 = new_model_list[0].calculate_power_flow()
    compare_result(new_result_0, new_expected_result, rtol=0.0, atol=1e-8)
    # at index 0 and 2 should be the same objects, check if changing the object at index 0
    # and obtaining a power flow result is ident to the result at index 2
    new_result_2 = new_model_list[2].calculate_power_flow()
    compare_result(new_result_2, new_expected_result, rtol=0.0, atol=1e-8)

    result = model.calculate_power_flow()
    compare_result(result, sym_output, rtol=0.0, atol=1e-8)


def test_repr_and_str(model: PowerGridModel, empty_model: PowerGridModel):
    repr_empty_model_expected = "PowerGridModel (0 components)\n"
    assert repr_empty_model_expected == repr(empty_model)
    assert repr_empty_model_expected == str(empty_model)

    repr_model_expected = "PowerGridModel (3 components)\n  - node: 1\n  - source: 1\n  - sym_load: 1\n"
    assert repr_model_expected == repr(model)
    assert repr_model_expected == str(model)


def test_get_indexer(model: PowerGridModel):
    ids = np.array([2, 2])
    expected_indexer = np.array([0, 0])
    indexer = model.get_indexer(CT.sym_load, ids)
    np.testing.assert_allclose(expected_indexer, indexer)


def test_batch_power_flow(model: PowerGridModel, update_batch: BatchDataset, sym_output_batch):
    result = model.calculate_power_flow(update_data=update_batch)
    compare_result(result, sym_output_batch, rtol=0.0, atol=1e-8)


def test_construction_error(input):
    input[CT.sym_load][AT.id][0] = 0
    with pytest.raises(PowerGridError, match="Conflicting id detected:"):
        PowerGridModel(input)


def test_single_calculation_error(model: PowerGridModel):
    with pytest.raises(IterationDiverge, match="Iteration failed to converge after"):
        model.calculate_power_flow(max_iterations=1, error_tolerance=1e-100)
    with pytest.raises(InvalidCalculationMethod, match="The calculation method is invalid for this calculation!"):
        model.calculate_state_estimation(calculation_method="iterative_current")

    for calculation_method in ("linear", "newton_raphson", "iterative_current", "linear_current", "iterative_linear"):
        with pytest.raises(InvalidCalculationMethod):
            model.calculate_short_circuit(calculation_method=calculation_method)


def test_batch_calculation_error(model: PowerGridModel, update_batch):
    # wrong id
    update_batch[CT.sym_load]["data"][AT.id][1] = 5
    # with error
    with pytest.raises(PowerGridBatchError) as e:
        model.calculate_power_flow(update_data=update_batch)
    error = e.value
    np.testing.assert_allclose(error.failed_scenarios, [1])
    np.testing.assert_allclose(error.succeeded_scenarios, [0])
    assert "The id cannot be found:" in error.error_messages[0]


def test_batch_calculation_error_continue(model: PowerGridModel, update_batch, sym_output_batch):
    # wrong id
    update_batch[CT.sym_load]["data"][AT.id][1] = 5
    result = model.calculate_power_flow(update_data=update_batch, continue_on_batch_error=True)
    # assert error
    error = model.batch_error
    assert error is not None
    np.testing.assert_allclose(error.failed_scenarios, [1])
    np.testing.assert_allclose(error.succeeded_scenarios, [0])
    assert "The id cannot be found:" in error.error_messages[0]
    # assert value result for scenario 0
    result = {CT.node: result[CT.node][error.succeeded_scenarios, :]}
    expected_result = {CT.node: sym_output_batch[CT.node][error.succeeded_scenarios, :]}
    compare_result(result, expected_result, rtol=0.0, atol=1e-8)
    # general error before the batch
    with pytest.raises(PowerGridError, match="The calculation method is invalid for this calculation!"):
        model.calculate_state_estimation(
            calculation_method="iterative_current",
            update_data={CT.source: initialize_array(DT.update, CT.source, shape=(5, 0))},
            continue_on_batch_error=True,
        )


def test_empty_input():
    node = initialize_array(DT.input, CT.node, 0)
    line = initialize_array(DT.input, CT.line, 0)
    sym_load = initialize_array(DT.input, CT.sym_load, 0)
    source = initialize_array(DT.input, CT.source, 0)

    input_data = {
        CT.node: node,
        CT.line: line,
        CT.sym_load: sym_load,
        CT.source: source,
    }

    assert_valid_input_data(input_data)
    model = PowerGridModel(input_data, system_frequency=50.0)

    result = model.calculate_power_flow()

    assert result == {}


@pytest.fixture
def input_sym_load_col(input_row):
    return compatibility_convert_row_columnar_dataset(
        input_row,
        {
            CT.node: None,
            CT.source: None,
            CT.sym_load: ComponentAttributeFilterOptions.relevant,
        },
        DT.input,
    )


@pytest.fixture(params=[pytest.param("input_row", id="input_row"), pytest.param("input_sym_load_col", id="input_col")])
def minimal_input(request):
    return request.getfixturevalue(request.param)


def update_sym_load_row():
    sym_load = initialize_array(DT.update, CT.sym_load, (2, 1))
    sym_load[AT.id] = [[2], [2]]
    sym_load[AT.q_specified] = [[100.0], [300.0]]
    return {CT.sym_load: sym_load}


def update_sym_load_row_optional_id():
    sym_load = initialize_array(DT.update, CT.sym_load, (2, 1))
    sym_load[AT.q_specified] = [[100.0], [300.0]]
    return {CT.sym_load: sym_load}


def update_sym_load_row_invalid_id():
    sym_load = initialize_array(DT.update, CT.sym_load, (2, 1))
    sym_load[AT.id] = [[2], [5]]
    sym_load[AT.q_specified] = [[100.0], [300.0]]
    return {CT.sym_load: sym_load}


def update_sym_load_col(update_sym_load_row):
    return compatibility_convert_row_columnar_dataset(
        update_sym_load_row, ComponentAttributeFilterOptions.relevant, DT.update
    )


def update_sym_load_sparse(update_data):
    return {
        CT.sym_load: {
            "data": update_data[CT.sym_load].reshape(-1),
            "indptr": np.array([0, 1, 2]),
        },
    }


@pytest.mark.parametrize(
    "minimal_update",
    [
        pytest.param(update_sym_load_row(), id="update_dense_row"),
        pytest.param(update_sym_load_col(update_sym_load_row()), id="update_dense_col"),
        pytest.param(update_sym_load_sparse(update_sym_load_row()), id="update_sparse_row"),
        pytest.param(update_sym_load_col(update_sym_load_sparse(update_sym_load_row())), id="update_sparse_col"),
    ],
)
def test_update_ids_batch(minimal_update, minimal_input):
    output_data = PowerGridModel(minimal_input).calculate_power_flow(update_data=minimal_update)
    np.testing.assert_almost_equal(output_data[CT.node][AT.u], np.array([[90.0], [70.0]]))


@pytest.mark.parametrize(
    "minimal_update",
    [
        pytest.param(update_sym_load_row_optional_id(), id="update_dense_row"),
        pytest.param(update_sym_load_col(update_sym_load_row_optional_id()), id="update_dense_col"),
        pytest.param(update_sym_load_sparse(update_sym_load_row_optional_id()), id="update_sparse_row"),
        pytest.param(
            update_sym_load_col(update_sym_load_sparse(update_sym_load_row_optional_id())), id="update_sparse_col"
        ),
    ],
)
def test_update_id_optional(minimal_update, minimal_input):
    output_data = PowerGridModel(minimal_input).calculate_power_flow(update_data=minimal_update)
    np.testing.assert_almost_equal(output_data[CT.node][AT.u], np.array([[90.0], [70.0]]))


def test_update_id_mixed(minimal_input):
    update_sym_load_no_id = initialize_array(DT.update, CT.sym_load, (3, 1))
    update_sym_load_no_id[AT.p_specified] = [[30e6], [15e5], [0]]

    update_source_indptr = np.array([0, 1, 1, 2])
    update_source = initialize_array(DT.update, CT.source, 2)
    update_source[AT.id] = 1
    update_source[AT.status] = 0

    update_batch = {
        CT.sym_load: update_sym_load_no_id,
        CT.source: {"indptr": update_source_indptr, "data": update_source},
    }

    _output_data = PowerGridModel(minimal_input).calculate_power_flow(update_data=update_batch)


@pytest.mark.parametrize(
    "minimal_update",
    [
        update_sym_load_row_invalid_id(),
        update_sym_load_col(update_sym_load_row_invalid_id()),
        update_sym_load_sparse(update_sym_load_row_invalid_id()),
        update_sym_load_col(update_sym_load_sparse(update_sym_load_row_invalid_id())),
    ],
)
def test_update_id_error(minimal_update, minimal_input):
    with pytest.raises(PowerGridBatchError) as e:
        PowerGridModel(minimal_input).calculate_power_flow(update_data=minimal_update)
    assert e.value.failed_scenarios == [1]
    assert "The id cannot be found: 5" in e.value.error_messages[0]


@pytest.fixture
def input_data__irrelevant_components_test():
    node = initialize_array(DT.input, CT.node, 2)
    node[AT.id] = np.array([1, 2])
    node[AT.u_rated] = [10000, 400]

    transformer = initialize_array(DT.input, CT.transformer, 1)
    transformer[AT.id] = [3]
    transformer[AT.from_node] = [1]
    transformer[AT.to_node] = [2]
    transformer[AT.from_status] = [1]
    transformer[AT.to_status] = [1]
    transformer[AT.u1] = [10000]
    transformer[AT.u2] = [400]
    transformer[AT.sn] = [100000]
    transformer[AT.uk] = [0.1]
    transformer[AT.pk] = [1000]
    transformer[AT.i0] = [1.0e-6]
    transformer[AT.p0] = [0.1]
    transformer[AT.winding_from] = [2]
    transformer[AT.winding_to] = [1]
    transformer[AT.clock] = [5]
    transformer[AT.tap_side] = [0]
    transformer[AT.tap_pos] = [3]
    transformer[AT.tap_min] = [-11]
    transformer[AT.tap_max] = [9]
    transformer[AT.tap_size] = [100]

    sym_load = initialize_array(DT.input, CT.sym_load, 1)
    sym_load[AT.id] = [4]
    sym_load[AT.node] = [2]
    sym_load[AT.status] = [1]
    sym_load[AT.type] = [LoadGenType.const_power]
    sym_load[AT.p_specified] = [1000.0]
    sym_load[AT.q_specified] = [200.0]

    source = initialize_array(DT.input, CT.source, 1)
    source[AT.id] = [5]
    source[AT.node] = [1]
    source[AT.status] = [1]
    source[AT.u_ref] = [1.0]

    sym_current_sensor = initialize_array(DT.input, CT.sym_current_sensor, 1)
    sym_current_sensor[AT.id] = [6]
    sym_current_sensor[AT.measured_object] = [3]
    sym_current_sensor[AT.measured_terminal_type] = [MeasuredTerminalType.branch_to]
    sym_current_sensor[AT.angle_measurement_type] = [AngleMeasurementType.local_angle]
    sym_current_sensor[AT.i_sigma] = [100]
    sym_current_sensor[AT.i_angle_sigma] = [0.1]
    sym_current_sensor[AT.i_measured] = [1000.0]
    sym_current_sensor[AT.i_angle_measured] = [0.2]

    sym_voltage_sensor = initialize_array(DT.input, CT.sym_voltage_sensor, 1)
    sym_voltage_sensor[AT.id] = [7]
    sym_voltage_sensor[AT.measured_object] = [1]
    sym_voltage_sensor[AT.u_sigma] = [1.0]
    sym_voltage_sensor[AT.u_measured] = [10000.0]

    return {
        CT.node: node,
        CT.transformer: transformer,
        CT.sym_load: sym_load,
        CT.source: source,
        CT.sym_current_sensor: sym_current_sensor,
        CT.sym_voltage_sensor: sym_voltage_sensor,
    }


@pytest.fixture
def input_data__voltage_regulator():
    voltage_regulator = initialize_array(DT.input, CT.voltage_regulator, 1)
    voltage_regulator[AT.id] = [8]
    voltage_regulator[AT.regulated_object] = [4]
    voltage_regulator[AT.status] = [1]
    voltage_regulator[AT.u_ref] = [1.05]
    return {CT.voltage_regulator: voltage_regulator}


@pytest.fixture
def input_data__transformer_tap_regulator():
    transformer_tap_regulator = initialize_array(DT.input, CT.transformer_tap_regulator, 1)
    transformer_tap_regulator[AT.id] = [8]
    transformer_tap_regulator[AT.regulated_object] = [3]
    transformer_tap_regulator[AT.status] = [1]
    transformer_tap_regulator[AT.control_side] = [BranchSide.to_side]
    transformer_tap_regulator[AT.u_set] = [400.0]
    transformer_tap_regulator[AT.u_band] = [20.0]
    return {CT.transformer_tap_regulator: transformer_tap_regulator}


@pytest.mark.parametrize("regulator_input", ["input_data__voltage_regulator", "input_data__transformer_tap_regulator"])
def test_irrelevant_components__power_flow(input_data__irrelevant_components_test, regulator_input, request):
    regulator = request.getfixturevalue(regulator_input)
    input_data = {**input_data__irrelevant_components_test, **regulator}
    model = PowerGridModel(input_data)
    result = model.calculate_power_flow()

    assert CT.transformer in result
    assert CT.sym_load in result
    assert CT.source in result
    assert CT.node in result
    assert CT.sym_voltage_sensor not in result
    assert CT.sym_current_sensor not in result
    if CT.voltage_regulator in regulator:
        assert CT.voltage_regulator in result
    else:
        assert CT.transformer_tap_regulator in result


@pytest.mark.parametrize("regulator_input", ["input_data__voltage_regulator", "input_data__transformer_tap_regulator"])
def test_irrelevant_components__state_estimation(input_data__irrelevant_components_test, regulator_input, request):
    regulator = request.getfixturevalue(regulator_input)
    input_data = {**input_data__irrelevant_components_test, **regulator}
    model = PowerGridModel(input_data)
    result = model.calculate_state_estimation()

    assert CT.transformer in result
    assert CT.sym_load in result
    assert CT.source in result
    assert CT.node in result
    assert CT.sym_voltage_sensor in result
    assert CT.sym_current_sensor in result
    assert CT.voltage_regulator not in result
    assert CT.transformer_tap_regulator not in result


@pytest.mark.parametrize("regulator_input", ["input_data__voltage_regulator", "input_data__transformer_tap_regulator"])
def test_irrelevant_components__short_circuit(input_data__irrelevant_components_test, regulator_input, request):
    regulator = request.getfixturevalue(regulator_input)
    input_data = {**input_data__irrelevant_components_test, **regulator}
    model = PowerGridModel(input_data)
    result = model.calculate_short_circuit()

    assert CT.transformer in result
    assert CT.sym_load in result
    assert CT.source in result
    assert CT.node in result
    assert CT.voltage_regulator not in result
    assert CT.transformer_tap_regulator not in result
    assert CT.transformer_tap_regulator not in result
    assert CT.sym_voltage_sensor not in result
    assert CT.sym_current_sensor not in result


def test_calculation_termination():
    node = initialize_array(DT.input, CT.node, 1)
    node[AT.id] = 0
    node[AT.u_rated] = 100.0

    source = initialize_array(DT.input, CT.source, 1)
    source[AT.id] = 1
    source[AT.node] = 0
    source[AT.status] = 1
    source[AT.u_ref] = 1.0
    source[AT.sk] = 1000.0
    source[AT.rx_ratio] = 0.0

    sym_load = initialize_array(DT.input, CT.sym_load, 1)
    sym_load[AT.id] = 2
    sym_load[AT.node] = 0
    sym_load[AT.status] = 1
    sym_load[AT.type] = 2
    sym_load[AT.p_specified] = 0.0
    sym_load[AT.q_specified] = 500.0

    input_data = {
        CT.node: node,
        CT.source: source,
        CT.sym_load: sym_load,
    }

    source = initialize_array(DT.update, CT.source, (10000_000, 1))
    source[AT.id] = 2
    source[AT.u_ref] = 1
    source[AT.u_ref][0] = 1.1

    gigantic_update_data = {CT.source: source}

    model = PowerGridModel(input_data)
    result = model.calculate_power_flow(update_data=gigantic_update_data)
