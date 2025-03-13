// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>

namespace power_grid_model::math_solver::newton_raphson_se {
template class NewtonRaphsonSESolver<symmetric_t>;
template class NewtonRaphsonSESolver<asymmetric_t>;
} // namespace power_grid_model::math_solver::newton_raphson_se
