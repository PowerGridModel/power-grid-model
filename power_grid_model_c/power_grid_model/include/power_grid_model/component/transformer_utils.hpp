// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_TRANSFORMER_UTILS_HPP
#define POWER_GRID_MODEL_COMPONENT_TRANSFORMER_UTILS_HPP

inline double tap_adjust_impedance(double tap_pos, double tap_min, double tap_max, double tap_nom, double xk,
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

// add tap

#endif
