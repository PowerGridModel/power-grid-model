# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Error handling
"""

from typing import Optional

import numpy as np

from power_grid_model.core.index_integer import Idx_np
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.errors import PowerGridBatchError, PowerGridError

VALIDATOR_MSG = "\nTry validate_input_data() or validate_batch_data() to validate your data.\n"


def find_error(batch_size: int = 1) -> Optional[ValueError]:
    """

    Args:
        batch_size:

    Returns:

    """
    error_code: int = pgc.err_code()
    if error_code == 0:
        return None
    elif error_code == 1:
        error_message = pgc.err_msg()
        error_message += VALIDATOR_MSG
        return PowerGridError(error_message)
    elif error_code == 2:
        error_message = "There are errors in the batch calculation." + VALIDATOR_MSG
        error = PowerGridBatchError(error_message)
        n_fails = pgc.n_failed_scenarios()
        failed_idxptr = pgc.failed_scenarios()
        failed_msgptr = pgc.batch_errs()
        error.failed_scenarios = np.ctypeslib.as_array(failed_idxptr, shape=(n_fails,)).copy()
        error.error_messages = [failed_msgptr[i].decode() for i in range(n_fails)]  # type: ignore
        all_scenarios = np.arange(batch_size, dtype=Idx_np)
        mask = np.ones(batch_size, dtype=np.bool)
        mask[error.failed_scenarios] = False
        error.succeeded_scenarios = all_scenarios[mask]
        return error
    else:
        return ValueError("Unknown error!")


def assert_no_error(batch_size: int = 1):
    """

    Returns:

    """
    error = find_error(batch_size=batch_size)
    if error is not None:
        raise error
