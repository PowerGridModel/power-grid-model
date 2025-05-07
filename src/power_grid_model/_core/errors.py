# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains error classes for library-internal use.
"""


import numpy as np


class PowerGridError(RuntimeError):
    """Generic power grid error."""


class PowerGridBatchError(PowerGridError):
    """Error occurs in batch calculation."""

    failed_scenarios: np.ndarray
    succeeded_scenarios: np.ndarray
    error_messages: list[str]
    errors: list[PowerGridError]


class InvalidArguments(PowerGridError):
    """A (combination of) input arguments is not valid."""


class MissingCaseForEnumError(InvalidArguments):
    """An enum value is not covered in a for loop.

    This usually happens when an invalid combination of (enum) settings is provided."""


class ConflictVoltage(PowerGridError):
    """There is a confliciting voltage"""


class InvalidBranch(PowerGridError):
    """A branch is invalid."""


class InvalidBranch3(PowerGridError):
    """A branch3 is invalid."""


class InvalidTransformerClock(PowerGridError):
    """Invalid transformer clock found."""


class SparseMatrixError(PowerGridError):
    """Attempting to invert a non-invertible matrix."""


class NotObservableError(SparseMatrixError):
    """Attempting to solve a non-observable system."""


class IterationDiverge(PowerGridError):
    """Unable to iteratively converge to an optimum within the set number of iterations and precision."""


class MaxIterationReached(IterationDiverge):
    """Maximum number of iterations reached."""


class InvalidID(PowerGridError):
    """An ID is invalid."""


class IDNotFound(InvalidID):
    """A reference to a non-existent ID was provided."""


class InvalidMeasuredObject(InvalidID):
    """A provided measured object is invalid."""


class InvalidRegulatedObject(InvalidID):
    """A provided regulated object is invalid."""


class IDWrongType(InvalidID):
    """A referenced ID points to a component that cannot be referenced here."""


class ConflictID(InvalidID):
    """Conflicting IDs found."""


class InvalidCalculationMethod(PowerGridError):
    """Invalid calculation method provided."""


class AutomaticTapCalculationError(PowerGridError):
    """Automatic tap changer with tap at LV side is unsupported for automatic tap changing calculation."""


class AutomaticTapInputError(PowerGridError):
    """Automatic tap changer has invalid configuration."""


class ConflictingAngleMeasurementType(PowerGridError):
    """Conflicting angle measurement types found."""


class InvalidShortCircuitPhaseOrType(PowerGridError):
    """Invalid (combination of) short circuit types and phase(s) provided."""


class PowerGridSerializationError(PowerGridError):
    """Error occurs during (de-)serialization."""


class PowerGridDatasetError(PowerGridError):
    """Error occurs during dataset handling."""


class PowerGridNotImplementedError(PowerGridError):
    """The functionality is either not supported or not yet implemented."""


class PowerGridUnreachableHitError(PowerGridError):
    """Supposedly unreachable code was hit.

    This usually means a failed assumption and may be caused by a bug in the PGM library."""
