// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::main_core {

inline CalculationInfo& merge_into(CalculationInfo& destination, CalculationInfo const& source) {
    static constexpr auto key =
        LogEvent::iterative_pf_solver_max_num_iter; // TODO(mgovers) also add LogEvent::max_num_iter; this is a bug
                                                    // in main
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
