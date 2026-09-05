// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "container_queries.hpp"
#include "math_output_queries.hpp"
#include "state.hpp"
#include "state_queries.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../component/base.hpp"
#include "../component/branch.hpp"
#include "../component/branch3.hpp"
#include "../component/current_sensor.hpp"
#include "../component/edge.hpp"
#include "../component/fault.hpp"
#include "../component/load_gen.hpp"
#include "../component/node.hpp"
#include "../component/power_sensor.hpp"
#include "../component/regulator.hpp"
#include "../component/shunt.hpp"
#include "../component/source.hpp"
#include "../component/transformer_tap_regulator.hpp"
#include "../component/voltage_regulator.hpp"
#include "../component/voltage_sensor.hpp"

#include <algorithm>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <vector>

namespace power_grid_model::main_core {

namespace detail {
template <typename T, typename U>
concept assignable_to = std::assignable_from<U, T>;

template <typename MeasuredComponent, class ComponentContainer, typename... StatusArgs>
constexpr bool measured_component_active(MainModelState<ComponentContainer> const& state, Idx const obj_seq,
                                         StatusArgs... status_args) {
    if constexpr (common::component_container_c<ComponentContainer, MeasuredComponent>) {
        return get_component_by_sequence<MeasuredComponent>(state.components, obj_seq).status(status_args...);
    } else {
        // A missing component type makes its terminal type unreachable in a valid model. Keep reduced containers
        // compatible with the previous topology-only behavior.
        return true;
    }
}

template <class ComponentContainer>
constexpr bool measured_terminal_active(MeasuredTerminalType const terminal_type,
                                        MainModelState<ComponentContainer> const& state, Idx const obj_seq) {
    switch (terminal_type) {
        using enum MeasuredTerminalType;

    case branch_from:
        return measured_component_active<Branch>(state, obj_seq, BranchSide::from);
    case branch_to:
        return measured_component_active<Branch>(state, obj_seq, BranchSide::to);
    case source:
        return measured_component_active<Source>(state, obj_seq);
    case shunt:
        return measured_component_active<Shunt>(state, obj_seq);
    case load:
        [[fallthrough]];
    case generator:
        return measured_component_active<GenericLoadGen>(state, obj_seq);
    case branch3_1:
        return measured_component_active<Branch3>(state, obj_seq, Branch3Side::side_1);
    case branch3_2:
        return measured_component_active<Branch3>(state, obj_seq, Branch3Side::side_2);
    case branch3_3:
        return measured_component_active<Branch3>(state, obj_seq, Branch3Side::side_3);
    case node:
        return true;
    default:
        throw MissingCaseForEnumError{"measured_terminal_active()", terminal_type};
    }
}

template <typename Component, typename IndexType, class ComponentContainer, non_owning_view_c ComponentOutput,
          functor_c ResFunc>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             std::invocable<ResFunc, Component const&, IndexType> &&
             assignable_to<std::invoke_result_t<ResFunc, Component const&, IndexType>,
                           std::ranges::range_reference_t<ComponentOutput>> &&
             std::convertible_to<IndexType, std::ranges::range_value_t<decltype(comp_base_sequence<Component>(
                                                MainModelState<ComponentContainer>{}))>>
constexpr void produce_output(MainModelState<ComponentContainer> const& state, ComponentOutput output, ResFunc func) {
    std::ranges::transform(get_component_citer<Component>(state.components), comp_base_sequence<Component>(state),
                           std::ranges::begin(output), func);
}

} // namespace detail

// output node
template <std::derived_from<Node> Component, class ComponentContainer, steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& node, MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx2D const& topo_id) {
    using sym = decode_symmetry_v<SolverOutputType>;

    auto const& math_id = get_math_id<Component>(state, topo_id.group);

    if (math_id.group == disconnected) {
        return node.template get_null_output<sym>();
    }

    return node.template get_output<sym>(get_voltage_output(math_output, math_id),
                                         get_bus_injection_output_from_topo_id(math_output, topo_id));
}
template <std::derived_from<Node> Component, class ComponentContainer,
          short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& node, MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx2D const& topo_id) {
    auto const& math_id = get_math_id<Component>(state, topo_id.group);

    if (math_id.group == disconnected) {
        return node.get_null_sc_output();
    }

    return node.get_sc_output(get_voltage_output(math_output, math_id));
}

// output link
template <std::same_as<Link> Component, class ComponentContainer, steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& link, MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx2D const& topo_id) {
    using sym = decode_symmetry_v<SolverOutputType>;

    if (topo_id.group == disconnected || state.topo_comp_coup->node[topo_id.group].group == disconnected) {
        return link.template get_null_output<sym>();
    }
    if (topo_id.pos == disconnected) {
        return link.template get_energized_zero_output<sym>();
    }
    if (!link.edge_status()) {
        return link.template get_energized_zero_output<sym>();
    }
    return link.template get_output<sym>(math_output.supernode_output[topo_id.group].link[topo_id.pos]);
}
template <std::same_as<Link> Component, class ComponentContainer, short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline auto output_result(Component const& link, MainModelState<ComponentContainer> const& state,
                          MathOutput<std::vector<SolverOutputType>> const& math_output, Idx2D const& topo_id) {
    if (topo_id.group == disconnected || state.topo_comp_coup->node[topo_id.group].group == disconnected) {
        return link.get_null_sc_output();
    }
    if (topo_id.pos == disconnected) {
        return link.get_energized_zero_sc_output();
    }
    if (!link.edge_status()) {
        return link.get_energized_zero_sc_output();
    }
    return link.get_sc_output(math_output.supernode_output[topo_id.group].link[topo_id.pos]);
}

// output branch
template <std::derived_from<Edge> Component, steady_state_solver_output_type SolverOutputType>
    requires(!std::same_as<Component, Link>) // TODO(mgovers): cleanup v2: change back to only derived_from<Branch>
constexpr auto output_result(Component const& branch, MathOutput<std::vector<SolverOutputType>> const& math_output,
                             Idx2D math_id) {
    using sym = decode_symmetry_v<SolverOutputType>;

    if (math_id.group == disconnected) {
        return branch.template get_null_output<sym>();
    }
    return branch.template get_output<sym>(get_component_output<Component>(math_output, math_id));
}
// TODO(mgovers): cleanup v2: change back to only derived_from<Branch>
template <std::derived_from<Edge> Component, short_circuit_solver_output_type SolverOutputType>
    requires(!std::same_as<Component, Link>) // TODO(mgovers): cleanup v2: change back to only derived_from<Branch>
inline auto output_result(Component const& branch, MathOutput<std::vector<SolverOutputType>> const& math_output,
                          Idx2D const& math_id) {
    if (math_id.group == disconnected) {
        return branch.get_null_sc_output();
    }
    return branch.get_sc_output(get_component_output<Component>(math_output, math_id));
}

// output branch3
template <std::derived_from<Branch3> Component, steady_state_solver_output_type SolverOutputType>
constexpr auto output_result(Component const& branch3, MathOutput<std::vector<SolverOutputType>> const& math_output,
                             Idx2DBranch3 const& math_id) {
    using sym = decode_symmetry_v<SolverOutputType>;

    if (math_id.group == disconnected) {
        return branch3.template get_null_output<sym>();
    }

    return branch3.template get_output<sym>(
        get_component_output<Branch3>(math_output, {.group = math_id.group, .pos = math_id.pos[0]}),
        get_component_output<Branch3>(math_output, {.group = math_id.group, .pos = math_id.pos[1]}),
        get_component_output<Branch3>(math_output, {.group = math_id.group, .pos = math_id.pos[2]}));
}
template <std::derived_from<Branch3> Component, short_circuit_solver_output_type SolverOutputType>
inline auto output_result(Component const& branch3, MathOutput<std::vector<SolverOutputType>> const& math_output,
                          Idx2DBranch3 const& math_id) {
    if (math_id.group == disconnected) {
        return branch3.get_null_sc_output();
    }

    return branch3.get_sc_output(
        get_component_output<Branch3>(math_output, {.group = math_id.group, .pos = math_id.pos[0]}),
        get_component_output<Branch3>(math_output, {.group = math_id.group, .pos = math_id.pos[1]}),
        get_component_output<Branch3>(math_output, {.group = math_id.group, .pos = math_id.pos[2]}));
}

// output source
template <std::derived_from<Source> Component, steady_state_solver_output_type SolverOutputType>
constexpr auto output_result(Component const& source, MathOutput<std::vector<SolverOutputType>> const& math_output,
                             Idx2D const& math_id) {
    using sym = decode_symmetry_v<SolverOutputType>;

    if (math_id.group == disconnected) {
        return source.template get_null_output<sym>();
    }
    return source.template get_output<sym>(get_component_output<Source>(math_output, math_id));
}
template <std::derived_from<Source> Component, short_circuit_solver_output_type SolverOutputType>
inline auto output_result(Component const& source, MathOutput<std::vector<SolverOutputType>> const& math_output,
                          Idx2D const& math_id) {
    if (math_id.group == disconnected) {
        return source.get_null_sc_output();
    }
    return source.get_sc_output(get_component_output<Source>(math_output, math_id));
}

// output load gen
template <std::derived_from<GenericLoadGen> Component, steady_state_solver_output_type SolverOutputType>
constexpr auto output_result(Component const& load_gen, MathOutput<std::vector<SolverOutputType>> const& math_output,
                             Idx2D const& math_id) {
    using sym = decode_symmetry_v<SolverOutputType>;

    if (math_id.group == disconnected) {
        return load_gen.template get_null_output<sym>();
    }
    return load_gen.template get_output<sym>(get_component_output<GenericLoadGen>(math_output, math_id));
}
template <std::derived_from<GenericLoadGen> Component, short_circuit_solver_output_type SolverOutputType>
inline auto output_result(Component const& load_gen, MathOutput<std::vector<SolverOutputType>> const& /*math_output*/,
                          Idx2D const& /*math_id*/) {
    return load_gen.get_null_sc_output();
}

// output shunt
template <std::derived_from<Shunt> Component, steady_state_solver_output_type SolverOutputType>
constexpr auto output_result(Component const& shunt, MathOutput<std::vector<SolverOutputType>> const& math_output,
                             Idx2D const& math_id) {
    using sym = decode_symmetry_v<SolverOutputType>;

    if (math_id.group == disconnected) {
        return shunt.template get_null_output<sym>();
    }
    return shunt.template get_output<sym>(get_component_output<Shunt>(math_output, math_id));
}
template <std::derived_from<Shunt> Component, short_circuit_solver_output_type SolverOutputType>
inline auto output_result(Component const& shunt, MathOutput<std::vector<SolverOutputType>> const& math_output,
                          Idx2D const& math_id) {
    if (math_id.group == disconnected) {
        return shunt.get_null_sc_output();
    }
    return shunt.get_sc_output(get_component_output<Shunt>(math_output, math_id));
}

// output voltage sensor
template <std::derived_from<GenericVoltageSensor> Component, class ComponentContainer,
          steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& voltage_sensor, MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx const node_seq) {
    using sym = decode_symmetry_v<SolverOutputType>;

    Idx2D const node_math_id = state.topo_comp_coup->node[node_seq];
    if (node_math_id.group == disconnected) {
        return voltage_sensor.template get_null_output<sym>();
    }
    return voltage_sensor.template get_output<sym>(get_voltage_output(math_output, node_math_id));
}
template <std::derived_from<GenericVoltageSensor> Component, class ComponentContainer,
          short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline auto output_result(Component const& voltage_sensor, MainModelState<ComponentContainer> const& /* state */,
                          MathOutput<std::vector<SolverOutputType>> const& /* math_output */,
                          Idx const /* node_seq */) {
    return voltage_sensor.get_null_sc_output();
}

// output power sensor
template <std::derived_from<GenericPowerSensor> Component, class ComponentContainer,
          steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& power_sensor, MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx const obj_seq) {
    using sym = decode_symmetry_v<SolverOutputType>;

    auto const terminal_type = power_sensor.get_terminal_type();
    Idx2D const obj_math_id = [&]() {
        switch (terminal_type) {
            using enum MeasuredTerminalType;

        case branch_from:
            [[fallthrough]];
        case branch_to:
            return state.topo_comp_coup->branch[obj_seq];
        case source:
            return state.topo_comp_coup->source[obj_seq];
        case shunt:
            return state.topo_comp_coup->shunt[obj_seq];
        case load:
            [[fallthrough]];
        case generator:
            return state.topo_comp_coup->load_gen[obj_seq];
        // from branch3, get relevant math object branch based on the measured side
        case branch3_1:
            return Idx2D{state.topo_comp_coup->branch3[obj_seq].group, state.topo_comp_coup->branch3[obj_seq].pos[0]};
        case branch3_2:
            return Idx2D{state.topo_comp_coup->branch3[obj_seq].group, state.topo_comp_coup->branch3[obj_seq].pos[1]};
        case branch3_3:
            return Idx2D{state.topo_comp_coup->branch3[obj_seq].group, state.topo_comp_coup->branch3[obj_seq].pos[2]};
        case node:
            return state.topo_comp_coup->node[obj_seq];
        default:
            throw MissingCaseForEnumError{std::format("{} output_result()", Component::name), terminal_type};
        }
    }();

    if (obj_math_id.group == disconnected || !detail::measured_terminal_active(terminal_type, state, obj_seq)) {
        return power_sensor.template get_null_output<sym>();
    }

    switch (terminal_type) {
        using enum MeasuredTerminalType;

    case branch_from:
        // all power sensors in branch3 are at from side in the mathematical model
        [[fallthrough]];
    case branch3_1:
        [[fallthrough]];
    case branch3_2:
        [[fallthrough]];
    case branch3_3:
        return power_sensor.template get_output<sym>(get_component_output<Branch>(math_output, obj_math_id).s_f);
    case branch_to:
        return power_sensor.template get_output<sym>(get_component_output<Branch>(math_output, obj_math_id).s_t);
    case source:
        return power_sensor.template get_output<sym>(get_component_output<Source>(math_output, obj_math_id).s);
    case shunt:
        return power_sensor.template get_output<sym>(get_component_output<Shunt>(math_output, obj_math_id).s);
    case load:
        [[fallthrough]];
    case generator:
        return power_sensor.template get_output<sym>(get_component_output<GenericLoadGen>(math_output, obj_math_id).s);
    case node:
        return power_sensor.template get_output<sym>(get_bus_injection_output_from_math_id(math_output, obj_math_id));
    default:
        throw MissingCaseForEnumError{std::format("{} output_result()", Component::name), terminal_type};
    }
}
template <std::derived_from<GenericPowerSensor> Component, class ComponentContainer,
          short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto
output_result(Component const& power_or_current_sensor, MainModelState<ComponentContainer> const& /* state */,
              MathOutput<std::vector<SolverOutputType>> const& /* math_output */, Idx const /* obj_seq */) {
    return power_or_current_sensor.get_null_sc_output();
}

// output current sensor
template <std::derived_from<GenericCurrentSensor> Component, class ComponentContainer,
          steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& current_sensor, MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx const obj_seq) {
    using sym = decode_symmetry_v<SolverOutputType>;

    auto const terminal_type = current_sensor.get_terminal_type();
    Idx2D const obj_math_id = [&]() {
        switch (terminal_type) {
            using enum MeasuredTerminalType;

        case branch_from:
            [[fallthrough]];
        case branch_to:
            return state.topo_comp_coup->branch[obj_seq];
        // from branch3, get relevant math object branch based on the measured side
        case branch3_1:
            return Idx2D{state.topo_comp_coup->branch3[obj_seq].group, state.topo_comp_coup->branch3[obj_seq].pos[0]};
        case branch3_2:
            return Idx2D{state.topo_comp_coup->branch3[obj_seq].group, state.topo_comp_coup->branch3[obj_seq].pos[1]};
        case branch3_3:
            return Idx2D{state.topo_comp_coup->branch3[obj_seq].group, state.topo_comp_coup->branch3[obj_seq].pos[2]};
        default:
            throw MissingCaseForEnumError{std::format("{} output_result()", Component::name), terminal_type};
        }
    }();

    if (obj_math_id.group == disconnected || !detail::measured_terminal_active(terminal_type, state, obj_seq)) {
        return current_sensor.template get_null_output<sym>();
    }

    auto const topological_index = get_topology_index<Branch>(state.components, obj_math_id);
    auto const branch_nodes = get_branch_nodes<Branch>(state, topological_index);
    auto const node_from_math_id = get_math_id<Node>(state, branch_nodes[0]);
    auto const node_to_math_id = get_math_id<Node>(state, branch_nodes[1]);

    switch (terminal_type) {
        using enum MeasuredTerminalType;

    case branch_from:
        // all power sensors in branch3 are at from side in the mathematical model
        [[fallthrough]];
    case branch3_1:
        [[fallthrough]];
    case branch3_2:
        [[fallthrough]];
    case branch3_3:
        return current_sensor.template get_output<sym>(get_component_output<Branch>(math_output, obj_math_id).i_f,
                                                       get_voltage_output(math_output, node_from_math_id));
    case branch_to:
        return current_sensor.template get_output<sym>(get_component_output<Branch>(math_output, obj_math_id).i_t,
                                                       get_voltage_output(math_output, node_to_math_id));
    default:
        throw MissingCaseForEnumError{std::format("{} output_result()", Component::name), terminal_type};
    }
}
template <std::derived_from<GenericCurrentSensor> Component, class ComponentContainer,
          short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto
output_result(Component const& power_or_current_sensor, MainModelState<ComponentContainer> const& /* state */,
              MathOutput<std::vector<SolverOutputType>> const& /* math_output */, Idx const /* obj_seq */) {
    return power_or_current_sensor.get_null_sc_output();
}

// output fault
template <std::derived_from<Fault> Component, class ComponentContainer,
          steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             model_component_state_c<MainModelState, ComponentContainer, Node>
constexpr auto output_result(Component const& fault, MainModelState<ComponentContainer> const& /* state */,
                             MathOutput<std::vector<SolverOutputType>> const& /* math_output */, Idx2D /* math_id */) {
    return fault.get_output();
}
template <std::derived_from<Fault> Component, class ComponentContainer,
          short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             model_component_state_c<MainModelState, ComponentContainer, Node>
inline auto output_result(Component const& fault, MainModelState<ComponentContainer> const& state,
                          MathOutput<std::vector<SolverOutputType>> const& math_output, Idx2D math_id) {
    if (math_id.group == disconnected) {
        return fault.get_null_sc_output();
    }

    auto const u_rated = get_component<Node>(state.components, fault.get_fault_object()).u_rated();
    return fault.get_sc_output(get_component_output<Fault>(math_output, math_id), u_rated);
}

// output transformer tap regulator
template <std::derived_from<TransformerTapRegulator> Component, class ComponentContainer,
          steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& transformer_tap_regulator,
                             MainModelState<ComponentContainer> const& /* state */,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx const /* obj_seq */) {
    if (auto const it = std::ranges::find_if(
            math_output.optimizer_output.transformer_tap_positions,
            [regulated_object = transformer_tap_regulator.regulated_object()](auto const& transformer_tap_pos) {
                return transformer_tap_pos.transformer_id == regulated_object;
            });
        it != std::end(math_output.optimizer_output.transformer_tap_positions)) {
        return transformer_tap_regulator.get_output(it->tap_position);
    }
    return transformer_tap_regulator.get_null_output();
}
template <std::derived_from<TransformerTapRegulator> Component, class ComponentContainer,
          short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto
output_result(Component const& transformer_tap_regulator, MainModelState<ComponentContainer> const& /* state */,
              MathOutput<std::vector<SolverOutputType>> const& /* math_output */, Idx const /* obj_seq */) {
    return transformer_tap_regulator.get_null_sc_output();
}

// output voltage regulator
template <std::derived_from<VoltageRegulator> Component, class ComponentContainer,
          steady_state_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& voltage_regulator, MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, Idx const obj_seq) {
    if (Idx2D const load_gen_math_id = state.topo_comp_coup->load_gen[obj_seq];
        load_gen_math_id.group != disconnected) {
        for (auto const& vr_output : math_output.solver_output[load_gen_math_id.group].voltage_regulator) {
            if (vr_output.generator_id == voltage_regulator.regulated_object()) {
                return voltage_regulator.get_output(vr_output);
            }
        }
    }
    return voltage_regulator.get_null_output();
}

template <std::derived_from<VoltageRegulator> Component, class ComponentContainer,
          short_circuit_solver_output_type SolverOutputType>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr auto output_result(Component const& voltage_regulator, MainModelState<ComponentContainer> const& /* state */,
                             MathOutput<std::vector<SolverOutputType>> const& /* math_output */,
                             Idx const /* obj_seq */) {
    return voltage_regulator.get_null_sc_output();
}

template <std::same_as<Link> Component, class ComponentContainer, solver_output_type SolverOutputType,
          non_owning_view_c ComponentOutput>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void output_result(MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, ComponentOutput output) {
    if (auto const& link_topo_ids = state.reduced_topology->topo_node_coup.coupling.user_links_to_topo_nodes;
        std::ranges::ssize(link_topo_ids) ==
        get_component_size<Component>(
            state.components)) { // TODO(mgovers): cleanup v2: this should be the only code path remaining
        std::ranges::transform(
            get_component_citer<Component>(state.components), link_topo_ids, std::ranges::begin(output),
            [&state, &math_output](Component const& link, Idx2D const& topo_id) {
                return output_result<Component, ComponentContainer>(link, state, math_output, topo_id);
            });
    } else {
        detail::produce_output<Component, Idx2D>(state, output,
                                                 [&math_output](Component const& link, Idx2D const& math_id) {
                                                     return output_result<Edge>(link, math_output, math_id);
                                                 });
    }
}

// output base component
template <std::derived_from<Base> Component, class ComponentContainer, solver_output_type SolverOutputType,
          non_owning_view_c ComponentOutput>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             requires(Component const& component, MathOutput<std::vector<SolverOutputType>> const& math_output,
                      Idx2D math_id) {
                 {
                     output_result<Component>(component, math_output, math_id)
                 } -> detail::assignable_to<std::ranges::range_reference_t<ComponentOutput>>;
             }
constexpr void output_result(MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, ComponentOutput output) {
    detail::produce_output<Component, Idx2D>(state, output, [&math_output](Component const& component, Idx2D math_id) {
        return output_result<Component>(component, math_output, math_id);
    });
}
template <std::derived_from<Base> Component, class ComponentContainer, solver_output_type SolverOutputType,
          non_owning_view_c ComponentOutput>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             requires(Component const& component, MainModelState<ComponentContainer> const& state,
                      MathOutput<std::vector<SolverOutputType>> const& math_output, Idx2D math_id) {
                 {
                     output_result<Component>(component, state, math_output, math_id)
                 } -> detail::assignable_to<std::ranges::range_reference_t<ComponentOutput>>;
             } &&
             (!std::same_as<Component, Link>) // TODO(mgovers): cleanup v2: this requirement should no longer be needed
                                              // after link output is cleaned up
constexpr void output_result(MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, ComponentOutput output) {
    detail::produce_output<Component, Idx2D>(
        state, output, [&state, &math_output](Component const& component, Idx2D const math_id) {
            return output_result<Component>(component, state, math_output, math_id);
        });
}
template <std::derived_from<Base> Component, class ComponentContainer, solver_output_type SolverOutputType,
          non_owning_view_c ComponentOutput>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             requires(Component const& component, MainModelState<ComponentContainer> const& state,
                      MathOutput<std::vector<SolverOutputType>> const& math_output, Idx obj_seq) {
                 {
                     output_result<Component>(component, state, math_output, obj_seq)
                 } -> detail::assignable_to<std::ranges::range_reference_t<ComponentOutput>>;
             }
constexpr void output_result(MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, ComponentOutput output) {
    detail::produce_output<Component, Idx>(
        state, output, [&state, &math_output](Component const& component, Idx const obj_seq) {
            return output_result<Component, ComponentContainer>(component, state, math_output, obj_seq);
        });
}
template <std::derived_from<Base> Component, class ComponentContainer, solver_output_type SolverOutputType,
          non_owning_view_c ComponentOutput>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             requires(Component const& component, MathOutput<std::vector<SolverOutputType>> const& math_output,
                      Idx2DBranch3 const& math_id) {
                 {
                     output_result<Component>(component, math_output, math_id)
                 } -> detail::assignable_to<std::ranges::range_reference_t<ComponentOutput>>;
             }
constexpr void output_result(MainModelState<ComponentContainer> const& state,
                             MathOutput<std::vector<SolverOutputType>> const& math_output, ComponentOutput output) {
    detail::produce_output<Component, Idx2DBranch3>(
        state, output, [&math_output](Component const& component, Idx2DBranch3 const& math_id) {
            return output_result<Component>(component, math_output, math_id);
        });
}
template <std::derived_from<Base> Component, class ComponentContainer, typename SolverOutputType,
          non_owning_view_c ComponentOutput>
    requires model_component_state_c<MainModelState, ComponentContainer, Component> &&
             requires(Component const& component, MainModelState<ComponentContainer> const& state,
                      MathOutput<SolverOutputType> const& math_output, Idx2D const& topo_id) {
                 {
                     output_result<Component>(component, state, math_output, topo_id)
                 } -> detail::assignable_to<std::ranges::range_reference_t<ComponentOutput>>;
             }
constexpr void output_result(MainModelState<ComponentContainer> const& state,
                             MathOutput<SolverOutputType> const& math_output, ComponentOutput output) {
    detail::produce_output<Component, Idx2D>(
        state, output, [&state, &math_output](Component const& component, Idx2D const& topo_id) {
            return output_result<Component, ComponentContainer>(component, state, math_output, topo_id);
        });
}
// vector overload
template <std::derived_from<Base> Component, class ComponentContainer, typename SolverOutputType, class T>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
constexpr void output_result(MainModelState<ComponentContainer> const& state,
                             MathOutput<SolverOutputType> const& math_output, std::vector<T>& output) {
    return output_result<Component>(state, math_output, by_ref(output));
}
} // namespace power_grid_model::main_core
