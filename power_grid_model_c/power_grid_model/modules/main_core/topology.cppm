// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Dense>
#include <concepts>
#include <type_traits>
#include <vector>

import power_grid_model.main_core.core_utils;
import power_grid_model.common.common;
import power_grid_model.calculation_parameters;
import power_grid_model.common.enums;
import power_grid_model.component.branch;
import power_grid_model.component.edge;
import power_grid_model.component.branch3;
import power_grid_model.component.current_sensor;
import power_grid_model.component.link;
import power_grid_model.component.load_gen;
import power_grid_model.component.node;
import power_grid_model.component.power_sensor;
import power_grid_model.component.regulator;
import power_grid_model.component.shunt;
import power_grid_model.component.source;
import power_grid_model.component.voltage_sensor;
import power_grid_model.container_fwd;
import power_grid_model.common.exception;
import power_grid_model.component.base;
import power_grid_model.main_core.container_queries;
import power_grid_model.auxiliary.input;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.typing;
import power_grid_model.component.appliance;
import power_grid_model.component.component;
import power_grid_model.component.sensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/main_core/topology.hpp>

export module power_grid_model.main_core.topology;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::construct_components_connections;
using power_grid_model::main_core::construct_topology;
} // namespace power_grid_model::main_core
