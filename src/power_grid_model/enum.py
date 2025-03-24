# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Common enumerations used by the power-grid-model library.

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
    short_circuit = 2


class CalculationMethod(IntEnum):
    """Calculation Methods"""

    linear = 0
    newton_raphson = 1
    iterative_linear = 2
    iterative_current = 3
    linear_current = 4
    iec60909 = 5


class TapChangingStrategy(IntEnum):
    """Tap Changing Strategies"""

    disabled = 0
    """
    Disable automatic tap adjustment
    """
    any_valid_tap = 1
    """
    Adjust tap position automatically; optimize for any value in the voltage band
    """
    min_voltage_tap = 2
    """
    Adjust tap position automatically; optimize for the lower end of the voltage band
    """
    max_voltage_tap = 3
    """
    Adjust tap position automatically; optimize for the higher end of the voltage band
    """
    fast_any_tap = 4
    """
    Adjust tap position automatically; optimize for any value in the voltage band; binary search
    """


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


class FaultType(IntEnum):
    """The type of fault represented by a fault component"""

    three_phase = 0
    single_phase_to_ground = 1
    two_phase = 2
    two_phase_to_ground = 3
    nan = -128
    """
    Unspecified fault type. Needs to be overloaded at the latest in the update_data
    """


class FaultPhase(IntEnum):
    """The faulty phase(s) affected by the provided fault type"""

    abc = 0
    """
    All phases are faulty in a three-phase fault
    """
    a = 1
    """
    The first phase is faulty in a single-phase-to-ground fault
    """
    b = 2
    """
    The second phase is faulty in a single-phase-to-ground fault
    """
    c = 3
    """
    The third phase is faulty in a single-phase-to-ground fault
    """
    ab = 4
    """
    The first and second phase are faulty in a two-phase or two-phase-to-ground fault
    """
    ac = 5
    """
    The first and third phase are faulty in a two-phase or two-phase-to-ground fault
    """
    bc = 6
    """
    The second and third phase are faulty in a two-phase or two-phase-to-ground fault
    """
    default_value = -1
    """
    Use the default fault phase. Depends on the fault_type.
    """
    nan = -128
    """
    Unspecified fault phase. Needs to be overloaded at the latest in the update_data
    """


class ShortCircuitVoltageScaling(IntEnum):
    """Voltage scaling for short circuit calculations"""

    minimum = 0
    maximum = 1


class _ExperimentalFeatures(IntEnum):
    """Experimental features"""

    disabled = 0
    enabled = 1


class ComponentAttributeFilterOptions(IntEnum):
    """Filter option component or attribute"""

    everything = 0
    """Filter all components/attributes"""
    relevant = 1
    """Filter only non-empty components/attributes that contain non-NaN values"""
