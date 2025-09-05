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

template <typename Tuple>
using tuple_type_identities_to_tuple_types_t = typename tuple_type_identities_to_tuple_types<Tuple>::type;

template <typename... Types, typename... SelectTypes>
constexpr auto filter_tuple_types(std::tuple<std::type_identity<Types>...>,
                                  std::tuple<std::type_identity<SelectTypes>...>) {
    constexpr auto sub_type_in_type = []<typename T>() { return (std::is_same_v<T, SelectTypes> || ...); };

    return std::tuple_cat(std::conditional_t<sub_type_in_type.template operator()<Types>(),
                                             std::tuple<typename std::type_identity<Types>>, std::tuple<>>{}...);
}

// concept validate_component_types_c =
//     dependent_type_check<Source, Node, ComponentType...> && dependent_type_check<Line, Node, ComponentType...> &&
//     dependent_type_check<ThreeWindingTransformer, Node, ComponentType...> &&
//     dependent_type_check<Shunt, Node, ComponentType...> &&
//     dependent_type_check<GenericLoadGen, Node, ComponentType...> &&
//     dependent_type_check<GenericVoltageSensor, Node, ComponentType...>;

// main model implementation template
template <class T, class U> struct MainModelType;

template <class... ExtraRetrievableType, class... ComponentType>
// TODO: discussion on checking dependent types can also be done here.
// requires validate_component_types_c<ComponentType...>
struct MainModelType<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> {

    using ComponentContainer = Container<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentType...>;
    using MainModelState = main_core::MainModelState<ComponentContainer>;
    using ExtraRetrievableTypesTuple = std::tuple<std::type_identity<ExtraRetrievableType>...>;
    using ComponentTypesTuple = std::tuple<ComponentType...>;

    // TODO: Clean up using-s
    using _AllTypesTuple =
        std::tuple<std::type_identity<ComponentType>..., std::type_identity<ExtraRetrievableType>...>;
    // TODO: Would making a unique be necessary? We have Node mentioned as a ExtraRetrievableType and ComponentType so
    // summing up is possible but maybe not appropriate. Would it be better to handle them both separately? using
    // _AllTypesTupleUnique = typename tuple_type_identities_to_tuple_types<decltype(filter_tuple_types(
    // _TopologyTypesTuple{}, _AllTypesTuple{}))>::type;

    using _ComponentTypesTuple = std::tuple<std::type_identity<ComponentType>...>;
    using _TopologyTypesTuple =
        std::tuple<std::type_identity<Node>, std::type_identity<Branch>, std::type_identity<Branch3>,
                   std::type_identity<Source>, std::type_identity<Shunt>, std::type_identity<GenericLoadGen>,
                   std::type_identity<GenericVoltageSensor>, std::type_identity<GenericPowerSensor>,
                   std::type_identity<GenericCurrentSensor>, std::type_identity<Regulator>>;
    using _TopologyConnectionTypes =
        std::tuple<std::type_identity<Branch>, std::type_identity<Branch3>, std::type_identity<Source>>;

    using TopologyTypesTuple =
        tuple_type_identities_to_tuple_types_t<decltype(filter_tuple_types(_TopologyTypesTuple{}, _AllTypesTuple{}))>;
    using TopologyConnectionTypesTuple = tuple_type_identities_to_tuple_types_t<decltype(filter_tuple_types(
        _TopologyConnectionTypes{}, _AllTypesTuple{}))>;

    static constexpr size_t n_component_types = main_core::utils::n_types<ComponentType...>;
};

} // namespace power_grid_model::main_core::utils
