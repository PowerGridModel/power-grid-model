// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_UTILS_HPP
#define POWER_GRID_MODEL_CPP_UTILS_HPP

#include "basics.hpp"
#include "handle.hpp"

#include <algorithm>
#include <complex>
#include <limits>
#include <type_traits>

namespace power_grid_model_cpp {
template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, // NOLINT(modernize-use-constraints,
                                                                 // modernize-type-traits)
                               bool>::type
is_nan(T x) {
    return std::isnan(x);
}
template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, // NOLINT(modernize-use-constraints,
                                                                 // modernize-type-traits)
                               bool>::type
is_nan(std::complex<T> const& x) {
    return is_nan(x.real()) || is_nan(x.imag());
}
inline bool is_nan(int32_t x) { return x == std::numeric_limits<int32_t>::min(); }
inline bool is_nan(int8_t x) { return x == std::numeric_limits<int8_t>::min(); }
template <typename T, std::size_t N> inline bool is_nan(std::array<T, N> const& array) {
    return std::any_of(array.begin(), array.end(), [](T const& element) { return is_nan(element); });
}

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
        return std::forward<Functor>(f).template operator()<int32_t>(std::forward<Args>(args)...);
    case PGM_int8:
        return std::forward<Functor>(f).template operator()<int8_t>(std::forward<Args>(args)...);
    case PGM_double:
        return std::forward<Functor>(f).template operator()<double>(std::forward<Args>(args)...);
    case PGM_double3:
        return std::forward<Functor>(f).template operator()<std::array<double, 3>>(std::forward<Args>(args)...);
    default:
        throw UnsupportedPGM_CType();
    }
}

} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_UTILS_HPP
