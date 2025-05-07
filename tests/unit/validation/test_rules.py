# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from unittest import mock

import numpy as np
import pytest

from power_grid_model import ComponentType, LoadGenType, initialize_array, power_grid_meta_data
from power_grid_model._core.dataset_definitions import ComponentTypeLike
from power_grid_model._core.utils import compatibility_convert_row_columnar_dataset
from power_grid_model.enum import Branch3Side, BranchSide, FaultPhase, FaultType
from power_grid_model.validation._rules import (
    all_between,
    all_between_or_at,
    all_boolean,
    all_cross_unique,
    all_enabled_identical,
    all_finite,
    all_greater_or_equal,
    all_greater_than,
    all_greater_than_or_equal_to_zero,
    all_greater_than_zero,
    all_identical,
    all_less_or_equal,
    all_less_than,
    all_not_two_values_equal,
    all_not_two_values_zero,
    all_unique,
    all_valid_clocks,
    all_valid_enum_values,
    all_valid_fault_phases,
    all_valid_ids,
    no_strict_subset_missing,
    none_match_comparison,
    none_missing,
    not_all_missing,
)
from power_grid_model.validation.errors import (
    ComparisonError,
    FaultPhaseError,
    InfinityError,
    InvalidEnumValueError,
    InvalidIdError,
    MissingValueError,
    MultiComponentNotUniqueError,
    MultiFieldValidationError,
    NotBetweenError,
    NotBetweenOrAtError,
    NotBooleanError,
    NotGreaterOrEqualError,
    NotGreaterThanError,
    NotIdenticalError,
    NotLessOrEqualError,
    NotLessThanError,
    NotUniqueError,
    SameValueError,
    TwoValuesZeroError,
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

    nan_value = power_grid_meta_data["input"]["transformer"].nans["tap_pos"]
    transformer_array = initialize_array("input", "transformer", 3)
    transformer_array["id"] = [1, 2, 3]
    transformer_array["tap_pos"] = [nan_value, 1, nan_value]
    transformer_array["tap_nom"] = [2, 1, nan_value]
    valid = {"transformer": transformer_array}
    errors = all_between_or_at(valid, "transformer", "tap_pos", 0, 2, transformer_array["tap_nom"], 0)
    assert not errors

    errors = all_between_or_at(valid, "transformer", "tap_pos", 1, 2, transformer_array["tap_nom"], 0)
    assert len(errors) == 1
    assert NotBetweenOrAtError("transformer", "tap_pos", [3], (1, 2)) in errors


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

    nan_value = power_grid_meta_data["input"]["transformer"].nans["tap_pos"]
    transformer_array = initialize_array("input", "transformer", 3)
    transformer_array["id"] = [1, 2, 3]
    transformer_array["tap_pos"] = [nan_value, 0, nan_value]
    transformer_array["tap_nom"] = [1, 1, nan_value]
    valid = {"transformer": transformer_array}

    errors = none_match_comparison(
        valid, "transformer", "tap_pos", np.equal, 0, ComparisonError, transformer_array["tap_nom"], 0
    )
    assert len(errors) == 1
    assert ComparisonError("transformer", "tap_pos", [2, 3], 0) in errors


def test_all_identical():
    dtype = [("id", "i4"), ("foo", "i4")]
    data = {
        "bar": np.array([(0, 10), (1, 10), (2, 10)], dtype=dtype),
        "baz": np.array([(3, 11), (4, 12), (5, 12)], dtype=dtype),
    }
    errors = all_identical(data, "bar", "foo")
    assert not errors

    errors = all_identical(data, "baz", "foo")
    assert len(errors) == 1
    assert NotIdenticalError("baz", "foo", ids=[3, 4, 5], values=[11, 12, 12]) in errors


def test_all_enabled_identical():
    dtype = [("id", "i4"), ("status", "i4"), ("foo", "i4")]
    data = {
        "bar": np.array([(0, 1, 10), (1, 1, 10), (2, 0, 10), (3, 0, 11), (4, 0, 12)], dtype=dtype),
        "baz": np.array([(5, 1, 14), (6, 1, 14), (7, 0, 14), (8, 1, 15), (9, 0, 16)], dtype=dtype),
    }
    errors = all_enabled_identical(data, "bar", "foo", "status")
    assert not errors

    errors = all_enabled_identical(data, "baz", "foo", "status")
    assert len(errors) == 1
    assert NotIdenticalError("baz", "foo", ids=[5, 6, 8], values=[14, 14, 15]) in errors


def test_all_unique():
    valid = {"test": np.array([(333,), (444,), (555,), (666,), (777,)], dtype=[("id", "i4")])}
    errors = all_unique(valid, "test", "id")
    assert not errors

    invalid = {"test": np.array([(333,), (444,), (555,), (444,), (555,), (555,)], dtype=[("id", "i4")])}
    errors = all_unique(invalid, "test", "id")
    assert len(errors) == 1

    # Note that each id occurs as often in the error as it occurred in the 'id' field.
    assert NotUniqueError("test", "id", [444, 444, 555, 555, 555]) in errors

    invalid = {
        "test": np.array(
            [(1, 333), (2, 444), (3, 555), (4, 444), (5, 555), (6, 555)], dtype=[("id", "i4"), ("other", "i4")]
        )
    }
    errors = all_unique(invalid, "test", "other")
    assert len(errors) == 1

    # Note that each id with duplicate values in the field is represented.
    assert NotUniqueError("test", "other", [2, 3, 4, 5, 6]) in errors


@pytest.mark.parametrize("cross_only", [pytest.param(True, id="cross_only"), pytest.param(False, id="not_cross_only")])
def test_all_cross_unique(cross_only):
    valid = {
        "node": np.array([(1, 0.1), (2, 0.2), (3, 0.3)], dtype=[("id", "i4"), ("foo", "f8")]),
        "line": np.array([(4, 0.4), (5, 0.5), (6, 0.6), (7, 0.7)], dtype=[("id", "i4"), ("bar", "f8")]),
    }
    errors = all_cross_unique(valid, [(ComponentType.node, "foo"), (ComponentType.line, "bar")], cross_only=cross_only)
    assert not errors

    invalid = {
        "node": np.array([(1, 0.1), (2, 0.2), (3, 0.2)], dtype=[("id", "i4"), ("foo", "f8")]),
        "line": np.array([(4, 0.2), (5, 0.1), (6, 0.3), (7, 0.3)], dtype=[("id", "i4"), ("bar", "f8")]),
    }
    errors = all_cross_unique(
        invalid, [(ComponentType.node, "foo"), (ComponentType.line, "bar")], cross_only=cross_only
    )
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
    valid_load = initialize_array("input", "sym_load", 2)
    valid_load["id"] = [1, 2]
    valid_load["type"] = LoadGenType.const_power
    valid = {"sym_load": valid_load}
    errors = all_valid_enum_values(valid, "sym_load", "type", LoadGenType)
    assert not errors

    invalid_load = initialize_array("input", "sym_load", 2)
    invalid_load["id"] = [1, 2]
    invalid_load["type"] = [LoadGenType.const_power, 5]
    invalid = {"sym_load": invalid_load}
    errors = all_valid_enum_values(invalid, "sym_load", "type", LoadGenType)
    assert len(errors) == 1
    assert InvalidEnumValueError("sym_load", "type", [2], LoadGenType) in errors

    valid = {"sym_load": initialize_array("input", "sym_load", 20)}
    valid["sym_load"]["id"] = np.arange(20)
    valid["sym_load"]["type"] = 0
    errors = all_valid_enum_values(valid, "sym_load", "type", LoadGenType)
    assert not errors

    valid = {"transformer_tap_regulator": initialize_array("input", "transformer_tap_regulator", 5)}
    valid["transformer_tap_regulator"]["id"] = np.arange(5)
    valid["transformer_tap_regulator"]["control_side"] = np.arange(-1, 4)
    errors = all_valid_enum_values(valid, "transformer_tap_regulator", "control_side", [BranchSide, Branch3Side])
    assert len(errors) == 1
    assert (
        InvalidEnumValueError("transformer_tap_regulator", "control_side", [0, 4], [BranchSide, Branch3Side]) in errors
    )


def test_all_valid_ids():
    # This data is for testing purpuse
    # The values in the data do not make sense for a real grid
    node = initialize_array("input", "node", 3)
    node["id"] = [1, 2, 3]
    source = initialize_array("input", "source", 3)
    source["id"] = [4, 5, 6]
    line = initialize_array("input", "line", 3)
    line["id"] = [7, 8, 9]
    line["from_node"] = [1, 2, 6]
    line["to_node"] = [0, 0, 1]

    input_data = {
        "node": node,
        "source": source,
        "line": line,
    }

    errors = all_valid_ids(input_data, "line", "from_node", ["node", "source"])
    assert not errors

    errors = all_valid_ids(input_data, "line", "from_node", "node", to_node=0)
    assert not errors

    errors = all_valid_ids(input_data, "line", "from_node", "source", to_node=1)
    assert not errors

    errors = all_valid_ids(input_data, "line", "from_node", "node")
    assert len(errors) == 1
    assert InvalidIdError("line", "from_node", [9], ["node"]) in errors

    errors = all_valid_ids(input_data, "line", "from_node", "source")
    assert len(errors) == 1
    assert InvalidIdError("line", "from_node", [7, 8], ["source"]) in errors


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
    errors = all_finite(invalid, {"foo_test": ["foo"]})
    assert len(errors) == 1
    assert InfinityError("foo_test", "foo", [2]) not in errors
    assert InfinityError("bar_test", "bar", [6]) in errors


def test_none_missing():
    dfoo = [("id", "i4"), ("foo", "f8"), ("bar", "(3,)f8"), ("baz", "i4"), ("bla", "i1"), ("ok", "i1")]
    dbar = [("id", "i4"), ("foobar", "f8")]

    def _mock_nan_type(component: ComponentTypeLike, field: str):
        return {
            "foo_test": {
                "id": np.iinfo("i4").min,
                "foo": np.nan,
                "bar": np.nan,
                "baz": np.iinfo("i4").min,
                "bla": np.iinfo("i1").min,
                "ok": -1,
            },
            "bar_test": {"id": np.iinfo("i4").min, "foobar": np.nan},
        }[component][field]

    with mock.patch("power_grid_model.validation._rules._nan_type", _mock_nan_type):
        valid = {
            "foo_test": np.array(
                [
                    (1, 3.1, (4.2, 4.3, 4.4), 1, 6, 0),
                    (2, 5.2, (3.3, 3.4, 3.5), 2, 7, 0),
                    (3, 7.3, (8.4, 8.5, 8.6), 3, 8, 0),
                ],
                dtype=dfoo,
            ),
            "bar_test": np.array([(4, 0.4), (5, 0.5)], dtype=dbar),
        }
        errors = none_missing(data=valid, component="foo_test", fields=["foo", "bar", "baz"])
        assert len(errors) == 0

        invalid = {
            "foo_test": np.array(
                [
                    (1, np.nan, (np.nan, np.nan, np.nan), np.iinfo("i4").min, np.iinfo("i1").min, 0),
                    (2, np.nan, (4.2, 4.3, 4.4), 3, 7, 0),
                    (3, 7.3, (np.nan, np.nan, np.nan), 5, 8, 0),
                    (4, 8.3, (8.4, 8.5, 8.6), np.iinfo("i4").min, 9, 0),
                    (5, 9.3, (9.4, 9.5, 9.6), 6, np.iinfo("i1").min, 0),
                    (6, 10.3, (10.4, 10.5, 10.6), 7, 11, 0),
                ],
                dtype=dfoo,
            ),
            "bar_test": np.array([(4, 0.4), (5, np.nan)], dtype=dbar),
        }

        errors = none_missing(data=invalid, component="foo_test", fields="foo")
        assert len(errors) == 1
        assert errors == [MissingValueError("foo_test", "foo", [1, 2])]

        errors = none_missing(data=invalid, component="foo_test", fields="bar")
        assert len(errors) == 1
        assert errors == [MissingValueError("foo_test", "bar", [1, 3])]

        errors = none_missing(data=invalid, component="foo_test", fields="baz")
        assert len(errors) == 1
        assert errors == [MissingValueError("foo_test", "baz", [1, 4])]

        errors = none_missing(data=invalid, component="foo_test", fields="bla")
        assert len(errors) == 1
        assert errors == [MissingValueError("foo_test", "bla", [1, 5])]

        errors = none_missing(data=invalid, component="foo_test", fields="ok")
        assert len(errors) == 0

        for fields in (("foo", "bar", "baz", "bla", "ok"), ("foo", "bar"), ()):
            errors = none_missing(data=invalid, component="foo_test", fields=fields)
            expected = []
            for field in fields:
                expected += none_missing(data=invalid, component="foo_test", fields=field)
            assert errors == expected

        assert none_missing(
            data={
                "foo_test": {
                    "id": invalid["foo_test"]["id"],
                    "foo": invalid["foo_test"]["foo"],
                    "bar": invalid["foo_test"]["bar"],
                    "baz": invalid["foo_test"]["baz"],
                    "bla": invalid["foo_test"]["bla"],
                    "ok": invalid["foo_test"]["ok"],
                },
                "bar_test": {
                    "id": invalid["bar_test"]["id"],
                    "foobar": invalid["bar_test"]["foobar"],
                },
            },
            component="foo_test",
            fields=("foo", "bar", "baz", "bla", "ok"),
        ) == none_missing(data=invalid, component="foo_test", fields=("foo", "bar", "baz", "bla", "ok"))


def test_no_strict_subset_missing():
    dfoo = [("id", "i4"), ("foo", "f8"), ("bar", "(3,)f8"), ("baz", "i4")]

    def _mock_nan_type(component: ComponentTypeLike, field: str):
        return {
            "foo_test": {"id": np.iinfo("i4").min, "foo": np.nan, "bar": np.nan, "baz": np.iinfo("i4").min},
            "bar_test": {"id": np.iinfo("i4").min, "foobar": np.nan},
        }[component][field]

    with mock.patch("power_grid_model.validation._rules._nan_type", _mock_nan_type):
        valid = {
            "foo_test": np.array(
                [
                    (1, 3.1, (4.2, 4.3, 4.4), 1),
                    (2, np.nan, (np.nan, np.nan, np.nan), np.iinfo("i4").min),
                    (3, 7.3, (8.4, 8.5, 8.6), 3),
                ],
                dtype=dfoo,
            )
        }
        errors = no_strict_subset_missing(valid, ["foo", "bar", "baz"], "foo_test")
        assert len(errors) == 0

        invalid = {
            "foo_test": np.array(
                [
                    (1, np.nan, (np.nan, np.nan, np.nan), np.iinfo("i4").min),
                    (2, np.nan, (4.2, 4.3, 4.4), 3),
                    (3, np.nan, (np.nan, np.nan, np.nan), 1),
                ],
                dtype=dfoo,
            )
        }

        errors = no_strict_subset_missing(invalid, ["foo", "bar", "baz"], "foo_test")
        assert len(errors) == 1
        assert errors == [MultiFieldValidationError("foo_test", ["foo", "bar", "baz"], [2, 3])]

        errors = no_strict_subset_missing(invalid, ["foo", "bar"], "foo_test")
        assert len(errors) == 1
        assert errors == [MultiFieldValidationError("foo_test", ["foo", "bar"], [2])]

        errors = no_strict_subset_missing(invalid, ["bar"], "foo_test")
        assert len(errors) == 0


def test_not_all_missing():
    dfoo = [("id", "i4"), ("foo", "f8"), ("bar", "(3,)f8"), ("baz", "i4")]

    def _mock_nan_type(component: ComponentTypeLike, field: str):
        return {
            "foo_test": {"id": np.iinfo("i4").min, "foo": np.nan, "bar": np.nan, "baz": np.iinfo("i4").min},
            "bar_test": {"id": np.iinfo("i4").min, "foobar": np.nan},
        }[component][field]

    with mock.patch("power_grid_model.validation._rules._nan_type", _mock_nan_type):
        valid = {
            "foo_test": np.array(
                [
                    (1, 3.1, (4.2, 4.3, 4.4), 1),
                    (2, 5.2, (3.3, 3.4, 3.5), 2),
                    (3, 7.3, (8.4, 8.5, 8.6), 3),
                ],
                dtype=dfoo,
            )
        }
        errors = not_all_missing(valid, ["foo", "bar", "baz"], "foo_test")
        assert len(errors) == 0

        invalid = {
            "foo_test": np.array(
                [
                    (1, np.nan, (np.nan, np.nan, np.nan), np.iinfo("i4").min),
                    (2, np.nan, (4.2, 4.3, 4.4), 3),
                    (3, np.nan, (np.nan, np.nan, np.nan), 1),
                ],
                dtype=dfoo,
            )
        }

        errors = not_all_missing(invalid, ["foo", "bar", "baz"], "foo_test")
        assert len(errors) == 1
        assert errors == [MultiFieldValidationError("foo_test", ["foo", "bar", "baz"], [1])]

        with pytest.raises(ValueError) as excinfo:
            not_all_missing(invalid, ["bar"], "foo_test")

        assert excinfo.type == ValueError
        assert (
            str(excinfo.value)
            == "The fields parameter must contain at least 2 fields. Otherwise use the none_missing function."
        )


@pytest.mark.skip("No unit tests available for all_valid_clocks")
def test_all_valid_clocks():
    raise NotImplementedError(f"Unit test for {all_valid_clocks}")


def test_all_valid_fault_phases():
    dtype = [("id", "i4"), ("foo", "i4"), ("bar", "i4"), ("baz", "i4")]
    valid = {
        "fault": np.array(
            [
                (0, FaultType.three_phase, FaultPhase.abc, 100),
                (1, FaultType.three_phase, FaultPhase.default_value, 101),
                (2, FaultType.three_phase, FaultPhase.nan, 102),
                (3, FaultType.single_phase_to_ground, FaultPhase.a, 103),
                (4, FaultType.single_phase_to_ground, FaultPhase.b, 104),
                (5, FaultType.single_phase_to_ground, FaultPhase.c, 105),
                (6, FaultType.single_phase_to_ground, FaultPhase.default_value, 106),
                (7, FaultType.single_phase_to_ground, FaultPhase.nan, 107),
                (8, FaultType.two_phase, FaultPhase.ab, 108),
                (9, FaultType.two_phase, FaultPhase.ac, 109),
                (10, FaultType.two_phase, FaultPhase.bc, 110),
                (11, FaultType.two_phase, FaultPhase.default_value, 111),
                (12, FaultType.two_phase, FaultPhase.nan, 112),
                (13, FaultType.two_phase_to_ground, FaultPhase.ab, 113),
                (14, FaultType.two_phase_to_ground, FaultPhase.ac, 114),
                (15, FaultType.two_phase_to_ground, FaultPhase.bc, 115),
                (16, FaultType.two_phase_to_ground, FaultPhase.default_value, 116),
                (17, FaultType.two_phase_to_ground, FaultPhase.nan, 117),
            ],
            dtype=dtype,
        ),
        "bla": np.array([(18, FaultType.three_phase, FaultPhase.a, 118)], dtype=dtype),
    }
    errors = all_valid_fault_phases(valid, "fault", "foo", "bar")
    assert not errors

    invalid = {
        "fault": np.array(
            [
                (0, FaultType.three_phase, FaultPhase.a, 100),
                (1, FaultType.three_phase, FaultPhase.b, 101),
                (2, FaultType.three_phase, FaultPhase.c, 102),
                (3, FaultType.three_phase, FaultPhase.ab, 103),
                (4, FaultType.three_phase, FaultPhase.ac, 104),
                (5, FaultType.three_phase, FaultPhase.bc, 105),
                (6, FaultType.single_phase_to_ground, FaultPhase.abc, 106),
                (7, FaultType.single_phase_to_ground, FaultPhase.ab, 107),
                (8, FaultType.single_phase_to_ground, FaultPhase.ac, 108),
                (9, FaultType.single_phase_to_ground, FaultPhase.bc, 109),
                (10, FaultType.two_phase, FaultPhase.abc, 110),
                (11, FaultType.two_phase, FaultPhase.a, 111),
                (12, FaultType.two_phase, FaultPhase.b, 112),
                (13, FaultType.two_phase, FaultPhase.c, 113),
                (14, FaultType.two_phase_to_ground, FaultPhase.abc, 114),
                (15, FaultType.two_phase_to_ground, FaultPhase.a, 115),
                (16, FaultType.two_phase_to_ground, FaultPhase.b, 116),
                (17, FaultType.two_phase_to_ground, FaultPhase.c, 117),
                (18, FaultType.nan, FaultPhase.abc, 118),
                (19, FaultType.nan, FaultPhase.a, 119),
                (20, FaultType.nan, FaultPhase.b, 120),
                (21, FaultType.nan, FaultPhase.c, 121),
                (22, FaultType.nan, FaultPhase.ab, 122),
                (23, FaultType.nan, FaultPhase.ac, 123),
                (24, FaultType.nan, FaultPhase.bc, 124),
                (25, FaultType.nan, FaultPhase.default_value, 125),
            ],
            dtype=dtype,
        ),
        "bar": np.array([(26, FaultType.three_phase, FaultPhase.abc, 26)], dtype=dtype),
    }
    errors = all_valid_fault_phases(invalid, "fault", "foo", "bar")
    assert len(errors) == 1
    assert FaultPhaseError("fault", fields=["foo", "bar"], ids=list(range(26))) in errors
