// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef POWER_GRID_MODEL_CPP_UTILS_HPP
#define POWER_GRID_MODEL_CPP_UTILS_HPP

#include "basics.hpp"
#include "handle.hpp"
#include "meta_data.hpp"

#include <array>
#include <complex>
#include <limits>

namespace power_grid_model_cpp {
inline bool is_nan(IntS const x) { return x == std::numeric_limits<IntS>::min(); }
inline bool is_nan(ID const x) { return x == std::numeric_limits<ID>::min(); }
inline bool is_nan(double const x) { return std::isnan(x); }
inline bool is_nan(std::complex<double> const& x) { return is_nan(x.real()) || is_nan(x.imag()); }
inline bool is_nan(std::array<double, 3> const& array) {
    return is_nan(array[0]) || is_nan(array[1]) || is_nan(array[2]);
}
inline bool is_nan(std::array<std::complex<double>, 3> const& array) {
    return is_nan(array[0]) || is_nan(array[1]) || is_nan(array[2]);
}

constexpr double nan = std::numeric_limits<double>::quiet_NaN();
constexpr int8_t na_IntS = std::numeric_limits<int8_t>::min();
constexpr ID na_IntID = std::numeric_limits<ID>::min();

template <std::same_as<double> T> constexpr T nan_value() { return nan; }
template <std::same_as<std::array<double, 3>> T> constexpr T nan_value() { return {nan, nan, nan}; }
template <std::same_as<ID> T> constexpr T nan_value() { return na_IntID; }
template <std::same_as<int8_t> T> constexpr T nan_value() { return na_IntS; }

class UnsupportedPGM_CType : public PowerGridError {
  public:
    UnsupportedPGM_CType()
        : PowerGridError{[&]() {
              using namespace std::string_literals;
              return "Unsupported PGM_Ctype"s;
          }()} {}
};

template <class Functor, class... Args>
decltype(auto) pgm_type_func_selector(enum PGM_CType type, Functor&& f, Args&&... args) {
    switch (type) {
    case PGM_int32:
        return std::forward<Functor>(f).template operator()<ID>(std::forward<Args>(args)...);
    case PGM_int8:
        return std::forward<Functor>(f).template operator()<IntS>(std::forward<Args>(args)...);
    case PGM_double:
        return std::forward<Functor>(f).template operator()<double>(std::forward<Args>(args)...);
    case PGM_double3:
        return std::forward<Functor>(f).template operator()<std::array<double, 3>>(std::forward<Args>(args)...);
    default:
        throw UnsupportedPGM_CType();
    }
}

template <class Functor, class... Args>
decltype(auto) pgm_type_func_selector(MetaAttribute const* attribute, Functor&& f, Args&&... args) {
    return pgm_type_func_selector(MetaData::attribute_ctype(attribute), std::forward<Functor>(f),
                                  std::forward<Args>(args)...);
}

} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_UTILS_HPP
