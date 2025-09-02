// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../all_components.hpp"
#include "../container.hpp"
#include "state.hpp"

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

template <typename Tuple> struct tuple_type_identities_to_tuple_types;

template <typename... Ts> struct tuple_type_identities_to_tuple_types<std::tuple<std::type_identity<Ts>...>> {
    using type = std::tuple<Ts...>;
};

template <typename... Types, typename... SubTypes>
constexpr auto filter_tuple_types(std::tuple<std::type_identity<Types>...> /*types_tuple*/,
                                  std::tuple<std::type_identity<SubTypes>...> /*sub_types_tuple*/) {
    constexpr auto sub_type_in_type = []<typename T>() { return (std::is_same_v<T, SubTypes> || ...); };

    return std::tuple_cat(std::conditional_t<sub_type_in_type.template operator()<Types>(),
                                             std::tuple<typename std::type_identity<Types>>, std::tuple<>>{}...);
}

// main model implementation template
template <class T, class U> struct MainModelType;

template <class... ExtraRetrievableType, class... ComponentType>
struct MainModelType<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> {

    // using ComponentContainer = Container<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentType...>;
    // using MainModelState = main_core::MainModelState<ComponentContainer>;
    using ComponentTypesTuple = std::tuple<std::type_identity<ComponentType>...>;

    using TopologyTypesTuple =
        std::tuple<std::type_identity<Node>, std::type_identity<Branch>, std::type_identity<Branch3>,
                   std::type_identity<Source>, std::type_identity<Shunt>, std::type_identity<GenericLoadGen>,
                   std::type_identity<GenericVoltageSensor>, std::type_identity<GenericPowerSensor>,
                   std::type_identity<GenericCurrentSensor>, std::type_identity<Regulator>>;
    // using TopologyConnectionTypes = std::tuple<std::type_identity<Branch>, std::type_identity<Branch3>,
    // std::type_identity<Source>>;

    using topology_sequence = typename tuple_type_identities_to_tuple_types<decltype(filter_tuple_types(
        ComponentTypesTuple{}, TopologyTypesTuple{}))>::type;

    static constexpr size_t n_component_types = main_core::utils::n_types<ComponentType...>;
};

} // namespace power_grid_model::main_core::utils
