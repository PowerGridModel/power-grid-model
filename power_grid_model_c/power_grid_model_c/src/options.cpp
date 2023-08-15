// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "power_grid_model_c/options.h"

#include "options.hpp"

namespace {
using namespace power_grid_model;
} // namespace

// options
PGM_Options* PGM_create_options(const PGM_Handle* /* handle */) { return new PGM_Options{}; }
void PGM_destroy_options(PGM_Options* opt) { delete opt; }
void PGM_set_calculation_type(const PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx type) {
    opt->calculation_type = type;
}
void PGM_set_calculation_method(const PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx method) {
    opt->calculation_method = method;
}
void PGM_set_symmetric(const PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx sym) { opt->symmetric = sym; }
void PGM_set_err_tol(const PGM_Handle* /* handle */, PGM_Options* opt, double err_tol) { opt->err_tol = err_tol; }
void PGM_set_max_iter(const PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx max_iter) { opt->max_iter = max_iter; }
void PGM_set_threading(const PGM_Handle* /* handle */, PGM_Options* opt, PGM_Idx threading) {
    opt->threading = threading;
}
