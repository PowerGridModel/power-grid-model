// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <type_traits>
#include <vector>

namespace power_grid_model {

// id type
using ID = int32_t;
// idx type
using Idx = int64_t;
using IdxVector = std::vector<Idx>;

using IntS = int8_t;

// struct of indexing to sub modules
struct Idx2D {
    Idx group; // sequence number of outer module/groups
    Idx pos;   // sequence number inside the group

    friend constexpr bool operator==(Idx2D x, Idx2D y) = default;
};

struct Idx2DHash {
    std::size_t operator()(const Idx2D& idx) const {
        size_t const h1 = std::hash<Idx>{}(idx.group);
        size_t const h2 = std::hash<Idx>{}(idx.pos);
        return h1 ^ (h2 << 1);
    }
};

struct symmetric_t {};
struct asymmetric_t {};

template <typename T>
concept symmetry_tag = std::derived_from<T, symmetric_t> || std::derived_from<T, asymmetric_t>;

template <symmetry_tag T> constexpr bool is_symmetric_v = std::derived_from<T, symmetric_t>;
template <symmetry_tag T> constexpr bool is_asymmetric_v = std::derived_from<T, asymmetric_t>;

template <symmetry_tag T> using other_symmetry_t = std::conditional_t<is_symmetric_v<T>, asymmetric_t, symmetric_t>;

// math constant
using namespace std::complex_literals;
using DoubleComplex = std::complex<double>;

using std::numbers::inv_sqrt3;
using std::numbers::pi;
using std::numbers::sqrt3;

constexpr DoubleComplex a2{-0.5, -sqrt3 / 2.0};
constexpr DoubleComplex a{-0.5, sqrt3 / 2.0};
constexpr double deg_30 = 1.0 / 6.0 * pi;
constexpr double deg_120 = 2.0 / 3.0 * pi;
constexpr double deg_240 = 4.0 / 3.0 * pi;
constexpr double numerical_tolerance = 1e-8;
constexpr double nan = std::numeric_limits<double>::quiet_NaN();
constexpr IntS na_IntS = std::numeric_limits<IntS>::min();
constexpr ID na_IntID = std::numeric_limits<ID>::min();
constexpr Idx na_Idx = std::numeric_limits<Idx>::min();

// power grid constant
constexpr double base_power_3p = 1e6;
constexpr double base_power_1p = base_power_3p / 3.0;
template <symmetry_tag sym> constexpr double u_scale = is_symmetric_v<sym> ? 1.0 : inv_sqrt3;
template <symmetry_tag sym> constexpr double base_power = is_symmetric_v<sym> ? base_power_3p : base_power_1p;
// links are direct line between nodes with infinite element_admittance in theory
// for numerical calculation, a big link element_admittance is assigned
// 1e6 Siemens element_admittance in 10kV network
constexpr double g_link = 1e6 / (base_power_3p / 10e3 / 10e3);
constexpr DoubleComplex y_link{g_link, g_link};
// default source short circuit power
constexpr double default_source_sk = 1e10; // 10 GVA 10^10
constexpr double default_source_rx_ratio = 0.1;
constexpr double default_source_z01_ratio = 1.0;

// some usual vector
using DoubleVector = std::vector<double>;
using ComplexVector = std::vector<std::complex<double>>;
using IntSVector = std::vector<IntS>;

template <class T, class... Ts>
concept is_in_list_c = (std::same_as<std::remove_const_t<T>, Ts> || ...);

namespace capturing {
// perfect forward into void
template <class... T>
constexpr void into_the_void(T&&... /*ignored*/) { // NOLINT(cppcoreguidelines-missing-std-forward)
    // do nothing; the constexpr allows all compilers to optimize this away
}
} // namespace capturing

// functor to include all
struct IncludeAll {
    template <class... T> consteval bool operator()(T&&... args) const {
        capturing::into_the_void(std::forward<T>(args)...);
        return true;
    }
};
constexpr IncludeAll include_all{};

} // namespace power_grid_model
