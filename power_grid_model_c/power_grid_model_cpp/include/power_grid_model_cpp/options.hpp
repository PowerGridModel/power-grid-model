// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_OPTIONS_HPP
#define POWER_GRID_MODEL_CPP_OPTIONS_HPP

#include <memory>

#include "options.h"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Options {
public:
    power_grid_model_cpp::Handle handle;

    Options() : handle(), options_{PGM_create_options(handle.get()), details::DeleterFunctor<&PGM_destroy_options>()} {}

    ~Options() = default;
    
    static void set_calculation_type(PGM_Handle* provided_handle, PGM_Options* opt, PGM_Idx type) {
        PGM_set_calculation_type(provided_handle, opt, type);
    }
    void set_calculation_type(PGM_Options* opt, PGM_Idx type) {
        PGM_set_calculation_type(handle.get(), opt, type);
    }

    static void set_calculation_method(PGM_Handle* provided_handle, PGM_Options* opt, PGM_Idx method) {
        PGM_set_calculation_method(provided_handle, opt, method);
    }
    void set_calculation_method(PGM_Options* opt, PGM_Idx method) {
        PGM_set_calculation_method(handle.get(), opt, method);
    }

    static void set_symmetric(PGM_Handle* provided_handle, PGM_Options* opt, PGM_Idx sym) {
        PGM_set_symmetric(provided_handle, opt, sym);
    }
    void set_symmetric(PGM_Options* opt, PGM_Idx sym) {
        PGM_set_symmetric(handle.get(), opt, sym);
    }

    static void set_err_tol(PGM_Handle* provided_handle, PGM_Options* opt, double err_tol) {
        PGM_set_err_tol(provided_handle, opt, err_tol);
    }
    void set_err_tol(PGM_Options* opt, double err_tol) {
        PGM_set_err_tol(handle.get(), opt, err_tol);
    }

    static void set_max_iter(PGM_Handle* provided_handle, PGM_Options* opt, PGM_Idx max_iter) {
        PGM_set_max_iter(provided_handle, opt, max_iter);
    }
    void set_max_iter(PGM_Options* opt, PGM_Idx max_iter) {
        PGM_set_max_iter(handle.get(), opt, max_iter);
    }

    static void set_threading(PGM_Handle* provided_handle, PGM_Options* opt, PGM_Idx threading) {
        PGM_set_threading(provided_handle, opt,threading);
    }
    void set_threading(PGM_Options* opt, PGM_Idx threading) {
        PGM_set_threading(handle.get(), opt,threading);
    }

    static void set_short_circuit_voltage_scaling(PGM_Handle* provided_handle, PGM_Options* opt,
                                                   PGM_Idx short_circuit_voltage_scaling) {
        PGM_set_short_circuit_voltage_scaling(provided_handle, opt, short_circuit_voltage_scaling);
    }
    void set_short_circuit_voltage_scaling(PGM_Options* opt, PGM_Idx short_circuit_voltage_scaling) {
        PGM_set_short_circuit_voltage_scaling(handle.get(), opt, short_circuit_voltage_scaling);
    }

    static void set_tap_changing_strategy(PGM_Handle* provided_handle, PGM_Options* opt, PGM_Idx tap_changing_strategy) {
        PGM_set_tap_changing_strategy(provided_handle, opt, tap_changing_strategy);
    }
    void set_tap_changing_strategy(PGM_Options* opt, PGM_Idx tap_changing_strategy) {
        PGM_set_tap_changing_strategy(handle.get(), opt, tap_changing_strategy);
    }

    static void set_experimental_features(PGM_Handle* provided_handle, PGM_Options* opt, PGM_Idx experimental_features) {
        PGM_set_experimental_features(provided_handle, opt, experimental_features);
    }
    void set_experimental_features(PGM_Options* opt, PGM_Idx experimental_features) {
        PGM_set_experimental_features(handle.get(), opt, experimental_features);
    }
private:
    std::unique_ptr<PGM_Options, details::DeleterFunctor<&PGM_destroy_options>> options_;
};
} // namespace power_grid_model_cpp

#endif