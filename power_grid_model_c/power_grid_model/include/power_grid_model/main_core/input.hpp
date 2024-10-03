// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"
#include "state_queries.hpp"

#include "../all_components.hpp"
#include "../common/iterator_facade.hpp"

#include <unordered_set>

namespace power_grid_model::main_core {

constexpr std::array<Branch3Side, 3> const branch3_sides = {Branch3Side::side_1, Branch3Side::side_2,
                                                            Branch3Side::side_3};

// template to construct components
// using forward interators
// different selection based on component type
template <std::derived_from<Base> Component, class ComponentContainer, std::forward_iterator ForwardIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void add_component(MainModelState<ComponentContainer>& state, ForwardIterator begin, ForwardIterator end,
                          double system_frequency) {
    using ComponentView = std::conditional_t<std::same_as<decltype(*begin), typename Component::InputType const&>,
                                             typename Component::InputType const&, typename Component::InputType>;

    reserve_component<Component>(state, std::distance(begin, end));
    // do sanity check on the transformer tap regulator
    std::vector<Idx2D> regulated_objects;
    // loop to add component
    for (auto it = begin; it != end; ++it) {
        ComponentView const input = *it;
        ID const id = input.id;
        // construct based on type of component
        if constexpr (std::derived_from<Component, Node>) {
            emplace_component<Component>(state, id, input);
        } else if constexpr (std::derived_from<Component, Branch>) {
            double const u1 = get_component<Node>(state, input.from_node).u_rated();
            double const u2 = get_component<Node>(state, input.to_node).u_rated();
            // set system frequency for line
            if constexpr (std::same_as<Component, Line> || std::same_as<Component, AsymLine>) {
                emplace_component<Component>(state, id, input, system_frequency, u1, u2);
            } else {
                emplace_component<Component>(state, id, input, u1, u2);
            }
        } else if constexpr (std::derived_from<Component, Branch3>) {
            double const u1 = get_component<Node>(state, input.node_1).u_rated();
            double const u2 = get_component<Node>(state, input.node_2).u_rated();
            double const u3 = get_component<Node>(state, input.node_3).u_rated();
            emplace_component<Component>(state, id, input, u1, u2, u3);
        } else if constexpr (std::derived_from<Component, Appliance>) {
            double const u = get_component<Node>(state, input.node).u_rated();
            emplace_component<Component>(state, id, input, u);
        } else if constexpr (std::derived_from<Component, GenericVoltageSensor>) {
            double const u = get_component<Node>(state, input.measured_object).u_rated();
            emplace_component<Component>(state, id, input, u);
        } else if constexpr (std::derived_from<Component, GenericPowerSensor>) {
            // it is not allowed to place a sensor at a link
            if (get_component_idx_by_id(state, input.measured_object).group == get_component_type_index<Link>(state)) {
                throw InvalidMeasuredObject("Link", "PowerSensor");
            }
            ID const measured_object = input.measured_object;
            // check correctness of measured component type based on measured terminal type
            switch (input.measured_terminal_type) {
                using enum MeasuredTerminalType;

            case branch_from:
                [[fallthrough]];
            case branch_to:
                get_component<Branch>(state, measured_object);
                break;
            case branch3_1:
                [[fallthrough]];
            case branch3_2:
                [[fallthrough]];
            case branch3_3:
                get_component<Branch3>(state, measured_object);
                break;
            case shunt:
                get_component<Shunt>(state, measured_object);
                break;
            case source:
                get_component<Source>(state, measured_object);
                break;
            case load:
                get_component<GenericLoad>(state, measured_object);
                break;
            case generator:
                get_component<GenericGenerator>(state, measured_object);
                break;
            case node:
                get_component<Node>(state, measured_object);
                break;
            default:
                throw MissingCaseForEnumError{std::format("{} item retrieval", GenericPowerSensor::name),
                                              input.measured_terminal_type};
            }

            emplace_component<Component>(state, id, input);
        } else if constexpr (std::derived_from<Component, GenericCurrentSensor>) {
            // it is not allowed to place a sensor at a link
            if (get_component_idx_by_id(state, input.measured_object).group == get_component_type_index<Link>(state)) {
                throw InvalidMeasuredObject("Link", "CurrentSensor");
            }
            // check correctness and get node based on measured terminal type
            ID const node = [&state, measured_object = input.measured_object,
                             measured_terminal_type = input.measured_terminal_type] {
                switch (measured_terminal_type) {
                    using enum MeasuredTerminalType;
                    using enum Branch3Side;

                case branch_from:
                    return get_component<Branch>(state, measured_object).node(BranchSide::from);
                case branch_to:
                    return get_component<Branch>(state, measured_object).node(BranchSide::to);
                case branch3_1:
                    return get_component<Branch3>(state, measured_object).node(side_1);
                case branch3_2:
                    return get_component<Branch3>(state, measured_object).node(side_2);
                case branch3_3:
                    return get_component<Branch3>(state, measured_object).node(side_2);
                default:
                    throw MissingCaseForEnumError{std::format("{} item retrieval", GenericCurrentSensor::name),
                                                  measured_terminal_type};
                }
            }();

            double const u_rated = get_component<Node>(state, node).u_rated();

            emplace_component<Component>(state, id, input, u_rated);
        } else if constexpr (std::derived_from<Component, Fault>) {
            // check that fault object exists (currently, only faults at nodes are supported)
            get_component<Node>(state, input.fault_object);
            emplace_component<Component>(state, id, input);
        } else if constexpr (std::derived_from<Component, TransformerTapRegulator>) {
            Idx2D const regulated_object_idx = get_component_idx_by_id(state, input.regulated_object);
            regulated_objects.push_back(regulated_object_idx);

            ID const regulated_terminal = [&input, &state, &regulated_object_idx] {
                using enum ControlSide;

                if (regulated_object_idx.group == get_component_type_index<Transformer>(state)) {
                    auto const& regulated_object = get_component<Transformer>(state, regulated_object_idx);
                    switch (input.control_side) {
                    case to:
                        [[fallthrough]];
                    case from:
                        return regulated_object.node(static_cast<BranchSide>(input.control_side));
                    default:
                        throw MissingCaseForEnumError{std::format("{} item retrieval", Component::name),
                                                      input.control_side};
                    }
                } else if (regulated_object_idx.group == get_component_type_index<ThreeWindingTransformer>(state)) {
                    auto const& regulated_object = get_component<ThreeWindingTransformer>(state, regulated_object_idx);
                    switch (input.control_side) {
                    case side_1:
                        [[fallthrough]];
                    case side_2:
                        [[fallthrough]];
                    case side_3:
                        return regulated_object.node(static_cast<Branch3Side>(input.control_side));
                    default:
                        throw MissingCaseForEnumError{std::format("{} item retrieval", Component::name),
                                                      input.control_side};
                    }
                } else {
                    throw InvalidRegulatedObject(input.regulated_object, Component::name);
                }
            }();

            if (regulated_object_idx.group != get_component_type_index<Transformer>(state) &&
                regulated_object_idx.group != get_component_type_index<ThreeWindingTransformer>(state)) {
                throw InvalidRegulatedObject(input.regulated_object, Component::name);
            }

            auto const regulated_object_type = get_component<Base>(state, regulated_object_idx).math_model_type();
            double const u_rated = get_component<Node>(state, regulated_terminal).u_rated();

            emplace_component<Component>(state, id, input, regulated_object_type, u_rated);
        }
    }
    // Make sure that each regulated object has at most one regulator
    const std::unordered_set<Idx2D, Idx2DHash> unique_regulated_objects(regulated_objects.begin(),
                                                                        regulated_objects.end());
    if (unique_regulated_objects.size() != regulated_objects.size()) {
        // There are duplicates
        throw DuplicativelyRegulatedObject{};
    }
}

} // namespace power_grid_model::main_core
