// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/handle.h"

#include "handle.hpp"
#include "input_sanitization.hpp"

#include <algorithm>
#include <type_traits>

namespace {
using namespace power_grid_model;

using power_grid_model_c::clear_error;
using power_grid_model_c::compile_time_safe_cast;
} // namespace

// create and destroy handle
PGM_Handle* PGM_create_handle() {
    return new PGM_Handle{}; // NOSONAR(S5025)
}
void PGM_destroy_handle(PGM_Handle* handle) {
    delete handle; // NOSONAR(S5025)
}

// error handling
PGM_Idx PGM_error_code(PGM_Handle const* handle) {
    return handle != nullptr ? compile_time_safe_cast<PGM_Idx>(handle->err_code) : PGM_Idx{0};
}
char const* PGM_error_message(PGM_Handle const* handle) {
    return handle != nullptr ? handle->err_msg.c_str() : nullptr;
}
PGM_Idx PGM_n_failed_scenarios(PGM_Handle const* handle) {
    return handle != nullptr ? compile_time_safe_cast<PGM_Idx>(std::ssize(handle->failed_scenarios)) : PGM_Idx{0};
}
PGM_Idx const* PGM_failed_scenarios(PGM_Handle const* handle) {
    return handle != nullptr ? compile_time_safe_cast<PGM_Idx const*>(handle->failed_scenarios.data()) : nullptr;
}
char const** PGM_batch_errors(PGM_Handle const* handle) {
    if (handle == nullptr) {
        return nullptr;
    }
    auto const& handle_ref = *handle;
    handle_ref.batch_errs_c_str.clear();
    std::ranges::transform(handle_ref.batch_errs, std::back_inserter(handle_ref.batch_errs_c_str),
                           [](auto const& x) { return x.c_str(); });
    return handle_ref.batch_errs_c_str.data();
}
void PGM_clear_error(PGM_Handle* handle) { clear_error(handle); }
