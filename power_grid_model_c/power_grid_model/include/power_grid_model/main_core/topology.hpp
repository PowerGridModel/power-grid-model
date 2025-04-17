// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"
#include "state_queries.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {

namespace detail {

template <typename Component, class ComponentContainer, typename ResType, typename ResFunc>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             std::invocable<std::remove_cvref_t<ResFunc>, Component const&> &&
             std::convertible_to<std::invoke_result_t<ResFunc, Component const&>, ResType>
constexpr void register_topo_components(MainModelState<ComponentContainer> const& state, std::vector<ResType>& target,
                                        ResFunc&& func) {
    auto const begin = get_component_citer<Component>(state).begin();
    auto const end = get_component_citer<Component>(state).end();

    target.resize(std::distance(begin, end));
    std::transform(begin, end, target.begin(), std::forward<ResFunc>(func));
}

} // namespace detail

template <std::same_as<Node> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    comp_topo.n_node = get_component_size<Node>(state);
}

template <std::same_as<Branch> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.branch_node_idx, [&state](Branch const& branch) {
        return BranchIdx{get_component_sequence_idx<Node>(state, branch.from_node()),
                         get_component_sequence_idx<Node>(state, branch.to_node())};
    });
}

template <std::same_as<Branch3> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.branch3_node_idx, [&state](Branch3 const& branch3) {
        return Branch3Idx{get_component_sequence_idx<Node>(state, branch3.node_1()),
                          get_component_sequence_idx<Node>(state, branch3.node_2()),
                          get_component_sequence_idx<Node>(state, branch3.node_3())};
    });
}

template <std::same_as<Source> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.source_node_idx, [&state](Source const& source) {
        return get_component_sequence_idx<Node>(state, source.node());
    });
}

template <std::same_as<Shunt> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(state, comp_topo.shunt_node_idx, [&state](Shunt const& shunt) {
        return get_component_sequence_idx<Node>(state, shunt.node());
    });
}

template <std::same_as<GenericLoadGen> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.load_gen_node_idx,
        [&state](GenericLoadGen const& load_gen) { return get_component_sequence_idx<Node>(state, load_gen.node()); });

    detail::register_topo_components<Component>(state, comp_topo.load_gen_type,
                                                [](GenericLoadGen const& load_gen) { return load_gen.type(); });
}

template <std::same_as<GenericVoltageSensor> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.voltage_sensor_node_idx, [&state](GenericVoltageSensor const& voltage_sensor) {
            return get_component_sequence_idx<Node>(state, voltage_sensor.measured_object());
        });
}

template <std::same_as<GenericPowerSensor> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.power_sensor_object_idx, [&state](GenericPowerSensor const& power_sensor) {
            using enum MeasuredTerminalType;

            auto const measured_object = power_sensor.measured_object();

            switch (power_sensor.get_terminal_type()) {
            case branch_from:
                [[fallthrough]];
            case branch_to:
                return get_component_sequence_idx<Branch>(state, measured_object);
            case source:
                return get_component_sequence_idx<Source>(state, measured_object);
            case shunt:
                return get_component_sequence_idx<Shunt>(state, measured_object);
            case load:
                [[fallthrough]];
            case generator:
                return get_component_sequence_idx<GenericLoadGen>(state, measured_object);
            case branch3_1:
                [[fallthrough]];
            case branch3_2:
                [[fallthrough]];
            case branch3_3:
                return get_component_sequence_idx<Branch3>(state, measured_object);
            case node:
                return get_component_sequence_idx<Node>(state, measured_object);
            default:
                throw MissingCaseForEnumError("Power sensor idx to seq transformation",
                                              power_sensor.get_terminal_type());
            }
        });

    detail::register_topo_components<Component>(
        state, comp_topo.power_sensor_terminal_type,
        [](GenericPowerSensor const& power_sensor) { return power_sensor.get_terminal_type(); });
}

template <std::same_as<GenericCurrentSensor> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.current_sensor_object_idx, [&state](GenericCurrentSensor const& current_sensor) {
            using enum MeasuredTerminalType;

            auto const measured_object = current_sensor.measured_object();

            switch (current_sensor.get_terminal_type()) {
            case branch_from:
                [[fallthrough]];
            case branch_to:
                return get_component_sequence_idx<Branch>(state, measured_object);
            case branch3_1:
                [[fallthrough]];
            case branch3_2:
                [[fallthrough]];
            case branch3_3:
                return get_component_sequence_idx<Branch3>(state, measured_object);
            default:
                throw MissingCaseForEnumError("Current sensor idx to seq transformation",
                                              current_sensor.get_terminal_type());
            }
        });

    detail::register_topo_components<Component>(
        state, comp_topo.current_sensor_terminal_type,
        [](GenericCurrentSensor const& current_sensor) { return current_sensor.get_terminal_type(); });
}

template <std::derived_from<Regulator> Component, class ComponentContainer>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void register_topology_components(MainModelState<ComponentContainer> const& state,
                                            ComponentTopology& comp_topo) {
    detail::register_topo_components<Component>(
        state, comp_topo.regulated_object_idx, [&state](Regulator const& regulator) {
            switch (regulator.regulated_object_type()) {
            case ComponentType::branch:
                return get_component_sequence_idx<Branch>(state, regulator.regulated_object());
            case ComponentType::branch3:
                return get_component_sequence_idx<Branch3>(state, regulator.regulated_object());
            default:
                throw MissingCaseForEnumError("Regulator idx to seq transformation", regulator.regulated_object_type());
            }
        });

    detail::register_topo_components<Component>(state, comp_topo.regulated_object_type, [](Regulator const& regulator) {
        return regulator.regulated_object_type();
    });
}

} // namespace power_grid_model::main_core
