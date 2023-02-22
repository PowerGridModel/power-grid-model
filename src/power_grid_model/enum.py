# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
Common Enumerations

Note: these enumeration match the C++ arithmetic core, so don't change the values unless you change them in C++ as well

"""

from enum import IntEnum

# Value names are defined in lower case instead of upper case
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


class Branch3Side(IntEnum):
    """Branch3 Sides"""

    side_1 = 0
    side_2 = 1
    side_3 = 2


class CalculationType(IntEnum):
    """Calculation Types"""

    power_flow = 0
    state_estimation = 1


class CalculationMethod(IntEnum):
    """Calculation Methods"""

    linear = 0
    newton_raphson = 1
    iterative_linear = 2
    iterative_current = 3
    linear_current = 4


class MeasuredTerminalType(IntEnum):
    """The type of asset measured by a (power) sensor"""

    branch_from = 0
    """
    Measuring the from-terminal between a branch (except link) and a node
    """
    branch_to = 1
    """
    Measuring the to-terminal between a branch (except link) and a node
    """
    source = 2
    """
    Measuring the terminal between a source and a node
    """
    shunt = 3
    """
    Measuring the terminal between a shunt and a node
    """
    load = 4
    """
    Measuring the terminal between a load and a node
    """
    generator = 5
    """
    Measuring the terminal between a generator and a node
    """
    branch3_1 = 6
    """
    Measuring the terminal-1 between a branch3 and a node
    """
    branch3_2 = 7
    """
    Measuring the terminal-2 between a branch3 and a node
    """
    branch3_3 = 8
    """
    Measuring the terminal-3 between a branch3 and a node
    """
    node = 9
    """
    Measuring the total power injection into a node
    """
