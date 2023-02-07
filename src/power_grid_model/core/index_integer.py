# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
Definition of integers used by the calculation core
"""

# define internal index integer
from ctypes import c_int32, c_int64

import numpy as np

# pylint: disable=C0103
Idx_c = c_int64
# pylint: disable=C0103
Idx_np = np.int64
# pylint: disable=C0103
ID_c = c_int32
# pylint: disable=C0103
ID_np = np.int32
