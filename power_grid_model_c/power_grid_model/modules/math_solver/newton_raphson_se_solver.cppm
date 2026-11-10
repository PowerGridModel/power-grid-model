// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <cassert>
#include <complex>
#include <concepts>
#include <functional>
#include <limits>
#include <vector>

import power_grid_model.math_solver.block_matrix;
import power_grid_model.common.common;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.math_solver.common_solver_functions;
import power_grid_model.math_solver.measured_values;
import power_grid_model.calculation_parameters;
import power_grid_model.common.enums;
import power_grid_model.common.statistics;
import power_grid_model.common.exception;
import power_grid_model.math_solver.y_bus;
import power_grid_model.math_solver.observability;
import power_grid_model.math_solver.sparse_lu_solver;
import power_grid_model.common.logging;
import power_grid_model.common.timer;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.typing;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>

export module power_grid_model.math_solver.newton_raphson_se_solver;

export namespace power_grid_model::math_solver {
using power_grid_model::math_solver::NewtonRaphsonSESolver;
} // namespace power_grid_model::math_solver
