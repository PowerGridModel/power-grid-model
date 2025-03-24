// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../all_components.hpp"
#include "../container.hpp"

#include <array>
#include <vector>

namespace power_grid_model::main_core::utils {

constexpr Idx invalid_index{-1};

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

} // namespace power_grid_model::main_core::utils
