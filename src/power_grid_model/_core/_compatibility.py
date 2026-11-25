# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Module containing compatibility utilities for the v1 to v2 migration.
"""

import inspect
import warnings
from collections.abc import Callable
from typing import Any, get_overloads


def _get_v2_signatures(
    function: Callable[..., Any], v2_overload_indices: tuple[int, ...] | None
) -> tuple[inspect.Signature, ...]:
    overloads = get_overloads(function)
    if v2_overload_indices is None:
        signature = inspect.signature(function)
        parameter_kinds = {parameter.kind for parameter in signature.parameters.values()}
        if inspect.Parameter.VAR_POSITIONAL in parameter_kinds and inspect.Parameter.VAR_KEYWORD in parameter_kinds:
            if overloads:
                raise ValueError(
                    "Specify v2_overload_indices for overloaded callables implemented with *args and **kwargs."
                )
            raise ValueError("Cannot bind a callable implemented only with *args and **kwargs.")
        signatures: tuple[inspect.Signature, ...] = (signature,)
    else:
        if not overloads:
            raise ValueError("v2_overload_indices requires an overloaded callable.")
        if not v2_overload_indices:
            raise ValueError("v2_overload_indices must select at least one overload.")
        if any(index < 0 or index >= len(overloads) for index in v2_overload_indices):
            raise IndexError(f"v2_overload_indices must be between 0 and {len(overloads) - 1}.")
        signatures = tuple(inspect.signature(overloads[index]) for index in v2_overload_indices)

    return tuple(
        signature.replace(
            parameters=[
                parameter
                for parameter in signature.parameters.values()
                if parameter.kind != inspect.Parameter.VAR_POSITIONAL
            ]
        )
        for signature in signatures
    )


def _bind_v2_signatures(
    signatures: tuple[inspect.Signature, ...], args: tuple[Any, ...], kwargs: dict[str, Any]
) -> tuple[inspect.BoundArguments | None, TypeError | None]:
    v2_error: TypeError | None = None
    for signature in signatures:
        try:
            bound_arguments = signature.bind(*args, **kwargs)
        except TypeError as error:
            if v2_error is None:
                v2_error = error
        else:
            return bound_arguments, v2_error
    return None, v2_error


def _bind_v1_compatible_signature(
    signatures: tuple[inspect.Signature, ...], args: tuple[Any, ...], kwargs: dict[str, Any]
) -> inspect.BoundArguments | None:
    for signature in signatures:
        v1_signature = signature.replace(
            parameters=[
                parameter.replace(kind=inspect.Parameter.POSITIONAL_OR_KEYWORD)
                if parameter.kind == inspect.Parameter.KEYWORD_ONLY
                else parameter
                for parameter in signature.parameters.values()
            ]
        )
        try:
            bound_arguments = v1_signature.bind(*args, **kwargs)
        except TypeError:
            continue
        warnings.warn(
            "The following positional arguments will become keyword arguments in v2. "
            "Please migrate to keyword arguments.",
            DeprecationWarning,
            stacklevel=4,
        )
        return bound_arguments
    return None


def bind_v2_arguments_with_v1_compatibility(
    function: Callable[..., Any],
    *args: Any,
    v2_overload_indices: tuple[int, ...] | None = None,
    **kwargs: Any,
) -> dict[str, Any]:
    """Bind v2 arguments, or accept v1 positional arguments with a deprecation warning.

    ``v2_overload_indices`` selects v2 overloads by their order in ``typing.get_overloads``.
    Without it, the implementation signature is used; catch-all implementations require explicit selection.
    """
    v2_signatures = _get_v2_signatures(function, v2_overload_indices)
    bound_arguments, v2_error = _bind_v2_signatures(v2_signatures, args, kwargs)
    if bound_arguments is None:
        bound_arguments = _bind_v1_compatible_signature(v2_signatures, args, kwargs)

    if bound_arguments is None:
        if v2_error is None:
            raise RuntimeError("No v2 signatures were available for argument binding.")
        raise v2_error from None

    bound_arguments.apply_defaults()
    return bound_arguments.arguments
