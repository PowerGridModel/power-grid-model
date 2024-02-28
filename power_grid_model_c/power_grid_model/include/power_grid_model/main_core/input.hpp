// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {

// template to construct components
// using forward interators
// different selection based on component type
template <std::derived_from<Base> Component, class ComponentContainer, std::forward_iterator ForwardIterator>
    requires model_component_state<MainModelState, ComponentContainer, Component>
inline void add_component(MainModelState<ComponentContainer>& state, ForwardIterator begin, ForwardIterator end,
                          double system_frequency) {
    size_t const size = std::distance(begin, end);
    state.components.template reserve<Component>(size);
    // loop to add component
    for (auto it = begin; it != end; ++it) {
        auto const& input = *it;
        ID const id = input.id;
        // construct based on type of component
        if constexpr (std::derived_from<Component, Node>) {
            state.components.template emplace<Component>(id, input);
        } else if constexpr (std::derived_from<Component, Branch>) {
            double const u1 = state.components.template get_item<Node>(input.from_node).u_rated();
            double const u2 = state.components.template get_item<Node>(input.to_node).u_rated();
            // set system frequency for line
            if constexpr (std::same_as<Component, Line>) {
                state.components.template emplace<Component>(id, input, system_frequency, u1, u2);
            } else {
                state.components.template emplace<Component>(id, input, u1, u2);
            }
        } else if constexpr (std::derived_from<Component, Branch3>) {
            double const u1 = state.components.template get_item<Node>(input.node_1).u_rated();
            double const u2 = state.components.template get_item<Node>(input.node_2).u_rated();
            double const u3 = state.components.template get_item<Node>(input.node_3).u_rated();
            state.components.template emplace<Component>(id, input, u1, u2, u3);
        } else if constexpr (std::derived_from<Component, Appliance>) {
            double const u = state.components.template get_item<Node>(input.node).u_rated();
            state.components.template emplace<Component>(id, input, u);
        } else if constexpr (std::derived_from<Component, GenericVoltageSensor>) {
            double const u = state.components.template get_item<Node>(input.measured_object).u_rated();
            state.components.template emplace<Component>(id, input, u);
        } else if constexpr (std::derived_from<Component, GenericPowerSensor>) {
            // it is not allowed to place a sensor at a link
            if (state.components.get_idx_by_id(input.measured_object).group ==
                state.components.template get_type_idx<Link>()) {
                throw InvalidMeasuredObject("Link", "PowerSensor");
            }
            ID const measured_object = input.measured_object;
            // check correctness of measured component type based on measured terminal type
            switch (input.measured_terminal_type) {
                using enum MeasuredTerminalType;

            case branch_from:
            case branch_to:
                state.components.template get_item<Branch>(measured_object);
                break;
            case branch3_1:
            case branch3_2:
            case branch3_3:
                state.components.template get_item<Branch3>(measured_object);
                break;
            case shunt:
                state.components.template get_item<Shunt>(measured_object);
                break;
            case source:
                state.components.template get_item<Source>(measured_object);
                break;
            case load:
                state.components.template get_item<GenericLoad>(measured_object);
                break;
            case generator:
                state.components.template get_item<GenericGenerator>(measured_object);
                break;
            case node:
                state.components.template get_item<Node>(measured_object);
                break;
            default:
                throw MissingCaseForEnumError(std::string(GenericPowerSensor::name) + " item retrieval",
                                              input.measured_terminal_type);
            }

            state.components.template emplace<Component>(id, input);
        } else if constexpr (std::derived_from<Component, Fault>) {
            // check that fault object exists (currently, only faults at nodes are supported)
            state.components.template get_item<Node>(input.fault_object);
            state.components.template emplace<Component>(id, input);
        }
    }
}

} // namespace power_grid_model::main_core
