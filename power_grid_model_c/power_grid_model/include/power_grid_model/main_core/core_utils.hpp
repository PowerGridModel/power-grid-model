// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <array>
#include <vector>
namespace power_grid_model::main_core::utils {

namespace detail {

template <typename Tuple, class Functor, std::size_t... Indices>
constexpr void run_functor_with_tuple_index_return_void(Functor&& functor, std::index_sequence<Indices...> /*unused*/) {
    if constexpr (sizeof...(Indices) == 1) {
        (std::forward<Functor>(functor).template operator()<std::tuple_element_t<Indices, Tuple>>(), ...);
    } else {
        (functor.template operator()<std::tuple_element_t<Indices, Tuple>>(), ...);
        capturing::into_the_void(std::forward<Functor>(functor));
    }
}

template <typename Tuple, class Functor, std::size_t... Indices>
constexpr auto run_functor_with_tuple_index_return_array(Functor&& functor,
                                                         std::index_sequence<Indices...> /*unused*/) {
    if constexpr (sizeof...(Indices) == 1) {
        return std::array {
            std::forward<Functor>(functor).template operator()<std::tuple_element_t<Indices, Tuple>>()...
        };
    } else {
        auto result = std::array { functor.template operator()<std::tuple_element_t<Indices, Tuple>>()... };
        capturing::into_the_void(std::forward<Functor>(functor));
        return result;
    }
}

} // namespace detail

constexpr Idx sequential{-1};
constexpr Idx invalid_index{-1};

template <typename Tuple, class Functor> constexpr void run_functor_with_tuple_return_void(Functor&& functor) {
    detail::run_functor_with_tuple_index_return_void<Tuple>(std::forward<Functor>(functor),
                                                            std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template <typename Tuple, class Functor> constexpr auto run_functor_with_tuple_return_array(Functor&& functor) {
    return detail::run_functor_with_tuple_index_return_array<Tuple>(
        std::forward<Functor>(functor), std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

} // namespace power_grid_model::main_core::utils
