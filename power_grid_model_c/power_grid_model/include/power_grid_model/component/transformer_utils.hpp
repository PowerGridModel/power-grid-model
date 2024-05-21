// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"
#include "../common/enum.hpp"

namespace power_grid_model {

namespace detail {
template <typename T>
concept enum_c = std::is_enum_v<T>;
} // namespace detail

template <typename T>
concept transformer_c = component_c<T> && requires(T const& t, typename T::UpdateType u, typename T::SideType s) {
                                              { t.node(s) } -> std::same_as<ID>;
                                              { t.status(s) } -> std::convertible_to<bool>;

                                              { t.tap_side() } -> std::same_as<typename T::SideType>;
                                              { t.tap_pos() } -> std::convertible_to<IntS>;
                                              { t.tap_min() } -> std::convertible_to<IntS>;
                                              { t.tap_max() } -> std::convertible_to<IntS>;
                                              { t.tap_nom() } -> std::convertible_to<IntS>;
                                          };

constexpr double tap_adjust_impedance(double tap_pos, double tap_min, double tap_max, double tap_nom, double xk,
                                      double xk_min, double xk_max) {
    if (tap_pos <= std::max(tap_nom, tap_max) && tap_pos >= std::min(tap_nom, tap_max)) {
        if (tap_max == tap_nom) {
            return xk;
        }
        double const xk_increment_per_tap = (xk_max - xk) / (tap_max - tap_nom);
        return xk + (tap_pos - tap_nom) * xk_increment_per_tap;
    }
    if (tap_min == tap_nom) {
        return xk;
    }
    double const xk_increment_per_tap = (xk_min - xk) / (tap_min - tap_nom);
    return xk + (tap_pos - tap_nom) * xk_increment_per_tap;
}

constexpr bool is_valid_clock(IntS clock, WindingType winding_from, WindingType winding_to) {
    using enum WindingType;

    bool const clock_in_range = 0 <= clock && clock <= 12;
    bool const clock_is_even = (clock % 2) == 0;

    bool const is_from_wye = winding_from == wye || winding_from == wye_n;
    bool const is_to_wye = winding_to == wye || winding_to == wye_n;

    // even clock number is only possible when both sides are wye winding or both sides aren't
    // and conversely for odd clock number
    bool const correct_clock_winding = (clock_is_even == (is_from_wye == is_to_wye));

    return clock_in_range && correct_clock_winding;
}

// add tap

} // namespace power_grid_model
