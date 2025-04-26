# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Error handling
"""

import re

import numpy as np

from power_grid_model._core.errors import (
    AutomaticTapCalculationError,
    AutomaticTapInputError,
    ConflictID,
    ConflictingAngleMeasurementType,
    ConflictVoltage,
    IDNotFound,
    IDWrongType,
    InvalidArguments,
    InvalidBranch,
    InvalidBranch3,
    InvalidCalculationMethod,
    InvalidMeasuredObject,
    InvalidRegulatedObject,
    InvalidShortCircuitPhaseOrType,
    InvalidTransformerClock,
    IterationDiverge,
    MaxIterationReached,
    MissingCaseForEnumError,
    NotObservableError,
    PowerGridBatchError,
    PowerGridDatasetError,
    PowerGridError,
    PowerGridNotImplementedError,
    PowerGridSerializationError,
    PowerGridUnreachableHitError,
    SparseMatrixError,
)
from power_grid_model._core.index_integer import IdxNp
from power_grid_model._core.power_grid_core import power_grid_core as pgc

VALIDATOR_MSG = "\nTry validate_input_data() or validate_batch_data() to validate your data.\n"
# error codes
PGM_NO_ERROR = 0
PGM_REGULAR_ERROR = 1
PGM_BATCH_ERROR = 2
PGM_SERIALIZATION_ERROR = 3

_MISSING_CASE_FOR_ENUM_RE = re.compile(r" is not implemented for (.+) #(-?\d+)!\n")
_INVALID_ARGUMENTS_RE = re.compile(r" is not implemented for ")  # multiple different flavors
_CONFLICT_VOLTAGE_RE = re.compile(
    r"Conflicting voltage for line (-?\d+)\n voltage at from node (-?\d+) is (.*)\n"
    r" voltage at to node (-?\d+) is (.*)\n"
)
_INVALID_BRANCH_RE = re.compile(r"Branch (-?\d+) has the same from- and to-node (-?\d+),\n This is not allowed!\n")
_INVALID_BRANCH3_RE = re.compile(
    r"Branch3 (-?\d+) is connected to the same node at least twice. Node 1\/2\/3: (-?\d+)\/(-?\d+)\/(-?\d+),\n"
    r" This is not allowed!\n"
)
_INVALID_TRANSFORMER_CLOCK_RE = re.compile(r"Invalid clock for transformer (-?\d+), clock (-?\d+)\n")
_SPARSE_MATRIX_ERROR_RE = re.compile(r"Sparse matrix error")  # multiple different flavors
_NOT_OBSERVABLE_ERROR_RE = re.compile(r"Not enough measurements available for state estimation.\n")
_ITERATION_DIVERGE_RE = re.compile(r"Iteration failed to converge")  # potentially multiple different flavors
_MAX_ITERATION_REACHED_RE = re.compile(r"Maximum number of iterations reached")
_CONFLICT_ID_RE = re.compile(r"Conflicting id detected: (-?\d+)\n")
_ID_NOT_FOUND_RE = re.compile(r"The id cannot be found: (-?\d+)\n")
_INVALID_MEASURED_OBJECT_RE = re.compile(r"(\w+) measurement is not supported for object of type (\w+)")
_INVALID_REGULATED_OBJECT_RE = re.compile(
    r"(\w+) regulator is not supported for object "
)  # potentially multiple different flavors
_AUTOMATIC_TAP_CALCULATION_ERROR_RE = re.compile(
    r"Automatic tap changing regulator with tap_side at LV side is not supported. Found at id (-?\d+)\n"
)
_AUTOMATIC_TAP_INPUT_ERROR_RE = re.compile(r"Automatic tap changer has invalid configuration")

_ID_WRONG_TYPE_RE = re.compile(r"Wrong type for object with id (-?\d+)\n")
_CONFLICTING_ANGLE_MEASUREMENT_TYPE_RE = re.compile(r"Conflicting angle measurement type")
_INVALID_CALCULATION_METHOD_RE = re.compile(r"The calculation method is invalid for this calculation!")
_INVALID_SHORT_CIRCUIT_PHASE_OR_TYPE_RE = re.compile(r"short circuit type")  # multiple different flavors
_POWER_GRID_DATASET_ERROR_RE = re.compile(r"Dataset error: ")  # multiple different flavors
_POWER_GRID_UNREACHABLE_HIT_RE = re.compile(r"Unreachable code hit when executing ")  # multiple different flavors
_POWER_GRID_SEARCH_OPT_INCMPT_RE = re.compile(r"Search method is incompatible with optimization strategy: ")
_POWER_GRID_NOT_IMPLEMENTED_ERROR_RE = re.compile(r"The functionality is either not supported or not yet implemented!")

_ERROR_MESSAGE_PATTERNS = {
    _MISSING_CASE_FOR_ENUM_RE: MissingCaseForEnumError,
    _INVALID_ARGUMENTS_RE: InvalidArguments,
    _CONFLICT_VOLTAGE_RE: ConflictVoltage,
    _INVALID_BRANCH_RE: InvalidBranch,
    _INVALID_BRANCH3_RE: InvalidBranch3,
    _INVALID_TRANSFORMER_CLOCK_RE: InvalidTransformerClock,
    _SPARSE_MATRIX_ERROR_RE: SparseMatrixError,
    _NOT_OBSERVABLE_ERROR_RE: NotObservableError,
    _ITERATION_DIVERGE_RE: IterationDiverge,
    _MAX_ITERATION_REACHED_RE: MaxIterationReached,
    _CONFLICT_ID_RE: ConflictID,
    _ID_NOT_FOUND_RE: IDNotFound,
    _INVALID_MEASURED_OBJECT_RE: InvalidMeasuredObject,
    _INVALID_REGULATED_OBJECT_RE: InvalidRegulatedObject,
    _AUTOMATIC_TAP_CALCULATION_ERROR_RE: AutomaticTapCalculationError,
    _AUTOMATIC_TAP_INPUT_ERROR_RE: AutomaticTapInputError,
    _ID_WRONG_TYPE_RE: IDWrongType,
    _CONFLICTING_ANGLE_MEASUREMENT_TYPE_RE: ConflictingAngleMeasurementType,
    _INVALID_CALCULATION_METHOD_RE: InvalidCalculationMethod,
    _INVALID_SHORT_CIRCUIT_PHASE_OR_TYPE_RE: InvalidShortCircuitPhaseOrType,
    _POWER_GRID_DATASET_ERROR_RE: PowerGridDatasetError,
    _POWER_GRID_UNREACHABLE_HIT_RE: PowerGridUnreachableHitError,
    _POWER_GRID_SEARCH_OPT_INCMPT_RE: PowerGridUnreachableHitError,
    _POWER_GRID_NOT_IMPLEMENTED_ERROR_RE: PowerGridNotImplementedError,
}


def _interpret_error(message: str, decode_error: bool = True) -> PowerGridError:
    if decode_error:
        for pattern, type_ in _ERROR_MESSAGE_PATTERNS.items():
            if pattern.search(message) is not None:
                return type_(message)

    return PowerGridError(message)


def find_error(batch_size: int = 1, decode_error: bool = True) -> RuntimeError | None:
    """
    Check if there is an error and return it

    Args:
        batch_size: (int, optional): Size of batch. Defaults to 1.
        decode_error (bool, optional): Decode the error message(s) to derived error classes. Defaults to True

    Returns: error object, can be none

    """
    error_code: int = pgc.error_code()
    if error_code == PGM_NO_ERROR:
        return None
    if error_code == PGM_REGULAR_ERROR:
        error_message = pgc.error_message()
        error_message += VALIDATOR_MSG
        return _interpret_error(error_message, decode_error=decode_error)
    if error_code == PGM_BATCH_ERROR:
        error_message = "There are errors in the batch calculation." + VALIDATOR_MSG
        error = PowerGridBatchError(error_message)
        n_fails = pgc.n_failed_scenarios()
        failed_idxptr = pgc.failed_scenarios()
        failed_msgptr = pgc.batch_errors()
        error.failed_scenarios = np.ctypeslib.as_array(failed_idxptr, shape=(n_fails,)).copy()
        error.error_messages = [failed_msgptr[i].decode() for i in range(n_fails)]  # type: ignore
        error.errors = [_interpret_error(message, decode_error=decode_error) for message in error.error_messages]
        all_scenarios = np.arange(batch_size, dtype=IdxNp)
        mask = np.ones(batch_size, dtype=np.bool_)
        mask[error.failed_scenarios] = False
        error.succeeded_scenarios = all_scenarios[mask]
        return error
    if error_code == PGM_SERIALIZATION_ERROR:
        return PowerGridSerializationError(pgc.error_message())
    return RuntimeError("Unknown error!")


def assert_no_error(batch_size: int = 1, decode_error: bool = True):
    """
    Assert there is no error in the last operation
    If there is an error, raise it

    Args:
        batch_size (int, optional): Size of batch. Defaults to 1.
        decode_error (bool, optional): Decode the error message(s) to derived error classes. Defaults to True

    Returns:

    """
    error = find_error(batch_size=batch_size, decode_error=decode_error)
    if error is not None:
        raise error


def handle_errors(
    continue_on_batch_error: bool, batch_size: int = 1, decode_error: bool = True
) -> PowerGridBatchError | None:
    """
    Handle any errors in the way that is specified.

    Args:
        continue_on_batch_error (bool): Return the error when the error type is a batch error instead of reraising it.
        batch_size (int, optional): Size of batch. Defaults to 1.
        decode_error (bool, optional): Decode the error message(s) to derived error classes. Defaults to True

    Raises:
        error: Any errors previously encountered, unless it was a batch error and continue_on_batch_error was True.

    Returns:
        PowerGridBatchError | None: None if there were no errors, or the previously encountered
                                    error if it was a batch error and continue_on_batch_error was True.
    """
    error: RuntimeError | None = find_error(batch_size=batch_size, decode_error=decode_error)
    if error is None:
        return None

    if continue_on_batch_error and isinstance(error, PowerGridBatchError):
        # continue on batch error
        return error

    # raise normal error
    raise error
