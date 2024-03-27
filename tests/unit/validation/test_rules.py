# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from enum import IntEnum

import numpy as np
import pytest

from power_grid_model import LoadGenType, initialize_array
from power_grid_model.enum import FaultPhase, FaultType
from power_grid_model.validation.errors import (
    ComparisonError,
    FaultPhaseError,
    InfinityError,
    InvalidEnumValueError,
    InvalidIdError,
    MultiComponentNotUniqueError,
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
from power_grid_model.validation.rules import (
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


@pytest.mark.skip("No unit tests available for none_missing")
def test_none_missing():
    raise NotImplementedError(f"Unit test for {none_missing}")


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
