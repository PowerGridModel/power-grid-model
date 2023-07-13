// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_MATH_OUTPUT_CONVERTER_HPP
#define POWER_GRID_MODEL_AUXILIARY_MATH_OUTPUT_CONVERTER_HPP

#include "../all_components.hpp"
#include "../calculation_parameters.hpp"

#include <concepts>
#include <vector>

namespace power_grid_model::auxiliary {

namespace detail {

template <typename T, typename U>
concept component_container = requires(T const& c) {
    { c.template citer<U>().begin() } -> std::forward_iterator;
    { c.template citer<U>().end() } -> std::forward_iterator;
    { *(c.template citer<U>().begin()) } -> std::same_as<U const&>;
    { *(c.template citer<U>().end()) } -> std::same_as<U const&>;
};

}  // namespace detail

// math output converter template
template <class T>
struct MathOutputConverter;

template <typename CompContainer>
struct MathOutputConverter {
    using ComponentContainer = CompContainer;

    // output node
    template <bool sym, std::same_as<Node> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, Node>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& /* comp_topo */,
                               std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        return std::transform(components.template citer<Component>().begin(),
                              components.template citer<Component>().end(), comp_coup.node.cbegin(), res_it,
                              [&math_output](Node const& node, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return node.get_null_output<sym>();
                                  }
                                  return node.get_output<sym>(math_output[math_id.group].u[math_id.pos],
                                                              math_output[math_id.group].bus_injection[math_id.pos]);
                              });
    }

    // output branch
    template <bool sym, std::derived_from<Branch> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, Branch>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& /* comp_topo */,
                               std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        return std::transform(components.template citer<Component>().begin(),
                              components.template citer<Component>().end(),
                              comp_coup.branch.cbegin() + components.template get_start_idx<Branch, Component>(),
                              res_it, [&math_output](Branch const& branch, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return branch.get_null_output<sym>();
                                  }
                                  return branch.get_output<sym>(math_output[math_id.group].branch[math_id.pos]);
                              });
    }

    // output branch3
    template <bool sym, std::derived_from<Branch3> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, Branch3>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& /* comp_topo */,
                               std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        return std::transform(components.template citer<Component>().begin(),
                              components.template citer<Component>().end(),
                              comp_coup.branch3.cbegin() + components.template get_start_idx<Branch3, Component>(),
                              res_it, [&math_output](Branch3 const& branch3, Idx2DBranch3 math_id) {
                                  if (math_id.group == -1) {
                                      return branch3.get_null_output<sym>();
                                  }

                                  return branch3.get_output<sym>(math_output[math_id.group].branch[math_id.pos[0]],
                                                                 math_output[math_id.group].branch[math_id.pos[1]],
                                                                 math_output[math_id.group].branch[math_id.pos[2]]);
                              });
    }

    // output source, load_gen, shunt individually
    template <bool sym, std::same_as<Appliance> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, Appliance>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& comp_topo, std::vector<MathOutput<sym>> const& math_output,
                               ResIt res_it) {
        res_it = output_result<sym, Source>(components, comp_coup, comp_topo, math_output, res_it);
        res_it = output_result<sym, GenericLoadGen>(components, comp_coup, comp_topo, math_output, res_it);
        res_it = output_result<sym, Shunt>(components, comp_coup, comp_topo, math_output, res_it);
        return res_it;
    }

    // output source
    template <bool sym, std::same_as<Source> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, Source>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& /* comp_topo */,
                               std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        return std::transform(components.template citer<Component>().begin(),
                              components.template citer<Component>().end(), comp_coup.source.cbegin(), res_it,
                              [&math_output](Source const& source, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return source.get_null_output<sym>();
                                  }
                                  return source.get_output<sym>(math_output[math_id.group].source[math_id.pos]);
                              });
    }

    // output load gen
    template <bool sym, std::derived_from<GenericLoadGen> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, GenericLoadGen>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& /* comp_topo */,
                               std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        return std::transform(
            components.template citer<Component>().begin(), components.template citer<Component>().end(),
            comp_coup.load_gen.cbegin() + components.template get_start_idx<GenericLoadGen, Component>(), res_it,
            [&math_output](GenericLoadGen const& load_gen, Idx2D math_id) {
                if (math_id.group == -1) {
                    return load_gen.get_null_output<sym>();
                }
                return load_gen.get_output<sym>(math_output[math_id.group].load_gen[math_id.pos]);
            });
    }

    // output shunt
    template <bool sym, std::same_as<Shunt> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, Shunt>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& /* comp_topo */,
                               std::vector<MathOutput<sym>> const& math_output, ResIt res_it) {
        return std::transform(components.template citer<Component>().begin(),
                              components.template citer<Component>().end(), comp_coup.shunt.cbegin(), res_it,
                              [&math_output](Shunt const& shunt, Idx2D math_id) {
                                  if (math_id.group == -1) {
                                      return shunt.get_null_output<sym>();
                                  }
                                  return shunt.get_output<sym>(math_output[math_id.group].shunt[math_id.pos]);
                              });
    }

    // output voltage sensor
    template <bool sym, std::derived_from<GenericVoltageSensor> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, GenericVoltageSensor>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& comp_topo, std::vector<MathOutput<sym>> const& math_output,
                               ResIt res_it) {
        return std::transform(
            components.template citer<Component>().begin(), components.template citer<Component>().end(),
            comp_topo.voltage_sensor_node_idx.cbegin() +
                components.template get_start_idx<GenericVoltageSensor, Component>(),
            res_it, [&comp_coup, &math_output](GenericVoltageSensor const& voltage_sensor, Idx const node_seq) {
                Idx2D const node_math_id = comp_coup.node[node_seq];
                if (node_math_id.group == -1) {
                    return voltage_sensor.get_null_output<sym>();
                }
                return voltage_sensor.get_output<sym>(math_output[node_math_id.group].u[node_math_id.pos]);
            });
    }

    // output power sensor
    template <bool sym, std::derived_from<GenericPowerSensor> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, GenericPowerSensor>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& comp_topo, std::vector<MathOutput<sym>> const& math_output,
                               ResIt res_it) {
        return std::transform(
            components.template citer<Component>().begin(), components.template citer<Component>().end(),
            comp_topo.power_sensor_object_idx.cbegin() +
                components.template get_start_idx<GenericPowerSensor, Component>(),
            res_it, [&comp_coup, &math_output](GenericPowerSensor const& power_sensor, Idx const obj_seq) {
                auto const terminal_type = power_sensor.get_terminal_type();
                Idx2D const obj_math_id = [&]() {
                    switch (terminal_type) {
                        using enum MeasuredTerminalType;

                        case branch_from:
                        case branch_to:
                            return comp_coup.branch[obj_seq];
                        case source:
                            return comp_coup.source[obj_seq];
                        case shunt:
                            return comp_coup.shunt[obj_seq];
                        case load:
                        case generator:
                            return comp_coup.load_gen[obj_seq];
                        // from branch3, get relevant math object branch based on the measured side
                        case branch3_1:
                            return Idx2D{comp_coup.branch3[obj_seq].group, comp_coup.branch3[obj_seq].pos[0]};
                        case branch3_2:
                            return Idx2D{comp_coup.branch3[obj_seq].group, comp_coup.branch3[obj_seq].pos[1]};
                        case branch3_3:
                            return Idx2D{comp_coup.branch3[obj_seq].group, comp_coup.branch3[obj_seq].pos[2]};
                        case node:
                            return comp_coup.node[obj_seq];
                        default:
                            throw MissingCaseForEnumError(std::string(GenericPowerSensor::name) + " output_result()",
                                                          terminal_type);
                    }
                }();

                if (obj_math_id.group == -1) {
                    return power_sensor.get_null_output<sym>();
                }

                switch (terminal_type) {
                    using enum MeasuredTerminalType;

                    case branch_from:
                    // all power sensors in branch3 are at from side in the mathematical model
                    case branch3_1:
                    case branch3_2:
                    case branch3_3:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].branch[obj_math_id.pos].s_f);
                    case branch_to:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].branch[obj_math_id.pos].s_t);
                    case source:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].source[obj_math_id.pos].s);
                    case shunt:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].shunt[obj_math_id.pos].s);
                    case load:
                    case generator:
                        return power_sensor.get_output<sym>(math_output[obj_math_id.group].load_gen[obj_math_id.pos].s);
                    case node:
                        return power_sensor.get_output<sym>(
                            math_output[obj_math_id.group].bus_injection[obj_math_id.pos]);
                    default:
                        throw MissingCaseForEnumError(std::string(GenericPowerSensor::name) + " output_result()",
                                                      terminal_type);
                }
            });
    }

    // output power sensor
    template <bool sym, std::same_as<Fault> Component, std::forward_iterator ResIt>
    requires detail::component_container<ComponentContainer, Fault>
    static ResIt output_result(ComponentContainer const& components, ComponentToMathCoupling const& comp_coup,
                               ComponentTopology const& /* comp_topo */,
                               std::vector<MathOutput<sym>> const& /* math_output */, ResIt res_it) {
        return std::transform(components.template citer<Component>().begin(),
                              components.template citer<Component>().end(), comp_coup.fault.cbegin(), res_it,
                              [](Fault const& fault, Idx2D /* math_id */) {
                                  return fault.get_output();
                              });
    }
};

}  // namespace power_grid_model::auxiliary

#endif
