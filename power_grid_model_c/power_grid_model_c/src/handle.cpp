// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/handle.h"

#include "handle.hpp"
#include "input_sanitization.hpp"

#include <power_grid_model/common/typing.hpp>

#include <algorithm>

namespace {
using namespace power_grid_model;

using power_grid_model_c::clear_error;
using power_grid_model_c::safe_bool;
using power_grid_model_c::safe_ptr_get;
using power_grid_model_c::safe_ptr_maybe_nullptr;
using power_grid_model_c::to_c_size;
} // namespace

// create and destroy handle
PGM_Handle* PGM_create_handle() { return new PGM_Handle{}; }
void PGM_destroy_handle(PGM_Handle* handle) { delete handle; }

// error handling
PGM_Idx PGM_error_code(PGM_Handle const* handle) { return handle ? handle->err_code : PGM_Idx{0}; }
char const* PGM_error_message(PGM_Handle const* handle) {
    auto const* safe_handle = safe_ptr_maybe_nullptr(handle);
    return safe_handle ? safe_handle->err_msg.c_str() : nullptr;
}
PGM_Idx PGM_n_failed_scenarios(PGM_Handle const* handle) {
    auto const* safe_handle = safe_ptr_maybe_nullptr(handle);
    return safe_handle ? to_c_size(safe_handle->failed_scenarios.size()) : PGM_Idx{0};
}
PGM_Idx const* PGM_failed_scenarios(PGM_Handle const* handle) {
    auto const* safe_handle = safe_ptr_maybe_nullptr(handle);
    return safe_handle ? safe_handle->failed_scenarios.data() : nullptr;
}
char const** PGM_batch_errors(PGM_Handle const* handle) {
    if (!safe_ptr_maybe_nullptr(handle)) {
        return nullptr;
    }
    auto const& safe_handle = safe_ptr_get(handle);
    safe_handle.batch_errs_c_str.clear();
    std::ranges::transform(safe_handle.batch_errs, std::back_inserter(safe_handle.batch_errs_c_str),
                           [](auto const& x) { return x.c_str(); });
    return safe_handle.batch_errs_c_str.data();
}
void PGM_clear_error(PGM_Handle* handle) { clear_error(handle); }
