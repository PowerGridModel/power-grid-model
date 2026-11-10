// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>
#include <ranges>
#include <type_traits>

import power_grid_model.main_core.core_utils;
import power_grid_model.common.common;
import power_grid_model.calculation_parameters;
import power_grid_model.common.enums;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.link_solver;
import power_grid_model.component.branch;
import power_grid_model.component.branch3;
import power_grid_model.component.fault;
import power_grid_model.component.load_gen;
import power_grid_model.component.appliance;
import power_grid_model.component.shunt;
import power_grid_model.component.source;
import power_grid_model.main_core.state;
import power_grid_model.component.link;
import power_grid_model.component.node;
import power_grid_model.main_core.math_output_queries;
import power_grid_model.main_core.state_queries;
import power_grid_model.all_components;
import power_grid_model.auxiliary.input;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.common.component_list;
import power_grid_model.common.exception;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.matrix_utils;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.typing;
import power_grid_model.component.asym_line;
import power_grid_model.component.base;
import power_grid_model.component.component;
import power_grid_model.component.current_sensor;
import power_grid_model.component.edge;
import power_grid_model.component.generic_branch;
import power_grid_model.component.line;
import power_grid_model.component.line_utils;
import power_grid_model.component.power_sensor;
import power_grid_model.component.regulator;
import power_grid_model.component.sensor;
import power_grid_model.component.three_winding_transformer;
import power_grid_model.component.transformer;
import power_grid_model.component.transformer_tap_regulator;
import power_grid_model.component.transformer_utils;
import power_grid_model.component.voltage_regulator;
import power_grid_model.component.voltage_sensor;
import power_grid_model.container_fwd;
import power_grid_model.main_core.container_queries;

#define PGM_CXX_MODULE_PURVIEW
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
