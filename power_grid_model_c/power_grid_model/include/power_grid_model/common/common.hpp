// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cassert>
#include <cmath>
#include <complex>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numbers>
#include <ranges>
#include <type_traits>
#include <utility>
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

    friend constexpr bool operator==(Idx2D x, Idx2D y) { return x.group == y.group && x.pos == y.pos; }
};

struct Idx2DHash {
    std::size_t operator()(Idx2D const& idx) const {
        size_t const h1 = std::hash<Idx>{}(idx.group);
        size_t const h2 = std::hash<Idx>{}(idx.pos);
        return h1 ^ (h2 << 1U);
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
constexpr double deg_30 = (1.0 / 6.0) * pi;
constexpr double deg_120 = (2.0 / 3.0) * pi;
constexpr double deg_240 = (4.0 / 3.0) * pi;
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
// transformer low susceptance ratio for floating zero sequence
constexpr double transformer_low_susceptance_ratio = 1e-8;

// some usual vector
using DoubleVector = std::vector<double>;
using ComplexVector = std::vector<std::complex<double>>;
using IntSVector = std::vector<IntS>;

template <class T, class... Ts>
concept is_in_list_c = (std::same_as<std::remove_const_t<T>, Ts> || ...);

template <class T, class... Ts>
concept derives_from_any_in_list_c = (std::derived_from<T, Ts> || ...);

namespace capturing {
// capture into void
template <class... T> constexpr void into_the_void(T const&... /*ignored*/) {
    // do nothing; the constexpr allows all compilers to optimize this away
}
} // namespace capturing

// functor to include all
struct IncludeAll {
    template <class... T> consteval bool operator()(T const&... /* args */) const { return true; }
};
constexpr IncludeAll include_all{};

// define a non-owning view
namespace detail {
template <class> struct is_owning_view : std::false_type {};
template <class R> struct is_owning_view<std::ranges::owning_view<R>> : std::true_type {};
} // namespace detail
template <class R>
concept non_owning_view_c = std::ranges::view<R> && !detail::is_owning_view<std::remove_cvref_t<R>>::value;

// by ref adaptor to pass to functions which accepts std::ranges::view
template <class R>
concept owning_container_c = std::ranges::viewable_range<R> && !std::ranges::view<R> && !std::ranges::borrowed_range<R>;
constexpr auto by_ref(owning_container_c auto& r) noexcept { return std::ranges::ref_view(r); }
constexpr auto by_ref(owning_container_c auto const& r) noexcept { return std::ranges::ref_view(r); }
constexpr auto by_const_ref(owning_container_c auto& r) noexcept { return by_ref(std::as_const(r)); }

// pgm functor concept
// it should be cheap to copy, so it should be trivially copyable and small in size
template <class T>
concept functor_c = std::is_trivially_copyable_v<T> && (sizeof(T) <= 256);

// function to handle periodic mapping
template <typename T> constexpr T map_to_cyclic_range(T value, T period) {
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type (integral or floating-point)");
    if constexpr (std::is_integral_v<T>) {
        return static_cast<T>((value % period + period) % period);
    } else {
        if consteval {
            T quotient = value / period;
            Idx const floored_quotient =
                (quotient >= T{0}) ? static_cast<Idx>(quotient) : static_cast<Idx>(quotient) - 1;
            T result = value - static_cast<T>(floored_quotient) * period;
            return result;
        }
        return std::fmod(std::fmod(value, period) + period, period);
    }
}

} // namespace power_grid_model
