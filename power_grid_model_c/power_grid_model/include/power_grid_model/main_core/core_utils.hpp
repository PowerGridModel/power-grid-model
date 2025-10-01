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
    (std::forward<Functor>(functor).template operator()<std::tuple_element_t<Indices, Tuple>>(), ...);
}

} // namespace detail

constexpr Idx sequential{-1};
constexpr Idx invalid_index{-1};

template <typename Tuple, class Functor> constexpr void run_functor_with_tuple_return_void(Functor&& functor) {
    detail::run_functor_with_tuple_index_return_void<Tuple>(std::forward<Functor>(functor),
                                                            std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

} // namespace power_grid_model::main_core::utils
