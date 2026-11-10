// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>

import power_grid_model.math_solver.iterative_current_pf_solver;
import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.exception;
import power_grid_model.math_solver.y_bus;
import power_grid_model.common.logging;
import power_grid_model.common.timer;
import power_grid_model.math_solver.iterative_linear_se_solver;
import power_grid_model.math_solver.linear_pf_solver;
import power_grid_model.math_solver.math_solver_dispatch;
import power_grid_model.math_solver.newton_raphson_pf_solver;
import power_grid_model.math_solver.newton_raphson_se_solver;
import power_grid_model.math_solver.short_circuit_solver;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.typing;
import power_grid_model.math_solver.block_matrix;
import power_grid_model.math_solver.common_solver_functions;
import power_grid_model.math_solver.iterative_pf_solver;
import power_grid_model.math_solver.measured_values;
import power_grid_model.math_solver.observability;
import power_grid_model.math_solver.sparse_lu_solver;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/math_solver/math_solver.hpp>

export module power_grid_model.math_solver.math_solver;

export namespace power_grid_model {
using power_grid_model::MathSolver;
} // namespace power_grid_model
