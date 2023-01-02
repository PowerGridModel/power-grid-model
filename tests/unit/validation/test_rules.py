# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from enum import IntEnum

import numpy as np
import pytest

from power_grid_model.validation.errors import (
    ComparisonError,
    InfinityError,
    InvalidEnumValueError,
    InvalidIdError,
    MultiComponentNotUniqueError,
    NotBetweenError,
    NotBetweenOrAtError,
    NotBooleanError,
    NotGreaterOrEqualError,
    NotGreaterThanError,
    NotLessOrEqualError,
    NotLessThanError,
    NotUniqueError,
    SameValueError,
    TwoValuesZeroError,
)
from power_grid_model.validation.rules import (
    all_between,
    all_between_or_at,
    all_boolean,
    all_cross_unique,
    all_finite,
    all_greater_or_equal,
    all_greater_than,
    all_greater_than_or_equal_to_zero,
    all_greater_than_zero,
    all_less_or_equal,
    all_less_than,
    all_not_two_values_equal,
    all_not_two_values_zero,
    all_unique,
    all_valid_clocks,
    all_valid_enum_values,
    all_valid_ids,
    none_match_comparison,
    none_missing,
)


def test_all_greater_than_zero():
    valid = {
        "test": np.array(
            [(1, 0.1), (2, 0.2), (3, 0.3), (4, np.nan), (5, np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_greater_than_zero(valid, "test", "value")
    assert not errors

    invalid = {"test": np.array([(1, 0.1), (2, 0.0), (3, -0.3), (4, -np.inf)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_greater_than_zero(invalid, "test", "value")
    assert len(errors) == 1
    assert NotGreaterThanError("test", "value", [2, 3, 4], ref_value=0) in errors


def test_all_greater_than_or_equal_to_zero():
    valid = {
        "test": np.array(
            [(1, 0.1), (2, 0.2), (3, 0.3), (4, np.nan), (5, np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_greater_than_or_equal_to_zero(valid, "test", "value")
    assert not errors

    invalid = {"test": np.array([(1, 0.1), (2, 0.0), (3, -0.3), (4, -np.inf)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_greater_than_or_equal_to_zero(invalid, "test", "value")
    assert len(errors) == 1
    assert NotGreaterOrEqualError("test", "value", [3, 4], ref_value=0) in errors


def test_all_between():
    valid = {"test": np.array([(1, -0.4), (2, -0.2), (3, 0.4), (4, np.nan)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_between(valid, "test", "value", -0.5, 0.5)
    assert not errors

    valid = {"test": np.array([(1, -0.4), (2, -0.2), (3, 0.4), (4, np.nan)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_between(valid, "test", "value", 0.5, -0.5)
    assert not errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.5), (4, np.inf), (5, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_between(invalid, "test", "value", -0.5, 0.5)
    assert len(errors) == 1
    assert NotBetweenError("test", "value", [1, 3, 4, 5], (-0.5, 0.5)) in errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.5), (4, np.inf), (5, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_between(invalid, "test", "value", 0.5, -0.5)
    assert len(errors) == 1
    assert NotBetweenError("test", "value", [1, 3, 4, 5], (0.5, -0.5)) in errors


def test_all_between_or_at():
    valid = {
        "test": np.array([(1, -0.2), (2, -0.1), (3, 0.1), (4, 0.2), (5, np.nan)], dtype=[("id", "i4"), ("value", "f8")])
    }
    errors = all_between_or_at(valid, "test", "value", -0.2, 0.2)
    assert not errors

    valid = {
        "test": np.array([(1, -0.2), (2, -0.1), (3, 0.1), (4, 0.2), (5, np.nan)], dtype=[("id", "i4"), ("value", "f8")])
    }
    errors = all_between_or_at(valid, "test", "value", 0.2, -0.2)
    assert not errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.2), (4, 0.5), (5, np.inf), (6, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_between_or_at(invalid, "test", "value", -0.2, 0.2)
    assert len(errors) == 1
    assert NotBetweenOrAtError("test", "value", [1, 4, 5, 6], (-0.2, 0.2)) in errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.2), (4, 0.5), (5, np.inf), (6, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_between_or_at(invalid, "test", "value", 0.2, -0.2)
    assert len(errors) == 1
    assert NotBetweenOrAtError("test", "value", [1, 4, 5, 6], (0.2, -0.2)) in errors


def test_all_greater_than():
    valid = {"test": np.array([(1, 0.0), (2, 0.2), (3, 0.5), (4, np.nan)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_greater_than(valid, "test", "value", -0.2)
    assert not errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.5), (4, np.inf), (5, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_greater_than(invalid, "test", "value", -0.2)
    assert len(errors) == 1
    assert NotGreaterThanError("test", "value", [1, 2, 5], -0.2) in errors


def test_all_greater_or_equal():
    valid = {"test": np.array([(1, -0.2), (2, 0.2), (3, 0.5), (4, np.nan)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_greater_or_equal(valid, "test", "value", -0.2)
    assert not errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.5), (4, np.inf), (5, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_greater_or_equal(invalid, "test", "value", -0.2)
    assert len(errors) == 1
    assert NotGreaterOrEqualError("test", "value", [1, 5], -0.2) in errors


def test_all_less_than():
    invalid = {"test": np.array([(1, -0.5), (2, -0.4), (3, -0.3), (4, np.nan)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_less_than(invalid, "test", "value", -0.2)
    assert not errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.5), (4, np.inf), (5, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_less_than(invalid, "test", "value", -0.2)
    assert len(errors) == 1
    assert NotLessThanError("test", "value", [2, 3, 4], -0.2) in errors


def test_all_less_or_equal():
    valid = {"test": np.array([(1, -0.5), (2, -0.3), (3, -0.2), (4, np.nan)], dtype=[("id", "i4"), ("value", "f8")])}
    errors = all_less_or_equal(valid, "test", "value", -0.2)
    assert not errors

    invalid = {
        "test": np.array(
            [(1, -0.5), (2, -0.2), (3, 0.5), (4, np.inf), (5, -np.inf)], dtype=[("id", "i4"), ("value", "f8")]
        )
    }
    errors = all_less_or_equal(invalid, "test", "value", -0.2)
    assert len(errors) == 1
    assert NotLessOrEqualError("test", "value", [3, 4], -0.2) in errors


def test_none_match_comparison():
    data = {
        "test": np.array(
            [(1, 0.1), (2, 0.2), (3, 0.3), (4, np.nan), (5, np.inf), (6, -np.inf)],
            dtype=[("id", "i4"), ("value", "f8")],
        )
    }
    errors = none_match_comparison(
        data=data, component="test", field="value", compare_fn=np.equal, ref_value=0.2, error=ComparisonError
    )
    assert len(errors) == 1
    assert ComparisonError("test", "value", [2], 0.2) in errors


def test_all_unique():
    valid = {"test": np.array([(333,), (444,), (555,), (666,), (777,)], dtype=[("id", "i4")])}
    errors = all_unique(valid, "test", "id")
    assert not errors

    invalid = {"test": np.array([(333,), (444,), (555,), (444,), (555,), (555,)], dtype=[("id", "i4")])}
    errors = all_unique(invalid, "test", "id")
    assert len(errors) == 1

    # Note that each id occurs as often in the error as it occurred in the 'id' field.
    assert NotUniqueError("test", "id", [444, 444, 555, 555, 555]) in errors


@pytest.mark.parametrize("cross_only", [pytest.param(True, id="cross_only"), pytest.param(False, id="not_cross_only")])
def test_all_cross_unique(cross_only):
    valid = {
        "node": np.array([(1, 0.1), (2, 0.2), (3, 0.3)], dtype=[("id", "i4"), ("foo", "f8")]),
        "line": np.array([(4, 0.4), (5, 0.5), (6, 0.6), (7, 0.7)], dtype=[("id", "i4"), ("bar", "f8")]),
    }
    errors = all_cross_unique(valid, [("node", "foo"), ("line", "bar")], cross_only=cross_only)
    assert not errors

    invalid = {
        "node": np.array([(1, 0.1), (2, 0.2), (3, 0.2)], dtype=[("id", "i4"), ("foo", "f8")]),
        "line": np.array([(4, 0.2), (5, 0.1), (6, 0.3), (7, 0.3)], dtype=[("id", "i4"), ("bar", "f8")]),
    }
    errors = all_cross_unique(invalid, [("node", "foo"), ("line", "bar")], cross_only=cross_only)
    assert len(errors) == 1
    error = errors.pop()
    assert isinstance(error, MultiComponentNotUniqueError)
    assert ("node", "foo") in error.field
    assert ("line", "bar") in error.field
    assert ("node", 1) in error.ids
    assert ("node", 2) in error.ids
    assert ("node", 3) in error.ids
    assert ("line", 4) in error.ids
    assert ("line", 5) in error.ids
    assert "node.foo" in error.field_str
    assert "line.bar" in error.field_str
    assert "node" in error.component_str
    assert "line" in error.component_str

    if cross_only:
        assert ("line", 6) not in error.ids
        assert ("line", 7) not in error.ids
    else:
        assert ("line", 6) in error.ids
        assert ("line", 7) in error.ids


def test_all_valid_enum_values():
    class MyEnum(IntEnum):
        alpha = 2
        bravo = 5

    # TODO replace this hack with some patch to power_grid_meta_data or nan_type
    from power_grid_model import power_grid_meta_data

    power_grid_meta_data["input"]["test"] = {"nans": {"value": -128}}

    valid = {"test": np.array([(1, 2), (2, 5)], dtype=[("id", "i4"), ("value", "i4")])}
    errors = all_valid_enum_values(valid, "test", "value", MyEnum)
    assert not errors

    invalid = {"test": np.array([(1, 2), (2, 4)], dtype=[("id", "i4"), ("value", "i4")])}
    errors = all_valid_enum_values(invalid, "test", "value", MyEnum)
    assert len(errors) == 1
    assert InvalidEnumValueError("test", "value", [2], MyEnum) in errors
    # TODO replace this hack with some patch to power_grid_meta_data or nan_type
    del power_grid_meta_data["input"]["test"]

    # try with a real enum LoadGenType
    # this is a bug in numpy
    from power_grid_model import LoadGenType, initialize_array

    valid = {"sym_load": initialize_array("input", "sym_load", 20)}
    valid["sym_load"]["id"] = np.arange(20)
    valid["sym_load"]["type"] = 0
    errors = all_valid_enum_values(valid, "sym_load", "type", LoadGenType)
    assert not errors


def test_all_valid_ids():
    input_data = {
        "mountain": np.array([(1,), (2,), (3,)], dtype=[("id", "i4")]),
        "planet": np.array([(4,), (5,), (6,)], dtype=[("id", "i4")]),
        "flag": np.array([(7, 1, "m"), (8, 2, "m"), (9, 6, "p")], dtype=[("id", "i4"), ("obj", "i4"), ("type", "U1")]),
    }

    errors = all_valid_ids(input_data, "flag", "obj", ["mountain", "planet"])
    assert not errors

    errors = all_valid_ids(input_data, "flag", "obj", "mountain", type="m")
    assert not errors

    errors = all_valid_ids(input_data, "flag", "obj", "planet", type="p")
    assert not errors

    errors = all_valid_ids(input_data, "flag", "obj", "mountain")
    assert len(errors) == 1
    assert InvalidIdError("flag", "obj", [9], ["mountain"]) in errors

    errors = all_valid_ids(input_data, "flag", "obj", "planet")
    assert len(errors) == 1
    assert InvalidIdError("flag", "obj", [7, 8], ["planet"]) in errors


def test_all_boolean():
    valid = {"test": np.array([(1, 0), (2, 1), (3, 0)], dtype=[("id", "i4"), ("value", "i1")])}
    errors = all_boolean(valid, "test", "value")
    assert not errors

    valid = {"test": np.array([(1, 0), (2, 1), (3, 2)], dtype=[("id", "i4"), ("value", "i1")])}
    errors = all_boolean(valid, "test", "value")
    assert len(errors) == 1
    assert NotBooleanError("test", "value", [3]) in errors


def test_all_not_two_values_zero():
    dtype = [("id", "i4"), ("foo", "f8"), ("bar", "f8")]
    valid = {
        "test": np.array(
            [(1, 0.0, 1.0), (2, 0.2, 0.0), (3, 0.3, 3.0), (4, np.nan, 0.0), (5, np.inf, 0.0), (6, -np.inf, 0.0)],
            dtype=dtype,
        )
    }
    errors = all_not_two_values_zero(valid, "test", "foo", "bar")
    assert not errors

    invalid = {"test": np.array([(1, 0.1, 1.0), (2, 0.0, 0.0), (3, 0.3, 3.0)], dtype=dtype)}
    errors = all_not_two_values_zero(invalid, "test", "foo", "bar")
    assert len(errors) == 1
    assert TwoValuesZeroError("test", ["foo", "bar"], [2]) in errors


def test_all_not_two_values_equal():
    dtype = [("id", "i4"), ("foo", "f8"), ("bar", "f8")]
    valid = {"test": np.array([(1, 0.1, 1.0), (2, 0.2, 2.0), (3, 0.3, 3.0), (4, np.nan, np.nan)], dtype=dtype)}
    errors = all_not_two_values_equal(valid, "test", "foo", "bar")
    assert not errors

    invalid = {"test": np.array([(1, 0.1, 1.0), (2, 2.0, 2.0), (3, 0.3, 3.0), (4, np.inf, np.inf)], dtype=dtype)}
    errors = all_not_two_values_equal(invalid, "test", "foo", "bar")
    assert len(errors) == 1
    assert SameValueError("test", ["foo", "bar"], [2, 4]) in errors


def test_all_finite():
    dfoo = [("id", "i4"), ("foo", "f8")]
    dbar = [("id", "i4"), ("bar", "f8")]
    valid = {
        "foo_test": np.array([(1, 0.1), (2, 0.2), (3, np.nan)], dtype=dfoo),
        "bar_test": np.array([(4, 0.4), (5, -0.5), (6, np.nan)], dtype=dbar),
    }
    errors = all_finite(valid)
    assert not errors

    dfoo = [("id", "i4"), ("foo", "f8")]
    dbar = [("id", "i4"), ("bar", "f8")]
    invalid = {
        "foo_test": np.array([(1, 0.1), (2, np.inf), (3, -0.3)], dtype=dfoo),
        "bar_test": np.array([(4, 0.4), (5, 0.5), (6, -np.inf)], dtype=dbar),
    }
    errors = all_finite(invalid)
    assert len(errors) == 2
    assert InfinityError("foo_test", "foo", [2]) in errors
    assert InfinityError("bar_test", "bar", [6]) in errors


@pytest.mark.skip("No unit tests available for none_missing")
def test_none_missing():
    raise NotImplementedError(f"Unit test for {none_missing}")


@pytest.mark.skip("No unit tests available for all_valid_clocks")
def test_all_valid_clocks():
    raise NotImplementedError(f"Unit test for {all_valid_clocks}")
