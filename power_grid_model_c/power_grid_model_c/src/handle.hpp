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

#include <exception>
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

namespace power_grid_model_c {
constexpr void clear_error(PGM_Handle* handle) {
    if (handle != nullptr) {
        *handle = PGM_Handle{};
    }
}

struct DefaultExceptionHandler {
    void operator()(PGM_Handle& handle) const noexcept { handle_all_errors(handle, PGM_regular_error); }

    static void handle_all_errors(PGM_Handle& handle, PGM_Idx error_code,
                                  std::string_view extra_message = "") noexcept {
        std::exception_ptr const ex_ptr = std::current_exception();
        try {
            std::rethrow_exception(ex_ptr);
        } catch (std::exception const& ex) { // NOSONAR(S1181)
            handle_regular_error(handle, ex, error_code, extra_message);
        } catch (...) { // NOSONAR(S2738)
            handle_unkown_error(handle);
        }
    }

    static void handle_regular_error(PGM_Handle& handle, std::exception const& ex, PGM_Idx error_code,
                                     std::string_view extra_message = "") noexcept {
        handle.err_code = error_code;
        handle.err_msg = ex.what();
        if (!extra_message.empty()) {
            handle.err_msg += extra_message;
        }
    }

    static void handle_unkown_error(PGM_Handle& handle) noexcept {
        handle.err_code = PGM_regular_error;
        handle.err_msg = "Unknown error!\n";
    }
};

// Call function with exception handling using Lippincott pattern
//
// The handle itself is inherently unsafe, as checking for safety would then raise, which in turn would require another
// handle, etc. Therefore, only a nullptr check can be done here.
template <class Functor, class ExceptionHandler = DefaultExceptionHandler>
    requires std::invocable<Functor> && std::invocable<ExceptionHandler const&, PGM_Handle&>
inline auto call_with_catch(PGM_Handle* handle, Functor func, ExceptionHandler const& exception_handler = {}) noexcept(
    noexcept(exception_handler(std::declval<PGM_Handle&>()))) -> std::invoke_result_t<Functor> {
    using ReturnValueType = std::remove_cvref_t<std::invoke_result_t<Functor>>;

    static constexpr std::conditional_t<std::is_void_v<ReturnValueType>, int, ReturnValueType> empty{};

    try {
        if (handle != nullptr) {
            clear_error(handle);
        }
        return func();
    } catch (...) { // NOSONAR(S2738) // Lippincott pattern handles all exceptions if the handle is provided
        if (handle != nullptr) {
            exception_handler(*handle);
        }
        return static_cast<ReturnValueType>(empty);
    }
}
} // namespace power_grid_model_c
