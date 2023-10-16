// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_TYPING_HPP
#define POWER_GRID_MODEL_TYPING_HPP

#include <concepts>

namespace power_grid_model {
template <std::integral T, std::integral U>
    requires std::convertible_to<U, T>
constexpr auto narrow_cast(U value) {
    if constexpr (std::same_as<T, U>) {
        return value;
    }
    assert(std::in_range<T>(value));
    return static_cast<T>(value);
}
} // namespace power_grid_model

#endif
