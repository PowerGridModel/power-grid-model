// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {

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
