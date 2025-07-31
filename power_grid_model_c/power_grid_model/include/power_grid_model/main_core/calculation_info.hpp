// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::main_core {

inline CalculationInfo& merge_into(CalculationInfo& destination, CalculationInfo const& source) {
    static auto const key = Timer::make_key(2226, "Max number of iterations");
    for (auto const& [k, v] : source) {
        if (k == key) {
            destination[k] = std::max(destination[k], v);
        } else {
            destination[k] += v;
        }
    }
    return destination;
}

} // namespace power_grid_model::main_core
