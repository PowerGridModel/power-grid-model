// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::main_core {

inline Logger& merge_into(Logger& destination, CalculationInfo const& source) {
    for (const auto& [tag, value] : source.report()) {
        destination.log(tag, value);
    }
    return destination;
}

} // namespace power_grid_model::main_core
