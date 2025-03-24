// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_math_solver_pf.hpp"

#include <power_grid_model/math_solver/linear_pf_solver.hpp>

#include <doctest/doctest.h>

TYPE_TO_STRING_AS("LinearPFSolver<symmetric_t>",
                  power_grid_model::math_solver::LinearPFSolver<power_grid_model::symmetric_t>);
TYPE_TO_STRING_AS("LinearPFSolver<asymmetric_t>",
                  power_grid_model::math_solver::LinearPFSolver<power_grid_model::asymmetric_t>);

namespace power_grid_model::math_solver {
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_pf_id, LinearPFSolver<symmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_pf_id, LinearPFSolver<asymmetric_t>);
} // namespace power_grid_model::math_solver
