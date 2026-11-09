// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>
#include <memory>
#include <type_traits>

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
import power_grid_model.common.logging;
import power_grid_model.math_solver.y_bus;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/math_solver/math_solver_dispatch.hpp>

export module power_grid_model.math_solver.math_solver_dispatch;

export namespace power_grid_model {
using power_grid_model::MathSolverDispatcher;
using power_grid_model::MathSolverProxy;

namespace math_solver {
using power_grid_model::math_solver::math_solver_tag;
} // namespace math_solver
} // namespace power_grid_model

export namespace power_grid_model::math_solver {
using power_grid_model::math_solver::MathSolverBase;
}
