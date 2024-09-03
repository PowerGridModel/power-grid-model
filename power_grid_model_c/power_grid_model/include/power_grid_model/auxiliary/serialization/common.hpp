// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <concepts>

namespace power_grid_model::meta_data::detail {

struct row_based_t {};
struct columnar_t {};
constexpr row_based_t row_based{};
constexpr columnar_t columnar{};

template <typename T>
concept row_based_or_columnar_c = std::derived_from<T, row_based_t> || std::derived_from<T, columnar_t>;

} // namespace power_grid_model::meta_data::detail
