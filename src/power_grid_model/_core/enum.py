# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Common enumerations for library-internal use.

Note: these enumeration match the C++ arithmetic core, so don't change the values unless you change them in C++ as well

"""

import warnings
from enum import EnumMeta, IntEnum, nonmember

# Value names are defined in lower case instead of upper case


class _DeprecationAwareEnumMeta(EnumMeta):
    """
    Metaclass that supports deprecated enum members.

    In the enum class body, define _deprecated_members as a dict that is explicitly marked 'nonmember'
    mapping deprecated member names to deprecation warning messages. All access
    patterns (Foo.bar, Foo['bar'], Foo(bar_value), Foo[bar_value]) will emit a DeprecationWarning.
    """

    def _warn_if_deprecated(cls, member):
        msg = cls.__dict__.get("_deprecated_members", {}).get(member.name)
        if msg:
            warnings.warn(msg, DeprecationWarning, stacklevel=3)

    def __getattribute__(cls, name: str):
        result = super().__getattribute__(name)
        if isinstance(result, cls):
            cls._warn_if_deprecated(result)
        return result

    def __getitem__(cls, key):
        if isinstance(key, int):
            member = cls._value2member_map_.get(key)
            if member is None:
                raise KeyError(key)
        else:
            member = super().__getitem__(key)
        cls._warn_if_deprecated(member)
        return member

    def __call__(cls, value, names=None, *args, **kwargs):
        if names is not None:
            return super().__call__(value, names, *args, **kwargs)
        result = super().__call__(value)
        if isinstance(result, cls):
            cls._warn_if_deprecated(result)
        return result


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


class MeasuredTerminalType(IntEnum, metaclass=_DeprecationAwareEnumMeta):
    """The type of asset measured by a (power) sensor.

    NOTE: MeasuredTerminalType.node is deprecated and will be removed in a future release.
    The reason for deprecation is that, contrary to the other terminal types, it does not represent a real terminal.
    It is a fundamentally different concept that can lead to confusion and errors.
    Please use one of the other terminal types instead.
    """

    _deprecated_members = nonmember(
        {
            "node": "MeasuredTerminalType.node is deprecated and will be removed in a future release.",
        }
    )

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
    [DEPRECATED] Measuring the total power injection into a node
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
