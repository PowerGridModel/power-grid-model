// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS

#include <power_grid_model/math_solver/iterative_current_pf_solver.hpp>
#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_pf_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>

namespace power_grid_model::math_solver {

namespace newton_raphson_pf {
template class NewtonRaphsonPFSolver<symmetric_t>;
template class NewtonRaphsonPFSolver<asymmetric_t>;
} // namespace newton_raphson_pf

namespace newton_raphson_se {
template class NewtonRaphsonSESolver<symmetric_t>;
template class NewtonRaphsonSESolver<asymmetric_t>;
} // namespace newton_raphson_se

namespace iterative_current_pf {
template class IterativeCurrentPFSolver<symmetric_t>;
template class IterativeCurrentPFSolver<asymmetric_t>;
} // namespace iterative_current_pf

namespace iterative_linear_se {
template class IterativeLinearSESolver<symmetric_t>;
template class IterativeLinearSESolver<asymmetric_t>;
} // namespace iterative_linear_se

} // namespace power_grid_model::math_solver
