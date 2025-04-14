# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import numpy as np
import pytest

from power_grid_model import ComponentType, DatasetType, LoadGenType, initialize_array
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset, is_columnar
from power_grid_model.enum import ComponentAttributeFilterOptions
from power_grid_model.validation import validate_batch_data
from power_grid_model.validation.errors import MultiComponentNotUniqueError, NotBetweenOrAtError, NotBooleanError


@pytest.fixture
def original_input_data() -> dict[str, np.ndarray]:
    node = initialize_array(DatasetType.input, ComponentType.node, 4)
    node["id"] = [1, 2, 3, 4]
    node["u_rated"] = 10.5e3
    line = initialize_array(DatasetType.input, ComponentType.line, 4)

    line["id"] = [5, 6, 7, 8]
    line["from_node"] = [1, 2, 3, 1]
    line["to_node"] = [2, 3, 1, 2]
    line["from_status"] = 0
    line["to_status"] = 0
    line["r1"] = 1.0
    line["x1"] = 2.0
    line["c1"] = 3.0
    line["tan1"] = 4.0
    line["i_n"] = 5.0

    asym_load = initialize_array(DatasetType.input, ComponentType.asym_load, 2)
    asym_load["id"] = [9, 10]
    asym_load["node"] = [1, 2]
    asym_load["status"] = [1, 1]
    asym_load["type"] = [LoadGenType.const_power, LoadGenType.const_power]
    asym_load["p_specified"] = [[11e6, 12e6, 13e6], [21e6, 22e6, 23e6]]
    asym_load["q_specified"] = [[11e5, 12e5, 13e5], [21e5, 22e5, 23e5]]

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
    line["id"] = [[5, 6], [6, 7], [7, 5]]
    line["from_status"] = [[1, 1], [1, 1], [1, 1]]

    # Add batch for asym_load, which has 2-D array for p_specified
    asym_load = initialize_array(DatasetType.update, ComponentType.asym_load, (3, 2))
    asym_load["id"] = [[9, 10], [9, 10], [9, 10]]

    return {"line": line, "asym_load": asym_load}


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
    if is_columnar(input_data["node"]):
        input_data["node"]["id"][-1] = 123
        input_data["line"]["id"][-1] = 123
    else:
        input_data["node"][-1]["id"] = 123
        input_data["line"][-1]["id"] = 123
    errors = validate_batch_data(input_data, batch_data)
    assert len(errors) == 3
    assert [MultiComponentNotUniqueError([("line", "id"), ("node", "id")], [("line", 123), ("node", 123)])] == errors[0]
    assert [MultiComponentNotUniqueError([("line", "id"), ("node", "id")], [("line", 123), ("node", 123)])] == errors[1]
    assert [MultiComponentNotUniqueError([("line", "id"), ("node", "id")], [("line", 123), ("node", 123)])] == errors[2]


def test_validate_batch_data_update_error(input_data, batch_data):
    batch_data["line"]["from_status"] = np.array([[12, 34], [0, -128], [56, 78]])
    errors = validate_batch_data(input_data, batch_data)
    assert len(errors) == 2
    assert 1 not in errors
    assert len(errors[0]) == 1
    assert len(errors[2]) == 1
    assert errors[0] == [NotBooleanError("line", "from_status", [5, 6])]
    assert errors[2] == [NotBooleanError("line", "from_status", [5, 7])]


def test_validate_batch_data_transformer_tap_nom():
    node_input = initialize_array(DatasetType.input, ComponentType.node, 2)
    node_input["id"] = [1, 2]
    node_input["u_rated"] = [400, 10500]

    transformer_input = initialize_array(DatasetType.input, ComponentType.transformer, 1)
    transformer_input["id"] = 3
    transformer_input["from_node"] = 1
    transformer_input["to_node"] = 2
    transformer_input["from_status"] = 1
    transformer_input["to_status"] = 1
    transformer_input["u1"] = 10750
    transformer_input["u2"] = 420
    transformer_input["sn"] = 400000
    transformer_input["uk"] = 0.04
    transformer_input["pk"] = 3750
    transformer_input["i0"] = 0.002230015414744929
    transformer_input["p0"] = 515
    transformer_input["winding_from"] = 2
    transformer_input["winding_to"] = 1
    transformer_input["clock"] = 5
    transformer_input["tap_side"] = 0
    transformer_input["tap_pos"] = 1
    transformer_input["tap_min"] = 1
    transformer_input["tap_max"] = 5
    transformer_input["tap_size"] = 250
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

    error = NotBetweenOrAtError(ComponentType.transformer, "tap_nom", [3], ("tap_min", "tap_max"))
    assert result == {0: [error], 1: [error]}
