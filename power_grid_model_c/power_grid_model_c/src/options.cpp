// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/options.h"

#include "options.hpp"

namespace {
using namespace power_grid_model;
} // namespace

// options
PGM_Options* PGM_create_options(PGM_Handle* /* handle */) { return new PGM_Options{}; }
void PGM_destroy_options(PGM_Options* opt) { delete opt; }
void PGM_set_calculation_type(PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx type) {
    opt->calculation_type = type;
}
void PGM_set_calculation_method(PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx method) {
    opt->calculation_method = method;
}
void PGM_set_symmetric(PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx sym) { opt->symmetric = sym; }
void PGM_set_err_tol(PGM_Handle* /* handle */, PGM_Options* opt, double err_tol) { opt->err_tol = err_tol; }
void PGM_set_max_iter(PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx max_iter) { opt->max_iter = max_iter; }
void PGM_set_threading(PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx threading) { opt->threading = threading; }
void PGM_set_short_circuit_voltage_scaling(PGM_Handle* /* handle */, PGM_Options* opt,
                                           PGM_Idx short_circuit_voltage_scaling) {
    opt->short_circuit_voltage_scaling = short_circuit_voltage_scaling;
}
void PGM_set_tap_changing_strategy(PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx tap_changing_strategy) {
    opt->tap_changing_strategy = tap_changing_strategy;
}
void PGM_set_experimental_features(PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx experimental_features) {
    opt->experimental_features = experimental_features;
}
