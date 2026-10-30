// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/main_core/container_queries.hpp>

export module power_grid_model.main_core.container_queries;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::emplace_component;
using power_grid_model::main_core::get_component;
using power_grid_model::main_core::get_component_by_sequence;
using power_grid_model::main_core::get_component_citer;
using power_grid_model::main_core::get_component_group_idx;
using power_grid_model::main_core::get_component_idx_by_id;
using power_grid_model::main_core::get_component_sequence_idx;
using power_grid_model::main_core::get_component_sequence_offset;
using power_grid_model::main_core::get_component_size;
using power_grid_model::main_core::get_component_type_index;
using power_grid_model::main_core::get_topology_index;
using power_grid_model::main_core::reserve_component;
} // namespace power_grid_model::main_core
