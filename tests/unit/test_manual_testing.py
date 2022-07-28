# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import io
from pathlib import Path
from unittest.mock import MagicMock, mock_open, patch

import numpy as np
import pytest

from power_grid_model.manual_testing import (
    _compact_json_dump,
    _inject_extra_info,
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
    array_i4 = np.array([10, 2, -(2**31), 40], dtype=np.dtype("i4"))
    assert not is_nan(array_i4)
    array_i1 = np.array([1, 0, -(2**7), 1], dtype=np.dtype("i1"))
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
    with pytest.raises(ValueError, match="Invalid attribute 'u' for line input data."):
        convert_python_to_numpy({"line": [{"id": 1, "u": 10.5e3}]}, "input")
    with pytest.raises(ValueError, match="Invalid 'id' value for line input data."):
        convert_python_to_numpy({"line": [{"id": "my_line", "u_rated": 10.5e3}]}, "input")


def test_convert_python_to_numpy__raises_type_error():
    with pytest.raises(TypeError, match="Data should be either a list or a dictionary!"):
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


@patch("json.dump")
@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.manual_testing.convert_numpy_to_python")
@patch("power_grid_model.manual_testing._inject_extra_info")
def test_export_json_data_extra_info(
    extra_info_mock: MagicMock, convert_mock: MagicMock, _open_mock: MagicMock, _json_dump_mock: MagicMock
):
    convert_mock.return_value = {"foo": [{"id": 123}]}
    export_json_data(json_file=Path(), data={}, extra_info={123: "Extra information"})
    extra_info_mock.assert_called_once_with(data={"foo": [{"id": 123}]}, extra_info={123: "Extra information"})


def test_inject_extra_info_single():
    data = {"node": [{"id": 0, "foo": 123}, {"id": 1, "bar": 456}], "line": [{"id": 2, "baz": 789}]}
    extra_info = {2: 42, 1: {"sheet": "Nodes", "Number": "00123"}}
    _inject_extra_info(data=data, extra_info=extra_info)
    assert data == {
        "node": [{"id": 0, "foo": 123}, {"id": 1, "bar": 456, "extra": {"sheet": "Nodes", "Number": "00123"}}],
        "line": [{"id": 2, "baz": 789, "extra": 42}],
    }


def test_inject_extra_info_batch():
    data = [
        {"node": [{"id": 0, "foo": 111}, {"id": 1, "bar": 222}], "line": [{"id": 2, "baz": 333}]},
        {"node": [{"id": 0, "foo": 444}, {"id": 1, "bar": 555}], "line": [{"id": 2, "baz": 666}]},
    ]
    extra_info = [{2: 42, 1: {"sheet": "Nodes", "Number": "00123"}}, {2: 43, 0: None}]
    _inject_extra_info(data=data, extra_info=extra_info)
    assert data == [
        {
            "node": [{"id": 0, "foo": 111}, {"id": 1, "bar": 222, "extra": {"sheet": "Nodes", "Number": "00123"}}],
            "line": [{"id": 2, "baz": 333, "extra": 42}],
        },
        {
            "node": [{"id": 0, "foo": 444, "extra": None}, {"id": 1, "bar": 555}],
            "line": [{"id": 2, "baz": 666, "extra": 43}],
        },
    ]


def test_inject_extra_info_batch_copy_info():
    data = [
        {"node": [{"id": 0, "foo": 111}, {"id": 1, "bar": 222}], "line": [{"id": 2, "baz": 333}]},
        {"node": [{"id": 0, "foo": 444}, {"id": 1, "bar": 555}], "line": [{"id": 2, "baz": 666}]},
    ]
    extra_info = {2: 42, 1: {"sheet": "Nodes", "Number": "00123"}}
    _inject_extra_info(data=data, extra_info=extra_info)
    assert data == [
        {
            "node": [{"id": 0, "foo": 111}, {"id": 1, "bar": 222, "extra": {"sheet": "Nodes", "Number": "00123"}}],
            "line": [{"id": 2, "baz": 333, "extra": 42}],
        },
        {
            "node": [{"id": 0, "foo": 444}, {"id": 1, "bar": 555, "extra": {"sheet": "Nodes", "Number": "00123"}}],
            "line": [{"id": 2, "baz": 666, "extra": 42}],
        },
    ]


def test_inject_extra_info_single_dataset_with_batch_info():
    data = {"node": [{"id": 0, "foo": 123}, {"id": 1, "bar": 456}], "line": [{"id": 2, "baz": 789}]}
    extra_info = [{2: 42, 1: {"sheet": "Nodes", "Number": "00123"}}, {2: 43, 0: None}]
    with pytest.raises(TypeError):
        _inject_extra_info(data=data, extra_info=extra_info)


def test_compact_json_dump():
    data = {
        "node": [{"id": 1, "x": 2}, {"id": 3, "x": 4}],
        "line": [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}],
    }

    string_stream = io.StringIO()
    _compact_json_dump(data, string_stream, indent=2, max_level=0)
    assert (
        string_stream.getvalue()
        == """{"node": [{"id": 1, "x": 2}, {"id": 3, "x": 4}], "line": [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}]}"""
    )

    string_stream = io.StringIO()
    _compact_json_dump(data, string_stream, indent=2, max_level=1)
    assert (
        string_stream.getvalue()
        == """{
  "node": [{"id": 1, "x": 2}, {"id": 3, "x": 4}],
  "line": [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}]
}"""
    )

    string_stream = io.StringIO()
    _compact_json_dump(data, string_stream, indent=2, max_level=2)
    assert (
        string_stream.getvalue()
        == """{
  "node":
    [{"id": 1, "x": 2}, {"id": 3, "x": 4}],
  "line":
    [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}]
}"""
    )

    string_stream = io.StringIO()
    _compact_json_dump(data, string_stream, indent=2, max_level=3)
    assert (
        string_stream.getvalue()
        == """{
  "node":
    [
      {"id": 1, "x": 2},
      {"id": 3, "x": 4}
    ],
  "line":
    [
      {"id": 5, "x": 6},
      {"id": 7, "x": {"y": 8.1, "z": 8.2}}
    ]
}"""
    )


def test_compact_json_dump_string():
    data = "test"

    string_stream = io.StringIO()
    _compact_json_dump(data, string_stream, indent=2, max_level=2)
    assert string_stream.getvalue() == '"test"'


def test_compact_json_dump_deep():
    data = {
        "foo": 1,
        "bar": {"x": 2, "y": 3},
    }

    string_stream = io.StringIO()
    _compact_json_dump(data, string_stream, indent=2, max_level=10)
    assert (
        string_stream.getvalue()
        == """{
  "foo": 1,
  "bar":
    {
      "x": 2,
      "y": 3
    }
}"""
    )


def test_compact_json_dump_batch():
    data = [
        {
            "node": [{"id": 1, "x": 2}, {"id": 3, "x": 4}],
            "line": [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}],
        },
        {
            "line": [{"id": 9, "x": 10}, {"id": 11, "x": 12}],
        },
    ]
    string_stream = io.StringIO()
    _compact_json_dump(data, string_stream, indent=2, max_level=4)
    assert (
        string_stream.getvalue()
        == """[
  {
    "node":
      [
        {"id": 1, "x": 2},
        {"id": 3, "x": 4}
      ],
    "line":
      [
        {"id": 5, "x": 6},
        {"id": 7, "x": {"y": 8.1, "z": 8.2}}
      ]
  },
  {
    "line":
      [
        {"id": 9, "x": 10},
        {"id": 11, "x": 12}
      ]
  }
]"""
    )
