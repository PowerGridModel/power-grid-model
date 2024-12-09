// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_math_solver_pf.hpp"

#include <power_grid_model/math_solver/newton_raphson_pf_solver.hpp>

#include <doctest/doctest.h>

TYPE_TO_STRING_AS("NewtonRaphsonPFSolver<symmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonPFSolver<power_grid_model::symmetric_t>);
TYPE_TO_STRING_AS("NewtonRaphsonPFSolver<asymmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonPFSolver<power_grid_model::asymmetric_t>);

namespace power_grid_model::math_solver {
namespace {
using newton_raphson_pf::PFJacBlock;
} // namespace

TEST_CASE("Test block") {
    SUBCASE("symmetric") {
        PFJacBlock<symmetric_t> b{};
        b.h() += 1.0;
        b.n() += 2.0;
        b.m() += 3.0;
        b.l() += 4.0;
        CHECK(b.h() == 1.0);
        CHECK(b.n() == 2.0);
        CHECK(b.m() == 3.0);
        CHECK(b.l() == 4.0);
    }

    SUBCASE("Asymmetric") {
        PFJacBlock<asymmetric_t> b{};
        RealTensor<asymmetric_t> const h{1.0};
        RealTensor<asymmetric_t> const n{2.0};
        RealTensor<asymmetric_t> const m{3.0};
        RealTensor<asymmetric_t> const l{4.0};
        b.h() += h;
        b.n() += n;
        b.m() += m;
        b.l() += l;
        check_close<asymmetric_t>(b.h(), h, numerical_tolerance);
        check_close<asymmetric_t>(b.n(), n, numerical_tolerance);
        check_close<asymmetric_t>(b.m(), m, numerical_tolerance);
        check_close<asymmetric_t>(b.l(), l, numerical_tolerance);
    }
}

TEST_CASE_TEMPLATE_INVOKE(test_math_solver_pf_id, NewtonRaphsonPFSolver<symmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_pf_id, NewtonRaphsonPFSolver<asymmetric_t>);

} // namespace power_grid_model::math_solver
