// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "core_utils.hpp"

#include "../calculation_parameters.hpp"
#include "../link_solver.hpp"
#include "../main_core/math_output_queries.hpp"
#include "../main_core/state.hpp"
#include "../main_core/state_queries.hpp"

#include <ranges>

namespace power_grid_model::main_core {
namespace detail {

template <typename ComponentSolverOutputType>
    requires symmetry_tag<decode_symmetry_v<ComponentSolverOutputType>> &&
             (std::same_as<ComponentSolverOutputType,
                           ApplianceSolverOutput<decode_symmetry_v<ComponentSolverOutputType>>> ||
              std::same_as<ComponentSolverOutputType,
                           ApplianceShortCircuitSolverOutput<decode_symmetry_v<ComponentSolverOutputType>>>)
inline auto const& get_injection(ComponentSolverOutputType const& component_output) {
    if constexpr (requires { component_output.s; }) {
        return component_output.s;
    } else if constexpr (requires { component_output.i; }) {
        return component_output.i;
    } else {
        static_assert(false, "ComponentSolverOutputType must have either s or i member");
    }
}

template <typename ComponentType>
inline Idx get_node_sequence_idx(main_model_state_c auto const& state, Idx const& component_idx) {
    if constexpr (std::same_as<ComponentType, Source>) {
        return state.comp_topo
            ->source_node_idx[get_component_sequence_offset<Source, ComponentType>(state.components) + component_idx];
    } else if constexpr (std::derived_from<ComponentType, GenericLoadGen>) {
        return state.comp_topo
            ->load_gen_node_idx[get_component_sequence_offset<GenericLoadGen, ComponentType>(state.components) +
                                component_idx];
    } else {
        static_assert(false, "Unsupported component type for node sequence index retrieval");
    }
}

template <typename ComponentType, typename SolverOutputType, typename AddToTarget>
    requires std::invocable<AddToTarget, Idx2D const&, ComplexValue<decode_symmetry_v<SolverOutputType>> const&>
inline void add_appliance_injection(main_model_state_c auto const& state,
                                    MathOutput<std::vector<SolverOutputType>> const& math_output,
                                    AddToTarget accumulate_injection) {
    for (auto const& [component_idx, component_math_id] : enumerate(comp_base_sequence<ComponentType>(state))) {
        if (component_math_id.group == disconnected) {
            continue;
        }
        auto const& component_output = get_component_output<ComponentType>(math_output, component_math_id);

        auto const& user_node_idx = get_node_sequence_idx<ComponentType>(state, component_idx);
        auto const& user_topo_id =
            state.reduced_topology->topo_node_coup.coupling.user_nodes_to_topo_nodes[user_node_idx];

        auto const injection = get_injection(component_output);
        accumulate_injection(user_topo_id, injection);
    }
}

template <main_model_state_c State, solver_output_type SolverOutput>
inline std::vector<SupernodeOutput<SolverOutput>>
solve_topological_nodes(State const& state, MathOutput<std::vector<SolverOutput>>& math_output) {
    using sym = decode_symmetry_v<SolverOutput>;

    std::vector<SupernodeOutput<SolverOutput>> supernode_output =
        state.reduced_topology->topo_node_coup.topo_nodes |
        std::views::transform([](auto const& topo_node) -> SupernodeOutput<SolverOutput> {
            (void)topo_node; // suppress unused variable warning when not steady-state solver output
            if constexpr (steady_state_solver_output_type<SolverOutput>) {
                return {.bus_injection = ComplexValueVector<sym>(topo_node.user_nodes.size()), .branch = {}};
            } else {
                return {};
            }
        }) |
        std::ranges::to<std::vector>();

    auto const accumulate_injection = [&supernode_output](Idx2D const& user_topo_id,
                                                          ComplexValue<sym> const& injection) {
        (void)supernode_output; // suppress unused variable warning when not steady-state solver output
        (void)user_topo_id;     // suppress unused variable warning when not steady-state solver output
        (void)injection;        // suppress unused variable warning when not steady-state solver output

        if constexpr (steady_state_solver_output_type<SolverOutput>) {
            supernode_output[user_topo_id.group].bus_injection[user_topo_id.pos] += injection;
        }
    };

    if constexpr (steady_state_solver_output_type<SolverOutput>) {
        using InjectionComponentTypesTuple = std::tuple<Source, SymLoad, SymGenerator, AsymLoad, AsymGenerator>;

        utils::run_functor_with_tuple_return_void<InjectionComponentTypesTuple>(
            [&state, &math_output, &accumulate_injection]<typename ComponentType>() {
                if constexpr (decltype(state.components)::template is_storageable_v<ComponentType>) {
                    add_appliance_injection<ComponentType>(state, math_output, accumulate_injection);
                }
            });
    }

    return supernode_output;
}
} // namespace detail

template <main_model_state_c State, typename SolverOutput>
inline void solve_topological_nodes(State const& state, MathOutput<std::vector<SolverOutput>>& math_output) {
    assert(std::ranges::empty(math_output.supernode_output));
    math_output.supernode_output = detail::solve_topological_nodes(state, math_output);
}
} // namespace power_grid_model::main_core
