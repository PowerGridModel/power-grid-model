// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>
#include <cassert>
#include <concepts>
#include <functional>
#include <type_traits>
#include <vector>

import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.typing;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.math_solver.math_solver_dispatch;
import power_grid_model.common.logging;
import power_grid_model.math_solver.y_bus;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/main_core/math_state.hpp>

export module power_grid_model.main_core.math_state;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::clear;
using power_grid_model::main_core::get_solvers;
using power_grid_model::main_core::get_y_bus;
using power_grid_model::main_core::MathState;
using power_grid_model::main_core::update_y_bus;
} // namespace power_grid_model::main_core
