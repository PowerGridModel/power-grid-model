// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../all_components.hpp"
#include "../container.hpp"
#include "state.hpp"
#include "update.hpp"

#include <array>
#include <vector>
namespace power_grid_model::main_core {

namespace detail {

template <typename Tuple> struct tuple_type_identities_to_tuple_types;

template <typename... Ts> struct tuple_type_identities_to_tuple_types<std::tuple<std::type_identity<Ts>...>> {
    using type = std::tuple<Ts...>;
};

template <typename Tuple>
using tuple_type_identities_to_tuple_types_t = typename tuple_type_identities_to_tuple_types<Tuple>::type;

template <typename... Types, typename... SelectTypes>
constexpr auto filter_tuple_types(std::tuple<std::type_identity<Types>...> /*unused*/,
                                  std::tuple<std::type_identity<SelectTypes>...> /*unused*/) {
    constexpr auto sub_type_in_type = []<typename T>() { return (std::is_same_v<T, SelectTypes> || ...); };

    return std::tuple_cat(std::conditional_t<sub_type_in_type.template operator()<Types>(),
                                             std::tuple<std::type_identity<Types>>, std::tuple<>>{}...);
}

} // namespace detail

template <class T, class U> class MainModelType;

// TODO: discussion on checking dependent types can also be done here.
template <class... ExtraRetrievableType, class... ComponentType>
class MainModelType<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> {

  public:
    using ComponentContainer = Container<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentType...>;
    using MainModelState = main_core::MainModelState<ComponentContainer>;
    using ComponentTypesTuple = std::tuple<ComponentType...>;

    static constexpr size_t n_types = sizeof...(ComponentType);
    // TODO Should not have to go via container_impl.
    template <class CompType>
    static constexpr size_t index_of_component = container_impl::get_cls_pos_v<CompType, ComponentType...>;

  private:
    static constexpr auto all_types_tuple_v_ =
        std::tuple<std::type_identity<ComponentType>..., std::type_identity<ExtraRetrievableType>...>{};
    // TODO: Would making a unique be necessary? We have Node mentioned as a ExtraRetrievableType and ComponentType so
    // summing up is possible but maybe not appropriate. Would it be better to handle them both separately? using

    static constexpr auto topology_types_tuple_v_ =
        std::tuple<std::type_identity<Node>, std::type_identity<Branch>, std::type_identity<Branch3>,
                   std::type_identity<Source>, std::type_identity<Shunt>, std::type_identity<GenericLoadGen>,
                   std::type_identity<GenericVoltageSensor>, std::type_identity<GenericPowerSensor>,
                   std::type_identity<GenericCurrentSensor>, std::type_identity<Regulator>>{};

    static constexpr auto topology_connection_types_tuple_v_ =
        std::tuple<std::type_identity<Branch>, std::type_identity<Branch3>, std::type_identity<Source>>{};

  public:
    using TopologyTypesTuple = detail::tuple_type_identities_to_tuple_types_t<decltype(detail::filter_tuple_types(
        topology_types_tuple_v_, all_types_tuple_v_))>;
    using TopologyConnectionTypesTuple =
        detail::tuple_type_identities_to_tuple_types_t<decltype(detail::filter_tuple_types(
            topology_connection_types_tuple_v_, all_types_tuple_v_))>;

    // Update related types
    using OwnedUpdateDataset = std::tuple<std::vector<typename ComponentType::UpdateType>...>;
    using SequenceIdxView = std::array<std::span<Idx2D const>, n_types>;
    using UpdateIndependence = std::array<main_core::update::independence::UpdateCompProperties, n_types>;
    using SequenceIdx = std::array<std::vector<Idx2D>, n_types>;
    using SequenceIdxRefWrappers = std::array<std::reference_wrapper<std::vector<Idx2D> const>, n_types>;
    using ComponentFlags = std::array<bool, n_types>;

    // Clean these 2. They are unused
    static constexpr auto branch_param_in_seq_map =
        std::array{index_of_component<Line>, index_of_component<Link>, index_of_component<Transformer>};
    static constexpr auto shunt_param_in_seq_map = std::array{index_of_component<Shunt>};

    template <class Functor> static constexpr void run_functor_with_all_component_types_return_void(Functor&& functor) {
        (std::forward<Functor>(functor).template operator()<ComponentType>(), ...);
    }
    template <class Functor>
    static constexpr auto run_functor_with_all_component_types_return_array(Functor&& functor) {
        return std::array { std::forward<Functor>(functor).template operator()<ComponentType>()... };
    }
};

} // namespace power_grid_model::main_core
