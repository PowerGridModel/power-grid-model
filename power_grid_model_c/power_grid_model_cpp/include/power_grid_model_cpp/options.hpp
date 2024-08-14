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

    OptionsC* get() const { return options_.get(); }

    static void set_calculation_type(Options& options, Idx type) {
        PGM_set_calculation_type(options.handle_.get(), options.get(), type);
        options.handle_.check_error();
    }
    void set_calculation_type(Idx type) { set_calculation_type(*this, type); }

    static void set_calculation_method(Options& options, Idx method) {
        PGM_set_calculation_method(options.handle_.get(), options.get(), method);
        options.handle_.check_error();
    }
    void set_calculation_method(Idx method) { set_calculation_method(*this, method); }

    static void set_symmetric(Options& options, Idx sym) {
        PGM_set_symmetric(options.handle_.get(), options.get(), sym);
        options.handle_.check_error();
    }
    void set_symmetric(Idx sym) { set_symmetric(*this, sym); }

    static void set_err_tol(Options& options, double err_tol) {
        PGM_set_err_tol(options.handle_.get(), options.get(), err_tol);
        options.handle_.check_error();
    }
    void set_err_tol(double err_tol) { set_err_tol(*this, err_tol); }

    static void set_max_iter(Options& options, Idx max_iter) {
        PGM_set_max_iter(options.handle_.get(), options.get(), max_iter);
        options.handle_.check_error();
    }
    void set_max_iter(Idx max_iter) { set_max_iter(*this, max_iter); }

    static void set_threading(Options& options, Idx threading) {
        PGM_set_threading(options.handle_.get(), options.get(), threading);
        options.handle_.check_error();
    }
    void set_threading(Idx threading) { set_threading(*this, threading); }

    static void set_short_circuit_voltage_scaling(Options& options, Idx short_circuit_voltage_scaling) {
        PGM_set_short_circuit_voltage_scaling(options.handle_.get(), options.get(), short_circuit_voltage_scaling);
        options.handle_.check_error();
    }
    void set_short_circuit_voltage_scaling(Idx short_circuit_voltage_scaling) {
        set_short_circuit_voltage_scaling(*this, short_circuit_voltage_scaling);
    }

    static void set_tap_changing_strategy(Options& options, Idx tap_changing_strategy) {
        PGM_set_tap_changing_strategy(options.handle_.get(), options.get(), tap_changing_strategy);
        options.handle_.check_error();
    }
    void set_tap_changing_strategy(Idx tap_changing_strategy) {
        set_tap_changing_strategy(*this, tap_changing_strategy);
    }

    static void set_experimental_features(Options& options, Idx experimental_features) {
        PGM_set_experimental_features(options.handle_.get(), options.get(), experimental_features);
        options.handle_.check_error();
    }
    void set_experimental_features(Idx experimental_features) {
        set_experimental_features(*this, experimental_features);
    }

  private:
    Handle handle_{};
    detail::UniquePtr<OptionsC, PGM_destroy_options> options_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_OPTIONS_HPP
