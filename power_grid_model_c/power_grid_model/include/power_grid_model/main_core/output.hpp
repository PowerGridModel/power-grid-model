// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_OUTPUT_HPP
#define POWER_GRID_MODEL_MAIN_CORE_OUTPUT_HPP

#include "state.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {

namespace detail {

template <std::derived_from<Base> BaseComponent, std::derived_from<Base> Component, class ComponentContainer>
requires std::derived_from<Component, BaseComponent> &&
    model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_sequence_offset(MainModelState<ComponentContainer> const& state) {
    return state.components.template get_start_idx<BaseComponent, Component>();
}

template <std::same_as<Node> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->node.cbegin();
}

template <std::derived_from<Branch> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->branch.cbegin() + comp_sequence_offset<Branch, Component>(state);
}

template <std::derived_from<Branch3> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->branch3.cbegin() + comp_sequence_offset<Branch3, Component>(state);
}

template <std::same_as<Source> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->source.cbegin();
}

template <std::derived_from<GenericLoadGen> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->load_gen.cbegin() + comp_sequence_offset<GenericLoadGen, Component>(state);
}

template <std::same_as<Shunt> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.topo_comp_coup->shunt.cbegin();
}

template <std::derived_from<GenericVoltageSensor> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_topo->voltage_sensor_node_idx.cbegin() +
           comp_sequence_offset<GenericVoltageSensor, Component>(state);
}

template <std::derived_from<GenericPowerSensor> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_topo->power_sensor_object_idx.cbegin() +
           comp_sequence_offset<GenericPowerSensor, Component>(state);
}

template <std::same_as<Fault> Component, class ComponentContainer>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr auto comp_base_sequence_cbegin(MainModelState<ComponentContainer> const& state) {
    return state.comp_coup.fault.cbegin();
}

template <typename Component, typename IndexType, class ComponentContainer, std::forward_iterator ResIt,
          typename ResFunc>
requires model_component_state<MainModelState, ComponentContainer, Component> &&
    std::invocable<std::remove_cvref_t<ResFunc>, Component const&, IndexType>&&
        std::convertible_to<std::invoke_result_t<ResFunc, Component const&, IndexType>, std::iter_value_t<ResIt>>&&
            std::convertible_to<IndexType, decltype(*comp_base_sequence_cbegin<Component>(
                                               MainModelState<ComponentContainer>{}))> constexpr ResIt
            produce_output(MainModelState<ComponentContainer> const& state, ResIt res_it, ResFunc&& func) {
    return std::transform(state.components.template citer<Component>().begin(),
                          state.components.template citer<Component>().end(),
                          comp_base_sequence_cbegin<Component>(state), res_it, func);
}

}  // namespace detail

// output node
template <std::same_as<Node> Component, class ComponentContainer, steady_state_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Node, Idx2D>(state, res_it, [&math_output](Node const& node, Idx2D math_id) {
        constexpr auto sym = symmetric_math_output_type<MathOutputType>;

        if (math_id.group == -1) {
            return node.get_null_output<sym>();
        }
        return node.get_output<sym>(math_output[math_id.group].u[math_id.pos],
                                    math_output[math_id.group].bus_injection[math_id.pos]);
    });
}
template <std::same_as<Node> Component, class ComponentContainer, short_circuit_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Node, Idx2D>(state, res_it, [&math_output](Node const& node, Idx2D math_id) {
        if (math_id.group == -1) {
            return node.get_null_sc_output();
        }
        return node.get_sc_output(math_output[math_id.group].u_bus[math_id.pos]);
    });
}

// output branch
template <std::derived_from<Branch> Component, class ComponentContainer, steady_state_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it, [&math_output](Branch const& branch, Idx2D math_id) {
        constexpr auto sym = symmetric_math_output_type<MathOutputType>;

        if (math_id.group == -1) {
            return branch.get_null_output<sym>();
        }
        return branch.get_output<sym>(math_output[math_id.group].branch[math_id.pos]);
    });
}
template <std::derived_from<Branch> Component, class ComponentContainer, short_circuit_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it, [&math_output](Branch const& branch, Idx2D math_id) {
        if (math_id.group == -1) {
            return branch.get_null_sc_output();
        }
        return branch.get_sc_output(math_output[math_id.group].branch[math_id.pos]);
    });
}

// output branch3
template <std::derived_from<Branch3> Component, class ComponentContainer, steady_state_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2DBranch3>(
        state, res_it, [&math_output](Branch3 const& branch3, Idx2DBranch3 math_id) {
            constexpr auto sym = symmetric_math_output_type<MathOutputType>;

            if (math_id.group == -1) {
                return branch3.get_null_output<sym>();
            }

            auto const& branches = math_output[math_id.group].branch;
            return branch3.get_output<sym>(branches[math_id.pos[0]], branches[math_id.pos[1]],
                                           branches[math_id.pos[2]]);
        });
}
template <std::derived_from<Branch3> Component, class ComponentContainer, short_circuit_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2DBranch3>(
        state, res_it, [&math_output](Branch3 const& branch3, Idx2DBranch3 math_id) {
            if (math_id.group == -1) {
                return branch3.get_null_sc_output();
            }

            auto const& branches = math_output[math_id.group].branch;
            return branch3.get_sc_output(branches[math_id.pos[0]], branches[math_id.pos[1]], branches[math_id.pos[2]]);
        });
}

// output source, load_gen, shunt individually
template <std::same_as<Appliance> Component, class ComponentContainer, math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    res_it = output_result<Source>(state, math_output, res_it);
    res_it = output_result<GenericLoadGen>(state, math_output, res_it);
    res_it = output_result<Shunt>(state, math_output, res_it);
    return res_it;
}

// output source
template <std::same_as<Source> Component, class ComponentContainer, steady_state_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it, [&math_output](Source const& source, Idx2D math_id) {
        constexpr auto sym = symmetric_math_output_type<MathOutputType>;

        if (math_id.group == -1) {
            return source.get_null_output<sym>();
        }
        return source.get_output<sym>(math_output[math_id.group].source[math_id.pos]);
    });
}
template <std::same_as<Source> Component, class ComponentContainer, short_circuit_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it, [&math_output](Source const& source, Idx2D math_id) {
        if (math_id.group == -1) {
            return source.get_null_sc_output();
        }
        return source.get_sc_output(math_output[math_id.group].source[math_id.pos]);
    });
}

// output load gen
template <std::derived_from<GenericLoadGen> Component, class ComponentContainer,
          steady_state_math_output_type MathOutputType, std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(
        state, res_it, [&math_output](GenericLoadGen const& load_gen, Idx2D math_id) {
            constexpr auto sym = symmetric_math_output_type<MathOutputType>;

            if (math_id.group == -1) {
                return load_gen.get_null_output<sym>();
            }
            return load_gen.get_output<sym>(math_output[math_id.group].load_gen[math_id.pos]);
        });
}
template <std::derived_from<GenericLoadGen> Component, class ComponentContainer,
          short_circuit_math_output_type MathOutputType, std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& /* math_output */, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it,
                                                    [](GenericLoadGen const& load_gen, Idx2D /* math_id */) {
                                                        return load_gen.get_null_sc_output();
                                                    });
}

// output shunt
template <std::same_as<Shunt> Component, class ComponentContainer, steady_state_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it, [&math_output](Shunt const& shunt, Idx2D math_id) {
        constexpr auto sym = symmetric_math_output_type<MathOutputType>;

        if (math_id.group == -1) {
            return shunt.get_null_output<sym>();
        }
        return shunt.get_output<sym>(math_output[math_id.group].shunt[math_id.pos]);
    });
}
template <std::same_as<Shunt> Component, class ComponentContainer, short_circuit_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it, [&math_output](Shunt const& shunt, Idx2D math_id) {
        if (math_id.group == -1) {
            return shunt.get_null_sc_output();
        }
        return shunt.get_sc_output(math_output[math_id.group].shunt[math_id.pos]);
    });
}

// output voltage sensor
template <std::derived_from<GenericVoltageSensor> Component, class ComponentContainer,
          steady_state_math_output_type MathOutputType, std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx>(
        state, res_it, [&state, &math_output](GenericVoltageSensor const& voltage_sensor, Idx const node_seq) {
            constexpr auto sym = symmetric_math_output_type<MathOutputType>;

            Idx2D const node_math_id = state.topo_comp_coup->node[node_seq];
            if (node_math_id.group == -1) {
                return voltage_sensor.get_null_output<sym>();
            }
            return voltage_sensor.get_output<sym>(math_output[node_math_id.group].u[node_math_id.pos]);
        });
}
template <std::derived_from<GenericVoltageSensor> Component, class ComponentContainer,
          short_circuit_math_output_type MathOutputType, std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& /* math_output */, ResIt res_it) {
    return detail::produce_output<Component, Idx>(
        state, res_it, [](GenericVoltageSensor const& voltage_sensor, Idx const /* node_seq */) {
            return voltage_sensor.get_null_sc_output();
        });
}

// output power sensor
template <std::derived_from<GenericPowerSensor> Component, class ComponentContainer,
          steady_state_math_output_type MathOutputType, std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx>(
        state, res_it, [&state, &math_output](GenericPowerSensor const& power_sensor, Idx const obj_seq) {
            constexpr auto sym = symmetric_math_output_type<MathOutputType>;

            auto const terminal_type = power_sensor.get_terminal_type();
            Idx2D const obj_math_id = [&]() {
                switch (terminal_type) {
                    using enum MeasuredTerminalType;

                    case branch_from:
                    case branch_to:
                        return state.topo_comp_coup->branch[obj_seq];
                    case source:
                        return state.topo_comp_coup->source[obj_seq];
                    case shunt:
                        return state.topo_comp_coup->shunt[obj_seq];
                    case load:
                    case generator:
                        return state.topo_comp_coup->load_gen[obj_seq];
                    // from branch3, get relevant math object branch based on the measured side
                    case branch3_1:
                        return Idx2D{state.topo_comp_coup->branch3[obj_seq].group,
                                     state.topo_comp_coup->branch3[obj_seq].pos[0]};
                    case branch3_2:
                        return Idx2D{state.topo_comp_coup->branch3[obj_seq].group,
                                     state.topo_comp_coup->branch3[obj_seq].pos[1]};
                    case branch3_3:
                        return Idx2D{state.topo_comp_coup->branch3[obj_seq].group,
                                     state.topo_comp_coup->branch3[obj_seq].pos[2]};
                    case node:
                        return state.topo_comp_coup->node[obj_seq];
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
                    return power_sensor.get_output<sym>(math_output[obj_math_id.group].bus_injection[obj_math_id.pos]);
                default:
                    throw MissingCaseForEnumError(std::string(GenericPowerSensor::name) + " output_result()",
                                                  terminal_type);
            }
        });
}
template <std::derived_from<GenericPowerSensor> Component, class ComponentContainer,
          short_circuit_math_output_type MathOutputType, std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& /* math_output */, ResIt res_it) {
    return detail::produce_output<Component, Idx>(state, res_it,
                                                  [](GenericPowerSensor const& power_sensor, Idx const /* node_seq */) {
                                                      return power_sensor.get_null_sc_output();
                                                  });
}

// output fault
template <std::same_as<Fault> Component, class ComponentContainer, steady_state_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& /* math_output */, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(state, res_it, [](Fault const& fault, Idx2D /* math_id */) {
        return fault.get_output();
    });
}
template <std::same_as<Fault> Component, class ComponentContainer, short_circuit_math_output_type MathOutputType,
          std::forward_iterator ResIt>
requires model_component_state<MainModelState, ComponentContainer, Component> &&
    model_component_state<MainModelState, ComponentContainer, Node>
constexpr ResIt output_result(MainModelState<ComponentContainer> const& state,
                              std::vector<MathOutputType> const& math_output, ResIt res_it) {
    return detail::produce_output<Component, Idx2D>(
        state, res_it, [&state, &math_output](Fault const& fault, Idx2D math_id) {
            if (math_id.group == -1) {
                return fault.get_null_sc_output();
            }

            auto const u_rated = state.components.template get_item<Node>(fault.get_fault_object()).u_rated();
            return fault.get_sc_output(math_output[math_id.group].fault[math_id.pos], u_rated);
        });
}

}  // namespace power_grid_model::main_core

#endif
