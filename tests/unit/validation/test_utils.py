# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np
import pytest
from power_grid_model import initialize_array
from power_grid_model.validation.errors import NotGreaterThanError
from power_grid_model.validation.utils import (
    UpdateData,
    BatchData,
    split_update_data_in_batches,
    split_numpy_array_in_batches,
    split_compressed_sparse_structure_in_batches,
    errors_to_string,
)
from power_grid_model.validation.utils import eval_field_expression, update_input_data


def test_eval_field_expression():
    data = np.array([(1.0, 2.0, 4.0), (8.0, 16.0, 0.0)], dtype=[("a", "f8"), ("b2", "f8"), ("c_3", "f8")])
    np.testing.assert_array_equal(eval_field_expression(data, "a"), np.array([1, 8]))
    np.testing.assert_array_equal(eval_field_expression(data, "a/b2"), np.array([0.5, 0.5]))
    np.testing.assert_array_equal(eval_field_expression(data, "a / b2"), np.array([0.5, 0.5]))
    np.testing.assert_array_equal(eval_field_expression(data, "a / c_3"), np.array([0.25, np.nan]))

    with pytest.raises(ValueError):
        eval_field_expression(data, "a / 1")
    with pytest.raises(ValueError):
        eval_field_expression(data, "a + 100")
    with pytest.raises(ValueError):
        eval_field_expression(data, "a + 100.123")
    with pytest.raises(ValueError):
        eval_field_expression(data, "a + b2")
    with pytest.raises(ValueError):
        eval_field_expression(data, "a - b2")
    with pytest.raises(ValueError):
        eval_field_expression(data, "a * b2")
    with pytest.raises(ValueError):
        eval_field_expression(data, "a * -b2")
    with pytest.raises(ValueError):
        eval_field_expression(data, "a + b2 + c_3")
    with pytest.raises(ValueError):
        eval_field_expression(data, "max(a, b2)")
    with pytest.raises(ValueError):
        eval_field_expression(data, "(a + b2) / c_3")

    with pytest.raises(KeyError):
        eval_field_expression(data, "a / b")


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
        match="Invalid data type list in update data for 'foo' " r"\(should be a 1D/2D Numpy structured array\).",
    ):
        split_numpy_array_in_batches(update_data, "foo")  # type: ignore


def test_split_numpy_array_in_batches_wrong_data_dim():
    update_date = np.array([[[1, 2, 3]]])
    with pytest.raises(
        TypeError,
        match="Invalid data dimension 3 in update data for 'foo' " r"\(should be a 1D/2D Numpy structured array\).",
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
    actual = split_compressed_sparse_structure_in_batches(update_data, indptr, "")
    assert_list_of_numpy_arrays_equal(expected, actual)


def test_split_compressed_sparse_structure_in_batches_wrong_data():
    data_1 = [1, 2, 3, 4, 5, 6, 7, 8]  # wrong type
    data_2 = np.array([[1, 2, 3, 4], [5, 6, 7, 8]])  # wrong dimension
    indptr = np.array([0, 4, 4, 8])
    with pytest.raises(TypeError, match="Invalid data type list in sparse update data for 'foo' "):
        split_compressed_sparse_structure_in_batches(data=data_1, indptr=indptr, component="foo")  # type: ignore
    with pytest.raises(TypeError, match="Invalid data type ndarray in sparse update data for 'bar' "):
        split_compressed_sparse_structure_in_batches(data=data_2, indptr=indptr, component="bar")


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
    with pytest.raises(TypeError, match="Invalid indptr data type list in update data for 'foo' "):
        split_compressed_sparse_structure_in_batches(data=update_data, indptr=indptr_1, component="foo")  # type: ignore
    with pytest.raises(TypeError, match="Invalid indptr data type ndarray in update data for 'foo' "):
        split_compressed_sparse_structure_in_batches(data=update_data, indptr=indptr_2, component="foo")  # type: ignore
    with pytest.raises(TypeError, match="Invalid indptr data type ndarray in update data for 'foo' "):
        split_compressed_sparse_structure_in_batches(data=update_data, indptr=indptr_3, component="foo")  # type: ignore


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
        match="Invalid indptr in update data for 'foo' "
        r"\(should start with 0, end with the number of objects \(8\) "
        r"and be monotonic increasing\).",
    ):
        split_compressed_sparse_structure_in_batches(data=update_data, indptr=indptr_1, component="foo")
    with pytest.raises(
        TypeError,
        match="Invalid indptr in update data for 'foo' "
        r"\(should start with 0, end with the number of objects \(8\) "
        r"and be monotonic increasing\).",
    ):
        split_compressed_sparse_structure_in_batches(data=update_data, indptr=indptr_2, component="foo")
    with pytest.raises(
        TypeError,
        match="Invalid indptr in update data for 'foo' "
        r"\(should start with 0, end with the number of objects \(8\) "
        r"and be monotonic increasing\).",
    ):
        split_compressed_sparse_structure_in_batches(data=update_data, indptr=indptr_3, component="foo")


def test_split_update_data_in_batches_one_batch_dense():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    bar = [("x", "i4"), ("y", "i4"), ("z", "i4")]
    update_data: UpdateData = {
        "foo": np.array([(111, 121, 131), (112, 122, 132), (113, 123, 133), (114, 124, 134)], dtype=foo),
        "bar": np.array([(211, 221, 231), (212, 222, 232), (213, 223, 233), (214, 224, 234)], dtype=bar),
    }
    expected: BatchData = [
        {
            "foo": np.array([(111, 121, 131), (112, 122, 132), (113, 123, 133), (114, 124, 134)], dtype=foo),
            "bar": np.array([(211, 221, 231), (212, 222, 232), (213, 223, 233), (214, 224, 234)], dtype=bar),
        }
    ]
    actual = split_update_data_in_batches(update_data)
    assert_list_of_dicts_of_numpy_arrays_equal(expected, actual)


def test_split_update_data_in_batches_two_batches_dense():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    bar = [("x", "i4"), ("y", "i4"), ("z", "i4")]
    update_data: UpdateData = {
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
    expected: BatchData = [
        {
            "foo": np.array([(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)], dtype=foo),
            "bar": np.array([(1211, 1221, 1231), (1212, 1222, 232), (1213, 1223, 1233), (1214, 1224, 1234)], dtype=bar),
        },
        {
            "foo": np.array([(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)], dtype=foo),
            "bar": np.array([(2211, 2221, 2231), (2212, 2222, 232), (2213, 2223, 2233), (2214, 2224, 2234)], dtype=bar),
        },
    ]
    actual = split_update_data_in_batches(update_data)
    assert_list_of_dicts_of_numpy_arrays_equal(expected, actual)


def test_split_update_data_in_batches_three_batches_sparse():
    foo = [("a", "i4"), ("b", "i4"), ("c", "i4")]
    bar = [("x", "i4"), ("y", "i4"), ("z", "i4")]
    update_data: UpdateData = {
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
    expected: BatchData = [
        {
            "foo": np.array([(1111, 1121, 1131), (1112, 1122, 132), (1113, 1123, 1133), (1114, 1124, 1134)], dtype=foo),
            "bar": np.array([(1211, 1221, 1231), (1212, 1222, 232), (1213, 1223, 1233), (1214, 1224, 1234)], dtype=bar),
        },
        {
            "foo": np.array([(2111, 2121, 2131), (2112, 2122, 232), (2113, 2123, 2133), (2114, 2124, 2134)], dtype=foo),
        },
        {"bar": np.array([(3211, 3221, 3231), (3212, 3222, 332), (3213, 3223, 3233), (3214, 3224, 3234)], dtype=bar)},
    ]
    actual = split_update_data_in_batches(update_data)
    assert_list_of_dicts_of_numpy_arrays_equal(expected, actual)


def test_split_update_data_in_batches_inconsistent_batch_sizes_two_components():
    update_data: UpdateData = {"foo": np.empty(shape=(3, 3)), "bar": np.empty(shape=(2, 3))}
    with pytest.raises(
        ValueError,
        match="Inconsistent number of batches in update data. "
        "Component 'bar' contains 2 batches, while 'foo' contained 3 batches.",
    ):
        split_update_data_in_batches(update_data)


def test_split_update_data_in_batches_inconsistent_batch_sizes_more_than_two_components():
    update_data: UpdateData = {
        "foo": np.empty(shape=(3, 3)),
        "bar": np.empty(shape=(3, 3)),
        "baz": np.empty(shape=(2, 3)),
    }
    with pytest.raises(
        ValueError,
        match="Inconsistent number of batches in update data. "
        "Component 'baz' contains 2 batches, while bar/foo contained 3 batches.",
    ):
        split_update_data_in_batches(update_data)


def test_split_update_data_in_batches_missing_key_sparse():
    update_data: UpdateData = {"foo": {"a": np.empty(3), "data": np.empty(3)}}
    with pytest.raises(
        KeyError,
        match="Missing 'indptr' in sparse update data for 'foo' "
        r"\(expected a python dictionary containing two keys: 'indptr' and 'data'\).",
    ):
        split_update_data_in_batches(update_data)


def test_split_update_data_in_batches_invalid_type_sparse():
    update_data: UpdateData = {"foo": "wrong type"}  # type: ignore
    with pytest.raises(
        TypeError,
        match="Invalid data type str in update data for 'foo' "
        r"\(should be a Numpy structured array or a python dictionary\).",
    ):
        split_update_data_in_batches(update_data)


def test_update_input_data():
    input_test = np.array(
        [(4, 4.0, 4.1), (5, 5.0, 5.1), (6, 6.0, 6.1), (1, 1.0, np.nan), (2, 2.0, np.nan), (3, 3.0, np.nan)],
        dtype=[("id", "i4"), ("foo", "f8"), ("bar", "f8")],
    )

    update_test = np.array([(6, np.nan), (5, 5.2), (2, np.nan), (3, 3.2)], dtype=[("id", "i4"), ("bar", "f8")])

    merged = update_input_data(input_data={"test": input_test}, update_data={"test": update_test})

    np.testing.assert_array_equal(merged["test"]["id"], [4, 5, 6, 1, 2, 3])
    np.testing.assert_array_equal(merged["test"]["foo"], [4.0, 5.0, 6.0, 1.0, 2.0, 3.0])
    np.testing.assert_array_equal(merged["test"]["bar"], [4.1, 5.2, 6.1, np.nan, np.nan, 3.2])


def test_update_input_data_int_nan():
    input_line = initialize_array("input", "line", 3)
    input_line["id"] = [1, 2, 3]
    input_line["from_status"] = [0, -128, -128]

    update_line = initialize_array("update", "line", 2)
    update_line["id"] = [1, 3]
    update_line["from_status"] = [-128, 1]

    merged = update_input_data(input_data={"line": input_line}, update_data={"line": update_line})

    np.testing.assert_array_equal(merged["line"]["from_status"], [0, -128, 1])


def test_errors_to_string_no_errors():
    assert errors_to_string(errors=None) == "the data: OK"
    assert errors_to_string(errors=[]) == "the data: OK"


def test_errors_to_string_list_1():
    errors = [NotGreaterThanError("node", "u_rated", [1], 0)]
    error_string = errors_to_string(errors=errors, name="input_data")
    assert (
        error_string == "There is a validation error in input_data:\n\tField 'u_rated' is not greater than zero "
        "for 1 node."
    )


def test_errors_to_string_list_1_details():
    errors = [NotGreaterThanError("node", "u_rated", [1], 0)]
    # Without id_lookup
    error_string = errors_to_string(errors=errors, name="input_data", details=True)
    assert (
        error_string == "There is a validation error in input_data:\n\n\t"
        "Field 'u_rated' is not greater than zero for 1 node.\n"
        "\t\tcomponent: node\n"
        "\t\tfield: 'u_rated'\n"
        "\t\tids: [1]\n"
        "\t\tref_value: zero\n"
    )

    # With id_lookup
    error_string = errors_to_string(errors=errors, name="input_data", details=True, id_lookup={1: "id_123"})
    assert (
        error_string == "There is a validation error in input_data:\n\n\t"
        "Field 'u_rated' is not greater than zero for 1 node.\n"
        "\t\tcomponent: node\n"
        "\t\tfield: 'u_rated'\n"
        "\t\tids: {1: 'id_123'}\n"
        "\t\tref_value: zero\n"
    )


def test_errors_to_string_list_2_no_details():
    errors = [NotGreaterThanError("node", "u_rated", [1], 0), NotGreaterThanError("line", "p_from", [2], 0)]
    error_string = errors_to_string(errors=errors, name="input_data")
    assert (
        error_string == "There are 2 validation errors in input_data:\n"
        "   1. Field 'u_rated' is not greater than zero for 1 node."
        "\n   2. Field 'p_from' is not greater than zero for 1 line."
    )


def test_errors_to_string_dict_2_no_details():
    errors = {1: [NotGreaterThanError("node", "u_rated", [1], 0)], 2: [NotGreaterThanError("line", "p_from", [2], 0)]}
    error_string = errors_to_string(errors=errors, name="input_data")
    assert (
        error_string == "There is a validation error in input_data, batch #1:\n\tField 'u_rated' is not greater "
        "than zero for 1 node."
        "\nThere is a validation error in input_data, batch #2:\n\tField 'p_from' is not greater "
        "than zero for 1 line."
    )
