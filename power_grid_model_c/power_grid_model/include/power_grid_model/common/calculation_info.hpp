// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <string>

namespace power_grid_model {

#if defined(__cpp_lib_hardware_interference_size)
constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
constexpr size_t cache_line_size = 64;
#endif

class alignas(cache_line_size) CalculationInfo : public std::map<std::string, double, std::less<>> {};

} // namespace power_grid_model
