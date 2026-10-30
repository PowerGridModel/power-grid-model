// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <ranges>

namespace power_grid_model {

struct IdxRange : public std::ranges::view_interface<IdxRange> {
  private:
    using underlying_t = std::ranges::iota_view<Idx, Idx>;
    underlying_t view_;

  public:
    constexpr IdxRange() = default;
    template <typename Stop>
        requires std::constructible_from<underlying_t, Idx, Stop>
    constexpr IdxRange(Stop stop) : view_{Idx{0}, stop} {}
    template <typename Start, typename Stop>
        requires std::constructible_from<underlying_t, Start, Stop>
    constexpr IdxRange(Start start, Stop stop) : view_{start, stop} {}

    constexpr auto begin() const { return view_.begin(); }
    constexpr auto end() const { return view_.end(); }
    constexpr auto begin() { return view_.begin(); }
    constexpr auto end() { return view_.end(); }

    constexpr auto size() const { return view_.size(); }
};

static_assert(std::ranges::sized_range<IdxRange>);

using IdxCount = std::ranges::iterator_t<IdxRange>;

#if __cpp_lib_ranges_enumerate >= 20202L
using std::views::enumerate;
#else
constexpr auto enumerate(std::ranges::viewable_range auto&& range_to_enumerate) {
    return std::views::zip(IdxRange{std::ssize(range_to_enumerate)}, range_to_enumerate);
}
#endif

} // namespace power_grid_model
