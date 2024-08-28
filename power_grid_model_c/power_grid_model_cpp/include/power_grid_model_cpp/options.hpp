// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_OPTIONS_HPP
#define POWER_GRID_MODEL_CPP_OPTIONS_HPP

#include "basics.hpp"
#include "handle.hpp"

#include "power_grid_model_c/options.h"

namespace power_grid_model_cpp {
class Options {
  public:
    Options() : options_{handle_.call_with(PGM_create_options)} {}

    RawOptions* get() const { return options_.get(); }

    static void set_calculation_type(Options const& options, Idx type) {
        options.handle_.call_with(PGM_set_calculation_type, options.get(), type);
    }
    void set_calculation_type(Idx type) const { set_calculation_type(*this, type); }

    static void set_calculation_method(Options const& options, Idx method) {
        options.handle_.call_with(PGM_set_calculation_method, options.get(), method);
    }
    void set_calculation_method(Idx method) const { set_calculation_method(*this, method); }

    static void set_symmetric(Options const& options, Idx sym) {
        options.handle_.call_with(PGM_set_symmetric, options.get(), sym);
    }
    void set_symmetric(Idx sym) const { set_symmetric(*this, sym); }

    static void set_err_tol(Options const& options, double err_tol) {
        options.handle_.call_with(PGM_set_err_tol, options.get(), err_tol);
    }
    void set_err_tol(double err_tol) const { set_err_tol(*this, err_tol); }

    static void set_max_iter(Options const& options, Idx max_iter) {
        options.handle_.call_with(PGM_set_max_iter, options.get(), max_iter);
    }
    void set_max_iter(Idx max_iter) const { set_max_iter(*this, max_iter); }

    static void set_threading(Options const& options, Idx threading) {
        options.handle_.call_with(PGM_set_threading, options.get(), threading);
    }
    void set_threading(Idx threading) const { set_threading(*this, threading); }

    static void set_short_circuit_voltage_scaling(Options const& options, Idx short_circuit_voltage_scaling) {
        options.handle_.call_with(PGM_set_short_circuit_voltage_scaling, options.get(), short_circuit_voltage_scaling);
    }
    void set_short_circuit_voltage_scaling(Idx short_circuit_voltage_scaling) const {
        set_short_circuit_voltage_scaling(*this, short_circuit_voltage_scaling);
    }

    static void set_tap_changing_strategy(Options const& options, Idx tap_changing_strategy) {
        options.handle_.call_with(PGM_set_tap_changing_strategy, options.get(), tap_changing_strategy);
    }
    void set_tap_changing_strategy(Idx tap_changing_strategy) const {
        set_tap_changing_strategy(*this, tap_changing_strategy);
    }

    static void set_experimental_features(Options const& options, Idx experimental_features) {
        options.handle_.call_with(PGM_set_experimental_features, options.get(), experimental_features);
    }
    void set_experimental_features(Idx experimental_features) const {
        set_experimental_features(*this, experimental_features);
    }

  private:
    Handle handle_{};
    detail::UniquePtr<RawOptions, &PGM_destroy_options> options_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_OPTIONS_HPP
