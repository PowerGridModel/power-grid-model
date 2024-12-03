// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_math_solver_se.hpp"

#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>

#include <doctest/doctest.h>

TYPE_TO_STRING_AS("IterativeLinearSESolver<symmetric_t>",
                  power_grid_model::math_solver::IterativeLinearSESolver<power_grid_model::symmetric_t>);
TYPE_TO_STRING_AS("IterativeLinearSESolver<asymmetric_t>",
                  power_grid_model::math_solver::IterativeLinearSESolver<power_grid_model::asymmetric_t>);

namespace power_grid_model::math_solver {
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_id, IterativeLinearSESolver<symmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_id, IterativeLinearSESolver<asymmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_zero_variance_id, IterativeLinearSESolver<symmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_measurements_id, IterativeLinearSESolver<symmetric_t>);
} // namespace power_grid_model::math_solver
