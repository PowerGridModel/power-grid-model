# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
Common Enumerations

Note: these enumeration match the C++ arithmetic core, so dont change the values unless you change thmn in C++ as well

"""

from enum import Enum, IntEnum


# pylint: disable=invalid-name


class LoadGenType(IntEnum):
    """Load and Generator Types"""

    const_power = 0
    const_impedance = 1
    const_current = 2


class WindingType(IntEnum):
    """Transformer Winding Types"""

    wye = 0
    wye_n = 1
    delta = 2
    zigzag = 3
    zigzag_n = 4


class BranchSide(IntEnum):
    """Branch Sides"""

    from_side = 0
    to_side = 1


class CalculationType(Enum):
    """Calculation Types"""

    power_flow = "power_flow"
    state_estimation = "state_estimation"


class CalculationMethod(IntEnum):
    """Calculation Methods"""

    linear = 0
    newton_raphson = 1
    iterative_linear = 2


class MeasuredTerminalType(IntEnum):
    """The type of asset measured by a (power) sensor"""

    branch_from = 0
    branch_to = 1
    source = 2
    shunt = 3
    load = 4
    generator = 5
