// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::main_core {

inline CalculationInfo& merge_into(CalculationInfo& dest, CalculationInfo const& src) {
    static auto const key = Timer::make_key(2226, "Max number of iterations");
    for (auto const& [k, v] : src) {
        if (k == key) {
            dest[k] = std::max(dest[k], v);
        } else {
            dest[k] += v;
        }
    }
    return dest;
}

} // namespace power_grid_model::main_core
