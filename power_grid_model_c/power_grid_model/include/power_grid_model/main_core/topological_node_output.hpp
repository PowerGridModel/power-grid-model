// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "core_utils.hpp"

#include "../common/common.hpp"

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
    std::span<BranchIdx const> links;
    UserNodeValueVector<sym> node_injection;
    UserNodeValueVector<sym> node_flow_from_branch;

    ComplexValueVector<sym> get_total_injection_per_node() const {
        assert(node_injection.size() == node_flow_from_branch.size());

        return std::views::zip(node_injection, node_flow_from_branch) | std::views::transform([](auto const& pair) {
                   auto const& [node_inj, branch_flow] = pair;
                   return ComplexValue<sym>(node_inj + branch_flow);
               }) |
               std::ranges::to<ComplexValueVector<sym>>();
    }
};

template <typename AddToTarget, typename ComponentType, typename SolverOutputType>
concept flow_accumulator_c = requires(AddToTarget accumulator, Idx2D const& user_topo_id,
                                      ComplexValue<decode_symmetry_v<SolverOutputType>> const& injection) {
    accumulator.template operator()<ComponentType>(user_topo_id, injection);
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
    } else {
        static_assert(false, "Unsupported component type for node sequence index retrieval");
    }
}

template <std::derived_from<Branch> ComponentType>
inline BranchIdx const& get_branch_sequence_idx(main_model_state_c auto const& state, Idx component_idx) {
    return state.comp_topo
        ->branch_node_idx[get_component_sequence_offset<Branch, ComponentType>(state.components) + component_idx];
}

struct AddApplianceInjection {
    template <typename ComponentType, typename SolverOutputType, typename AddToTarget>
        requires flow_accumulator_c<AddToTarget, ComponentType, SolverOutputType> &&
                 (is_in_list_c<ComponentType, Source, SymLoad, SymGenerator, AsymLoad, AsymGenerator> ||
                  (std::same_as<ComponentType, Fault> && short_circuit_solver_output_type<SolverOutputType>))
    void operator()(main_model_state_c auto const& state, MathOutput<std::vector<SolverOutputType>> const& math_output,
                    AddToTarget accumulate_injection) const {
        for (auto const& [component_idx, component_math_id] : enumerate(comp_base_sequence<ComponentType>(state))) {
            if (component_math_id.group == disconnected) {
                continue;
            }
            auto const& component_output = get_component_output<ComponentType>(math_output, component_math_id);

            auto const& user_node_idx = get_node_sequence_idx<ComponentType>(state, component_idx);
            auto const& user_topo_id =
                state.reduced_topology->topo_node_coup.coupling.user_nodes_to_topo_nodes[user_node_idx];
            accumulate_injection.template operator()<ComponentType>(user_topo_id, get_injection(component_output));
        }
    }

    template <typename ComponentType, typename SolverOutputType, typename AddToTarget>
        requires flow_accumulator_c<AddToTarget, ComponentType, SolverOutputType> &&
                 std::derived_from<ComponentType, Branch>
    void operator()(main_model_state_c auto const& state, MathOutput<std::vector<SolverOutputType>> const& math_output,
                    AddToTarget accumulate_injection) const {
        static_assert(!std::same_as<ComponentType, Link>);

        for (auto const& [component_idx, component_math_id] : enumerate(comp_base_sequence<ComponentType>(state))) {
            if (component_math_id.group == disconnected || component_math_id.pos == disconnected) {
                continue;
            }
            auto const& component_output = get_component_output<ComponentType>(math_output, component_math_id);

            auto const& branch_node_idx = get_branch_sequence_idx<ComponentType>(state, component_idx);
            for (auto const side : {BranchSide::from, BranchSide::to}) {
                auto const& user_node_idx = branch_node_idx[std::to_underlying(side)];
                auto const& user_topo_id =
                    state.reduced_topology->topo_node_coup.coupling.user_nodes_to_topo_nodes[user_node_idx];
                accumulate_injection.template operator()<ComponentType>(user_topo_id,
                                                                        get_injection(component_output, side));
            }
        }
    }
};

constexpr auto add_appliance_injection = AddApplianceInjection{};

template <typename InjectionComponentTypesTuple, main_model_state_c State, solver_output_type SolverOutput,
          typename AddToTarget>
inline void add_flows(State const& state, MathOutput<std::vector<SolverOutput>> const& math_output,
                      AddToTarget accumulate_injection) {
    utils::run_functor_with_tuple_return_void<InjectionComponentTypesTuple>(
        [&state, &math_output, &accumulate_injection]<typename ComponentType>() {
            if constexpr (decltype(state.components)::template is_storageable_v<ComponentType>) {
                add_appliance_injection.template operator()<ComponentType>(state, math_output, accumulate_injection);
            }
        });
}

template <symmetry_tag sym, typename LinkSolver>
    requires std::invocable<LinkSolver, std::vector<BranchIdx>, ComplexVector>
ComplexValueVector<sym> compute_link_solver(LinkSolver link_solver,
                                            SuperNodeSolverInput<sym> const& super_node_solver_input) {
    if constexpr (is_symmetric_v<sym>) {
        return link_solver(super_node_solver_input.links | std::ranges::to<std::vector>(),
                           super_node_solver_input.get_total_injection_per_node());
    } else {
        auto constexpr phase_number = Idx{3};
        auto const injection_per_node = super_node_solver_input.get_total_injection_per_node();
        auto const node_number = injection_per_node.size();
        std::array<ComplexVector, phase_number> injection_per_phase;

        std::ranges::for_each(injection_per_phase, [node_number](auto& injection) { injection.reserve(node_number); });

        for (auto const& node_injection : injection_per_node) {
            for (Idx const phase : IdxRange{phase_number}) {
                injection_per_phase[phase].emplace_back(node_injection(phase));
            }
        }

        auto const links = super_node_solver_input.links | std::ranges::to<std::vector>();
        auto result = ComplexValueVector<asymmetric_t>(links.size());
        for (Idx const phase : IdxRange{phase_number}) {
            auto const phase_result = link_solver(links, injection_per_phase[phase]);
            assert(phase_result.size() == result.size());

            for (auto&& [node_result, node_phase_value] : std::views::zip(result, phase_result)) {
                node_result(phase) = node_phase_value;
            }
        }
        return result;
    }
}

template <symmetry_tag sym, typename BranchSolverOutputType>
    requires symmetry_tag<decode_symmetry_v<BranchSolverOutputType>> &&
             (std::same_as<BranchSolverOutputType, BranchSolverOutput<decode_symmetry_v<BranchSolverOutputType>>> ||
              std::same_as<BranchSolverOutputType,
                           BranchShortCircuitSolverOutput<decode_symmetry_v<BranchSolverOutputType>>>)
std::vector<BranchSolverOutputType> get_link_output(ComplexValueVector<sym> const& link_solver_result) {
    std::vector<BranchSolverOutputType> link_output;
    link_output.reserve(link_solver_result.size());

    for (auto const& result : link_solver_result) {
        if constexpr (std::same_as<BranchSolverOutputType,
                                   BranchSolverOutput<decode_symmetry_v<BranchSolverOutputType>>>) {
            link_output.emplace_back(BranchSolverOutputType{.s_f = result, .s_t = -result});
        } else {
            link_output.emplace_back(BranchSolverOutputType{.i_f = result, .i_t = -result});
        }
    }
    return link_output;
}

template <typename LinkSolver, main_model_state_c State, solver_output_type SolverOutput>
    requires std::invocable<LinkSolver, std::vector<BranchIdx>, ComplexVector>
inline std::vector<SupernodeOutput<SolverOutput>>
solve_topological_nodes(LinkSolver link_solver, State const& state,
                        MathOutput<std::vector<SolverOutput>> const& math_output) {
    using sym = decode_symmetry_v<SolverOutput>;

    std::vector<SuperNodeSolverInput<sym>> link_solver_input =
        state.reduced_topology->topo_node_coup.topo_nodes |
        std::views::transform([](auto const& topo_node) -> SuperNodeSolverInput<sym> {
            auto const node_number = topo_node.user_nodes.size();
            return {.links = std::span{topo_node.user_links},
                    .node_injection = UserNodeValueVector<sym>(node_number),
                    .node_flow_from_branch = UserNodeValueVector<sym>(node_number)};
        }) |
        std::ranges::to<std::vector>();

    auto const accumulate_injection = [&link_solver_input]<typename ComponentType>(Idx2D const& user_topo_id,
                                                                                   ComplexValue<sym> const& injection) {
        if constexpr (std::derived_from<ComponentType, Branch>) {
            link_solver_input[user_topo_id.group].node_flow_from_branch[user_topo_id.pos] += injection;
        } else {
            link_solver_input[user_topo_id.group].node_injection[user_topo_id.pos] += injection;
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

    auto result =
        link_solver_input |
        std::views::transform([link_solver](auto const& super_node_solver_input) -> SupernodeOutput<SolverOutput> {
            if constexpr (steady_state_solver_output_type<SolverOutput>) {
                return SupernodeOutput<SolverOutput>{
                    .bus_injection = super_node_solver_input.node_injection,
                    .link = get_link_output<sym, BranchSolverOutput<sym>>(
                        compute_link_solver<sym>(link_solver, super_node_solver_input))};
            } else if constexpr (short_circuit_solver_output_type<SolverOutput>) {
                return SupernodeOutput<SolverOutput>{
                    .link = get_link_output<sym, BranchShortCircuitSolverOutput<sym>>(
                        compute_link_solver<sym>(link_solver, super_node_solver_input))};
            }
        }) |
        std::ranges::to<std::vector>();

    // TODO(mgovers): cleanup v2: solver output should be in current domain for all solvers; offload power output to
    // main_core/output.hpp branch output conversion function
    if constexpr (steady_state_solver_output_type<SolverOutput>) {
        std::ranges::for_each(std::views::zip(result, state.topo_comp_coup->node), [&math_output](auto&& pair) {
            auto& [supernode_output, topo_node_idx] = pair;
            if (topo_node_idx.group == disconnected || topo_node_idx.pos == disconnected) {
                return;
            }
            ComplexValue<sym> const topo_node_u_inv =
                ComplexValue<sym>{1.0} / math_output.solver_output[topo_node_idx.group].u[topo_node_idx.pos];
            std::ranges::for_each(supernode_output.link, [&topo_node_u_inv](auto& link) {
                link.i_f = conj(link.s_f * topo_node_u_inv);
                link.i_t = conj(link.s_t * topo_node_u_inv);
            });
        });
    }

    return result;
}
} // namespace detail

template <main_model_state_c State, typename SolverOutput>
inline void solve_topological_nodes(State const& state, MathOutput<std::vector<SolverOutput>>& math_output) {
    assert(std::ranges::empty(math_output.supernode_output));
    math_output.supernode_output =
        detail::solve_topological_nodes(link_solver::compute_loads_link_elements, state, math_output);
}
} // namespace power_grid_model::main_core
