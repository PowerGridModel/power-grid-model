// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "all_components.hpp"

// main model class
namespace power_grid_model::main_core::utils {

template <typename... ComponentType> constexpr size_t n_component_types = sizeof...(ComponentType);

// run functors with all component types
template <class... Types, class Functor> constexpr void run_functor_with_all_types_return_void(Functor functor) {
    (functor.template operator()<Types>(), ...);
}
template <class... Types, class Functor> constexpr auto run_functor_with_all_types_return_array(Functor functor) {
    return std::array { functor.template operator()<Types>()... };
}

} // namespace power_grid_model::main_core::utils
// TODO: (figueroa1395) move to main_core under utils.hpp
