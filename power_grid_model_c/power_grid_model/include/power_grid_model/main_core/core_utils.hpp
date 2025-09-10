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

namespace detail {

template <typename Tuple> struct tuple_type_identities_to_tuple_types;

template <typename... Ts> struct tuple_type_identities_to_tuple_types<std::tuple<std::type_identity<Ts>...>> {
    using type = std::tuple<Ts...>;
};

template <typename Tuple>
using tuple_type_identities_to_tuple_types_t = typename tuple_type_identities_to_tuple_types<Tuple>::type;

template <typename... Types, typename... SelectTypes>
constexpr auto filter_tuple_types(std::tuple<std::type_identity<Types>...> /* types*/,
                                  std::tuple<std::type_identity<SelectTypes>...> /* select_types*/) {
    constexpr auto sub_type_in_type = []<typename T>() { return (std::is_same_v<T, SelectTypes> || ...); };

    return std::tuple_cat(std::conditional_t<sub_type_in_type.template operator()<Types>(),
                                             std::tuple<std::type_identity<Types>>, std::tuple<>>{}...);
}

template <typename Tuple, class Functor, std::size_t... Indices>
constexpr void run_functor_with_tuple_index_return_void(Functor functor, std::index_sequence<Indices...>) {
    (functor.template operator()<std::tuple_element_t<Indices, Tuple>>(), ...);
}

} // namespace detail

constexpr Idx invalid_index{-1};

struct UpdateCompProperties {
    bool has_any_elements{false};                    // whether the component has any elements in the update data
    bool ids_all_na{false};                          // whether all ids are all NA
    bool ids_part_na{false};                         // whether some ids are NA but some are not
    bool dense{false};                               // whether the component is dense
    bool uniform{false};                             // whether the component is uniform
    bool is_columnar{false};                         // whether the component is columnar
    bool update_ids_match{false};                    // whether the ids match
    Idx elements_ps_in_update{utils::invalid_index}; // count of elements for this component per scenario in update
    Idx elements_in_base{utils::invalid_index};      // count of elements for this component per scenario in input

    constexpr bool no_id() const { return !has_any_elements || ids_all_na; }
    constexpr bool qualify_for_optional_id() const {
        return update_ids_match && ids_all_na && uniform && elements_ps_in_update == elements_in_base;
    }
    constexpr bool provided_ids_valid() const {
        return is_empty_component() || (update_ids_match && !(ids_all_na || ids_part_na));
    }
    constexpr bool is_empty_component() const { return !has_any_elements; }
    constexpr bool is_independent() const { return qualify_for_optional_id() || provided_ids_valid(); }
    constexpr Idx get_n_elements() const {
        assert(uniform || elements_ps_in_update == utils::invalid_index);

        return qualify_for_optional_id() ? elements_ps_in_update : na_Idx;
    }
};

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

template <typename Tuple, class Functor> constexpr void run_functor_with_tuple_return_void(Functor functor) {
    detail::run_functor_with_tuple_index_return_void<Tuple>(functor,
                                                            std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template <class T, class U> struct MainModelType;

// TODO: discussion on checking dependent types can also be done here.
template <class... ExtraRetrievableType, class... ComponentType>
struct MainModelType<ExtraRetrievableTypes<ExtraRetrievableType...>, ComponentList<ComponentType...>> {

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
    using UpdateIndependence = std::array<UpdateCompProperties, n_types>;
    using SequenceIdx = std::array<std::vector<Idx2D>, n_types>;
    using SequenceIdxRefWrappers = std::array<std::reference_wrapper<std::vector<Idx2D> const>, n_types>;
    using ComponentFlags = std::array<bool, n_types>;

    // Clean these 2. They are unused
    static constexpr auto branch_param_in_seq_map =
        std::array{index_of_component<Line>, index_of_component<Link>, index_of_component<Transformer>};
    static constexpr auto shunt_param_in_seq_map = std::array{index_of_component<Shunt>};

    template <class Functor> static constexpr void run_functor_with_all_component_types_return_void(Functor functor) {
        (functor.template operator()<ComponentType>(), ...);
    }
    template <class Functor> static constexpr auto run_functor_with_all_component_types_return_array(Functor functor) {
        return std::array { functor.template operator()<ComponentType>()... };
    }
};

} // namespace power_grid_model::main_core::utils
