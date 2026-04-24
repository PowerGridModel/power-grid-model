# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import warnings

import pytest

from power_grid_model.enum import MeasuredTerminalType


def lts_measured_terminal_types():
    with warnings.catch_warnings():  # backwards compatiblity
        warnings.simplefilter("ignore")
        return [value for value in MeasuredTerminalType if value != MeasuredTerminalType.node]


@pytest.mark.parametrize("value", lts_measured_terminal_types())
def test_measured_terminal_type__supported(value):
    warnings.simplefilter("error")
    assert MeasuredTerminalType[value.name] == value
    assert MeasuredTerminalType(value.value) == value
    assert getattr(MeasuredTerminalType, value.name) == value


def test_deprecated_measured_terminal_type__deprecated_node():
    with pytest.deprecated_call():
        MeasuredTerminalType.node

    with pytest.deprecated_call():
        MeasuredTerminalType["node"]

    with pytest.deprecated_call():
        MeasuredTerminalType(9)

    with warnings.catch_warnings():  # backwards compatiblity
        warnings.simplefilter("ignore")
        assert MeasuredTerminalType["node"] == MeasuredTerminalType.node
        assert MeasuredTerminalType(MeasuredTerminalType.node.value) == MeasuredTerminalType.node
        assert getattr(MeasuredTerminalType, "node") == MeasuredTerminalType.node
