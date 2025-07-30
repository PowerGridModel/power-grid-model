// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <string>

namespace power_grid_model {

// We would prefer to use std::hardware_destructive_interference_size, but as explained in
// https://discourse.llvm.org/t/rfc-c-17-hardware-constructive-destructive-interference-size/48674/7
// and https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Winterference-size, there is no
// good way to determine the size of the cache line in a portable way at compile-time.
constexpr size_t cache_line_size = 64;

class alignas(cache_line_size) CalculationInfo : public std::map<std::string, double, std::less<>> {};

} // namespace power_grid_model
