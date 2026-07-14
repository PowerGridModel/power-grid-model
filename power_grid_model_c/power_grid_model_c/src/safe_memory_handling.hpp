// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <exception>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace power_grid_model_c {
class FailedConstruction : public std::exception {
  public:
    FailedConstruction(std::string_view msg) : msg_{std::format("Failed to create new object: {}\n", msg)} {}
    FailedConstruction(std::bad_alloc const& ex) : FailedConstruction{std::string_view{ex.what()}} {}

    char const* what() const noexcept final { return msg_.c_str(); }

  private:
    std::string msg_;
};

template <class T, typename... Args>
    requires std::constructible_from<T, Args...>
inline T* create(Args&&... args) {
    try {
        return new T{std::forward<Args>(args)...}; // NOSONAR(S5025)
    } catch (std::bad_alloc const& ex) {
        throw FailedConstruction{ex};
    }
}
template <class T> inline void destroy(T* ptr) {
    delete ptr; // NOSONAR(S5025)
}
} // namespace power_grid_model_c
