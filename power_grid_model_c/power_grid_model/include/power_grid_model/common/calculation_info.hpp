// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <string>

#ifdef POWER_GRID_MODEL_CPP_BENCHMARK
#include <new>
#endif

namespace power_grid_model {

using CalculationInfo = std::map<std::string, double, std::less<>>;

} // namespace power_grid_model
