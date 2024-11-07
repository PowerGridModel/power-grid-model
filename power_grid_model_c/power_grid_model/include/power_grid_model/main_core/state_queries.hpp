// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {
template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
inline Idx get_component_type_index(MainModelState<ComponentContainer> const& state) {
    return state.components.template get_type_idx<ComponentType>();
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_component_size(MainModelState<ComponentContainer> const& state) {
    return state.components.template size<ComponentType>();
}

template <class ComponentContainer>
    requires main_model_state_c<MainModelState<ComponentContainer>>
inline Idx2D get_component_idx_by_id(MainModelState<ComponentContainer> const& state, ID id) {
    return state.components.get_idx_by_id(id);
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
inline Idx2D get_component_idx_by_id(MainModelState<ComponentContainer> const& state, ID id) {
    return state.components.template get_idx_by_id<ComponentType>(id);
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr Idx get_component_group_idx(MainModelState<ComponentContainer> const& state) {
    return state.components.template get_group_idx<ComponentType>();
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
inline Idx get_component_sequence(MainModelState<ComponentContainer> const& state, auto const& id_or_index) {
    return state.components.template get_seq<ComponentType>(id_or_index);
}

template <std::derived_from<Base> BaseComponent, std::derived_from<Base> Component, class ComponentContainer>
    requires std::derived_from<Component, BaseComponent> &&
             model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto get_component_sequence_offset(MainModelState<ComponentContainer> const& state) {
    return state.components.template get_start_idx<BaseComponent, Component>();
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto& get_component(MainModelState<ComponentContainer> const& state, auto const& id_or_index) {
    return state.components.template get_item<ComponentType>(id_or_index);
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto& get_component(MainModelState<ComponentContainer>& state, auto const& id_or_index) {
    return state.components.template get_item<ComponentType>(id_or_index);
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto& get_component_by_sequence(MainModelState<ComponentContainer> const& state, Idx sequence) {
    return state.components.template get_item_by_seq<ComponentType>(sequence);
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto& get_component_by_sequence(MainModelState<ComponentContainer>& state, Idx sequence) {
    return state.components.template get_item_by_seq<ComponentType>(sequence);
}

template <typename ComponentType, class ComponentContainer, typename... Args>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto emplace_component(MainModelState<ComponentContainer>& state, ID id, Args&&... args) {
    return state.components.template emplace<ComponentType>(id, std::forward<Args>(args)...);
}

template <typename ComponentType, class ComponentContainer, typename... Args>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr void reserve_component(MainModelState<ComponentContainer>& state, std::integral auto size) {
    state.components.template reserve<ComponentType>(size);
}

template <typename ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_component_citer(MainModelState<ComponentContainer> const& state) {
    return state.components.template citer<ComponentType>();
}

template <std::derived_from<Branch> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_topology_index(MainModelState<ComponentContainer> const& state, auto const& id_or_index) {
    return get_component_sequence<Branch>(state, id_or_index);
}

template <std::derived_from<Branch3> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_topology_index(MainModelState<ComponentContainer> const& state, auto const& id_or_index) {
    return get_component_sequence<Branch3>(state, id_or_index);
}

template <std::derived_from<Regulator> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_topology_index(MainModelState<ComponentContainer> const& state, auto const& id_or_index) {
    return get_component_sequence<Regulator>(state, id_or_index);
}

template <std::derived_from<Branch> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_branch_nodes(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.comp_topo->branch_node_idx[topology_sequence_idx];
}

template <std::derived_from<Branch3> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_branch_nodes(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.comp_topo->branch3_node_idx[topology_sequence_idx];
}

template <transformer_c ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType> &&
             requires(MainModelState<ComponentContainer> const& state, Idx const i) {
                 { get_branch_nodes<ComponentType>(state, i)[i] } -> std::convertible_to<Idx>;
             }
inline auto get_topo_node(MainModelState<ComponentContainer> const& state, Idx topology_index,
                          ControlSide control_side) {
    auto const& nodes = get_branch_nodes<ComponentType>(state, topology_index);
    auto const control_side_idx = static_cast<Idx>(control_side);

    assert(0 <= control_side_idx);
    assert(control_side_idx < static_cast<Idx>(nodes.size()));

    return nodes[control_side_idx];
}

template <std::derived_from<Node> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_math_id(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->node[topology_sequence_idx];
}

template <std::derived_from<Branch> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_math_id(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->branch[topology_sequence_idx];
}

template <std::derived_from<Branch3> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_math_id(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->branch3[topology_sequence_idx];
}

template <std::derived_from<Regulator> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_math_id(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->regulator[topology_sequence_idx];
}
} // namespace power_grid_model::main_core
