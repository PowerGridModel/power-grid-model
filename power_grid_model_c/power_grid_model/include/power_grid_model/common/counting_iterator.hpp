// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <ranges>

namespace power_grid_model {

// couting iterator
struct IdxRange : public std::ranges::iota_view<Idx, Idx> {
    using iterator = decltype(std::ranges::iota_view<Idx, Idx>{}.begin());

    using std::ranges::iota_view<Idx, Idx>::iota_view;

    // this overloads the iota_view constructor
    constexpr IdxRange(Idx stop) : std::ranges::iota_view<Idx, Idx>{Idx{0}, stop} {}
};
using IdxCount = typename IdxRange::iterator;

} // namespace power_grid_model
