// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <concepts>
#include <type_traits>

import power_grid_model.container_fwd;
import power_grid_model.component.base;
import power_grid_model.common.common;
import power_grid_model.component.branch;
import power_grid_model.component.branch3;
import power_grid_model.component.regulator;
import power_grid_model.component.shunt;
import power_grid_model.component.source;
import power_grid_model.component.component;
import power_grid_model.auxiliary.input;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.calculation_parameters;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.exception;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.typing;
import power_grid_model.component.appliance;
import power_grid_model.component.edge;

#define PGM_CXX_MODULE_PURVIEW
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
