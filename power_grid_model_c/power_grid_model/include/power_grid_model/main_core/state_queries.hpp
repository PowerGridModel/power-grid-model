// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "container_queries.hpp"
#include "state.hpp"

#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../component/branch.hpp"
#include "../component/branch3.hpp"
#include "../component/edge.hpp"
#include "../component/node.hpp"
#include "../component/regulator.hpp"
#include "../component/shunt.hpp"
#include "../component/source.hpp"
#include "../component/transformer_utils.hpp"

#include <cassert>
#include <concepts>

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
    auto const control_side_idx = narrow_cast<Idx>(std::to_underlying(control_side));

    assert(0 <= control_side_idx);
    assert(control_side_idx < std::ranges::ssize(nodes));

    return nodes[control_side_idx];
}

template <std::same_as<Node> ComponentType, class ComponentContainer>
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

template <std::derived_from<Shunt> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_math_id(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->shunt[topology_sequence_idx];
}

template <std::derived_from<Source> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_math_id(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->source[topology_sequence_idx];
}

template <std::derived_from<Regulator> ComponentType, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, ComponentType>
constexpr auto get_math_id(MainModelState<ComponentContainer> const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->regulator[topology_sequence_idx];
}

template <std::same_as<Node> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.reduced_topology->topo_node_coup.coupling.user_nodes_to_topo_nodes.cbegin();
}

template <std::derived_from<Branch> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->branch.cbegin() + get_component_sequence_offset<Branch, Component>(state.components);
}

template <std::same_as<Link> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    auto const& link_topo_ids = state.reduced_topology->topo_node_coup.coupling.user_links_to_topo_nodes;

    if (std::ranges::ssize(link_topo_ids) == get_component_size<Link>(state.components)) {
        // new path: links are not branches
        return link_topo_ids.cbegin();
    } else {
        // legacy path: links are branches
        return state.topo_comp_coup->branch.cbegin() + get_component_sequence_offset<Edge, Link>(state.components);
    }
}

template <std::derived_from<Branch3> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->branch3.cbegin() + get_component_sequence_offset<Branch3, Component>(state.components);
}

template <std::same_as<Source> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->source.cbegin();
}

template <std::derived_from<GenericLoadGen> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->load_gen.cbegin() +
           get_component_sequence_offset<GenericLoadGen, Component>(state.components);
}

template <std::same_as<Shunt> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->shunt.cbegin();
}

template <std::derived_from<GenericVoltageSensor> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_topo->voltage_sensor_node_idx.cbegin() +
           get_component_sequence_offset<GenericVoltageSensor, Component>(state.components);
}

template <std::derived_from<GenericPowerSensor> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_topo->power_sensor_object_idx.cbegin() +
           get_component_sequence_offset<GenericPowerSensor, Component>(state.components);
}

template <std::derived_from<GenericCurrentSensor> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_topo->current_sensor_object_idx.cbegin() +
           get_component_sequence_offset<GenericCurrentSensor, Component>(state.components);
}

template <std::same_as<Fault> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_coup.fault.cbegin();
}

template <std::derived_from<Regulator> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_topo->regulated_object_idx.cbegin() +
           get_component_sequence_offset<Regulator, Component>(state.components);
}

template <std::derived_from<Base> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence(MainModelState<ComponentContainer> const& state) {
    auto const start = comp_base_sequence_cbegin<Component>(state);
    return std::ranges::subrange{start, start + get_component_size<Component>(state.components)};
}

} // namespace power_grid_model::main_core
