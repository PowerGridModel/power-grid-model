// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/main_core/topological_node_output.hpp>

export module power_grid_model.main_core.topological_node_output;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::solve_topological_nodes;

namespace detail {
using power_grid_model::main_core::detail::add_appliance_injection;
using power_grid_model::main_core::detail::add_flows;
using power_grid_model::main_core::detail::compute_link_solver;
using power_grid_model::main_core::detail::ContributesToShortCircuitUserNodeInjection;
using power_grid_model::main_core::detail::ContributesToSteadyStateUserNodeInjection;
using power_grid_model::main_core::detail::get_branch3_sequence_idx;
using power_grid_model::main_core::detail::get_branch_sequence_idx;
using power_grid_model::main_core::detail::get_injection;
using power_grid_model::main_core::detail::get_link_output;
using power_grid_model::main_core::detail::get_node_sequence_idx;
using power_grid_model::main_core::detail::solve_topological_nodes;
using power_grid_model::main_core::detail::SuperNodeSolverInput;
using power_grid_model::main_core::detail::user_node_contribution_tag_t;
} // namespace detail
} // namespace power_grid_model::main_core

// boost::container::small_vector constructs its elements with a Boost-specific placement operator new. Clang resolves
// it from the global module fragment of the module providing the instantiated template (here: solve_topological_nodes),
// where it would otherwise be discarded because it is only named in dependent code. Using it here keeps it reachable.
namespace power_grid_model::detail::module_reachable {
inline void* topological_node_output_placement_new(void* p) { return ::new (p, boost_container_new_t()) int{}; }
} // namespace power_grid_model::detail::module_reachable
