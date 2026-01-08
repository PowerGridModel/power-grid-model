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
class IllegalOperationError : public std::exception {
  public:
    explicit IllegalOperationError(std::string_view message) : msg_{std::format("Illegal operation: {}\n", message)} {}

    char const* what() const noexcept final { return msg_.c_str(); }

  private:
    std::string msg_;
};

template <typename T, typename U> constexpr T compile_time_safe_cast(U const& value) noexcept {
    static_assert(std::convertible_to<U, T>,
                  "Type U is not convertible to type T; compile-time safe cast not possible");
    static_assert(std::same_as<T, std::common_type_t<T, U>>,
                  "Loss of precision during conversion possible; common type differs from target type");

    if constexpr (std::same_as<T, U>) {
        return value;
    } else {
        return static_cast<T>(value);
    }
}

template <std::integral T, std::integral U>
    requires std::convertible_to<U, T>
constexpr auto safe_cast(U value) {
    if constexpr (std::same_as<T, U>) {
        return value;
    } else {
        if (std::in_range<T>(value)) {
            return static_cast<T>(value);
        }
        throw IllegalOperationError{"Value out of range for target type"};
    }
}

template <std::integral T> constexpr T safe_int(PGM_Idx value) { return safe_cast<T>(value); }
template <std::integral T> constexpr T safe_size(PGM_Idx value) {
    if (value >= 0) {
        return safe_cast<T>(value);
    }
    throw IllegalOperationError{"Received negative value for size"};
}
template <std::integral T> constexpr PGM_Idx to_c_size(T value) {
    assert(value >= 0);
    return safe_cast<PGM_Idx>(value);
}

template <std::integral T> constexpr bool safe_bool(T value) { return value != T{0}; }
template <std::integral T> constexpr T to_c_bool(bool value) { return value ? T{1} : T{0}; }

// safe enum conversion from C integer type to C++ enum type.
//
// Just like that. It does not (and cannot) check if the value is one of the pre-defined names.
// However, it does ensure that there is no unexpected behavior like wrap-arounds during the conversion.
// E.g.: for an enum class with underlying type uint8_t, passing 256 would wrap around to 0 without this check.
template <typename T>
    requires std::is_enum_v<T>
constexpr T safe_enum(PGM_Idx value) {
    return static_cast<T>(safe_cast<std::underlying_type_t<T>>(value));
}
template <typename T>
    requires std::is_enum_v<T>
constexpr PGM_Idx to_c_enum(T value) {
    return safe_cast<PGM_Idx>(std::to_underlying(value));
}

// no-op sanitization for clarity that we did think about input sanitization but we deliberately allow nullptr
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
