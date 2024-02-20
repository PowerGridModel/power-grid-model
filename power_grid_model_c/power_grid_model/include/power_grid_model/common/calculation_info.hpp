// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMMON_CALCULATION_INFO_HPP
#define POWER_GRID_MODEL_COMMON_CALCULATION_INFO_HPP

#include <cstddef>
#include <functional>
#include <map>
#include <string>

#ifdef POWER_GRID_MODEL_CPP_BENCHMARK
#include <new>
#endif

namespace power_grid_model {

// calculation info
#ifdef POWER_GRID_MODEL_CPP_BENCHMARK
#if defined(__cpp_lib_hardware_interference_size)
constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
constexpr size_t cache_line_size = 64;
#endif
class alignas(cache_line_size) CalculationInfo : public std::map<std::string, double, std::less<>> {};
#else
using CalculationInfo = std::map<std::string, double, std::less<>>;
#endif

} // namespace power_grid_model

#endif
