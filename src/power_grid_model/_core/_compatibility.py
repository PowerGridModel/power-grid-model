# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Module containing compatibility utilities for the v1 to v2 migration.
"""

import inspect
import warnings
from collections.abc import Callable
from typing import Any


def bind_v2_arguments_with_v1_compatibility(function: Callable[..., Any], *args: Any, **kwargs: Any) -> dict[str, Any]:
    """Bind using the v2 signature, allow v1 positional keyword-only arguments with a warning, and return values."""
    signature = inspect.signature(function)
    v2_signature = signature.replace(
        parameters=[
            parameter
            for parameter in signature.parameters.values()
            if parameter.kind != inspect.Parameter.VAR_POSITIONAL
        ]
    )

    try:
        bound_arguments = v2_signature.bind(*args, **kwargs)
    except TypeError as v2_error:
        v1_signature = v2_signature.replace(
            parameters=[
                parameter.replace(kind=inspect.Parameter.POSITIONAL_OR_KEYWORD)
                if parameter.kind == inspect.Parameter.KEYWORD_ONLY
                else parameter
                for parameter in v2_signature.parameters.values()
            ]
        )
        try:
            bound_arguments = v1_signature.bind(*args, **kwargs)
        except TypeError:
            raise v2_error from None
        warnings.warn(
            "The following positional arguments will become keyword arguments in v2. "
            "Please migrate to keyword arguments.",
            DeprecationWarning,
            stacklevel=3,
        )

    bound_arguments.apply_defaults()
    return bound_arguments.arguments
