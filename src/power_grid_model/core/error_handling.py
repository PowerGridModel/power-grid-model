# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Error handling
"""

from typing import Optional

import numpy as np

from power_grid_model.core.index_integer import IdxNp
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.errors import PowerGridBatchError, PowerGridError

VALIDATOR_MSG = "\nTry validate_input_data() or validate_batch_data() to validate your data.\n"
# error codes
PGM_NO_ERROR = 0
PGM_REGULAR_ERROR = 1
PGM_BATCH_ERROR = 2


def find_error(batch_size: int = 1) -> Optional[RuntimeError]:
    """
    Check if there is an error and return it

    Args:
        batch_size: Size of batch

    Returns: error object, can be none

    """
    error_code: int = pgc.error_code()
    if error_code == PGM_NO_ERROR:
        return None
    if error_code == PGM_REGULAR_ERROR:
        error_message = pgc.error_message()
        error_message += VALIDATOR_MSG
        return PowerGridError(error_message)
    if error_code == PGM_BATCH_ERROR:
        error_message = "There are errors in the batch calculation." + VALIDATOR_MSG
        error = PowerGridBatchError(error_message)
        n_fails = pgc.n_failed_scenarios()
        failed_idxptr = pgc.failed_scenarios()
        failed_msgptr = pgc.batch_errors()
        error.failed_scenarios = np.ctypeslib.as_array(failed_idxptr, shape=(n_fails,)).copy()
        error.error_messages = [failed_msgptr[i].decode() for i in range(n_fails)]  # type: ignore
        all_scenarios = np.arange(batch_size, dtype=IdxNp)
        mask = np.ones(batch_size, dtype=np.bool_)
        mask[error.failed_scenarios] = False
        error.succeeded_scenarios = all_scenarios[mask]
        return error
    return RuntimeError("Unknown error!")


def assert_no_error(batch_size: int = 1):
    """
    Assert there is no error in the last operation
    If there is an error, raise it

    Returns:

    """
    error = find_error(batch_size=batch_size)
    if error is not None:
        raise error


def handle_errors(continue_on_batch_error: bool, batch_size: int = 1) -> Optional[PowerGridBatchError]:
    """
    Handle any errors in the way that is specified.

    Args:
        continue_on_batch_error (bool): Return the error when the error type is a batch error instead of reraising it.
        batch_size (int, optional): Size of batch. Defaults to 1.

    Raises:
        error: Any errors previously encountered, unless it was a batch error and continue_on_batch_error was True.

    Returns:
        Optional[PowerGridBatchError]: None if there were no errors, or the previously encountered
                                       error if it was a batch error and continue_on_batch_error was True.
    """
    error: Optional[RuntimeError] = find_error(batch_size=batch_size)
    if error is None:
        return None

    if continue_on_batch_error and isinstance(error, PowerGridBatchError):
        # continue on batch error
        return error

    # raise normal error
    raise error
