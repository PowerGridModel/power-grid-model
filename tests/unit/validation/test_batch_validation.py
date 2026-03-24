# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import numpy as np
import pytest

from power_grid_model import AttributeType as AT, ComponentType as CT, DatasetType, LoadGenType, initialize_array
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset, is_columnar
from power_grid_model.enum import ComponentAttributeFilterOptions
from power_grid_model.validation import validate_batch_data
from power_grid_model.validation.errors import MultiComponentNotUniqueError, NotBetweenOrAtError, NotBooleanError


@pytest.fixture
def original_input_data() -> dict[str, np.ndarray]:
    node = initialize_array(DatasetType.input, CT.node, 4)
    node[AT.id] = [1, 2, 3, 4]
    node[AT.u_rated] = 10.5e3
    line = initialize_array(DatasetType.input, CT.line, 4)

    line[AT.id] = [5, 6, 7, 8]
    line[AT.from_node] = [1, 2, 3, 1]
    line[AT.to_node] = [2, 3, 1, 2]
    line[AT.from_status] = 0
    line[AT.to_status] = 0
    line[AT.r1] = 1.0
    line[AT.x1] = 2.0
    line[AT.c1] = 3.0
    line[AT.tan1] = 4.0
    line[AT.i_n] = 5.0

    asym_load = initialize_array(DatasetType.input, CT.asym_load, 2)
    asym_load[AT.id] = [9, 10]
    asym_load[AT.node] = [1, 2]
    asym_load[AT.status] = [1, 1]
    asym_load[AT.type] = [LoadGenType.const_power, LoadGenType.const_power]
    asym_load[AT.p_specified] = [[11e6, 12e6, 13e6], [21e6, 22e6, 23e6]]
    asym_load[AT.q_specified] = [[11e5, 12e5, 13e5], [21e5, 22e5, 23e5]]

    return {
        CT.node: node,
        CT.line: line,
        CT.asym_load: asym_load,
    }


@pytest.fixture
def original_input_data_columnar_all(original_input_data):
    return compatibility_convert_row_columnar_dataset(
        original_input_data,
        ComponentAttributeFilterOptions.everything,
        DatasetType.input,
    )


@pytest.fixture
def original_input_data_columnar_relevant(original_input_data):
    return compatibility_convert_row_columnar_dataset(
        original_input_data, ComponentAttributeFilterOptions.relevant, DatasetType.input
    )


@pytest.fixture(
    params=[
        "original_input_data",
        "original_input_data_columnar_all",
        "original_input_data_columnar_relevant",
    ]
)
def input_data(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def original_batch_data() -> dict[str, np.ndarray]:
    line = initialize_array(DatasetType.update, CT.line, (3, 2))
    line[AT.id] = [[5, 6], [6, 7], [7, 5]]
    line[AT.from_status] = [[1, 1], [1, 1], [1, 1]]

    # Add batch for asym_load, which has 2-D array for p_specified
    asym_load = initialize_array(DatasetType.update, CT.asym_load, (3, 2))
    asym_load[AT.id] = [[9, 10], [9, 10], [9, 10]]

    return {CT.line: line, CT.asym_load: asym_load}


@pytest.fixture
def original_batch_data_columnar_all(original_batch_data):
    return compatibility_convert_row_columnar_dataset(
        original_batch_data,
        ComponentAttributeFilterOptions.everything,
        DatasetType.update,
    )


@pytest.fixture
def original_batch_data_columnar_relevant(original_batch_data):
    return compatibility_convert_row_columnar_dataset(
        original_batch_data,
        ComponentAttributeFilterOptions.relevant,
        DatasetType.update,
    )


@pytest.fixture(
    params=[
        "original_batch_data",
        "original_batch_data_columnar_all",
        "original_batch_data_columnar_relevant",
    ]
)
def batch_data(request):
    return request.getfixturevalue(request.param)


def test_validate_batch_data(input_data, batch_data):
    errors = validate_batch_data(input_data, batch_data)
    assert not errors


def test_validate_batch_data_input_error(input_data, batch_data):
    if is_columnar(input_data[CT.node]):
        input_data[CT.node][AT.id][-1] = 123
        input_data[CT.line][AT.id][-1] = 123
    else:
        input_data[CT.node][-1][AT.id] = 123
        input_data[CT.line][-1][AT.id] = 123
    errors = validate_batch_data(input_data, batch_data)

    n_input_validation_errors = 3
    assert errors is not None
    assert len(errors) == n_input_validation_errors
    assert [
        MultiComponentNotUniqueError(
            [(CT.line, AT.id), (CT.node, AT.id)],
            [(CT.line, 123), (CT.node, 123)],
        )
    ] == errors[0]
    assert [
        MultiComponentNotUniqueError(
            [(CT.line, AT.id), (CT.node, AT.id)],
            [(CT.line, 123), (CT.node, 123)],
        )
    ] == errors[1]
    assert [
        MultiComponentNotUniqueError(
            [(CT.line, AT.id), (CT.node, AT.id)],
            [(CT.line, 123), (CT.node, 123)],
        )
    ] == errors[2]


def test_validate_batch_data_update_error(input_data, batch_data):
    batch_data[CT.line][AT.from_status] = np.array([[12, 34], [0, -128], [56, 78]])
    errors = validate_batch_data(input_data, batch_data)
    n_update_validation_errors = 2
    assert errors is not None
    assert len(errors) == n_update_validation_errors
    assert 1 not in errors
    assert len(errors[0]) == 1
    assert len(errors[2]) == 1
    assert errors[0] == [NotBooleanError(CT.line, AT.from_status, [5, 6])]
    assert errors[2] == [NotBooleanError(CT.line, AT.from_status, [5, 7])]


def test_validate_batch_data_transformer_tap_nom():
    node_input = initialize_array(DatasetType.input, CT.node, 2)
    node_input[AT.id] = [1, 2]
    node_input[AT.u_rated] = [400, 10500]

    transformer_input = initialize_array(DatasetType.input, CT.transformer, 1)
    transformer_input[AT.id] = 3
    transformer_input[AT.from_node] = 1
    transformer_input[AT.to_node] = 2
    transformer_input[AT.from_status] = 1
    transformer_input[AT.to_status] = 1
    transformer_input[AT.u1] = 10750
    transformer_input[AT.u2] = 420
    transformer_input[AT.sn] = 400000
    transformer_input[AT.uk] = 0.04
    transformer_input[AT.pk] = 3750
    transformer_input[AT.i0] = 0.002230015414744929
    transformer_input[AT.p0] = 515
    transformer_input[AT.winding_from] = 2
    transformer_input[AT.winding_to] = 1
    transformer_input[AT.clock] = 5
    transformer_input[AT.tap_side] = 0
    transformer_input[AT.tap_pos] = 1
    transformer_input[AT.tap_min] = 1
    transformer_input[AT.tap_max] = 5
    transformer_input[AT.tap_size] = 250
    test_input_data = {
        CT.node: node_input,
        CT.transformer: transformer_input,
    }
    test_update_data = {CT.sym_load: initialize_array(DatasetType.update, CT.sym_load, (2, 0))}
    result = validate_batch_data(test_input_data, test_update_data)
    assert result is not None
    assert len(result) == test_update_data[CT.sym_load].shape[0]
    assert len(result[0]) == test_input_data[CT.transformer].shape[0]
    assert len(result[1]) == test_input_data[CT.transformer].shape[0]

    error = NotBetweenOrAtError(CT.transformer, AT.tap_nom, [3], (AT.tap_min, AT.tap_max))
    assert result == {0: [error], 1: [error]}
