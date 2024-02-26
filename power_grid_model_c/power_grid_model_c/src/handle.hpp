// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "forward_declarations.hpp"

#include "power_grid_model_c/handle.h"

#include <power_grid_model/batch_parameter.hpp>
#include <power_grid_model/common/common.hpp>

#include <string_view>

// context handle
struct PGM_Handle {
    power_grid_model::Idx err_code;
    std::string err_msg;
    power_grid_model::IdxVector failed_scenarios;
    std::vector<std::string> batch_errs;
    mutable std::vector<char const*> batch_errs_c_str;
    [[no_unique_address]] power_grid_model::BatchParameter batch_parameter;
};

template <class Exception = std::exception, class Functor>
auto call_with_catch(PGM_Handle* handle, Functor func, PGM_Idx error_code, std::string_view extra_msg = {})
    -> std::invoke_result_t<Functor> {
    if (handle) {
        PGM_clear_error(handle);
    }
    using ReturnValueType = std::remove_cvref_t<std::invoke_result_t<Functor>>;
    static std::conditional_t<std::is_void_v<ReturnValueType>, int, ReturnValueType> const empty{};
    try {
        return func();
    } catch (Exception const& e) {
        handle->err_code = error_code;
        handle->err_msg = std::string(e.what()) + std::string(extra_msg);
        return static_cast<ReturnValueType>(empty);
    }
}
