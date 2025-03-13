// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>

namespace power_grid_model::math_solver::iterative_linear_se {
template class IterativeLinearSESolver<symmetric_t>;
template class IterativeLinearSESolver<asymmetric_t>;
} // namespace power_grid_model::math_solver::iterative_linear_se
