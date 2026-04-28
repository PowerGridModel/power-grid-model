# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import warnings
from enum import IntEnum, nonmember

import pytest

from power_grid_model._core.enum import _DeprecationAwareEnumMeta as DeprecationAwareEnumMeta


class Foo(IntEnum, metaclass=DeprecationAwareEnumMeta):
    _deprecated_members = nonmember(
        {
            "baz": "Foo.baz is deprecated. Use Foo.bar instead.",
        }
    )

    bar = 0
    baz = 1


def test_deprecation_aware_enum_meta__non_deprecated_attribute():
    warnings.simplefilter("error")
    assert Foo.bar == Foo(0)
    assert Foo["bar"] == Foo(0)
    assert Foo[0] == Foo(0)
    assert Foo(0) == Foo.bar


def test_deprecation_aware_enum_meta__deprecated_attribute():
    with pytest.deprecated_call():
        Foo.baz

    with pytest.deprecated_call():
        Foo["baz"]

    with pytest.deprecated_call():
        Foo[1]

    with pytest.deprecated_call():
        Foo(1)

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        assert Foo["baz"] == Foo.baz
        assert Foo[1] == Foo.baz
        assert Foo(1) == Foo.baz
        assert getattr(Foo, "baz") == Foo.baz


def test_deprecation_aware_enum_meta__unknown_name_raises():
    with pytest.raises(KeyError):
        Foo["nonexistent"]


def test_deprecation_aware_enum_meta__unknown_value_raises():
    with pytest.raises(KeyError):
        Foo[99]


def test_deprecation_aware_enum_meta__loop():
    with pytest.deprecated_call():
        for member in Foo:
            assert Foo[member.name] == member
            assert Foo[member.value] == member
            assert getattr(Foo, member.name) == member
