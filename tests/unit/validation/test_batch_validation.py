# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import numpy as np
import pytest

from power_grid_model import AttributeType, ComponentType, DatasetType, LoadGenType, initialize_array
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset, is_columnar
from power_grid_model.enum import ComponentAttributeFilterOptions
from power_grid_model.validation import validate_batch_data
from power_grid_model.validation.errors import MultiComponentNotUniqueError, NotBetweenOrAtError, NotBooleanError


@pytest.fixture
def original_input_data() -> dict[str, np.ndarray]:
    node = initialize_array(DatasetType.input, ComponentType.node, 4)
    node[AttributeType.id] = [1, 2, 3, 4]
    node[AttributeType.u_rated] = 10.5e3
    line = initialize_array(DatasetType.input, ComponentType.line, 4)

    line[AttributeType.id] = [5, 6, 7, 8]
    line[AttributeType.from_node] = [1, 2, 3, 1]
    line[AttributeType.to_node] = [2, 3, 1, 2]
    line[AttributeType.from_status] = 0
    line[AttributeType.to_status] = 0
    line[AttributeType.r1] = 1.0
    line[AttributeType.x1] = 2.0
    line[AttributeType.c1] = 3.0
    line[AttributeType.tan1] = 4.0
    line[AttributeType.i_n] = 5.0

    asym_load = initialize_array(DatasetType.input, ComponentType.asym_load, 2)
    asym_load[AttributeType.id] = [9, 10]
    asym_load[AttributeType.node] = [1, 2]
    asym_load[AttributeType.status] = [1, 1]
    asym_load[AttributeType.type] = [LoadGenType.const_power, LoadGenType.const_power]
    asym_load[AttributeType.p_specified] = [[11e6, 12e6, 13e6], [21e6, 22e6, 23e6]]
    asym_load[AttributeType.q_specified] = [[11e5, 12e5, 13e5], [21e5, 22e5, 23e5]]

    return {
        ComponentType.node: node,
        ComponentType.line: line,
        ComponentType.asym_load: asym_load,
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
    line = initialize_array(DatasetType.update, ComponentType.line, (3, 2))
    line[AttributeType.id] = [[5, 6], [6, 7], [7, 5]]
    line[AttributeType.from_status] = [[1, 1], [1, 1], [1, 1]]

    # Add batch for asym_load, which has 2-D array for p_specified
    asym_load = initialize_array(DatasetType.update, ComponentType.asym_load, (3, 2))
    asym_load[AttributeType.id] = [[9, 10], [9, 10], [9, 10]]

    return {ComponentType.line: line, ComponentType.asym_load: asym_load}


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
    if is_columnar(input_data[ComponentType.node]):
        input_data[ComponentType.node][AttributeType.id][-1] = 123
        input_data[ComponentType.line][AttributeType.id][-1] = 123
    else:
        input_data[ComponentType.node][-1][AttributeType.id] = 123
        input_data[ComponentType.line][-1][AttributeType.id] = 123
    errors = validate_batch_data(input_data, batch_data)

    n_input_validation_errors = 3
    assert errors is not None
    assert len(errors) == n_input_validation_errors
    assert [
        MultiComponentNotUniqueError(
            [(ComponentType.line, AttributeType.id), (ComponentType.node, AttributeType.id)],
            [(ComponentType.line, 123), (ComponentType.node, 123)],
        )
    ] == errors[0]
    assert [
        MultiComponentNotUniqueError(
            [(ComponentType.line, AttributeType.id), (ComponentType.node, AttributeType.id)],
            [(ComponentType.line, 123), (ComponentType.node, 123)],
        )
    ] == errors[1]
    assert [
        MultiComponentNotUniqueError(
            [(ComponentType.line, AttributeType.id), (ComponentType.node, AttributeType.id)],
            [(ComponentType.line, 123), (ComponentType.node, 123)],
        )
    ] == errors[2]


def test_validate_batch_data_update_error(input_data, batch_data):
    batch_data[ComponentType.line][AttributeType.from_status] = np.array([[12, 34], [0, -128], [56, 78]])
    errors = validate_batch_data(input_data, batch_data)
    n_update_validation_errors = 2
    assert errors is not None
    assert len(errors) == n_update_validation_errors
    assert 1 not in errors
    assert len(errors[0]) == 1
    assert len(errors[2]) == 1
    assert errors[0] == [NotBooleanError(ComponentType.line, AttributeType.from_status, [5, 6])]
    assert errors[2] == [NotBooleanError(ComponentType.line, AttributeType.from_status, [5, 7])]


def test_validate_batch_data_transformer_tap_nom():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input[AttributeType.id] = [1, 2]
    node_input[AttributeType.u_rated] = [400, 10500]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input[AttributeType.id] = 3
    transformer_input[AttributeType.from_node] = 1
    transformer_input[AttributeType.to_node] = 2
    transformer_input[AttributeType.from_status] = 1
    transformer_input[AttributeType.to_status] = 1
    transformer_input[AttributeType.u1] = 10750
    transformer_input[AttributeType.u2] = 420
    transformer_input[AttributeType.sn] = 400000
    transformer_input[AttributeType.uk] = 0.04
    transformer_input[AttributeType.pk] = 3750
    transformer_input[AttributeType.i0] = 0.002230015414744929
    transformer_input[AttributeType.p0] = 515
    transformer_input[AttributeType.winding_from] = 2
    transformer_input[AttributeType.winding_to] = 1
    transformer_input[AttributeType.clock] = 5
    transformer_input[AttributeType.tap_side] = 0
    transformer_input[AttributeType.tap_pos] = 1
    transformer_input[AttributeType.tap_min] = 1
    transformer_input[AttributeType.tap_max] = 5
    transformer_input[AttributeType.tap_size] = 250
    test_input_data = {
        ComponentType.node: node_input,
        ComponentType.transformer: transformer_input,
    }
    test_update_data = {ComponentType.sym_load: initialize_array(DatasetType.update, ComponentType.sym_load, (2, 0))}
    result = validate_batch_data(test_input_data, test_update_data)
    assert result is not None
    assert len(result) == test_update_data[ComponentType.sym_load].shape[0]
    assert len(result[0]) == test_input_data[ComponentType.transformer].shape[0]
    assert len(result[1]) == test_input_data[ComponentType.transformer].shape[0]

    error = NotBetweenOrAtError(
        ComponentType.transformer,
        AttributeType.tap_nom,
        [3],
        (AttributeType.tap_min, AttributeType.tap_max),
    )
    assert result == {0: [error], 1: [error]}
