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
#include <type_traits>

namespace power_grid_model::main_core {
namespace detail {
template <symmetry_tag sym> struct SuperNodeSolverInput {
    ComplexValueVector<sym> bus_injection;
    ComplexValueVector<sym> bus_flow_from_branch;
};

template <typename Callable> Callable unwrap_callable(Callable callable) { return callable; }

template <typename Callable> Callable& unwrap_callable(std::reference_wrapper<Callable> callable_ref) {
    return callable_ref.get();
}

template <typename AddToTarget, typename ComponentType, typename SolverOutputType>
concept flow_accumulator_c = requires(AddToTarget accumulator, Idx2D const& user_topo_id,
                                      ComplexValue<decode_symmetry_v<SolverOutputType>> const& injection) {
    unwrap_callable(accumulator).template operator()<ComponentType>(user_topo_id, injection);
};

template <typename ComponentSolverOutputType>
    requires symmetry_tag<decode_symmetry_v<ComponentSolverOutputType>> &&
             (std::same_as<ComponentSolverOutputType,
                           ApplianceSolverOutput<decode_symmetry_v<ComponentSolverOutputType>>> ||
              std::same_as<ComponentSolverOutputType,
                           ApplianceShortCircuitSolverOutput<decode_symmetry_v<ComponentSolverOutputType>>> ||
              std::same_as<ComponentSolverOutputType,
                           FaultShortCircuitSolverOutput<decode_symmetry_v<ComponentSolverOutputType>>>)
inline auto const& get_injection(ComponentSolverOutputType const& component_output) {
    if constexpr (requires { component_output.s; }) {
        return component_output.s;
    } else if constexpr (requires { component_output.i; }) {
        return component_output.i;
    } else if constexpr (requires { component_output.i_fault; }) {
        return component_output.i_fault;
    } else {
        static_assert(false, "ComponentSolverOutputType must have either s or i member");
    }
}

template <typename BranchSolverOutputType>
    requires symmetry_tag<decode_symmetry_v<BranchSolverOutputType>> &&
             (std::same_as<BranchSolverOutputType, BranchSolverOutput<decode_symmetry_v<BranchSolverOutputType>>> ||
              std::same_as<BranchSolverOutputType,
                           BranchShortCircuitSolverOutput<decode_symmetry_v<BranchSolverOutputType>>>)
inline auto get_injection(BranchSolverOutputType const& branch, BranchSide side) {
    if constexpr (requires { branch.s_f; }) {
        return -1.0 * (side == BranchSide::from ? branch.s_f : branch.s_t);
    } else if constexpr (requires { branch.i_f; }) {
        return -1.0 * (side == BranchSide::from ? branch.i_f : branch.i_t);
    } else {
        static_assert(false, "BranchSolverOutputType must have either s_f/s_t or i_f/i_t member");
    }
}

template <typename ComponentType>
inline Idx get_node_sequence_idx(main_model_state_c auto const& state, Idx component_idx) {
    if constexpr (std::same_as<ComponentType, Source>) {
        return state.comp_topo
            ->source_node_idx[get_component_sequence_offset<Source, ComponentType>(state.components) + component_idx];
    } else if constexpr (std::derived_from<ComponentType, GenericLoadGen>) {
        return state.comp_topo
            ->load_gen_node_idx[get_component_sequence_offset<GenericLoadGen, ComponentType>(state.components) +
                                component_idx];
    } else if constexpr (std::same_as<ComponentType, Fault>) {
        auto const& fault = get_component_by_sequence<Fault>(state.components, component_idx);
        return get_component_sequence_idx<Node>(state.components, fault.get_fault_object());
    }

    else {
        static_assert(false, "Unsupported component type for node sequence index retrieval");
    }
}

template <std::derived_from<Branch> ComponentType>
inline Idx get_node_sequence_idx(main_model_state_c auto const& state, Idx component_idx, BranchSide side) {
    return state.comp_topo->branch_node_idx[get_component_sequence_offset<Branch, ComponentType>(state.components) +
                                            component_idx][std::to_underlying(side)];
}

template <typename ComponentType, typename SolverOutputType, typename AddToTarget>
    requires flow_accumulator_c<AddToTarget, ComponentType, SolverOutputType> &&
             (std::same_as<ComponentType, Source> || std::same_as<ComponentType, SymLoad> ||
              std::same_as<ComponentType, SymGenerator> || std::same_as<ComponentType, AsymLoad> ||
              std::same_as<ComponentType, AsymGenerator> ||
              (std::same_as<ComponentType, Fault> && short_circuit_solver_output_type<SolverOutputType>))
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
        unwrap_callable(accumulate_injection)
            .template operator()<ComponentType>(user_topo_id, get_injection(component_output));
    }
}

template <typename ComponentType, typename SolverOutputType, typename AddToTarget>
    requires flow_accumulator_c<AddToTarget, ComponentType, SolverOutputType> &&
             std::derived_from<ComponentType, Branch>
inline void add_appliance_injection(main_model_state_c auto const& state,
                                    MathOutput<std::vector<SolverOutputType>> const& math_output,
                                    AddToTarget accumulate_injection) {
    static_assert(!std::same_as<ComponentType, Link>);

    for (auto const& [component_idx, component_math_id] : enumerate(comp_base_sequence<ComponentType>(state))) {
        if (component_math_id.group == disconnected || component_math_id.pos == disconnected) {
            continue;
        }
        auto const& component_output = get_component_output<ComponentType>(math_output, component_math_id);

        for (auto const side : {BranchSide::from, BranchSide::to}) {
            auto const& user_node_idx = get_node_sequence_idx<ComponentType>(state, component_idx, side);
            auto const& user_topo_id =
                state.reduced_topology->topo_node_coup.coupling.user_nodes_to_topo_nodes[user_node_idx];
            unwrap_callable(accumulate_injection)
                .template operator()<ComponentType>(user_topo_id, get_injection(component_output, side));
        }
    }
}

template <typename InjectionComponentTypesTuple, main_model_state_c State, solver_output_type SolverOutput,
          typename AddToTarget>
inline void add_flows(State const& state, MathOutput<std::vector<SolverOutput>> const& math_output,
                      AddToTarget accumulate_injection) {
    utils::run_functor_with_tuple_return_void<InjectionComponentTypesTuple>(
        [&state, &math_output, &accumulate_injection]<typename ComponentType>() {
            if constexpr (decltype(state.components)::template is_storageable_v<ComponentType>) {
                add_appliance_injection<ComponentType>(state, math_output, accumulate_injection);
            }
        });
}

template <typename LinkSolver, main_model_state_c State, solver_output_type SolverOutput>
    requires std::invocable<LinkSolver, std::vector<BranchIdx>, std::vector<DoubleComplex>>
inline std::vector<SupernodeOutput<SolverOutput>>
solve_topological_nodes(LinkSolver link_solver, State const& state,
                        MathOutput<std::vector<SolverOutput>> const& math_output) {
    using sym = decode_symmetry_v<SolverOutput>;

    (void)link_solver; // suppress unused variable warning until link_solver is used for link output

    std::vector<detail::SuperNodeSolverInput<sym>> link_solver_input =
        state.reduced_topology->topo_node_coup.topo_nodes |
        std::views::transform([](auto const& topo_node) -> detail::SuperNodeSolverInput<sym> {
            return {.bus_injection = ComplexValueVector<sym>(topo_node.user_nodes.size()),
                    .bus_flow_from_branch = std::vector<ComplexValue<sym>>(topo_node.user_nodes.size())};
        }) |
        std::ranges::to<std::vector>();

    auto const accumulate_injection = [&link_solver_input]<typename ComponentType>(Idx2D const& user_topo_id,
                                                                                   ComplexValue<sym> const& injection) {
        if constexpr (std::derived_from<ComponentType, Branch>) {
            link_solver_input[user_topo_id.group].bus_flow_from_branch[user_topo_id.pos] += injection;
        } else {
            link_solver_input[user_topo_id.group].bus_injection[user_topo_id.pos] += injection;
        }
    };

    if constexpr (steady_state_solver_output_type<SolverOutput>) {
        using InjectionComponentTypesTuple =
            std::tuple<Source, SymLoad, SymGenerator, AsymLoad, AsymGenerator, Line, GenericBranch, Transformer>;
        add_flows<InjectionComponentTypesTuple>(state, math_output, accumulate_injection);
    } else if constexpr (short_circuit_solver_output_type<SolverOutput>) {
        using InjectionComponentTypesTuple = std::tuple<Source, Line, GenericBranch, Transformer, Fault>;
        add_flows<InjectionComponentTypesTuple>(state, math_output, accumulate_injection);
    }

    std::vector<SupernodeOutput<SolverOutput>> supernode_output =
        link_solver_input | std::views::transform([](auto const& super_node_solver_input) {
            (void)super_node_solver_input; // suppress unused variable warning until link_solver is used for link output
            if constexpr (steady_state_solver_output_type<SolverOutput>) {
                return SupernodeOutput<SolverOutput>{.bus_injection = super_node_solver_input.bus_injection,
                                                     .branch = {}};
            } else if constexpr (short_circuit_solver_output_type<SolverOutput>) {
                return SupernodeOutput<SolverOutput>{.branch = {}};
            }
        }) |
        std::ranges::to<std::vector>();
    return supernode_output;
}
} // namespace detail

template <main_model_state_c State, typename SolverOutput>
inline void solve_topological_nodes(State const& state, MathOutput<std::vector<SolverOutput>>& math_output) {
    assert(std::ranges::empty(math_output.supernode_output));
    math_output.supernode_output =
        detail::solve_topological_nodes(link_solver::compute_loads_link_elements, state, math_output);
}
} // namespace power_grid_model::main_core
