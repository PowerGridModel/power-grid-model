// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS

#include "math_solver.hpp"

#include <power_grid_model/math_solver/math_solver.hpp>

namespace power_grid_model {

MathSolverDispatcher const& get_math_solver_dispatcher() {
    static constexpr MathSolverDispatcher math_solver_dispatcher{math_solver::math_solver_tag<MathSolver>{}};
    return math_solver_dispatcher;
}

} // namespace power_grid_model
