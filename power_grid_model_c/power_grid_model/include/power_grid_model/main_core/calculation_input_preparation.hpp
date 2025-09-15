// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state_queries.hpp"

#include "../calculation_parameters.hpp"

#include <concepts>
#include <vector>

namespace power_grid_model::main_core {
constexpr Idx isolated_component{-1};
constexpr Idx not_connected{-1};

namespace detail {
template <calculation_input_type CalcInputType>
inline auto calculate_param(auto const& c, auto const&... extra_args)
    requires requires {
        { c.calc_param(extra_args...) };
    }
{
    return c.calc_param(extra_args...);
}

template <calculation_input_type CalcInputType>
inline auto calculate_param(auto const& c, auto const&... extra_args)
    requires requires {
        { c.template calc_param<typename CalcInputType::sym>(extra_args...) };
    }
{
    return c.template calc_param<typename CalcInputType::sym>(extra_args...);
}

/** This is a heavily templated member function because it operates on many different variables of many
 *different types, but the essence is ever the same: filling one member (vector) of the calculation calc_input
 *struct (soa) with the right calculation symmetric or asymmetric calculation parameters, in the same order as
 *the corresponding component are stored in the component topology. There is one such struct for each sub graph
 * / math model and all of them are filled within the same function call (i.e. Notice that calc_input is a
 *vector).
 *
 *  1. For each component, check if it should be included.
 *     By default, all component are included, except for some cases, like power sensors. For power sensors, the
 *     list of component contains all power sensors, but the preparation should only be done for one type of
 *power sensors at a time. Therefore, `included` will be a lambda function, such as:
 *
 *       [this](Idx i) { return state_.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::source;
 *}
 *
 *  2. Find the original component in the topology and retrieve its calculation parameters.
 *
 *  3. Fill the calculation parameters of the right math model.
 *
 *  @tparam CalcStructOut
 *      The calculation input type (soa) for the desired calculation (e.g. PowerFlowInput<sym> or
 *StateEstimationInput<sym>).
 *
 * @tparam CalcParamOut
 *      The data type for the desired calculation for the given ComponentIn (e.g. SourceCalcParam<sym> or
 *      VoltageSensorCalcParam<sym>).
 *
 * @tparam comp_vect
 *      The (pre-allocated and resized) vector of CalcParamOuts which shall be filled with component specific
 *      calculation parameters. Note that this is a pointer to member
 *
 * @tparam ComponentIn
 * 	    The component type for which we are collecting calculation parameters
 *
 * @tparam PredicateIn
 * 	    The lambda function type. The actual type depends on the captured variables, and will be
 *automatically deduced.
 *
 * @param component[in]
 *      The vector of component math indices to consider (e.g. state_.topo_comp_coup->source).
 *      When idx.group = -1, the original component is not assigned to a math model, so we can skip it.
 *
 * @param calc_input[out]
 *		Although this variable is called `input`, it is actually the output of this function, it stored
 *the calculation parameters for each math model, for each component of type ComponentIn.
 *
 * @param include
 *      A lambda function (Idx i -> bool) which returns true if the component at Idx i should be included.
 * 	    The default lambda `include_all` always returns `true`.
 */
template <calculation_input_type CalcStructOut, typename CalcParamOut,
          std::vector<CalcParamOut>(CalcStructOut::*comp_vect), class ComponentIn,
          std::invocable<Idx> PredicateIn = IncludeAll>
    requires std::convertible_to<std::invoke_result_t<PredicateIn, Idx>, bool>
void prepare_input(main_model_state_c auto const& state, std::vector<Idx2D> const& components,
                   std::vector<CalcStructOut>& calc_input, PredicateIn include = include_all) {
    for (Idx i = 0, n = narrow_cast<Idx>(components.size()); i != n; ++i) {
        if (include(i)) {
            Idx2D const math_idx = components[i];
            if (math_idx.group != isolated_component) {
                auto const& component = get_component_by_sequence<ComponentIn>(state.components, i);
                CalcStructOut& math_model_input = calc_input[math_idx.group];
                std::vector<CalcParamOut>& math_model_input_vect = math_model_input.*comp_vect;
                math_model_input_vect[math_idx.pos] = calculate_param<CalcStructOut>(component);
            }
        }
    }
}

template <calculation_input_type CalcStructOut, typename CalcParamOut,
          std::vector<CalcParamOut>(CalcStructOut::*comp_vect), class ComponentIn,
          std::invocable<Idx> PredicateIn = IncludeAll>
    requires std::convertible_to<std::invoke_result_t<PredicateIn, Idx>, bool>
void prepare_input(main_model_state_c auto const& state, std::vector<Idx2D> const& components,
                   std::vector<CalcStructOut>& calc_input, std::invocable<ComponentIn const&> auto extra_args,
                   PredicateIn include = include_all) {
    for (Idx i = 0, n = narrow_cast<Idx>(components.size()); i != n; ++i) {
        if (include(i)) {
            Idx2D const math_idx = components[i];
            if (math_idx.group != isolated_component) {
                auto const& component = get_component_by_sequence<ComponentIn>(state.components, i);
                CalcStructOut& math_model_input = calc_input[math_idx.group];
                std::vector<CalcParamOut>& math_model_input_vect = math_model_input.*comp_vect;
                math_model_input_vect[math_idx.pos] = calculate_param<CalcStructOut>(component, extra_args(component));
            }
        }
    }
}

template <symmetry_tag sym, IntSVector(StateEstimationInput<sym>::*component), class Component>
void prepare_input_status(main_model_state_c auto const& state, std::vector<Idx2D> const& objects,
                          std::vector<StateEstimationInput<sym>>& input) {
    for (Idx i = 0, n = narrow_cast<Idx>(objects.size()); i != n; ++i) {
        Idx2D const math_idx = objects[i];
        if (math_idx.group == isolated_component) {
            continue;
        }
        (input[math_idx.group].*component)[math_idx.pos] =
            main_core::get_component_by_sequence<Component>(state.components, i).status();
    }
}
} // namespace detail

template <symmetry_tag sym>
std::vector<PowerFlowInput<sym>> prepare_power_flow_input(main_model_state_c auto const& state, Idx n_math_solvers) {
    using detail::prepare_input;

    std::vector<PowerFlowInput<sym>> pf_input(n_math_solvers);
    for (Idx i = 0; i != n_math_solvers; ++i) {
        pf_input[i].s_injection.resize(state.math_topology[i]->n_load_gen());
        pf_input[i].source.resize(state.math_topology[i]->n_source());
    }
    prepare_input<PowerFlowInput<sym>, DoubleComplex, &PowerFlowInput<sym>::source, Source>(
        state, state.topo_comp_coup->source, pf_input);

    prepare_input<PowerFlowInput<sym>, ComplexValue<sym>, &PowerFlowInput<sym>::s_injection, GenericLoadGen>(
        state, state.topo_comp_coup->load_gen, pf_input);

    return pf_input;
}

template <symmetry_tag sym>
std::vector<StateEstimationInput<sym>> prepare_state_estimation_input(main_model_state_c auto const& state,
                                                                      Idx n_math_solvers) {
    using detail::prepare_input;
    using detail::prepare_input_status;

    std::vector<StateEstimationInput<sym>> se_input(n_math_solvers);

    for (Idx i = 0; i != n_math_solvers; ++i) {
        se_input[i].shunt_status.resize(state.math_topology[i]->n_shunt());
        se_input[i].load_gen_status.resize(state.math_topology[i]->n_load_gen());
        se_input[i].source_status.resize(state.math_topology[i]->n_source());
        se_input[i].measured_voltage.resize(state.math_topology[i]->n_voltage_sensor());
        se_input[i].measured_source_power.resize(state.math_topology[i]->n_source_power_sensor());
        se_input[i].measured_load_gen_power.resize(state.math_topology[i]->n_load_gen_power_sensor());
        se_input[i].measured_shunt_power.resize(state.math_topology[i]->n_shunt_power_power_sensor());
        se_input[i].measured_branch_from_power.resize(state.math_topology[i]->n_branch_from_power_sensor());
        se_input[i].measured_branch_to_power.resize(state.math_topology[i]->n_branch_to_power_sensor());
        se_input[i].measured_bus_injection.resize(state.math_topology[i]->n_bus_power_sensor());
        se_input[i].measured_branch_from_current.resize(state.math_topology[i]->n_branch_from_current_sensor());
        se_input[i].measured_branch_to_current.resize(state.math_topology[i]->n_branch_to_current_sensor());
    }

    prepare_input_status<sym, &StateEstimationInput<sym>::shunt_status, Shunt>(state, state.topo_comp_coup->shunt,
                                                                               se_input);
    prepare_input_status<sym, &StateEstimationInput<sym>::load_gen_status, GenericLoadGen>(
        state, state.topo_comp_coup->load_gen, se_input);
    prepare_input_status<sym, &StateEstimationInput<sym>::source_status, Source>(state, state.topo_comp_coup->source,
                                                                                 se_input);

    prepare_input<StateEstimationInput<sym>, VoltageSensorCalcParam<sym>, &StateEstimationInput<sym>::measured_voltage,
                  GenericVoltageSensor>(state, state.topo_comp_coup->voltage_sensor, se_input);
    prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_source_power, GenericPowerSensor>(
        state, state.topo_comp_coup->power_sensor, se_input,
        [&state](Idx i) { return state.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::source; });
    prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_load_gen_power, GenericPowerSensor>(
        state, state.topo_comp_coup->power_sensor, se_input, [&state](Idx i) {
            return state.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::load ||
                   state.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::generator;
        });
    prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_shunt_power, GenericPowerSensor>(
        state, state.topo_comp_coup->power_sensor, se_input,
        [&state](Idx i) { return state.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::shunt; });
    prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_branch_from_power, GenericPowerSensor>(
        state, state.topo_comp_coup->power_sensor, se_input, [&state](Idx i) {
            using enum MeasuredTerminalType;
            return state.comp_topo->power_sensor_terminal_type[i] == branch_from ||
                   // all branch3 sensors are at from side in the mathematical model
                   state.comp_topo->power_sensor_terminal_type[i] == branch3_1 ||
                   state.comp_topo->power_sensor_terminal_type[i] == branch3_2 ||
                   state.comp_topo->power_sensor_terminal_type[i] == branch3_3;
        });
    prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_branch_to_power, GenericPowerSensor>(
        state, state.topo_comp_coup->power_sensor, se_input,
        [&state](Idx i) { return state.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::branch_to; });
    prepare_input<StateEstimationInput<sym>, PowerSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_bus_injection, GenericPowerSensor>(
        state, state.topo_comp_coup->power_sensor, se_input,
        [&state](Idx i) { return state.comp_topo->power_sensor_terminal_type[i] == MeasuredTerminalType::node; });

    prepare_input<StateEstimationInput<sym>, CurrentSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_branch_from_current, GenericCurrentSensor>(
        state, state.topo_comp_coup->current_sensor, se_input, [&state](Idx i) {
            using enum MeasuredTerminalType;
            return state.comp_topo->current_sensor_terminal_type[i] == branch_from ||
                   // all branch3 sensors are at from side in the mathematical model
                   state.comp_topo->current_sensor_terminal_type[i] == branch3_1 ||
                   state.comp_topo->current_sensor_terminal_type[i] == branch3_2 ||
                   state.comp_topo->current_sensor_terminal_type[i] == branch3_3;
        });
    prepare_input<StateEstimationInput<sym>, CurrentSensorCalcParam<sym>,
                  &StateEstimationInput<sym>::measured_branch_to_current, GenericCurrentSensor>(
        state, state.topo_comp_coup->current_sensor, se_input, [&state](Idx i) {
            return state.comp_topo->current_sensor_terminal_type[i] == MeasuredTerminalType::branch_to;
        });

    return se_input;
}

template <symmetry_tag sym>
std::vector<ShortCircuitInput> prepare_short_circuit_input(main_model_state_c auto const& state,
                                                           ComponentToMathCoupling& comp_coup, Idx n_math_solvers,
                                                           ShortCircuitVoltageScaling voltage_scaling) {
    using detail::prepare_input;

    // TODO(mgovers) split component mapping from actual preparing
    std::vector<IdxVector> topo_fault_indices(state.math_topology.size());
    std::vector<IdxVector> topo_bus_indices(state.math_topology.size());

    for (Idx fault_idx{0}; fault_idx < state.components.template size<Fault>(); ++fault_idx) {
        auto const& fault = state.components.template get_item_by_seq<Fault>(fault_idx);
        if (fault.status()) {
            auto const node_idx = state.components.template get_seq<Node>(fault.get_fault_object());
            auto const topo_bus_idx = state.topo_comp_coup->node[node_idx];

            if (topo_bus_idx.group >= 0) { // Consider non-isolated objects only
                topo_fault_indices[topo_bus_idx.group].push_back(fault_idx);
                topo_bus_indices[topo_bus_idx.group].push_back(topo_bus_idx.pos);
            }
        }
    }

    auto fault_coup = std::vector<Idx2D>(state.components.template size<Fault>(),
                                         Idx2D{.group = isolated_component, .pos = not_connected});
    std::vector<ShortCircuitInput> sc_input(n_math_solvers);

    for (Idx i = 0; i != n_math_solvers; ++i) {
        auto map = build_dense_mapping(topo_bus_indices[i], state.math_topology[i]->n_bus());

        for (Idx reordered_idx{0}; reordered_idx < static_cast<Idx>(map.reorder.size()); ++reordered_idx) {
            fault_coup[topo_fault_indices[i][map.reorder[reordered_idx]]] = Idx2D{.group = i, .pos = reordered_idx};
        }

        sc_input[i].fault_buses = {from_dense, std::move(map.indvector), state.math_topology[i]->n_bus()};
        sc_input[i].faults.resize(state.components.template size<Fault>());
        sc_input[i].source.resize(state.math_topology[i]->n_source());
    }

    comp_coup = ComponentToMathCoupling{.fault = std::move(fault_coup)};

    prepare_input<ShortCircuitInput, FaultCalcParam, &ShortCircuitInput::faults, Fault>(
        state, comp_coup.fault, sc_input, [&state](Fault const& fault) {
            return state.components.template get_item<Node>(fault.get_fault_object()).u_rated();
        });
    prepare_input<ShortCircuitInput, DoubleComplex, &ShortCircuitInput::source, Source>(
        state, state.topo_comp_coup->source, sc_input, [&state, voltage_scaling](Source const& source) {
            return std::pair{state.components.template get_item<Node>(source.node()).u_rated(), voltage_scaling};
        });

    return sc_input;
}
} // namespace power_grid_model::main_core
