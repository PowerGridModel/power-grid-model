# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import numpy as np
import pytest

from power_grid_model import DatasetType, LoadGenType, initialize_array
from power_grid_model._utils import compatibility_convert_row_columnar_dataset, is_columnar
from power_grid_model.enum import ComponentAttributeFilterOptions
from power_grid_model.validation import validate_batch_data
from power_grid_model.validation.errors import MultiComponentNotUniqueError, NotBooleanError


@pytest.fixture
def original_input_data() -> dict[str, np.ndarray]:
    node = initialize_array("input", "node", 4)
    node["id"] = [1, 2, 3, 4]
    node["u_rated"] = 10.5e3
    line = initialize_array("input", "line", 4)

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

    asym_load = initialize_array("input", "asym_load", 2)
    asym_load["id"] = [9, 10]
    asym_load["node"] = [1, 2]
    asym_load["status"] = [1, 1]
    asym_load["type"] = [LoadGenType.const_power, LoadGenType.const_power]
    asym_load["p_specified"] = [[11e6, 12e6, 13e6], [21e6, 22e6, 23e6]]
    asym_load["q_specified"] = [[11e5, 12e5, 13e5], [21e5, 22e5, 23e5]]

    return {"node": node, "line": line, "asym_load": asym_load}


@pytest.fixture
def original_input_data_columnar_all(original_input_data):
    return compatibility_convert_row_columnar_dataset(
        original_input_data, ComponentAttributeFilterOptions.everything, DatasetType.input
    )


@pytest.fixture
def original_input_data_columnar_relevant(original_input_data):
    return compatibility_convert_row_columnar_dataset(
        original_input_data, ComponentAttributeFilterOptions.relevant, DatasetType.input
    )


@pytest.fixture(
    params=["original_input_data", "original_input_data_columnar_all", "original_input_data_columnar_relevant"]
)
def input_data(request):
    return request.getfixturevalue(request.param)


@pytest.fixture
def original_batch_data() -> dict[str, np.ndarray]:
    line = initialize_array("update", "line", (3, 2))
    line["id"] = [[5, 6], [6, 7], [7, 5]]
    line["from_status"] = [[1, 1], [1, 1], [1, 1]]

    # Add batch for asym_load, which has 2-D array for p_specified
    asym_load = initialize_array("update", "asym_load", (3, 2))
    asym_load["id"] = [[9, 10], [9, 10], [9, 10]]

    return {"line": line, "asym_load": asym_load}


@pytest.fixture
def original_batch_data_columnar_all(original_batch_data):
    return compatibility_convert_row_columnar_dataset(
        original_batch_data, ComponentAttributeFilterOptions.everything, DatasetType.update
    )


@pytest.fixture
def original_batch_data_columnar_relevant(original_batch_data):
    return compatibility_convert_row_columnar_dataset(
        original_batch_data, ComponentAttributeFilterOptions.relevant, DatasetType.update
    )


@pytest.fixture(
    params=["original_batch_data", "original_batch_data_columnar_all", "original_batch_data_columnar_relevant"]
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
    assert len(errors) == 3
    assert NotBooleanError("line", "from_status", [5, 6]) == errors[0][0]
    assert NotBooleanError("line", "from_status", [5, 7]) == errors[1][1]
    assert NotBooleanError("line", "from_status", [5, 6]) == errors[2][0]
