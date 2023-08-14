# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import io
from pathlib import Path
from unittest.mock import MagicMock, mock_open, patch

import numpy as np
import pytest

from power_grid_model.data_types import BatchDataset, BatchList, Dataset
from power_grid_model.utils import (
    compact_json_dump,
    convert_batch_dataset_to_batch_list,
    convert_dataset_to_python_dataset,
    convert_python_to_numpy,
    export_json_data,
    get_and_verify_batch_sizes,
    is_nan,
    split_numpy_array_in_batches,
    split_sparse_batches_in_batches,
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


def assert_list_of_numpy_arrays_equal(expected, actual):
    assert type(actual) is type(expected)
    assert len(actual) == len(expected)
    for i in range(len(expected)):
        assert isinstance(expected[i], type(actual[i]))
        np.testing.assert_array_equal(expected[i], actual[i])


def assert_list_of_dicts_of_numpy_arrays_equal(expected, actual):
    assert type(actual) is type(expected)
    assert len(actual) == len(expected)
    for i in range(len(expected)):
        assert isinstance(expected[i], type(actual[i]))
        assert actual[i].keys() == expected[i].keys()
        for key in expected[i]:
            np.testing.assert_array_equal(expected[i][key], actual[i][key])


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
    json_dict = convert_dataset_to_python_dataset(pgm_data)
    assert json_dict == two_nodes_one_line

    json_list = [two_nodes_one_line, two_nodes_two_lines, two_nodes_one_line]
    pgm_data_list = convert_python_to_numpy(json_list, "input")
    json_return_list = convert_dataset_to_python_dataset(pgm_data_list)
    assert json_return_list == json_list


def test_convert_python_to_numpy__extra_attributes():
    convert_python_to_numpy({"line": [{"id": 1, "u": 10.5e3}]}, "input", ignore_extra=True)
    with pytest.raises(ValueError, match="Invalid attribute 'u' for line input data."):
        convert_python_to_numpy({"line": [{"id": 1, "u": 10.5e3}]}, "input")


def test_convert_python_to_numpy_invalid_data_format():
    with pytest.raises(ValueError, match="Invalid 'id' value for line input data."):
        convert_python_to_numpy({"line": [{"id": "my_line", "u_rated": 10.5e3}]}, "input")


def test_convert_python_to_numpy__raises_type_error():
    with pytest.raises(TypeError, match="Data should be either a list or a dictionary!"):
        convert_python_to_numpy(123, "input")  # type: ignore


def test_convert_batch_to_list_data__zero_batches():
    assert convert_batch_dataset_to_batch_list({}) == []


@patch("json.dump")
@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.convert_dataset_to_python_dataset")
def test_export_json_data(convert_mock: MagicMock, open_mock: MagicMock, json_dump_mock: MagicMock):
    convert_mock.return_value = {"foo": [{"val": 123}]}
    data: Dataset = {}  # type: ignore
    export_json_data(json_file=Path("output.json"), data=data, indent=2)
    convert_mock.assert_called_once()
    json_dump_mock.assert_called_once_with({"foo": [{"val": 123}]}, open_mock(), indent=2)


def test_compact_json_dump():
    data = {
        "node": [{"id": 1, "x": 2}, {"id": 3, "x": 4}],
        "line": [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}],
    }

    string_stream = io.StringIO()
    compact_json_dump(data, string_stream, indent=2, max_level=0)
    assert (
        string_stream.getvalue()
        == """{"node": [{"id": 1, "x": 2}, {"id": 3, "x": 4}], "line": [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}]}"""
    )

    string_stream = io.StringIO()
    compact_json_dump(data, string_stream, indent=2, max_level=1)
    assert (
        string_stream.getvalue()
        == """{
  "node": [{"id": 1, "x": 2}, {"id": 3, "x": 4}],
  "line": [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}]
}
"""
    )

    string_stream = io.StringIO()
    compact_json_dump(data, string_stream, indent=2, max_level=2)
    assert (
        string_stream.getvalue()
        == """{
  "node":
    [{"id": 1, "x": 2}, {"id": 3, "x": 4}],
  "line":
    [{"id": 5, "x": 6}, {"id": 7, "x": {"y": 8.1, "z": 8.2}}]
}
"""
    )

    string_stream = io.StringIO()
    compact_json_dump(data, string_stream, indent=2, max_level=3)
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
}
"""
    )


def test_compact_json_dump_string():
    data = "test"

    string_stream = io.StringIO()
    compact_json_dump(data, string_stream, indent=2, max_level=2)
    assert string_stream.getvalue() == '"test"'


def test_compact_json_dump_deep():
    data = {
        "foo": 1,
        "bar": {"x": 2, "y": 3},
    }

    string_stream = io.StringIO()
    compact_json_dump(data, string_stream, indent=2, max_level=10)
    assert (
        string_stream.getvalue()
        == """{
  "foo": 1,
  "bar":
    {
      "x": 2,
      "y": 3
    }

}
"""
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
    compact_json_dump(data, string_stream, indent=2, max_level=4)
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
  }
,
  {
    "line":
      [
        {"id": 9, "x": 10},
        {"id": 11, "x": 12}
      ]
  }

]"""
    )


def test_split_numpy_array_in_batches_n1():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    update_data = np.array([(111, 121, 131), (112, 122, 132), (113, 123, 133), (114, 124, 134)], dtype=foo)
    expected = [np.array([(111, 121, 131), (112, 122, 132), (113, 123, 133), (114, 124, 134)], dtype=foo)]
    actual = split_numpy_array_in_batches(update_data, "")
    assert_list_of_numpy_arrays_equal(expected, actual)


def test_split_numpy_array_in_batches_n2():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    update_data = np.array(
        [
            [(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)],
            [(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)],
        ],
        dtype=foo,
    )
    expected = [
        np.array([(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)], dtype=foo),
        np.array([(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)], dtype=foo),
    ]
    actual = split_numpy_array_in_batches(update_data, "")
    assert_list_of_numpy_arrays_equal(expected, actual)


def test_split_numpy_array_in_batches_wrong_data_type():
    update_data = [1, 2, 3]
    with pytest.raises(
        TypeError,
        match="Invalid data type list in batch data for 'foo' " r"\(should be a 1D/2D Numpy structured array\).",
    ):
        split_numpy_array_in_batches(update_data, "foo")  # type: ignore


def test_split_numpy_array_in_batches_wrong_data_dim():
    update_date = np.array([[[1, 2, 3]]])
    with pytest.raises(
        TypeError,
        match="Invalid data dimension 3 in batch data for 'foo' " r"\(should be a 1D/2D Numpy structured array\).",
    ):
        split_numpy_array_in_batches(update_date, "foo")


def test_normalize_batch_data_structure_n3_sparse():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    update_data = np.array(
        [
            (1111, 1121, 1131),
            (1112, 1122, 132),
            (1113, 1123, 1133),
            (1114, 1124, 1134),
            (2111, 2121, 2131),
            (2112, 2122, 232),
            (2113, 2123, 2133),
            (2114, 2124, 2134),
        ],
        dtype=foo,
    )
    indptr = np.array([0, 4, 4, 8])
    expected = [
        np.array([(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)], dtype=foo),
        np.array([], dtype=foo),
        np.array([(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)], dtype=foo),
    ]
    actual = split_sparse_batches_in_batches(batch_data={"data": update_data, "indptr": indptr}, component="")
    assert_list_of_numpy_arrays_equal(expected, actual)


def test_split_compressed_sparse_structure_in_batches_wrong_data():
    data_1 = [1, 2, 3, 4, 5, 6, 7, 8]  # wrong type
    data_2 = np.array([[1, 2, 3, 4], [5, 6, 7, 8]])  # wrong dimension
    indptr = np.array([0, 4, 4, 8])
    with pytest.raises(TypeError, match="Invalid data type list in sparse batch data for 'foo' "):
        split_sparse_batches_in_batches(batch_data={"data": data_1, "indptr": indptr}, component="foo")  # type: ignore
    with pytest.raises(TypeError, match="Invalid data type ndarray in sparse batch data for 'bar' "):
        split_sparse_batches_in_batches(batch_data={"data": data_2, "indptr": indptr}, component="bar")


def test_split_compressed_sparse_structure_in_batches_wrong_indptr():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    update_data = np.array(
        [
            (1111, 1121, 1131),
            (1112, 1122, 132),
            (1113, 1123, 1133),
            (1114, 1124, 1134),
            (2111, 2121, 2131),
            (2112, 2122, 232),
            (2113, 2123, 2133),
            (2114, 2124, 2134),
        ],
        dtype=foo,
    )
    indptr_1 = [0, 4, 4, 8]  # wrong type
    indptr_2 = np.array([[0, 4], [4, 8]])  # wrong dimension
    indptr_3 = np.array([0.0, 4.0, 4.0, 8.0])  # wrong dtype
    with pytest.raises(TypeError, match="Invalid indptr data type list in batch data for 'foo' "):
        split_sparse_batches_in_batches(batch_data={"data": update_data, "indptr": indptr_1}, component="foo")  # type: ignore
    with pytest.raises(TypeError, match="Invalid indptr data type ndarray in batch data for 'foo' "):
        split_sparse_batches_in_batches(batch_data={"data": update_data, "indptr": indptr_2}, component="foo")  # type: ignore
    with pytest.raises(TypeError, match="Invalid indptr data type ndarray in batch data for 'foo' "):
        split_sparse_batches_in_batches(batch_data={"data": update_data, "indptr": indptr_3}, component="foo")  # type: ignore


def test_split_compressed_sparse_structure_in_batches_wrong_indptr_values():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    update_data = np.array(
        [
            (1111, 1121, 1131),
            (1112, 1122, 132),
            (1113, 1123, 1133),
            (1114, 1124, 1134),
            (2111, 2121, 2131),
            (2112, 2122, 232),
            (2113, 2123, 2133),
            (2114, 2124, 2134),
        ],
        dtype=foo,
    )
    indptr_1 = np.array([1, 4, 4, 8])
    indptr_2 = np.array([0, 4, 4, 9])
    indptr_3 = np.array([0, 4, 3, 8])
    with pytest.raises(
        TypeError,
        match="Invalid indptr in batch data for 'foo' "
        r"\(should start with 0, end with the number of objects \(8\) "
        r"and be monotonic increasing\).",
    ):
        split_sparse_batches_in_batches(batch_data={"data": update_data, "indptr": indptr_1}, component="foo")
    with pytest.raises(
        TypeError,
        match="Invalid indptr in batch data for 'foo' "
        r"\(should start with 0, end with the number of objects \(8\) "
        r"and be monotonic increasing\).",
    ):
        split_sparse_batches_in_batches(batch_data={"data": update_data, "indptr": indptr_2}, component="foo")
    with pytest.raises(
        TypeError,
        match="Invalid indptr in batch data for 'foo' "
        r"\(should start with 0, end with the number of objects \(8\) "
        r"and be monotonic increasing\).",
    ):
        split_sparse_batches_in_batches(batch_data={"data": update_data, "indptr": indptr_3}, component="foo")


def test_convert_batch_dataset_to_batch_list_one_batch_dense():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    bar = [("x", "i4"), ("y", "i4"), ("z", "i4")]
    update_data: BatchDataset = {
        "foo": np.array([(111, 121, 131), (112, 122, 132), (113, 123, 133), (114, 124, 134)], dtype=foo),
        "bar": np.array([(211, 221, 231), (212, 222, 232), (213, 223, 233), (214, 224, 234)], dtype=bar),
    }
    expected: BatchList = [
        {
            "foo": np.array([(111, 121, 131), (112, 122, 132), (113, 123, 133), (114, 124, 134)], dtype=foo),
            "bar": np.array([(211, 221, 231), (212, 222, 232), (213, 223, 233), (214, 224, 234)], dtype=bar),
        }
    ]
    actual = convert_batch_dataset_to_batch_list(update_data)
    assert_list_of_dicts_of_numpy_arrays_equal(expected, actual)


def test_convert_batch_dataset_to_batch_list_two_batches_dense():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    bar = [("x", "i4"), ("y", "i4"), ("z", "i4")]
    update_data: BatchDataset = {
        "foo": np.array(
            [
                [(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)],
                [(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)],
            ],
            dtype=foo,
        ),
        "bar": np.array(
            [
                [(1211, 1221, 1231), (1212, 1222, 232), (1213, 1223, 1233), (1214, 1224, 1234)],
                [(2211, 2221, 2231), (2212, 2222, 232), (2213, 2223, 2233), (2214, 2224, 2234)],
            ],
            dtype=bar,
        ),
    }
    expected: BatchList = [
        {
            "foo": np.array([(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)], dtype=foo),
            "bar": np.array([(1211, 1221, 1231), (1212, 1222, 232), (1213, 1223, 1233), (1214, 1224, 1234)], dtype=bar),
        },
        {
            "foo": np.array([(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)], dtype=foo),
            "bar": np.array([(2211, 2221, 2231), (2212, 2222, 232), (2213, 2223, 2233), (2214, 2224, 2234)], dtype=bar),
        },
    ]
    actual = convert_batch_dataset_to_batch_list(update_data)
    assert_list_of_dicts_of_numpy_arrays_equal(expected, actual)


def test_convert_batch_dataset_to_batch_list_three_batches_sparse():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    bar = [("x", "i4"), ("y", "i4"), ("z", "i4")]
    update_data: BatchDataset = {
        "foo": {
            "indptr": np.array([0, 4, 8, 8]),
            "data": np.array(
                [
                    (1111, 1121, 1131),
                    (1112, 1122, 132),
                    (1113, 1123, 1133),
                    (1114, 1124, 1134),
                    (2111, 2121, 2131),
                    (2112, 2122, 232),
                    (2113, 2123, 2133),
                    (2114, 2124, 2134),
                ],
                dtype=foo,
            ),
        },
        "bar": {
            "indptr": np.array([0, 4, 4, 8]),
            "data": np.array(
                [
                    (1211, 1221, 1231),
                    (1212, 1222, 232),
                    (1213, 1223, 1233),
                    (1214, 1224, 1234),
                    (3211, 3221, 3231),
                    (3212, 3222, 332),
                    (3213, 3223, 3233),
                    (3214, 3224, 3234),
                ],
                dtype=bar,
            ),
        },
    }
    expected: BatchList = [
        {
            "foo": np.array([(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)], dtype=foo),
            "bar": np.array([(1211, 1221, 1231), (1212, 1222, 232), (1213, 1223, 1233), (1214, 1224, 1234)], dtype=bar),
        },
        {
            "foo": np.array([(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)], dtype=foo),
        },
        {"bar": np.array([(3211, 3221, 3231), (3212, 3222, 332), (3213, 3223, 3233), (3214, 3224, 3234)], dtype=bar)},
    ]
    actual = convert_batch_dataset_to_batch_list(update_data)
    assert_list_of_dicts_of_numpy_arrays_equal(expected, actual)


def test_get_and_verify_batch_sizes_inconsistent_batch_sizes_two_components():
    update_data: BatchDataset = {"foo": np.empty(shape=(3, 3)), "bar": np.empty(shape=(2, 3))}
    with pytest.raises(
        ValueError,
        match="Inconsistent number of batches in batch data. "
        "Component 'bar' contains 2 batches, while 'foo' contained 3 batches.",
    ):
        get_and_verify_batch_sizes(update_data)


def test_convert_get_and_verify_batch_sizes_inconsistent_batch_sizes_more_than_two_components():
    update_data: BatchDataset = {
        "foo": np.empty(shape=(3, 3)),
        "bar": np.empty(shape=(3, 3)),
        "baz": np.empty(shape=(2, 3)),
    }
    with pytest.raises(
        ValueError,
        match="Inconsistent number of batches in batch data. "
        "Component 'baz' contains 2 batches, while bar/foo contained 3 batches.",
    ):
        get_and_verify_batch_sizes(update_data)


@patch("power_grid_model.utils.get_and_verify_batch_sizes")
def test_convert_batch_dataset_to_batch_list_missing_key_sparse(_mock: MagicMock):
    update_data: BatchDataset = {"foo": {"a": np.empty(3), "data": np.empty(3)}}
    with pytest.raises(
        KeyError,
        match="Missing 'indptr' in sparse batch data for 'foo' "
        r"\(expected a python dictionary containing two keys: 'indptr' and 'data'\).",
    ):
        convert_batch_dataset_to_batch_list(update_data)


@patch("power_grid_model.utils.get_and_verify_batch_sizes")
def test_convert_batch_dataset_to_batch_list_invalid_type_sparse(_mock: MagicMock):
    update_data: BatchDataset = {"foo": "wrong type"}  # type: ignore
    with pytest.raises(
        TypeError,
        match="Invalid data type str in batch data for 'foo' "
        r"\(should be a Numpy structured array or a python dictionary\).",
    ):
        convert_batch_dataset_to_batch_list(update_data)
