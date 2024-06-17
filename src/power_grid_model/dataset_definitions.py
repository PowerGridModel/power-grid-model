# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# This header file is automatically generated. DO NOT modify it manually!

"""Enums for PMG dataset and component types."""

from enum import Enum

# Value names are defined in lower case instead of upper case
# pylint: disable=invalid-name


class PowerGridDataTypes(Enum):
    """
    Types of single/batch datasets:

      - input: Dataset with attributes relevant to the grid configuration (e.g. id, from_node, from_status).

      - update: Dataset with attributes relevant to multiple scenarios (e.g. from_status, to_status).

      - sym_output: Dataset with attributes relevant to symmetrical steady state output of power flow or state
      estimation calculation (e.g. p_from, p_to).

      - asym_output: Dataset with attributes relevant to asymmetrical steady state output of power flow or state
      estimation calculation (e.g. p_from, p_to).

      - sc_output: Contains attributes relevant to symmetrical short circuit calculation output. Like for the
      asym_output, detailed data for all 3 phases will be provided where relevant (e.g. i_from, i_from_angle).
    """

    input = "input"

    sym_output = "sym_output"

    asym_output = "asym_output"

    update = "update"

    sc_output = "sc_output"


class PowerGridComponents(Enum):
    """Grid component types."""

    node = "node"

    line = "line"

    link = "link"

    transformer = "transformer"

    three_winding_transformer = "three_winding_transformer"

    sym_load = "sym_load"

    sym_gen = "sym_gen"

    asym_load = "asym_load"

    asym_gen = "asym_gen"

    shunt = "shunt"

    source = "source"

    sym_voltage_sensor = "sym_voltage_sensor"

    asym_voltage_sensor = "asym_voltage_sensor"

    sym_power_sensor = "sym_power_sensor"

    asym_power_sensor = "asym_power_sensor"

    fault = "fault"
