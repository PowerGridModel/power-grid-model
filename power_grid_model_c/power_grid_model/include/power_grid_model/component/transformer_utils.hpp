// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_TRANSFORMER_UTILS_HPP
#define POWER_GRID_MODEL_COMPONENT_TRANSFORMER_UTILS_HPP

#include "../enum.hpp"

namespace power_grid_model {

constexpr double tap_adjust_impedance(double tap_pos, double tap_min, double tap_max, double tap_nom, double xk,
                                      double xk_min, double xk_max) {
    if (tap_pos <= std::max(tap_nom, tap_max) && tap_pos >= std::min(tap_nom, tap_max)) {
        if (tap_max == tap_nom) {
            return xk;
        }
        else {
            double const xk_increment_per_tap = (xk_max - xk) / (tap_max - tap_nom);
            return xk + (tap_pos - tap_nom) * xk_increment_per_tap;
        }
    }
    else {
        if (tap_min == tap_nom) {
            return xk;
        }
        else {
            double const xk_increment_per_tap = (xk_min - xk) / (tap_min - tap_nom);
            return xk + (tap_pos - tap_nom) * xk_increment_per_tap;
        }
    }
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

}  // namespace power_grid_model

#endif
