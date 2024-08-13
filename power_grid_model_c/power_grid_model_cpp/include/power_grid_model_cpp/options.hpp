// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_OPTIONS_HPP
#define POWER_GRID_MODEL_CPP_OPTIONS_HPP

#include "power_grid_model_c/options.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Options {
  public:
    Options() : options_{PGM_create_options(handle_.get())} {}

    PGM_Options* get() const { return options_.get(); }

    static void set_calculation_type(Handle const& handle, Options& options, Idx type) {
        PGM_set_calculation_type(handle.get(), options.get(), type);
        handle.check_error();
    }
    void set_calculation_type(Idx type) { set_calculation_type(handle_, *this, type); }

    static void set_calculation_method(Handle const& handle, Options& options, Idx method) {
        PGM_set_calculation_method(handle.get(), options.get(), method);
        handle.check_error();
    }
    void set_calculation_method(Idx method) { set_calculation_method(handle_, *this, method); }

    static void set_symmetric(Handle const& handle, Options& options, Idx sym) {
        PGM_set_symmetric(handle.get(), options.get(), sym);
        handle.check_error();
    }
    void set_symmetric(Idx sym) { set_symmetric(handle_, *this, sym); }

    static void set_err_tol(Handle const& handle, Options& options, double err_tol) {
        PGM_set_err_tol(handle.get(), options.get(), err_tol);
        handle.check_error();
    }
    void set_err_tol(double err_tol) { set_err_tol(handle_, *this, err_tol); }

    static void set_max_iter(Handle const& handle, Options& options, Idx max_iter) {
        PGM_set_max_iter(handle.get(), options.get(), max_iter);
        handle.check_error();
    }
    void set_max_iter(Idx max_iter) { set_max_iter(handle_, *this, max_iter); }

    static void set_threading(Handle const& handle, Options& options, Idx threading) {
        PGM_set_threading(handle.get(), options.get(), threading);
        handle.check_error();
    }
    void set_threading(Idx threading) { set_threading(handle_, *this, threading); }

    static void set_short_circuit_voltage_scaling(Handle const& handle, Options& options,
                                                  Idx short_circuit_voltage_scaling) {
        PGM_set_short_circuit_voltage_scaling(handle.get(), options.get(), short_circuit_voltage_scaling);
        handle.check_error();
    }
    void set_short_circuit_voltage_scaling(Idx short_circuit_voltage_scaling) {
        set_short_circuit_voltage_scaling(handle_, *this, short_circuit_voltage_scaling);
    }

    static void set_tap_changing_strategy(Handle const& handle, Options& options, Idx tap_changing_strategy) {
        PGM_set_tap_changing_strategy(handle.get(), options.get(), tap_changing_strategy);
        handle.check_error();
    }
    void set_tap_changing_strategy(Idx tap_changing_strategy) {
        set_tap_changing_strategy(handle_, *this, tap_changing_strategy);
    }

    static void set_experimental_features(Handle const& handle, Options& options, Idx experimental_features) {
        PGM_set_experimental_features(handle.get(), options.get(), experimental_features);
        handle.check_error();
    }
    void set_experimental_features(Idx experimental_features) {
        set_experimental_features(handle_, *this, experimental_features);
    }

  private:
    Handle handle_{};
    UniquePtr<PGM_Options, PGM_destroy_options> options_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_OPTIONS_HPP
