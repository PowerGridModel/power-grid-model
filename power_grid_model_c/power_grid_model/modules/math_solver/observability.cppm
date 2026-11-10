// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

import power_grid_model.math_solver.measured_values;
import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.typing;
import power_grid_model.common.exception;
import power_grid_model.math_solver.y_bus;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/math_solver/observability.hpp>

export module power_grid_model.math_solver.observability;

export namespace power_grid_model::math_solver {
namespace detail {
using power_grid_model::math_solver::detail::assign_independent_sensors_radial;
using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;
using power_grid_model::math_solver::detail::complete_bidirectional_neighbourhood_info;
using power_grid_model::math_solver::detail::ConnectivityStatus;
using power_grid_model::math_solver::detail::find_spanning_tree_from_node;
using power_grid_model::math_solver::detail::necessary_condition;
using power_grid_model::math_solver::detail::ObservabilitySensorsResult;
using power_grid_model::math_solver::detail::prepare_starting_nodes;
using power_grid_model::math_solver::detail::scan_network_sensors;
using power_grid_model::math_solver::detail::sufficient_condition_meshed_without_voltage_phasor;
using power_grid_model::math_solver::detail::sufficient_condition_radial_with_voltage_phasor;
} // namespace detail

namespace observability {
using power_grid_model::math_solver::observability::observability_check;
using power_grid_model::math_solver::observability::ObservabilityResult;
} // namespace observability
} // namespace power_grid_model::math_solver
