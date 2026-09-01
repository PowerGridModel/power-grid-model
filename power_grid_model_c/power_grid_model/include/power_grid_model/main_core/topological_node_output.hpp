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
            if constexpr (steady_state_solver_output_type<SolverOutput>) {
                auto const n_links = narrow_cast<Idx>(std::ranges::ssize(topo_node.user_links));
                return {.bus_injection = ComplexValueVector<sym>(topo_node.user_nodes.size()),
                        .link = std::vector<BranchSolverOutput<sym>>(n_links)};
            } else if constexpr (short_circuit_solver_output_type<SolverOutput>) {
                auto const n_links = narrow_cast<Idx>(std::ranges::ssize(topo_node.user_links));
                return {.link = std::vector<BranchShortCircuitSolverOutput<sym>>(n_links)};
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

        // Solve for link flows within each topological node
        for (auto const& [topo_idx, topo_node] : enumerate(state.reduced_topology->topo_node_coup.topo_nodes)) {
            if (topo_node.user_links.empty()) {
                continue; // No links in this topological node
            }

            // Build mapping from global user node index to local index within this topological node
            std::unordered_map<Idx, Idx> global_to_local;
            for (auto const& [local_idx, global_idx] : enumerate(topo_node.user_nodes)) {
                global_to_local[global_idx] = local_idx;
            }

            // Remap user_links to use local indices, filtering out disconnected links
            std::vector<BranchIdx> local_links;
            std::vector<Idx> link_mapping; // Maps local_links indices to original user_links indices
            local_links.reserve(topo_node.user_links.size());
            link_mapping.reserve(topo_node.user_links.size());

            for (auto const& [link_idx, link] : enumerate(topo_node.user_links)) {
                auto const& [from_global, to_global] = link;
                // Skip links where either side is disconnected
                if (from_global == disconnected || to_global == disconnected) {
                    continue;
                }
                local_links.push_back(BranchIdx{global_to_local.at(from_global), global_to_local.at(to_global)});
                link_mapping.push_back(link_idx);
            }

            // If all links are disconnected, set null outputs and continue
            if (local_links.empty()) {
                for (auto& link_output : supernode_output[topo_idx].link) {
                    link_output = BranchSolverOutput<sym>{}; // Zero/null output
                }
                continue;
            }

            // Prepare node loads for link solver (always uses DoubleComplex = std::complex<double>)
            std::vector<DoubleComplex> node_loads;
            node_loads.reserve(topo_node.user_nodes.size());

            if constexpr (is_symmetric_v<sym>) {
                // Symmetric: direct copy
                for (auto const& injection : supernode_output[topo_idx].bus_injection) {
                    node_loads.push_back(injection);
                }
            } else {
                // Asymmetric: use average of 3 phases
                for (auto const& injection : supernode_output[topo_idx].bus_injection) {
                    node_loads.push_back((injection(0) + injection(1) + injection(2)) / 3.0);
                }
            }

            // Call link solver with local indices
            auto const link_flows = link_solver::compute_loads_link_elements(local_links, node_loads);

            // Convert link solver output to BranchSolverOutput format, placing them at correct indices
            for (auto const& [local_idx, flow] : enumerate(link_flows)) {
                auto const original_link_idx = link_mapping[local_idx];
                if constexpr (is_symmetric_v<sym>) {
                    // Symmetric: direct assignment
                    supernode_output[topo_idx].link[original_link_idx] = BranchSolverOutput<sym>{
                        .s_f = flow,
                        .s_t = -flow // Power out at to-side is negative of power in at from-side
                    };
                } else {
                    // Asymmetric: replicate to 3 phases
                    supernode_output[topo_idx].link[original_link_idx] = BranchSolverOutput<sym>{
                        .s_f = ComplexValue<sym>{flow, flow, flow}, .s_t = ComplexValue<sym>{-flow, -flow, -flow}};
                }
            }
        }
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
