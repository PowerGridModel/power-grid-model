# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Common enumerations for library-internal use.

Note: these enumeration match the C++ arithmetic core, so don't change the values unless you change them in C++ as well

"""

from enum import IntEnum

# Value names are defined in lower case instead of upper case
# pylint: disable=invalid-name


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


class AngleMeasurementType(IntEnum):
    """The type of the angle measurement for current sensors."""

    local_angle = 0
    """
    The angle is relative to the local voltage angle
    """
    global_angle = 1
    """
    The angle is relative to the global voltage angle.
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
