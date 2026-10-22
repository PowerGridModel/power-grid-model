# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import json

import numpy as np
import pytest

from power_grid_model import (
    AttributeType as AT,
    ComponentType as CT,
    DatasetType,
    PowerGridModel,
    StateOutputRequest,
    initialize_array,
)
from power_grid_model.errors import IDNotFound, PowerGridBatchError
from power_grid_model.utils import json_deserialize

input_data = {
    "version": "1.0",
    "type": DatasetType.input,
    "is_batch": False,
    "attributes": {},
    "data": {
        CT.sym_load: [
            {
                AT.id: 2,
                AT.node: 0,
                AT.status: 1,
                AT.type: 0,
                AT.p_specified: 0,
                AT.q_specified: 0,
            }
        ],
        CT.source: [
            {
                AT.id: 1,
                AT.node: 0,
                AT.status: 1,
                AT.u_ref: 1,
                AT.sk: 1e20,
            }
        ],
        CT.node: [{AT.id: 0, AT.u_rated: 10e3}],
    },
}

input_data_json = json.dumps(input_data)


def test_multi_dimensional_batch():
    input_dataset = json_deserialize(input_data_json)
    pgm = PowerGridModel(input_dataset)

    u_rated = 10e3
    u_ref = np.array([0.9, 1.0, 1.1], dtype=np.float64).reshape(-1, 1)
    p_specified = np.array([1e6, 2e6, 3e6, 4e6], dtype=np.float64).reshape(-1, 1)
    q_specified = np.array([0.1e6, 0.2e6, 0.3e6, 0.4e6, 0.5e6], dtype=np.float64).reshape(-1, 1)
    i_source_ref = np.abs(p_specified.reshape(1, -1, 1) + 1j * q_specified.reshape(1, 1, -1)) / (
        u_ref.reshape(-1, 1, 1) * u_rated * np.sqrt(3)
    )
    i_source_ref = i_source_ref.ravel()

    u_ref_batch = {CT.source: {AT.u_ref: u_ref}}
    p_specified_batch = {CT.sym_load: {AT.p_specified: p_specified}}
    q_specified_batch = {CT.sym_load: {AT.q_specified: q_specified}}

    result = pgm.calculate_power_flow(
        update_data=[u_ref_batch, p_specified_batch, q_specified_batch],
        output_component_types={CT.source: [AT.i]},
    )

    assert np.allclose(result[CT.source][AT.i].ravel(), i_source_ref)

    state_requests = [StateOutputRequest(y_bus=scenario_idx % 2 == 0) for scenario_idx in range(60)]
    _, model_states = pgm.calculate_power_flow(
        update_data=[u_ref_batch, p_specified_batch, q_specified_batch],
        output_component_types={CT.source: [AT.i]},
        get_model_state=state_requests,
    )

    assert isinstance(model_states, list)
    assert len(model_states) == len(state_requests)
    assert [state is not None for state in model_states] == [request.y_bus for request in state_requests]


def _load_batch(load_ids):
    return {CT.sym_load: {AT.id: np.array(load_ids, dtype=np.int32).reshape(-1, 1)}}


def _source_batch(source_ids):
    return {CT.source: {AT.id: np.array(source_ids, dtype=np.int32).reshape(-1, 1)}}


LOAD_ID = 2
SOURCE_ID = 1
# an update with an unknown id fails: in the outer dimension, it fails all scenarios of the inner dimensions
UNKNOWN_ID = 999


@pytest.mark.parametrize(
    ("outer_load_ids", "inner_source_ids", "expected_failed"),
    [
        pytest.param([UNKNOWN_ID, LOAD_ID], [SOURCE_ID, SOURCE_ID], [0, 1], id="first outer fails, last succeeds"),
        pytest.param([LOAD_ID, UNKNOWN_ID, LOAD_ID], [SOURCE_ID, SOURCE_ID], [2, 3], id="middle outer fails"),
        pytest.param([LOAD_ID, LOAD_ID], [UNKNOWN_ID, SOURCE_ID], [0, 2], id="inner fails in all outer scenarios"),
        pytest.param(
            [UNKNOWN_ID, LOAD_ID, UNKNOWN_ID, LOAD_ID],
            [SOURCE_ID, UNKNOWN_ID],
            [0, 1, 3, 4, 5, 7],
            id="failures in both dimensions",
        ),
        pytest.param([UNKNOWN_ID, UNKNOWN_ID], [UNKNOWN_ID] * 3, [0, 1, 2, 3, 4, 5], id="all scenarios fail"),
        pytest.param([LOAD_ID, LOAD_ID], [SOURCE_ID, SOURCE_ID], [], id="no scenario fails"),
    ],
)
def test_multi_dimensional_batch_reports_all_failed_scenarios(outer_load_ids, inner_source_ids, expected_failed):
    pgm = PowerGridModel(json_deserialize(input_data_json))
    total_size = len(outer_load_ids) * len(inner_source_ids)

    cartesian = [_load_batch(outer_load_ids), _source_batch(inner_source_ids)]
    flat = {
        CT.sym_load: {AT.id: np.repeat(outer_load_ids, len(inner_source_ids)).astype(np.int32).reshape(-1, 1)},
        CT.source: {AT.id: np.tile(inner_source_ids, len(outer_load_ids)).astype(np.int32).reshape(-1, 1)},
    }

    # a flat batch in which every scenario has an unknown id raises a regular error instead
    for update_data in (cartesian,) if len(expected_failed) == total_size else (cartesian, flat):
        pgm.calculate_power_flow(update_data=update_data, continue_on_batch_error=True)
        assert (pgm.batch_error.failed_scenarios.tolist() if pgm.batch_error else []) == expected_failed

    if expected_failed:
        with pytest.raises(PowerGridBatchError) as exc_info:
            pgm.calculate_power_flow(update_data=cartesian)
        assert exc_info.value.failed_scenarios.tolist() == expected_failed
        assert exc_info.value.succeeded_scenarios.tolist() == sorted(set(range(total_size)) - set(expected_failed))
        assert all(isinstance(error, IDNotFound) for error in exc_info.value.errors)


def test_multi_dimensional_batch_reports_failed_scenarios_of_three_dimensions():
    pgm = PowerGridModel(json_deserialize(input_data_json))
    # the first and the last outer scenario fail; the last scenarios of the other dimensions succeed
    update_data = [
        _load_batch([UNKNOWN_ID, LOAD_ID, UNKNOWN_ID]),
        _source_batch([SOURCE_ID, SOURCE_ID]),
        _source_batch([SOURCE_ID, SOURCE_ID]),
    ]

    pgm.calculate_power_flow(update_data=update_data, continue_on_batch_error=True)

    assert pgm.batch_error.failed_scenarios.tolist() == [0, 1, 2, 3, 8, 9, 10, 11]


def _diverging_load_model() -> PowerGridModel:
    """source -- node 1 -- line (1 + 1j ohm) -- node 2 -- load: a load of 50 MW has no power flow solution"""
    node = initialize_array(DatasetType.input, CT.node, 2)
    node[AT.id] = [1, 2]
    node[AT.u_rated] = 10e3

    line = initialize_array(DatasetType.input, CT.line, 1)
    line[AT.id] = 3
    line[AT.from_node] = 1
    line[AT.to_node] = 2
    line[AT.from_status] = 1
    line[AT.to_status] = 1
    line[AT.r1] = 1.0
    line[AT.x1] = 1.0
    line[AT.c1] = 0.0
    line[AT.tan1] = 0.0

    source = initialize_array(DatasetType.input, CT.source, 1)
    source[AT.id] = 4
    source[AT.node] = 1
    source[AT.status] = 1
    source[AT.u_ref] = 1.0

    load = initialize_array(DatasetType.input, CT.sym_load, 1)
    load[AT.id] = 5
    load[AT.node] = 2
    load[AT.status] = 1
    load[AT.type] = 0
    load[AT.p_specified] = 0.0
    load[AT.q_specified] = 0.0

    return PowerGridModel({CT.node: node, CT.line: line, CT.source: source, CT.sym_load: load})


def _load_p_specified_batch(p_specified):
    return {
        CT.sym_load: {
            AT.id: np.full((len(p_specified), 1), 5, dtype=np.int32),
            AT.p_specified: np.array(p_specified).reshape(-1, 1),
        }
    }


def _u_ref_batch(u_ref):
    return {
        CT.source: {
            AT.id: np.full((len(u_ref), 1), 4, dtype=np.int32),
            AT.u_ref: np.array(u_ref).reshape(-1, 1),
        }
    }


@pytest.mark.parametrize(
    ("outer_p", "expected_failed"),
    [
        pytest.param([50e6, 1e6], [0, 1], id="first outer fails, last converges"),
        pytest.param([1e6, 50e6, 1e6], [2, 3], id="middle outer fails"),
        pytest.param([50e6, 1e6, 50e6], [0, 1, 4, 5], id="first and last outer fail"),
        pytest.param([1e6, 1e6, 1e6], [], id="none fail"),
    ],
)
def test_multi_dimensional_batch_reports_all_diverged_scenarios(outer_p, expected_failed):
    pgm = _diverging_load_model()
    update_data = [_load_p_specified_batch(outer_p), _u_ref_batch([1.0, 1.0])]

    pgm.calculate_power_flow(update_data=update_data, continue_on_batch_error=True)

    assert (pgm.batch_error.failed_scenarios.tolist() if pgm.batch_error else []) == expected_failed
