// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/linear_pf_solver.hpp>

namespace power_grid_model::math_solver::linear_pf {
template class LinearPFSolver<symmetric_t>;
template class LinearPFSolver<asymmetric_t>;
} // namespace power_grid_model::math_solver::linear_pf
