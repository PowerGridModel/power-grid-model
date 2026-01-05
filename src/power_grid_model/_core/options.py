# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Option class
"""

from collections.abc import Callable
from typing import Any

from power_grid_model._core.power_grid_core import OptionsPtr, get_power_grid_core as get_pgc


class OptionSetter:
    """
    setter for options
    """

    _setter: Callable

    def __init__(self, setter):
        self._setter = setter

    def __set__(self, instance: "Options", value: Any):
        self._setter(instance.opt, value)

    def __get__(self, instance, owner):
        raise NotImplementedError("Cannot get option value!")


class Options:
    """
    Option class
    """

    _opt: OptionsPtr
    # option setter
    calculation_type = OptionSetter(get_pgc().set_calculation_type)
    calculation_method = OptionSetter(get_pgc().set_calculation_method)
    symmetric = OptionSetter(get_pgc().set_symmetric)
    error_tolerance = OptionSetter(get_pgc().set_err_tol)
    max_iterations = OptionSetter(get_pgc().set_max_iter)
    threading = OptionSetter(get_pgc().set_threading)
    tap_changing_strategy = OptionSetter(get_pgc().set_tap_changing_strategy)
    short_circuit_voltage_scaling = OptionSetter(get_pgc().set_short_circuit_voltage_scaling)
    experimental_features = OptionSetter(get_pgc().set_experimental_features)

    @property
    def opt(self) -> OptionsPtr:
        """

        Returns: Pointer to the option object

        """
        return self._opt

    def __new__(cls, *args, **kwargs):
        instance = super().__new__(cls, *args, **kwargs)
        instance._opt = get_pgc().create_options()
        return instance

    def __del__(self):
        get_pgc().destroy_options(self._opt)

    # not copyable
    def __copy__(self):
        raise NotImplementedError("Class not copyable")

    def __deepcopy__(self, memodict):
        raise NotImplementedError("class not copyable")
