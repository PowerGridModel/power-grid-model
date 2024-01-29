# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Error classes
"""

from typing import List

import numpy as np


class PowerGridError(RuntimeError):
    """
    Generic power grid error
    """


class PowerGridBatchError(PowerGridError):
    """
    Error occurs in batch calculation
    """

    failed_scenarios: np.ndarray
    succeeded_scenarios: np.ndarray
    error_messages: List[str]


class PowerGridSerializationError(PowerGridError):
    """
    Error occurs during (de-)serialization
    """
