# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Module containing compatibility utilities for the v1 to v2 migration.
"""

import functools
import inspect
import warnings
from collections.abc import Callable
from typing import Any

_POSITIONAL_KINDS = (inspect.Parameter.POSITIONAL_ONLY, inspect.Parameter.POSITIONAL_OR_KEYWORD)


def deprecate_positional_arguments[**P, R](function: Callable[P, R]) -> Callable[..., R]:
    """Accept keyword-only arguments of ``function`` positionally, with a DeprecationWarning (v1 compatibility).

    ``function`` must have its v2 signature. The returned callable accepts the v1 signature, in which all
    keyword-only parameters are also accepted positionally, in declaration order.

    Declare the v2 and legacy v1 signatures as ``@overload``s above the decorated implementation, so that static
    type checkers accept both. The return type is intentionally erased to ``Any``-compatible ``Callable[..., R]`` so
    that type checkers do not check the implementation against the legacy overloads.
    """
    v2_signature = inspect.signature(function)
    parameters = list(v2_signature.parameters.values())
    if any(parameter.kind == inspect.Parameter.VAR_POSITIONAL for parameter in parameters):
        raise TypeError("deprecate_positional_arguments does not support callables with *args.")

    n_v2_positional = sum(parameter.kind in _POSITIONAL_KINDS for parameter in parameters)
    migrated_names = [parameter.name for parameter in parameters if parameter.kind == inspect.Parameter.KEYWORD_ONLY]
    if not migrated_names:
        raise TypeError("deprecate_positional_arguments requires at least one keyword-only parameter.")

    v1_signature = v2_signature.replace(
        parameters=[
            parameter.replace(kind=inspect.Parameter.POSITIONAL_OR_KEYWORD)
            if parameter.kind == inspect.Parameter.KEYWORD_ONLY
            else parameter
            for parameter in parameters
        ]
    )

    @functools.wraps(function)
    def wrapper(*args: Any, **kwargs: Any) -> R:
        n_extra = len(args) - n_v2_positional
        if n_extra <= 0 or n_extra > len(migrated_names):
            # v2 call, or an invalid call for which Python produces the native error message
            return function(*args, **kwargs)

        names = migrated_names[:n_extra]
        for name in names:
            if name in kwargs:
                raise TypeError(f"{function.__qualname__}() got multiple values for argument '{name}'")
        warnings.warn(
            f"Passing {', '.join(repr(name) for name in names)} positionally to {function.__qualname__}() is "
            "deprecated and will raise a TypeError in power-grid-model v2. Pass them as keyword arguments instead.",
            DeprecationWarning,
            stacklevel=2,
        )
        return function(*args[:n_v2_positional], **dict(zip(names, args[n_v2_positional:])), **kwargs)

    # runtime introspection (inspect.signature, IDEs, docs) keeps showing the v1-compatible signature
    wrapper.__signature__ = v1_signature  # type: ignore[attr-defined]
    return wrapper
