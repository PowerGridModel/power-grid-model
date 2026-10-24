# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Error handling
"""

import numpy as np

from power_grid_model._core.error_codes import _ERROR_CODE_TO_EXCEPTION, _PgmCErrorCode
from power_grid_model._core.errors import PowerGridBatchError, PowerGridError
from power_grid_model._core.index_integer import IdxNp
from power_grid_model._core.power_grid_core import get_power_grid_core as get_pgc

VALIDATOR_MSG = "\nTry validate_input_data() or validate_batch_data() to validate your data.\n"


def _interpret_coded_error(message: str, error_code: int, decode_error: bool = True) -> PowerGridError:
    if not decode_error:
        return PowerGridError(message)

    type_ = _ERROR_CODE_TO_EXCEPTION[_PgmCErrorCode(error_code)]
    return type_(message)


def find_error(batch_size: int = 1, decode_error: bool = True) -> RuntimeError | None:
    """
    Check if there is an error and return it

    Args:
        batch_size: (int, optional): Size of batch. Defaults to 1.
        decode_error (bool, optional): Decode the error message(s) to derived error classes. Defaults to True

    Returns: error object, can be none

    """
    error_code: int = get_pgc().error_code()
    match error_code:
        case _PgmCErrorCode.NO_ERROR:
            return None
        case _PgmCErrorCode.BATCH_ERROR:
            error_message = "There are errors in the batch calculation." + VALIDATOR_MSG
            error = PowerGridBatchError(error_message)
            n_fails = get_pgc().n_failed_scenarios()
            failed_idxptr = get_pgc().failed_scenarios()
            failed_codeptr = get_pgc().batch_error_codes()
            failed_msgptr = get_pgc().batch_errors()
            error.failed_scenarios = np.ctypeslib.as_array(failed_idxptr, shape=(n_fails,)).copy()
            error.error_codes = np.ctypeslib.as_array(failed_codeptr, shape=(n_fails,)).copy()
            error.error_messages = [failed_msgptr[i].decode() for i in range(n_fails)]  # type: ignore
            error.errors = [
                _interpret_coded_error(message, error_code=scenario_error_code, decode_error=decode_error)
                for message, scenario_error_code in zip(error.error_messages, error.error_codes, strict=True)
            ]
            all_scenarios = np.arange(batch_size, dtype=IdxNp)
            mask = np.ones(batch_size, dtype=np.bool_)
            mask[error.failed_scenarios] = False
            error.succeeded_scenarios = all_scenarios[mask]
            return error
        case _ if error_code in _ERROR_CODE_TO_EXCEPTION:
            error_message = get_pgc().error_message() + VALIDATOR_MSG
            return _interpret_coded_error(error_message, error_code, decode_error=decode_error)
        case _:
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
