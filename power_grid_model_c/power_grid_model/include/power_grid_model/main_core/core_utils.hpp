// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../container.hpp"
#include "state.hpp"

#include <array>
#include <vector>
namespace power_grid_model::main_core::utils {

namespace detail {

template <typename Tuple, class Functor, std::size_t... Indices>
constexpr void run_functor_with_tuple_index_return_void(Functor&& functor, std::index_sequence<Indices...> /*unused*/) {
    (std::forward<Functor>(functor).template operator()<std::tuple_element_t<Indices, Tuple>>(), ...);
}

} // namespace detail

constexpr Idx invalid_index{-1};

/////////////////// To remove ///////////////////

template <class... ComponentTypes> constexpr size_t n_types = sizeof...(ComponentTypes);
template <class CompType, class... ComponentTypes>
constexpr size_t index_of_component = container_impl::get_cls_pos_v<CompType, ComponentTypes...>;

template <class... ComponentTypes> using SequenceIdx = std::array<std::vector<Idx2D>, n_types<ComponentTypes...>>;
template <class... ComponentTypes> using ComponentFlags = std::array<bool, n_types<ComponentTypes...>>;

// run functors with all component types
template <class... Types, class Functor> constexpr void run_functor_with_all_types_return_void(Functor functor) {
    (functor.template operator()<Types>(), ...);
}
template <class... Types, class Functor> constexpr auto run_functor_with_all_types_return_array(Functor functor) {
    return std::array { functor.template operator()<Types>()... };
}
/////////////////// To remove ///////////////////

template <typename Tuple, class Functor> constexpr void run_functor_with_tuple_return_void(Functor&& functor) {
    detail::run_functor_with_tuple_index_return_void<Tuple>(std::forward<Functor>(functor),
                                                            std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

} // namespace power_grid_model::main_core::utils
