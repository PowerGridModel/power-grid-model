// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "handle.hpp"

#include "power_grid_model/common/exception.hpp"

namespace power_grid_model_c {
class IllegalOperationError : public std::exception {
  public:
    explicit IllegalOperationError(std::string_view message) : msg_{std::format("Illegal operation: {}\n", message)} {}

    char const* what() const noexcept final { return msg_.c_str(); }

  private:
    std::string msg_;
};

// no-op sanitization for clarity that we did think about input input sanitization but we deliberately allow nullptr
template <typename T> constexpr T* safe_ptr_maybe_nullptr(T* ptr) { return ptr; }

template <typename T> constexpr T* safe_ptr(T* ptr) {
    if (ptr) {
        return safe_ptr_maybe_nullptr(ptr);
    }
    throw IllegalOperationError{"Received null pointer when not allowed"};
}

template <typename T> constexpr T& safe_ptr_get(T* ptr) { return *safe_ptr(ptr); }

constexpr std::string_view safe_str_view(char const* str) { return std::string_view{safe_ptr(str)}; }
} // namespace power_grid_model_c
