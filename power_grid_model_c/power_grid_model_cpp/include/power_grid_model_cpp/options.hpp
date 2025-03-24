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

    RawOptions* get() { return options_.get(); }
    RawOptions const* get() const { return options_.get(); }

    void set_calculation_type(Idx type) { handle_.call_with(PGM_set_calculation_type, get(), type); }

    void set_calculation_method(Idx method) { handle_.call_with(PGM_set_calculation_method, get(), method); }

    void set_symmetric(Idx sym) { handle_.call_with(PGM_set_symmetric, get(), sym); }

    void set_err_tol(double err_tol) { handle_.call_with(PGM_set_err_tol, get(), err_tol); }

    void set_max_iter(Idx max_iter) { handle_.call_with(PGM_set_max_iter, get(), max_iter); }

    void set_threading(Idx threading) { handle_.call_with(PGM_set_threading, get(), threading); }

    void set_short_circuit_voltage_scaling(Idx short_circuit_voltage_scaling) {
        handle_.call_with(PGM_set_short_circuit_voltage_scaling, get(), short_circuit_voltage_scaling);
    }

    void set_tap_changing_strategy(Idx tap_changing_strategy) {
        handle_.call_with(PGM_set_tap_changing_strategy, get(), tap_changing_strategy);
    }

    void set_experimental_features(Idx experimental_features) {
        handle_.call_with(PGM_set_experimental_features, get(), experimental_features);
    }

  private:
    Handle handle_{};
    detail::UniquePtr<RawOptions, &PGM_destroy_options> options_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_OPTIONS_HPP
