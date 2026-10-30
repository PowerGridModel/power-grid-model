// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/main_core/state_queries.hpp>

export module power_grid_model.main_core.state_queries;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::comp_base_sequence;
using power_grid_model::main_core::comp_base_sequence_cbegin;
using power_grid_model::main_core::get_branch_nodes;
using power_grid_model::main_core::get_math_id;
using power_grid_model::main_core::get_topo_node;
using power_grid_model::main_core::get_user_node_math_id;
} // namespace power_grid_model::main_core
