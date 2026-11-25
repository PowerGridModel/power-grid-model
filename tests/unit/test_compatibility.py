# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import inspect
import warnings
from pathlib import Path
from typing import Any, Literal, assert_type, overload

import pytest

from power_grid_model._core._compatibility import bind_v2_arguments_with_v1_compatibility

_UNSET = object()


def custom_free_function(shared_argument, *args, migrated_argument=_UNSET):
    common_kwargs = {} if migrated_argument is _UNSET else {"migrated_argument": migrated_argument}
    arguments = bind_v2_arguments_with_v1_compatibility(custom_free_function, shared_argument, *args, **common_kwargs)
    return arguments["migrated_argument"]


@overload
def custom_free_function_with_overloads(shape: int, *, migrated: str = "default") -> tuple[Literal["single"], str]: ...
@overload
def custom_free_function_with_overloads(
    shape: tuple[int, int], *, migrated: str = "default"
) -> tuple[Literal["batch"], str]: ...
@overload  # legacy overload
def custom_free_function_with_overloads(shape: int, migrated: str) -> tuple[Literal["single"], str]: ...
@overload  # legacy overload
def custom_free_function_with_overloads(shape: tuple[int, int], migrated: str) -> tuple[Literal["batch"], str]: ...
def custom_free_function_with_overloads(
    shape: int | tuple[int, int], *args: Any, migrated: Any = _UNSET
) -> tuple[str, str]:
    common_kwargs = {} if migrated is _UNSET else {"migrated": migrated}
    arguments = bind_v2_arguments_with_v1_compatibility(
        custom_free_function_with_overloads, shape, *args, **common_kwargs
    )
    migrated_value = arguments["migrated"]
    if migrated_value is _UNSET:
        migrated_value = "default"
    return ("single" if isinstance(shape, int) else "batch", migrated_value)


class CustomCompatibilityClass:
    def __init__(
        self,
        shared_argument,
        *args,
        migrated_argument=_UNSET,
        second_migrated_argument=_UNSET,
    ):
        common_kwargs = {}
        if migrated_argument is not _UNSET:
            common_kwargs["migrated_argument"] = migrated_argument
        if second_migrated_argument is not _UNSET:
            common_kwargs["second_migrated_argument"] = second_migrated_argument
        arguments = bind_v2_arguments_with_v1_compatibility(
            CustomCompatibilityClass.__init__, self, shared_argument, *args, **common_kwargs
        )
        self.migrated_argument = arguments["migrated_argument"]
        self.second_migrated_argument = arguments["second_migrated_argument"]

    def custom_method(self, shared_argument, *args, migrated_argument=_UNSET):
        common_kwargs = {} if migrated_argument is _UNSET else {"migrated_argument": migrated_argument}
        arguments = bind_v2_arguments_with_v1_compatibility(
            CustomCompatibilityClass.custom_method, self, shared_argument, *args, **common_kwargs
        )
        return arguments["migrated_argument"]


def test_v1_call_warns_once_at_the_user_callsite():
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always", DeprecationWarning)
        callsite_line = inspect.currentframe().f_lineno + 1
        instance = CustomCompatibilityClass("shared", "legacy")

    assert len(caught) == 1
    warning = caught[0]
    assert warning.category is DeprecationWarning
    assert warning.message.args
    assert Path(warning.filename).resolve() == Path(__file__).resolve()
    assert warning.lineno == callsite_line
    assert instance.migrated_argument == "legacy"
    assert instance.second_migrated_argument is _UNSET


def test_v2_call_uses_only_keywords_without_warning_or_error():
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        instance = CustomCompatibilityClass(
            shared_argument="shared",
            migrated_argument="new",
            second_migrated_argument="newer",
        )

    assert not caught
    assert instance.migrated_argument == "new"
    assert instance.second_migrated_argument == "newer"


@pytest.mark.parametrize(
    ("args", "kwargs", "expected_values"),
    [
        (("shared", "legacy-first", "legacy-second"), {}, ("legacy-first", "legacy-second")),
        (
            ("shared", "legacy-first"),
            {"second_migrated_argument": "keyword-second"},
            ("legacy-first", "keyword-second"),
        ),
    ],
)
def test_v1_migrated_values_bind_from_different_argument_shapes(args, kwargs, expected_values):
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always", DeprecationWarning)
        instance = CustomCompatibilityClass(*args, **kwargs)

    assert len(caught) == 1
    assert caught[0].category is DeprecationWarning
    assert (instance.migrated_argument, instance.second_migrated_argument) == expected_values


def test_invalid_call_raises_v2_positional_argument_error():
    with pytest.raises(
        TypeError,
        match="too many positional arguments",
    ):
        CustomCompatibilityClass("shared", "legacy-first", "legacy-second", "extra")


def test_free_function_compatibility_modes():
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always", DeprecationWarning)
        callsite_line = inspect.currentframe().f_lineno + 1
        migrated_argument = custom_free_function("shared", "legacy")

    assert len(caught) == 1
    assert caught[0].category is DeprecationWarning
    assert caught[0].lineno == callsite_line
    assert migrated_argument == "legacy"

    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        migrated_argument = custom_free_function("shared", migrated_argument="new")

    assert not caught
    assert migrated_argument == "new"

    with pytest.raises(TypeError, match="too many positional arguments"):
        custom_free_function("shared", "legacy", "extra")


def test_free_function_overloads_and_v1_migration() -> None:
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        single = custom_free_function_with_overloads(shape=1, migrated="keyword")
        batch = custom_free_function_with_overloads(shape=(2, 3), migrated="keyword")

    assert not caught
    assert_type(single, tuple[Literal["single"], str])
    assert_type(batch, tuple[Literal["batch"], str])
    assert single == ("single", "keyword")
    assert batch == ("batch", "keyword")

    with pytest.warns(DeprecationWarning, match="Please migrate to keyword arguments"):
        legacy_single = custom_free_function_with_overloads(1, "legacy")  # type: ignore[deprecated]

    with pytest.warns(DeprecationWarning, match="Please migrate to keyword arguments"):
        legacy_batch = custom_free_function_with_overloads((2, 3), "legacy")  # type: ignore[deprecated]

    assert_type(legacy_single, tuple[Literal["single"], str])
    assert_type(legacy_batch, tuple[Literal["batch"], str])
    assert legacy_single == ("single", "legacy")
    assert legacy_batch == ("batch", "legacy")


def test_non_init_method_compatibility_modes():
    instance = CustomCompatibilityClass("shared", migrated_argument="new")

    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always", DeprecationWarning)
        callsite_line = inspect.currentframe().f_lineno + 1
        migrated_argument = instance.custom_method("shared", "legacy")

    assert len(caught) == 1
    assert caught[0].category is DeprecationWarning
    assert caught[0].lineno == callsite_line
    assert migrated_argument == "legacy"

    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        migrated_argument = instance.custom_method("shared", migrated_argument="new")

    assert not caught
    assert migrated_argument == "new"

    with pytest.raises(TypeError, match="too many positional arguments"):
        instance.custom_method("shared", "legacy", "extra")
