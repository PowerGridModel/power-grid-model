# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import numpy as np
import pytest

from power_grid_model import initialize_array
from power_grid_model.data_types import BatchDataset, BatchList
from power_grid_model.validation.errors import NotGreaterThanError
from power_grid_model.validation.utils import errors_to_string, eval_field_expression, update_input_data


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


def test_update_input_data_asym_nans():
    input_load = initialize_array("input", "asym_load", 3)
    input_load["id"] = [1, 2, 3]
    input_load["p_specified"] = [[1.1, 1.2, 1.3], [2.1, np.nan, np.nan], [np.nan, np.nan, np.nan]]

    update_load = initialize_array("update", "asym_load", 3)
    update_load["id"] = [1, 2, 3]
    update_load["p_specified"] = [[np.nan, np.nan, np.nan], [np.nan, np.nan, 5.3], [6.1, 6.2, 6.3]]

    merged = update_input_data(input_data={"asym_load": input_load}, update_data={"asym_load": update_load})

    np.testing.assert_array_equal(
        merged["asym_load"]["p_specified"], [[1.1, 1.2, 1.3], [2.1, np.nan, 5.3], [6.1, 6.2, 6.3]]
    )


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
