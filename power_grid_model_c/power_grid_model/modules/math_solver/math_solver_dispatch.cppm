// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/math_solver/math_solver_dispatch.hpp>

export module power_grid_model.math_solver.math_solver_dispatch;

export namespace power_grid_model {
using power_grid_model::MathSolverDispatcher;
using power_grid_model::MathSolverProxy;

namespace math_solver {
using power_grid_model::math_solver::math_solver_tag;
} // namespace math_solver
} // namespace power_grid_model
