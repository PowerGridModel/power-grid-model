# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# This header file is automatically generated. DO NOT modify it manually!

"""Data types for power grid model dataset and component types."""

from typing import Literal

PowerGridDataTypes = Literal[
    "input",
    "sym_output",
    "asym_output",
    "update",
    "sc_output",
]

PowerGridComponents = Literal[
    "node",
    "line",
    "link",
    "transformer",
    "three_winding_transformer",
    "sym_load",
    "sym_gen",
    "asym_load",
    "asym_gen",
    "shunt",
    "source",
    "sym_voltage_sensor",
    "asym_voltage_sensor",
    "sym_power_sensor",
    "asym_power_sensor",
    "fault",
]
