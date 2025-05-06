# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Common enumerations used by the power-grid-model library.

Note: these enumeration match the C++ arithmetic core, so don't change the values unless you change them in C++ as well

"""

from power_grid_model._core.enum import (  # pylint: disable=unused-import
    AngleMeasurementType,
    Branch3Side,
    BranchSide,
    CalculationMethod,
    CalculationType,
    ComponentAttributeFilterOptions,
    FaultPhase,
    FaultType,
    LoadGenType,
    MeasuredTerminalType,
    ShortCircuitVoltageScaling,
    TapChangingStrategy,
    WindingType,
    _ExperimentalFeatures,
)
