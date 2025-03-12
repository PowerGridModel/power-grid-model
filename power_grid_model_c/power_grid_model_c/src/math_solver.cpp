// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS

#include <power_grid_model/math_solver/newton_raphson_pf_solver.hpp>

namespace power_grid_model::math_solver {

namespace newton_raphson_pf {
template class NewtonRaphsonPFSolver<symmetric_t>;
template class NewtonRaphsonPFSolver<asymmetric_t>;
} // namespace newton_raphson_pf

} // namespace power_grid_model::math_solver
