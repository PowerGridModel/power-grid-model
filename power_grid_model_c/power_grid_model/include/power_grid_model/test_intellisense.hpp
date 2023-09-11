// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_EXCEPTION_HPP
#define POWER_GRID_MODEL_EXCEPTION_HPP

#include <concepts>

namespace power_grid_model::test_intellisense {
using ID = int;

template <typename T>
concept base_input_c = requires(T t) {
    { t.id } -> std::same_as<ID&>;
};

template <typename T>
concept derived_input_c = base_input_c<T> && requires(T t) {
    { t.u_rated } -> std::same_as<double&>;
};

struct Derived {
    ID id{};
    double u_rated{};
};

static_assert(base_input_c<Derived>);
static_assert(derived_input_c<Derived>);
static_assert(base_input_c<Derived>);
static_assert(derived_input_c<Derived>);
} // namespace power_grid_model::test_intellisense

#endif
