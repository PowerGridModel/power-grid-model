// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_math_solver_se.hpp"

#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>

#include <doctest/doctest.h>

TYPE_TO_STRING_AS("NewtonRaphsonSESolver<symmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonSESolver<power_grid_model::symmetric_t>);
TYPE_TO_STRING_AS("NewtonRaphsonSESolver<asymmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonSESolver<power_grid_model::asymmetric_t>);

namespace power_grid_model::math_solver {
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_id, NewtonRaphsonSESolver<symmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_id, NewtonRaphsonSESolver<asymmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_zero_variance_id, NewtonRaphsonSESolver<symmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_se_measurements_id, NewtonRaphsonSESolver<symmetric_t>);
} // namespace power_grid_model::math_solver
