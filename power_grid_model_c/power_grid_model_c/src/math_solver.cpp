// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS

#ifdef PGM_USE_CXX_MODULES
// Workaround: Clang crashes when instantiating some Eigen templates that are only reachable through imported modules.
// Making Eigen textually available before any import avoids this.
#include <Eigen/Core>
#include <Eigen/Dense>
#endif

#include "math_solver.hpp"

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.math_solver.math_solver;
import power_grid_model.math_solver.math_solver_dispatch;
#else
#include <power_grid_model/math_solver/math_solver.hpp>
#include <power_grid_model/math_solver/math_solver_dispatch.hpp>
#endif

namespace power_grid_model_c {
using namespace power_grid_model;

MathSolverDispatcher const& get_math_solver_dispatcher() {
    static constexpr MathSolverDispatcher math_solver_dispatcher{math_solver::math_solver_tag<MathSolver>{}};
    return math_solver_dispatcher;
}
} // namespace power_grid_model_c
