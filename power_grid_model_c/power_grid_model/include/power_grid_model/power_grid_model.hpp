// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_POWER_GRID_MODEL_HPP
#define POWER_GRID_MODEL_POWER_GRID_MODEL_HPP

// main header for the model
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "boost/iterator/counting_iterator.hpp"

namespace power_grid_model {

// id type
using ID = int32_t;
// idx type
using Idx = int64_t;
using IdxVector = std::vector<Idx>;

using IntS = int8_t;

// couting iterator
using IdxCount = boost::counting_iterator<Idx>;

// struct of indexing to sub modules
struct Idx2D {
    Idx group;  // sequence number of outer module/groups
    Idx pos;    //  sequence number inside the group
};
inline bool operator==(Idx2D x, Idx2D y) {
    return x.group == y.group && x.pos == y.pos;
}

// math constant
using namespace std::complex_literals;
using DoubleComplex = std::complex<double>;
constexpr double sqrt3 = 1.73205080756887729;
constexpr double sqrt3_inv = 1.0 / sqrt3;
constexpr DoubleComplex a2{-0.5, -sqrt3 / 2.0};
constexpr DoubleComplex a{-0.5, sqrt3 / 2.0};
constexpr double pi = 3.14159265358979323;
constexpr double deg_30 = 1.0 / 6.0 * pi;
constexpr double deg_120 = 2.0 / 3.0 * pi;
constexpr double deg_240 = 4.0 / 3.0 * pi;
constexpr double numerical_tolerance = 1e-8;
constexpr double nan = std::numeric_limits<double>::quiet_NaN();
constexpr IntS na_IntS = std::numeric_limits<IntS>::min();
constexpr ID na_IntID = std::numeric_limits<ID>::min();

// power grid constant
constexpr double base_power_3p = 1e6;
constexpr double base_power_1p = base_power_3p / 3.0;
template <bool sym>
constexpr double u_scale = sym ? 1.0 : sqrt3_inv;
template <bool sym>
constexpr double base_power = sym ? base_power_3p : base_power_1p;
// links are direct line between nodes with infinite element_admittance in theory
// for numerical calculation, a big link element_admittance is assigned
// 1e6 Siemens element_admittance in 10kV network
constexpr double g_link = 1e6 / (base_power_3p / 10e3 / 10e3);
constexpr DoubleComplex y_link{g_link, g_link};
// default source short circuit power
constexpr double default_source_sk = 1e10;  // 10 GVA 10^10
constexpr double default_source_rx_ratio = 0.1;
constexpr double default_source_z01_ratio = 1.0;

// calculation info
using CalculationInfo = std::map<std::string, double>;
using Clock = std::chrono::high_resolution_clock;
using Duration = std::chrono::duration<double>;

// some usual vector
using DoubleVector = std::vector<double>;
using ComplexVector = std::vector<std::complex<double>>;
using IntSVector = std::vector<IntS>;

// component list
template <class... T>
struct ComponentList {};

// batch parameter
struct BatchParameter {
    bool independent;     // all update datasets consists of exactly the same components
    bool cache_topology;  // there are no changes in topology (branch, source) in the update datasets
};

}  // namespace power_grid_model

#endif
