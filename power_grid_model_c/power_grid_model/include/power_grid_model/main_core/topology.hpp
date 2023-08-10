// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_TOPOLOGY_HPP
#define POWER_GRID_MODEL_MAIN_CORE_TOPOLOGY_HPP

#include "state.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {
template <std::same_as<Node> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.n_node = state.components.template size<Node>();
}

template <std::same_as<Branch> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.branch_node_idx.resize(state.components.template size<Branch>());
    std::transform(state.components.template citer<Branch>().begin(), state.components.template citer<Branch>().end(),
                   comp_topo.branch_node_idx.begin(), [&state](Branch const& branch) {
                       return BranchIdx{state.components.template get_seq<Node>(branch.from_node()),
                                        state.components.template get_seq<Node>(branch.to_node())};
                   });
}

template <std::same_as<Branch3> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.branch3_node_idx.resize(state.components.template size<Branch3>());
    std::transform(state.components.template citer<Branch3>().begin(), state.components.template citer<Branch3>().end(),
                   comp_topo.branch3_node_idx.begin(), [&state](Branch3 const& branch3) {
                       return Branch3Idx{state.components.template get_seq<Node>(branch3.node_1()),
                                         state.components.template get_seq<Node>(branch3.node_2()),
                                         state.components.template get_seq<Node>(branch3.node_3())};
                   });
}

template <std::same_as<Source> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.source_node_idx.resize(state.components.template size<Source>());
    std::transform(state.components.template citer<Source>().begin(), state.components.template citer<Source>().end(),
                   comp_topo.source_node_idx.begin(),
                   [&state](Source const& source) { return state.components.template get_seq<Node>(source.node()); });
}

template <std::same_as<Shunt> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.shunt_node_idx.resize(state.components.template size<Shunt>());
    std::transform(state.components.template citer<Shunt>().begin(), state.components.template citer<Shunt>().end(),
                   comp_topo.shunt_node_idx.begin(),
                   [&state](Shunt const& shunt) { return state.components.template get_seq<Node>(shunt.node()); });
}

template <std::same_as<GenericLoadGen> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.load_gen_node_idx.resize(state.components.template size<GenericLoadGen>());
    std::transform(
        state.components.template citer<GenericLoadGen>().begin(),
        state.components.template citer<GenericLoadGen>().end(), comp_topo.load_gen_node_idx.begin(),
        [&state](GenericLoadGen const& load_gen) { return state.components.template get_seq<Node>(load_gen.node()); });
    comp_topo.load_gen_type.resize(state.components.template size<GenericLoadGen>());
    std::transform(state.components.template citer<GenericLoadGen>().begin(),
                   state.components.template citer<GenericLoadGen>().end(), comp_topo.load_gen_type.begin(),
                   [](GenericLoadGen const& load_gen) { return load_gen.type(); });
}

template <std::same_as<GenericVoltageSensor> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.voltage_sensor_node_idx.resize(state.components.template size<GenericVoltageSensor>());
    std::transform(state.components.template citer<GenericVoltageSensor>().begin(),
                   state.components.template citer<GenericVoltageSensor>().end(),
                   comp_topo.voltage_sensor_node_idx.begin(), [&state](GenericVoltageSensor const& voltage_sensor) {
                       return state.components.template get_seq<Node>(voltage_sensor.measured_object());
                   });
}

template <std::same_as<GenericPowerSensor> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {
    comp_topo.power_sensor_object_idx.resize(state.components.template size<GenericPowerSensor>());
    std::transform(state.components.template citer<GenericPowerSensor>().begin(),
                   state.components.template citer<GenericPowerSensor>().end(),
                   comp_topo.power_sensor_object_idx.begin(), [&state](GenericPowerSensor const& power_sensor) {
                       switch (power_sensor.get_terminal_type()) {
                           using enum MeasuredTerminalType;

                       case branch_from:
                       case branch_to:
                           return state.components.template get_seq<Branch>(power_sensor.measured_object());
                       case source:
                           return state.components.template get_seq<Source>(power_sensor.measured_object());
                       case shunt:
                           return state.components.template get_seq<Shunt>(power_sensor.measured_object());
                       case load:
                       case generator:
                           return state.components.template get_seq<GenericLoadGen>(power_sensor.measured_object());
                       case branch3_1:
                       case branch3_2:
                       case branch3_3:
                           return state.components.template get_seq<Branch3>(power_sensor.measured_object());
                       case node:
                           return state.components.template get_seq<Node>(power_sensor.measured_object());
                       default:
                           throw MissingCaseForEnumError("Power sensor idx to seq transformation",
                                                         power_sensor.get_terminal_type());
                       }
                   });
    comp_topo.power_sensor_terminal_type.resize(state.components.template size<GenericPowerSensor>());
    std::transform(state.components.template citer<GenericPowerSensor>().begin(),
                   state.components.template citer<GenericPowerSensor>().end(),
                   comp_topo.power_sensor_terminal_type.begin(),
                   [](GenericPowerSensor const& power_sensor) { return power_sensor.get_terminal_type(); });
}

template <std::same_as<Fault> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
void register_topology_components(MainModelState<ComponentContainer> const& state, ComponentTopology& comp_topo) {}
} // namespace power_grid_model::main_core

#endif
