// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_OPTIONS_HPP
#define POWER_GRID_MODEL_CPP_OPTIONS_HPP

#include "../power_grid_model_c/options.h"
#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model {

class Options {
  public:
    Options() : options_{PGM_create_options(handle_.get_ptr())} {}

    void set_calculation_type(Idx type) {
        PGM_set_calculation_type(handle_.get_ptr(), options_.get(), type);
        handle_.check_error();
    }

    void set_calculation_method(Idx method) {
        PGM_set_calculation_method(handle_.get_ptr(), options_.get(), method);
        handle_.check_error();
    }

    void set_symmetric(Idx sym) {
        PGM_set_symmetric(handle_.get_ptr(), options_.get(), sym);
        handle_.check_error();
    }

    void set_err_tol(double err_tol) {
        PGM_set_err_tol(handle_.get_ptr(), options_.get(), err_tol);
        handle_.check_error();
    }

    void set_max_iter(Idx max_iter) {
        PGM_set_max_iter(handle_.get_ptr(), options_.get(), max_iter);
        handle_.check_error();
    }

    void set_threading(Idx threading) {
        PGM_set_threading(handle_.get_ptr(), options_.get(), threading);
        handle_.check_error();
    }

    void set_short_circuit_voltage_scaling(Idx short_circuit_voltage_scaling) {
        PGM_set_short_circuit_voltage_scaling(handle_.get_ptr(), options_.get(), short_circuit_voltage_scaling);
        handle_.check_error();
    }

    void set_tap_changing_strategy(Idx tap_changing_strategy) {
        PGM_set_tap_changing_strategy(handle_.get_ptr(), options_.get(), tap_changing_strategy);
        handle_.check_error();
    }

    void set_experimental_features(Idx experimental_features) {
        PGM_set_experimental_features(handle_.get_ptr(), options_.get(), experimental_features);
        handle_.check_error();
    }

  private:
    Handle handle_{};
    UniquePtr<PGM_Options, PGM_destroy_options> options_;
};

} // namespace power_grid_model

#endif // POWER_GRID_MODEL_CPP_OPTIONS_HPP