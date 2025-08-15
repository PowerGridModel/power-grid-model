// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::main_core {

inline CalculationInfo& merge_into(CalculationInfo& destination, CalculationInfo const& source) {
    destination.merge(source);
    return destination;
}

} // namespace power_grid_model::main_core
