// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <string>

namespace power_grid_model {

using CalculationInfo = std::map<LogEvent, double>;

} // namespace power_grid_model
