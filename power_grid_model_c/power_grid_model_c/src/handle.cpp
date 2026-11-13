// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/common/error_codes.hpp"
#include <cstdint>
#include <type_traits>
#include <utility>
#define PGM_DLL_EXPORTS

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/handle.h"

#include "handle.hpp"
#include "input_sanitization.hpp"
#include "safe_memory_handling.hpp"

#include <algorithm>
#include <bit>
#include <iterator>
#include <limits>

namespace {
using namespace power_grid_model;

using power_grid_model_c::clear_error;
using power_grid_model_c::compile_time_safe_cast;
using power_grid_model_c::create;
using power_grid_model_c::destroy;
using power_grid_model_c::optional_safe_cast;
using power_grid_model_c::to_c_bool;

constexpr char const* version = PGM_VERSION;
} // namespace

PGM_Handle::PGM_Handle() = default;

PGM_Handle::~PGM_Handle() noexcept = default;
PGM_Handle::PGM_Handle(PGM_Handle&&) noexcept = default;
PGM_Handle& PGM_Handle::operator=(PGM_Handle&&) noexcept = default;

// create and destroy handle
PGM_Handle* PGM_create_handle() noexcept {
    try {
        return create<PGM_Handle>();
    } catch (...) {
        return nullptr;
    }
}
void PGM_destroy_handle(PGM_Handle* handle) noexcept { destroy(handle); }

// error handling
PGM_Idx PGM_error_code(PGM_Handle const* handle) noexcept {
    return handle != nullptr ? compile_time_safe_cast<PGM_Idx>(handle->err_code) : PGM_Idx{0};
}
char const* PGM_error_message(PGM_Handle const* handle) noexcept {
    return handle != nullptr ? handle->err_msg.c_str() : nullptr;
}
PGM_Idx PGM_n_failed_scenarios(PGM_Handle const* handle) noexcept {
    return handle != nullptr ? compile_time_safe_cast<PGM_Idx>(std::ssize(handle->failed_scenarios)) : PGM_Idx{0};
}
PGM_Idx const* PGM_failed_scenarios(PGM_Handle const* handle) noexcept {
    return handle != nullptr ? compile_time_safe_cast<PGM_Idx const*>(handle->failed_scenarios.data()) : nullptr;
}
PGM_Idx const* PGM_batch_error_codes(PGM_Handle const* handle) noexcept {
    return handle != nullptr ? compile_time_safe_cast<PGM_Idx const*>(handle->batch_error_codes.data()) : nullptr;
}
char const** PGM_batch_errors(PGM_Handle const* handle) noexcept {
    if (handle == nullptr) {
        return nullptr;
    }
    auto const& handle_ref = *handle;
    handle_ref.batch_errs_c_str.clear();
    std::ranges::transform(handle_ref.batch_errs, std::back_inserter(handle_ref.batch_errs_c_str),
                           [](auto const& x) { return x.c_str(); });
    return handle_ref.batch_errs_c_str.data();
}
void PGM_clear_error(PGM_Handle* handle) noexcept { clear_error(handle); }
char const* PGM_version(void) noexcept { return version; }

namespace {
using underlying_error_code_t = std::underlying_type_t<power_grid_model::PowerGridErrorCode>;

bool is_sub_error_code_of(underlying_error_code_t derived, underlying_error_code_t base) noexcept {
    using limits = std::numeric_limits<underlying_error_code_t>;
    constexpr auto width =
        compile_time_safe_cast<underlying_error_code_t>(power_grid_model::power_grid_error_code_level_width);
    constexpr auto max_width =
        compile_time_safe_cast<underlying_error_code_t>(power_grid_model::power_grid_error_code_max_width);

    static_assert(std::is_unsigned_v<underlying_error_code_t>);
    static_assert(limits::digits == max_width);
    static_assert(width > 0 && max_width % width == 0); // guarantees the shift below stays within [0, digits)

    // the root code (0) only has itself as sub code
    if (base == 0u) {
        return derived == 0u;
    }
    // keep as many low levels as base occupies; base != 0 implies n_levels in [1, max_width / width]
    auto const n_levels = static_cast<underlying_error_code_t>((std::bit_width(base) + width - 1u) / width);
    auto const mask = static_cast<underlying_error_code_t>(limits::max() >> (limits::digits - n_levels * width));
    return (derived & mask) == base;
}
} // namespace

PGM_Idx PGM_error_code_is_subcode_of(PGM_Idx error_code, PGM_Idx base_error_code) noexcept {
    auto const safe_error_code = optional_safe_cast<underlying_error_code_t>(error_code);
    auto const safe_base_error_code = optional_safe_cast<underlying_error_code_t>(base_error_code);
    return to_c_bool<PGM_Idx>(safe_error_code.has_value() && safe_base_error_code.has_value() &&
                              is_sub_error_code_of(*safe_error_code, *safe_base_error_code));
}

