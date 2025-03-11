// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/handle.h"

#include "handle.hpp"

#include <algorithm>

namespace {
using namespace power_grid_model;
} // namespace

// create and destroy handle
PGM_Handle* PGM_create_handle() { return new PGM_Handle{}; }
void PGM_destroy_handle(PGM_Handle* handle) { delete handle; }

// error handling
PGM_Idx PGM_error_code(PGM_Handle const* handle) { return handle->err_code; }
char const* PGM_error_message(PGM_Handle const* handle) { return handle->err_msg.c_str(); }
PGM_Idx PGM_n_failed_scenarios(PGM_Handle const* handle) { return static_cast<Idx>(handle->failed_scenarios.size()); }
PGM_Idx const* PGM_failed_scenarios(PGM_Handle const* handle) { return handle->failed_scenarios.data(); }
char const** PGM_batch_errors(PGM_Handle const* handle) {
    handle->batch_errs_c_str.clear();
    std::ranges::transform(handle->batch_errs, std::back_inserter(handle->batch_errs_c_str),
                           [](auto const& x) { return x.c_str(); });
    return handle->batch_errs_c_str.data();
}
void PGM_clear_error(PGM_Handle* handle) { *handle = PGM_Handle{}; }
