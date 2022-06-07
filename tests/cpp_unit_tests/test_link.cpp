// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "catch2/catch_test_macros.hpp"
#include "power_grid_model/component/link.hpp"

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test link") {
    LinkInput input{{{1}, 2, 3, true, true}};
    Link link{input, 10e3, 50e3};
    Branch& branch = link;
    double const base_i_from = base_power_1p / (10.0e3 / sqrt3);
    double const base_i_to = base_power_1p / (50.0e3 / sqrt3);
    DoubleComplex const u1f = 1.0;
    DoubleComplex const u1t = 0.9;
    ComplexValue<false> const uaf{1.0};
    ComplexValue<false> const uat{0.9};
    DoubleComplex const i1f = (u1f - u1t) * y_link * base_i_from;
    DoubleComplex const i1t = (u1t - u1f) * y_link * base_i_to;
    DoubleComplex const s_f = conj(i1f) * u1f * 10e3 * sqrt3;
    DoubleComplex const s_t = conj(i1t) * u1t * 50e3 * sqrt3;

    CHECK(link.math_model_type() == ComponentType::branch);

    SECTION("General") {
        CHECK(branch.base_i_from() == Approx(base_i_from));
        CHECK(branch.base_i_to() == Approx(base_i_to));
        CHECK(!branch.is_param_mutable());
        CHECK(branch.phase_shift() == 0.0);
    }

    SECTION("Invalid branch") {
        input.to_node = 2;
        CHECK_THROWS_AS(Link(input, 10e3, 50e3), InvalidBranch);
    }

    SECTION("Symmetric parameters") {
        // double connected
        BranchCalcParam<true> param = branch.calc_param<true>();
        CHECK(cabs(param.yff() - y_link) < numerical_tolerance);
        CHECK(cabs(param.ytt() - y_link) < numerical_tolerance);
        CHECK(cabs(param.ytf() + y_link) < numerical_tolerance);
        CHECK(cabs(param.yft() + y_link) < numerical_tolerance);
        // single connected
        CHECK(branch.set_status(false, na_IntS));
        param = branch.calc_param<true>();
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
    }

    SECTION("Symmetric results") {
        BranchOutput<true> output = branch.get_output<true>(1.0, 0.9);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == 0.0);
        CHECK(output.i_from == Approx(cabs(i1f)));
        CHECK(output.i_to == Approx(cabs(i1t)));
        CHECK(output.s_from == Approx(cabs(s_f)));
        CHECK(output.s_to == Approx(cabs(s_t)));
        CHECK(output.p_from == Approx(real(s_f)));
        CHECK(output.p_to == Approx(real(s_t)));
        CHECK(output.q_from == Approx(imag(s_f)));
        CHECK(output.q_to == Approx(imag(s_t)));
    }

    SECTION("Asymmetric results") {
        BranchOutput<false> output = branch.get_output<false>(uaf, uat);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == 0.0);
        CHECK(output.i_from(0) == Approx(cabs(i1f)));
        CHECK(output.i_to(1) == Approx(cabs(i1t)));
        CHECK(output.s_from(2) == Approx(cabs(s_f) / 3.0));
        CHECK(output.s_to(0) == Approx(cabs(s_t) / 3.0));
        CHECK(output.p_from(1) == Approx(real(s_f) / 3.0));
        CHECK(output.p_to(2) == Approx(real(s_t) / 3.0));
        CHECK(output.q_from(0) == Approx(imag(s_f) / 3.0));
        CHECK(output.q_to(1) == Approx(imag(s_t) / 3.0));
    }
}

}  // namespace power_grid_model