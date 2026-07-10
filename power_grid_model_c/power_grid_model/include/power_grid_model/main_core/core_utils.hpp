// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

namespace power_grid_model::main_core::utils {

namespace detail {

template <typename Tuple, std::size_t... Indices>
constexpr void run_functor_with_tuple_index_return_void(functor_c auto functor,
                                                        std::index_sequence<Indices...> /*unused*/) {
    if constexpr (sizeof...(Indices) == 1) {
        (functor.template operator()<std::tuple_element_t<Indices, Tuple>>(), ...);
    } else {
        (functor.template operator()<std::tuple_element_t<Indices, Tuple>>(), ...);
        capturing::into_the_void(functor);
    }
}

template <typename Tuple, std::size_t... Indices>
constexpr auto run_functor_with_tuple_index_return_array(functor_c auto functor,
                                                         std::index_sequence<Indices...> /*unused*/) {
    if constexpr (sizeof...(Indices) == 1) {
        return std::array { functor.template operator()<std::tuple_element_t<Indices, Tuple>>()... };
    } else {
        auto result = std::array { functor.template operator()<std::tuple_element_t<Indices, Tuple>>()... };
        capturing::into_the_void(functor);
        return result;
    }
}

} // namespace detail

constexpr Idx sequential{-1};
constexpr Idx invalid_index{-1};

template <typename Tuple> constexpr void run_functor_with_tuple_return_void(functor_c auto functor) {
    detail::run_functor_with_tuple_index_return_void<Tuple>(functor,
                                                            std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template <typename Tuple> constexpr auto run_functor_with_tuple_return_array(functor_c auto functor) {
    return detail::run_functor_with_tuple_index_return_array<Tuple>(
        functor, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

} // namespace power_grid_model::main_core::utils
