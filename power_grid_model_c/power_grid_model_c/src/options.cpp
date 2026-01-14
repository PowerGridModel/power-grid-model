// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/options.h"

#include "handle.hpp"
#include "input_sanitization.hpp"
#include "options.hpp"

namespace {
using namespace power_grid_model;

using power_grid_model_c::call_with_catch;
using power_grid_model_c::safe_ptr_get;
} // namespace

// options
PGM_Options* PGM_create_options(PGM_Handle* /* handle */) {
    return new PGM_Options{}; // NOSONAR(S5025)
}
void PGM_destroy_options(PGM_Options* opt) {
    delete opt; // NOSONAR(S5025)
}
void PGM_set_calculation_type(PGM_Handle* handle, PGM_Options* opt, PGM_Idx type) {
    call_with_catch(handle, [opt, type] { safe_ptr_get(opt).calculation_type = type; });
}
void PGM_set_calculation_method(PGM_Handle* handle, PGM_Options* opt, PGM_Idx method) {
    call_with_catch(handle, [opt, method] { safe_ptr_get(opt).calculation_method = method; });
}
void PGM_set_symmetric(PGM_Handle* handle, PGM_Options* opt, PGM_Idx sym) {
    call_with_catch(handle, [opt, sym] { safe_ptr_get(opt).symmetric = sym; });
}
void PGM_set_err_tol(PGM_Handle* handle, PGM_Options* opt, double err_tol) {
    call_with_catch(handle, [opt, err_tol] { safe_ptr_get(opt).err_tol = err_tol; });
}
void PGM_set_max_iter(PGM_Handle* handle, PGM_Options* opt, PGM_Idx max_iter) {
    call_with_catch(handle, [opt, max_iter] { safe_ptr_get(opt).max_iter = max_iter; });
}
void PGM_set_threading(PGM_Handle* handle, PGM_Options* opt, PGM_Idx threading) {
    call_with_catch(handle, [opt, threading] { safe_ptr_get(opt).threading = threading; });
}
void PGM_set_short_circuit_voltage_scaling(PGM_Handle* handle, PGM_Options* opt,
                                           PGM_Idx short_circuit_voltage_scaling) {
    call_with_catch(handle, [opt, short_circuit_voltage_scaling] {
        safe_ptr_get(opt).short_circuit_voltage_scaling = short_circuit_voltage_scaling;
    });
}
void PGM_set_tap_changing_strategy(PGM_Handle* handle, PGM_Options* opt, PGM_Idx tap_changing_strategy) {
    call_with_catch(handle,
                    [opt, tap_changing_strategy] { safe_ptr_get(opt).tap_changing_strategy = tap_changing_strategy; });
}
void PGM_set_experimental_features(PGM_Handle* handle, PGM_Options* opt, PGM_Idx experimental_features) {
    call_with_catch(handle,
                    [opt, experimental_features] { safe_ptr_get(opt).experimental_features = experimental_features; });
}
