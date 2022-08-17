// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_TRANSFORMER_UTILS_HPP
#define POWER_GRID_MODEL_COMPONENT_TRANSFORMER_UTILS_HPP

inline double tap_adjust_impedance(const double& tap_pos, const double& tap_min, const double& tap_max,
                                   const double& tap_nom, const double& xk, const double& xk_min,
                                   const double& xk_max) {
    double xk_increment_per_tap{};
    double xk_tap{};
    if (tap_pos <= std::max(tap_nom, tap_max) && tap_pos >= std::min(tap_nom, tap_max)) {
        if (tap_max == tap_nom) {
            xk_tap = xk;
        }
        else {
            xk_increment_per_tap = (xk_max - xk) / (tap_max - tap_nom);
            xk_tap = xk + (tap_pos - tap_nom) * xk_increment_per_tap;
        }
    }
    else {
        if (tap_min == tap_nom) {
            xk_tap = xk;
        }
        else {
            xk_increment_per_tap = (xk_min - xk) / (tap_min - tap_nom);
            xk_tap = xk + (tap_pos - tap_nom) * xk_increment_per_tap;
        }
    }
    return xk_tap;
}

#endif
