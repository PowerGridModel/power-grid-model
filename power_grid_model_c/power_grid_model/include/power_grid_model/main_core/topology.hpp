// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "container_queries.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {

namespace detail {

template <typename Component, class ComponentContainer, typename ResType, typename ResFunc>
    requires common::component_container_c<ComponentContainer, Component> &&
             std::invocable<std::remove_cvref_t<ResFunc>, Component const&> &&
             std::convertible_to<std::invoke_result_t<ResFunc, Component const&>, ResType>
constexpr void apply_registration(ComponentContainer const& components, std::vector<ResType>& target, ResFunc&& func) {
    auto const begin = components.template citer<Component>().begin();
    auto const end = components.template citer<Component>().end();

    target.resize(std::distance(begin, end));
    std::transform(begin, end, target.begin(), std::forward<ResFunc>(func));
}

template <std::same_as<Node> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    comp_topo.n_node = get_component_size<Node>(components);
}

template <std::same_as<Branch> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Node>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(components, comp_topo.branch_node_idx, [&components](Branch const& branch) {
        return BranchIdx{get_component_sequence_idx<Node>(components, branch.from_node()),
                         get_component_sequence_idx<Node>(components, branch.to_node())};
    });
}

template <std::same_as<Branch3> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Node>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(components, comp_topo.branch3_node_idx, [&components](Branch3 const& branch3) {
        return Branch3Idx{get_component_sequence_idx<Node>(components, branch3.node_1()),
                          get_component_sequence_idx<Node>(components, branch3.node_2()),
                          get_component_sequence_idx<Node>(components, branch3.node_3())};
    });
}

template <std::same_as<Source> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Node>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(components, comp_topo.source_node_idx, [&components](Source const& source) {
        return get_component_sequence_idx<Node>(components, source.node());
    });
}

template <std::same_as<Shunt> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Node>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(components, comp_topo.shunt_node_idx, [&components](Shunt const& shunt) {
        return get_component_sequence_idx<Node>(components, shunt.node());
    });
}

template <std::same_as<GenericLoadGen> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Node>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(components, comp_topo.load_gen_node_idx,
                                  [&components](GenericLoadGen const& load_gen) {
                                      return get_component_sequence_idx<Node>(components, load_gen.node());
                                  });

    apply_registration<Component>(components, comp_topo.load_gen_type,
                                  [](GenericLoadGen const& load_gen) { return load_gen.type(); });
}

template <std::same_as<GenericVoltageSensor> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Node>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(
        components, comp_topo.voltage_sensor_node_idx, [&components](GenericVoltageSensor const& voltage_sensor) {
            return get_component_sequence_idx<Node>(components, voltage_sensor.measured_object());
        });
}

template <std::same_as<GenericPowerSensor> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Branch, Source, Shunt, GenericLoadGen,
                                           Branch3, Node>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(
        components, comp_topo.power_sensor_object_idx, [&components](GenericPowerSensor const& power_sensor) {
            using enum MeasuredTerminalType;

            auto const measured_object = power_sensor.measured_object();

            switch (power_sensor.get_terminal_type()) {
            case branch_from:
                [[fallthrough]];
            case branch_to:
                return get_component_sequence_idx<Branch>(components, measured_object);
            case source:
                return get_component_sequence_idx<Source>(components, measured_object);
            case shunt:
                return get_component_sequence_idx<Shunt>(components, measured_object);
            case load:
                [[fallthrough]];
            case generator:
                return get_component_sequence_idx<GenericLoadGen>(components, measured_object);
            case branch3_1:
                [[fallthrough]];
            case branch3_2:
                [[fallthrough]];
            case branch3_3:
                return get_component_sequence_idx<Branch3>(components, measured_object);
            case node:
                return get_component_sequence_idx<Node>(components, measured_object);
            default:
                throw MissingCaseForEnumError("Power sensor idx to seq transformation",
                                              power_sensor.get_terminal_type());
            }
        });

    apply_registration<Component>(
        components, comp_topo.power_sensor_terminal_type,
        [](GenericPowerSensor const& power_sensor) { return power_sensor.get_terminal_type(); });
}

template <std::same_as<GenericCurrentSensor> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Branch, Branch3>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(components, comp_topo.current_sensor_object_idx,
                                  [&components](GenericCurrentSensor const& current_sensor) {
                                      using enum MeasuredTerminalType;

                                      auto const measured_object = current_sensor.measured_object();

                                      switch (current_sensor.get_terminal_type()) {
                                      case branch_from:
                                          [[fallthrough]];
                                      case branch_to:
                                          return get_component_sequence_idx<Branch>(components, measured_object);
                                      case branch3_1:
                                          [[fallthrough]];
                                      case branch3_2:
                                          [[fallthrough]];
                                      case branch3_3:
                                          return get_component_sequence_idx<Branch3>(components, measured_object);
                                      default:
                                          throw MissingCaseForEnumError("Current sensor idx to seq transformation",
                                                                        current_sensor.get_terminal_type());
                                      }
                                  });

    apply_registration<Component>(
        components, comp_topo.current_sensor_terminal_type,
        [](GenericCurrentSensor const& current_sensor) { return current_sensor.get_terminal_type(); });
}

template <std::derived_from<Regulator> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component, Branch, Branch3>
constexpr void register_topology_components(ComponentContainer const& components, ComponentTopology& comp_topo) {
    apply_registration<Component>(
        components, comp_topo.regulated_object_idx, [&components](Regulator const& regulator) {
            switch (regulator.regulated_object_type()) {
            case ComponentType::branch:
                return get_component_sequence_idx<Branch>(components, regulator.regulated_object());
            case ComponentType::branch3:
                return get_component_sequence_idx<Branch3>(components, regulator.regulated_object());
            default:
                throw MissingCaseForEnumError("Regulator idx to seq transformation", regulator.regulated_object_type());
            }
        });

    apply_registration<Component>(components, comp_topo.regulated_object_type,
                                  [](Regulator const& regulator) { return regulator.regulated_object_type(); });
}

template <std::same_as<Branch> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component>
constexpr void register_connections_components(ComponentContainer const& components, ComponentConnections& comp_conn) {
    apply_registration<Component>(components, comp_conn.branch_connected, [](Branch const& branch) {
        return BranchConnected{status_to_int(branch.from_status()), status_to_int(branch.to_status())};
    });
    apply_registration<Component>(components, comp_conn.branch_phase_shift,
                                  [](Branch const& branch) { return branch.phase_shift(); });
}
template <std::same_as<Branch3> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component>
constexpr void register_connections_components(ComponentContainer const& components, ComponentConnections& comp_conn) {
    apply_registration<Component>(components, comp_conn.branch3_connected, [](Branch3 const& branch3) {
        return Branch3Connected{status_to_int(branch3.status_1()), status_to_int(branch3.status_2()),
                                status_to_int(branch3.status_3())};
    });
    apply_registration<Component>(components, comp_conn.branch3_phase_shift,
                                  [](Branch3 const& branch3) { return branch3.phase_shift(); });
}

template <std::same_as<Source> Component, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, Component>
constexpr void register_connections_components(ComponentContainer const& components, ComponentConnections& comp_conn) {
    apply_registration<Component>(components, comp_conn.source_connected,
                                  [](Source const& source) { return source.status(); });
}

} // namespace detail

template <typename ModelType>
    requires common::component_container_c<typename ModelType::ComponentContainer, Node, Branch, Branch3, Source, Shunt,
                                           GenericLoadGen, GenericVoltageSensor, GenericPowerSensor,
                                           GenericCurrentSensor, Regulator>
ComponentTopology construct_topology(typename ModelType::ComponentContainer const& components) {
    ComponentTopology comp_topo;
    using TopologyTypesTuple = typename ModelType::TopologyTypesTuple;
    main_core::utils::run_functor_with_tuple_return_void<TopologyTypesTuple>(
        [&components, &comp_topo]<typename CompType>() {
            detail::register_topology_components<CompType>(components, comp_topo);
        });
    return comp_topo;
}

template <typename ModelType>
    requires common::component_container_c<typename ModelType::ComponentContainer, Branch, Branch3, Source>
ComponentConnections construct_components_connections(typename ModelType::ComponentContainer const& components) {
    ComponentConnections comp_conn;
    using TopologyConnectionTypesTuple = typename ModelType::TopologyConnectionTypesTuple;
    main_core::utils::run_functor_with_tuple_return_void<TopologyConnectionTypesTuple>(
        [&components, &comp_conn]<typename CompType>() {
            detail::register_connections_components<CompType>(components, comp_conn);
        });
    return comp_conn;
}

} // namespace power_grid_model::main_core
