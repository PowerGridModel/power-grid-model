# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Error classes
"""

from typing import List, Optional

import numpy as np

from power_grid_model.power_grid_core import power_grid_core as pgc

VALIDATOR_MSG = "\nTry validate_input_data() or validate_batch_data() to validate your data.\n"


class PowerGridError(Exception):
    pass


class PowerGridBatchError(Exception):
    failed_scenarios: np.ndarray
    error_messages: List[str]


def find_error() -> Optional[Exception]:
    """

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
        error.failed_scenarios = np.as_array(failed_idxptr, shape=(n_fails,)).copy()
        error.error_messages = [failed_msgptr[i].decode() for i in range(n_fails)]
        return error
    else:
        return Exception("Unknown error!")


def assert_error():
    """

    Returns:

    """
    error = find_error()
    if error is not None:
        raise error
