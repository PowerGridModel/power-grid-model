// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../main_core/container_queries.hpp"

#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../component/appliance.hpp"
#include "../component/asym_line.hpp"
#include "../component/base.hpp"
#include "../component/branch.hpp"
#include "../component/branch3.hpp"
#include "../component/current_sensor.hpp"
#include "../component/fault.hpp"
#include "../component/line.hpp"
#include "../component/link.hpp"
#include "../component/load_gen.hpp"
#include "../component/node.hpp"
#include "../component/power_sensor.hpp"
#include "../component/shunt.hpp"
#include "../component/source.hpp"
#include "../component/three_winding_transformer.hpp"
#include "../component/transformer.hpp"
#include "../component/transformer_tap_regulator.hpp"
#include "../component/voltage_regulator.hpp"
#include "../component/voltage_sensor.hpp"
#include "../container_fwd.hpp"

#include <array>
#include <concepts>
#include <format>
#include <ranges>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace power_grid_model::main_core {

constexpr std::array<Branch3Side, 3> const branch3_sides = {Branch3Side::side_1, Branch3Side::side_2,
                                                            Branch3Side::side_3};

// template to construct components
// using forward interators
// different selection based on component type
template <std::derived_from<Base> Component, class ComponentContainer, std::ranges::viewable_range Inputs>
    requires common::component_container_c<ComponentContainer, Component>
inline void add_component(ComponentContainer& components, Inputs&& component_inputs, double system_frequency) {
    using ComponentView =
        std::conditional_t<std::same_as<std::ranges::range_reference_t<Inputs>, typename Component::InputType const&>,
                           typename Component::InputType const&, typename Component::InputType>;

    reserve_component<Component>(components, std::ranges::size(component_inputs));
    // do sanity check on the transformer tap regulator
    std::vector<Idx2D> regulated_objects;
    // loop to add component

    for (auto const& input_proxy : std::views::all(std::forward<Inputs>(component_inputs))) {
        ComponentView const input = [&input_proxy]() -> ComponentView { return input_proxy; }();

        ID const id = input.id;
        // construct based on type of component
        if constexpr (std::derived_from<Component, Node>) {
            emplace_component<Component>(components, id, input);
        } else if constexpr (std::derived_from<Component, Branch>) {
            double const u1 = get_component<Node>(components, input.from_node).u_rated();
            double const u2 = get_component<Node>(components, input.to_node).u_rated();
            // set system frequency for line
            if constexpr (std::same_as<Component, Line> || std::same_as<Component, AsymLine>) {
                emplace_component<Component>(components, id, input, system_frequency, u1, u2);
            } else {
                emplace_component<Component>(components, id, input, u1, u2);
            }
        } else if constexpr (std::derived_from<Component, Branch3>) {
            double const u1 = get_component<Node>(components, input.node_1).u_rated();
            double const u2 = get_component<Node>(components, input.node_2).u_rated();
            double const u3 = get_component<Node>(components, input.node_3).u_rated();
            emplace_component<Component>(components, id, input, u1, u2, u3);
        } else if constexpr (std::derived_from<Component, Appliance>) {
            double const u = get_component<Node>(components, input.node).u_rated();
            emplace_component<Component>(components, id, input, u);
        } else if constexpr (std::derived_from<Component, GenericVoltageSensor>) {
            double const u = get_component<Node>(components, input.measured_object).u_rated();
            emplace_component<Component>(components, id, input, u);
        } else if constexpr (std::derived_from<Component, GenericPowerSensor>) {
            // it is not allowed to place a sensor at a link
            if (get_component_idx_by_id(components, input.measured_object).group ==
                get_component_type_index<Link>(components)) {
                throw InvalidMeasuredObject("Link", "PowerSensor");
            }
            ID const measured_object = input.measured_object;
            // check correctness of measured component type based on measured terminal type
            switch (input.measured_terminal_type) {
                using enum MeasuredTerminalType;

            case branch_from:
                [[fallthrough]];
            case branch_to:
                get_component<Branch>(components, measured_object);
                break;
            case branch3_1:
                [[fallthrough]];
            case branch3_2:
                [[fallthrough]];
            case branch3_3:
                get_component<Branch3>(components, measured_object);
                break;
            case shunt:
                get_component<Shunt>(components, measured_object);
                break;
            case source:
                get_component<Source>(components, measured_object);
                break;
            case load:
                get_component<GenericLoad>(components, measured_object);
                break;
            case generator:
                get_component<GenericGenerator>(components, measured_object);
                break;
            case node:
                get_component<Node>(components, measured_object);
                break;
            default:
                throw MissingCaseForEnumError{std::format("{} item retrieval", GenericPowerSensor::name),
                                              input.measured_terminal_type};
            }

            emplace_component<Component>(components, id, input);
        } else if constexpr (std::derived_from<Component, GenericCurrentSensor>) {
            // it is not allowed to place a sensor at a link
            if (get_component_idx_by_id(components, input.measured_object).group ==
                get_component_type_index<Link>(components)) {
                throw InvalidMeasuredObject("Link", "CurrentSensor");
            }
            // check correctness and get node based on measured terminal type
            ID const node = [&components, measured_object = input.measured_object,
                             measured_terminal_type = input.measured_terminal_type] {
                switch (measured_terminal_type) {
                    using enum MeasuredTerminalType;
                    using enum Branch3Side;

                case branch_from:
                    return get_component<Branch>(components, measured_object).node(BranchSide::from);
                case branch_to:
                    return get_component<Branch>(components, measured_object).node(BranchSide::to);
                case branch3_1:
                    return get_component<Branch3>(components, measured_object).node(side_1);
                case branch3_2:
                    return get_component<Branch3>(components, measured_object).node(side_2);
                case branch3_3:
                    return get_component<Branch3>(components, measured_object).node(side_3);
                default:
                    throw MissingCaseForEnumError{std::format("{} item retrieval", GenericCurrentSensor::name),
                                                  measured_terminal_type};
                }
            }();

            double const u_rated = get_component<Node>(components, node).u_rated();

            emplace_component<Component>(components, id, input, u_rated);
        } else if constexpr (std::derived_from<Component, Fault>) {
            // check that fault object exists (currently, only faults at nodes are supported)
            get_component<Node>(components, input.fault_object);
            emplace_component<Component>(components, id, input);
        } else if constexpr (std::derived_from<Component, TransformerTapRegulator>) {
            Idx2D const regulated_object_idx =
                main_core::get_component_idx_by_id<ComponentContainer>(components, input.regulated_object);
            regulated_objects.push_back(regulated_object_idx);

            ID const regulated_terminal = [&input, &components, &regulated_object_idx] {
                using enum ControlSide;

                if (regulated_object_idx.group == get_component_type_index<Transformer>(components)) {
                    auto const& regulated_object = get_component<Transformer>(components, regulated_object_idx);
                    switch (input.control_side) {
                    case to:
                        [[fallthrough]];
                    case from:
                        return regulated_object.node(static_cast<BranchSide>(input.control_side));
                    default:
                        throw MissingCaseForEnumError{std::format("{} item retrieval", Component::name),
                                                      input.control_side};
                    }
                } else if (regulated_object_idx.group ==
                           get_component_type_index<ThreeWindingTransformer>(components)) {
                    auto const& regulated_object =
                        get_component<ThreeWindingTransformer>(components, regulated_object_idx);
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

            if (regulated_object_idx.group != get_component_type_index<Transformer>(components) &&
                regulated_object_idx.group != get_component_type_index<ThreeWindingTransformer>(components)) {
                throw InvalidRegulatedObject(input.regulated_object, Component::name);
            }

            auto const regulated_object_type = get_component<Base>(components, regulated_object_idx).math_model_type();
            double const u_rated = get_component<Node>(components, regulated_terminal).u_rated();

            emplace_component<Component>(components, id, input, regulated_object_type, u_rated);
        } else if constexpr (std::derived_from<Component, VoltageRegulator>) {
            Idx2D const regulated_object_idx =
                main_core::get_component_idx_by_id<ComponentContainer>(components, input.regulated_object);
            regulated_objects.push_back(regulated_object_idx);

            // regulate generators
            // also allow loads, in order to have more flexibility when converting existing models
            if (regulated_object_idx.group != get_component_type_index<SymGenerator>(components) &&
                regulated_object_idx.group != get_component_type_index<AsymGenerator>(components) &&
                regulated_object_idx.group != get_component_type_index<SymLoad>(components) &&
                regulated_object_idx.group != get_component_type_index<AsymLoad>(components)) {
                throw InvalidRegulatedObject(input.regulated_object, Component::name);
            }

            auto const& regulated_object = get_component<Appliance>(components, regulated_object_idx);
            auto const regulated_object_type = regulated_object.math_model_type();
            emplace_component<Component>(components, id, input, regulated_object_type);
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
