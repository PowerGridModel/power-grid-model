# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path
from unittest.mock import patch, mock_open, MagicMock

import numpy as np
import pytest
from power_grid_model.manual_testing import (
    convert_batch_to_list_data,
    convert_numpy_to_python,
    convert_python_to_numpy,
    export_json_data,
    is_nan,
)


@pytest.fixture(name="two_nodes_one_line")
def two_nodes_one_line_fixture():
    return {
        "node": [{"id": 11, "u_rated": 10.5e3}, {"id": 12, "u_rated": 10.5e3}],
        "line": [
            {
                "id": 21,
                "from_node": 11,
                "to_node": 12,
                "from_status": 1,
                "to_status": 1,
                "r1": 0.11,
                "x1": 0.12,
                "c1": 4.1380285203892784e-05,
                "tan1": 0.1076923076923077,
                "i_n": 510.0,
            }
        ],
    }


@pytest.fixture(name="two_nodes_two_lines")
def two_nodes_two_lines_fixture():
    return {
        "node": [{"id": 11, "u_rated": 10.5e3}, {"id": 12, "u_rated": 10.5e3}],
        "line": [
            {
                "id": 21,
                "from_node": 11,
                "to_node": 12,
                "from_status": 1,
                "to_status": 1,
                "r1": 0.11,
                "x1": 0.12,
                "c1": 4.1380285203892784e-05,
                "tan1": 0.1076923076923077,
                "i_n": 510.0,
            },
            {
                "id": 31,
                "from_node": 11,
                "to_node": 12,
                "from_status": 1,
                "to_status": 1,
                "r1": 0.11,
                "x1": 0.12,
                "c1": 4.1380285203892784e-05,
                "tan1": 0.1076923076923077,
                "i_n": 510.0,
            },
        ],
    }


def test_is_nan():
    single_value = np.array([np.nan])
    assert is_nan(single_value)
    array_f8 = np.array([0.1, 0.2, np.nan], dtype=np.dtype("f8"))
    assert not is_nan(array_f8)
    array_i4 = np.array([10, 2, -(2 ** 31), 40], dtype=np.dtype("i4"))
    assert not is_nan(array_i4)
    array_i1 = np.array([1, 0, -(2 ** 7), 1], dtype=np.dtype("i1"))
    assert not is_nan(array_i1)
    nan_array = np.array([np.nan, np.nan, np.nan])
    assert is_nan(nan_array)


def test_convert_json_to_numpy(two_nodes_one_line, two_nodes_two_lines):
    pgm_data = convert_python_to_numpy(two_nodes_one_line, "input")
    assert len(pgm_data) == 2
    assert len(pgm_data["node"]) == 2
    assert pgm_data["node"][0]["id"] == 11
    assert pgm_data["node"][0]["u_rated"] == 10.5e3
    assert len(pgm_data["line"]) == 1

    json_list = [two_nodes_one_line, two_nodes_two_lines, two_nodes_one_line]
    pgm_data_batch = convert_python_to_numpy(json_list, "input")
    assert pgm_data_batch["node"].shape == (3, 2)
    assert np.allclose(pgm_data_batch["line"]["indptr"], [0, 1, 3, 4])


def test_round_trip_json_numpy_json(two_nodes_one_line, two_nodes_two_lines):
    pgm_data = convert_python_to_numpy(two_nodes_one_line, "input")
    json_dict = convert_numpy_to_python(pgm_data)
    assert json_dict == two_nodes_one_line

    json_list = [two_nodes_one_line, two_nodes_two_lines, two_nodes_one_line]
    pgm_data_list = convert_python_to_numpy(json_list, "input")
    json_return_list = convert_numpy_to_python(pgm_data_list)
    assert json_return_list == json_list


def test_convert_python_to_numpy__raises_value_error():
    with pytest.raises(ValueError, match="Invalid property 'u' for line input data."):
        convert_python_to_numpy({"line": [{"id": 1, "u": 10.5e3}]}, "input")
    with pytest.raises(ValueError, match="Invalid 'id' value for line input data."):
        convert_python_to_numpy({"line": [{"id": "my_line", "u_rated": 10.5e3}]}, "input")


def test_convert_python_to_numpy__raises_type_error():
    with pytest.raises(TypeError, match="Only list or dict is allowed in JSON data!"):
        convert_python_to_numpy(123, "input")


def test_convert_batch_to_list_data__zero_batches():
    assert convert_batch_to_list_data({}) == []


@patch("json.dump")
@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.manual_testing.convert_numpy_to_python")
def test_export_json_data(convert_mock: MagicMock, open_mock: MagicMock, json_dump_mock: MagicMock):
    convert_mock.return_value = {"foo": [{"val": 123}]}
    export_json_data(json_file=Path("output.json"), data={}, indent=2)
    convert_mock.assert_called_once()
    json_dump_mock.assert_called_once_with({"foo": [{"val": 123}]}, open_mock(), indent=2)
