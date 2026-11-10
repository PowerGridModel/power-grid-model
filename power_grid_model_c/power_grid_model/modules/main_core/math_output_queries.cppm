// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <cassert>
#include <concepts>
#include <vector>

import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.component.edge;
import power_grid_model.component.branch3;
import power_grid_model.component.fault;
import power_grid_model.component.load_gen;
import power_grid_model.component.shunt;
import power_grid_model.component.source;
import power_grid_model.component.branch;
import power_grid_model.auxiliary.input;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
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
import power_grid_model.component.base;
import power_grid_model.component.component;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/main_core/math_output_queries.hpp>

export module power_grid_model.main_core.math_output_queries;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::get_bus_injection_output_from_math_id;
using power_grid_model::main_core::get_bus_injection_output_from_topo_id;
using power_grid_model::main_core::get_component_output;
using power_grid_model::main_core::get_voltage_output;
} // namespace power_grid_model::main_core
