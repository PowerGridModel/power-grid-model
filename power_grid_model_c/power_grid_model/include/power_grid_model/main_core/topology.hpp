// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {

namespace detail {

template <typename Component, class ComponentContainer, typename ResType, typename ResFunc>
    requires model_component_state<MainModelState, ComponentContainer, Component> &&
             std::invocable<std::remove_cvref_t<ResFunc>, Component const&> &&
             std::convertible_to<std::invoke_result_t<ResFunc, Component const&>, ResType>
constexpr void register_topo_components(MainModelState<ComponentContainer> const& state, std::vector<ResType>& target,
                                        ResFunc&& func) {
    auto const begin = state.components.template citer<Component>().begin();
    auto const end = state.components.template citer<Component>().end();

    target.resize(std::distance(begin, end));
    std::transform(begin, end, target.begin(), func);
}

template <typename Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto get_seq(MainModelState<ComponentContainer> const& state, ID id) {
    return state.components.template get_seq<Component>(id);
}

} // namespace detail

template <std::same_as<Node> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    comp_topo.n_node = state.components.template size<Node>();
}

template <std::same_as<Branch> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.branch_node_idx, [&state](Branch const& branch) {
        return BranchIdx{detail::get_seq<Node>(state, branch.from_node()),
                         detail::get_seq<Node>(state, branch.to_node())};
    });
}

template <std::same_as<Branch3> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.branch3_node_idx, [&state](Branch3 const& branch3) {
        return Branch3Idx{detail::get_seq<Node>(state, branch3.node_1()),
                          detail::get_seq<Node>(state, branch3.node_2()),
                          detail::get_seq<Node>(state, branch3.node_3())};
    });
}

template <std::same_as<Source> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.source_node_idx, [&state](Source const& source) {
        return detail::get_seq<Node>(state, source.node());
    });
}

template <std::same_as<Shunt> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.shunt_node_idx, [&state](Shunt const& shunt) {
        return detail::get_seq<Node>(state, shunt.node());
    });
}

template <std::same_as<GenericLoadGen> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.load_gen_node_idx,
        [&state](GenericLoadGen const& load_gen) { return detail::get_seq<Node>(state, load_gen.node()); });

    detail::register_topo_components<Component>(state, comp_topo.load_gen_type,
                                                [](GenericLoadGen const& load_gen) { return load_gen.type(); });
}

template <std::same_as<GenericVoltageSensor> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.voltage_sensor_node_idx, [&state](GenericVoltageSensor const& voltage_sensor) {
            return detail::get_seq<Node>(state, voltage_sensor.measured_object());
        });
}

template <std::same_as<GenericPowerSensor> Component, class ComponentContainer>
    requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.power_sensor_object_idx, [&state](GenericPowerSensor const& power_sensor) {
            switch (power_sensor.get_terminal_type()) {
                using enum MeasuredTerminalType;

            case branch_from:
            case branch_to:
                return detail::get_seq<Branch>(state, power_sensor.measured_object());
            case source:
                return detail::get_seq<Source>(state, power_sensor.measured_object());
            case shunt:
                return detail::get_seq<Shunt>(state, power_sensor.measured_object());
            case load:
            case generator:
                return detail::get_seq<GenericLoadGen>(state, power_sensor.measured_object());
            case branch3_1:
            case branch3_2:
            case branch3_3:
                return detail::get_seq<Branch3>(state, power_sensor.measured_object());
            case node:
                return detail::get_seq<Node>(state, power_sensor.measured_object());
            default:
                throw MissingCaseForEnumError("Power sensor idx to seq transformation",
                                              power_sensor.get_terminal_type());
            }
        });

    detail::register_topo_components<Component>(
        state, comp_topo.power_sensor_terminal_type,
        [](GenericPowerSensor const& power_sensor) { return power_sensor.get_terminal_type(); });
}

} // namespace power_grid_model::main_core
