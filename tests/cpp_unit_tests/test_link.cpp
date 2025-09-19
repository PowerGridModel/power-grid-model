// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/link.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test link") {
    LinkInput input{.id = 1, .from_node = 2, .to_node = 3, .from_status = 1, .to_status = 1};
    Link link{input, 10e3, 50e3};
    Branch& branch = link;
    double const base_i_from = base_power_1p / (10.0e3 / sqrt3);
    double const base_i_to = base_power_1p / (50.0e3 / sqrt3);
    DoubleComplex const u1f = 1.0;
    DoubleComplex const u1t = 0.9;
    ComplexValue<asymmetric_t> const uaf{1.0};
    ComplexValue<asymmetric_t> const uat{0.9};
    DoubleComplex const i1f = (u1f - u1t) * y_link * base_i_from;
    DoubleComplex const i1t = (u1t - u1f) * y_link * base_i_to;
    DoubleComplex const s_f = conj(i1f) * u1f * 10e3 * sqrt3;
    DoubleComplex const s_t = conj(i1t) * u1t * 50e3 * sqrt3;

    // Short circuit results
    DoubleComplex const if_sc{1.0, 1.0};
    DoubleComplex const it_sc{2.0, 2.0 * sqrt3};
    ComplexValue<asymmetric_t> const if_sc_asym{1.0 + 1.0i};
    ComplexValue<asymmetric_t> const it_sc_asym{2.0 + (2.0i * sqrt3)};

    CHECK(link.math_model_type() == ComponentType::branch);

    SUBCASE("General") {
        CHECK(branch.status(BranchSide::from) == branch.from_status());
        CHECK(branch.status(BranchSide::to) == branch.to_status());
        CHECK(branch.base_i_from() == doctest::Approx(base_i_from));
        CHECK(branch.base_i_to() == doctest::Approx(base_i_to));
        CHECK(!branch.is_param_mutable());
        CHECK(branch.phase_shift() == 0.0);
    }

    SUBCASE("Invalid branch") {
        input.to_node = 2;
        CHECK_THROWS_AS(Link(input, 10e3, 50e3), InvalidBranch);
    }

    SUBCASE("Symmetric parameters") {
        // double connected
        BranchCalcParam<symmetric_t> param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - y_link) < numerical_tolerance);
        CHECK(cabs(param.ytt() - y_link) < numerical_tolerance);
        CHECK(cabs(param.ytf() + y_link) < numerical_tolerance);
        CHECK(cabs(param.yft() + y_link) < numerical_tolerance);
        // single connected
        CHECK(branch.set_status(false, na_IntS));
        param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
    }

    SUBCASE("Symmetric results") {
        BranchOutput<symmetric_t> const output = branch.get_output<symmetric_t>(1.0, 0.9);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == 0.0);
        CHECK(output.i_from == doctest::Approx(cabs(i1f)));
        CHECK(output.i_to == doctest::Approx(cabs(i1t)));
        CHECK(output.s_from == doctest::Approx(cabs(s_f)));
        CHECK(output.s_to == doctest::Approx(cabs(s_t)));
        CHECK(output.p_from == doctest::Approx(real(s_f)));
        CHECK(output.p_to == doctest::Approx(real(s_t)));
        CHECK(output.q_from == doctest::Approx(imag(s_f)));
        CHECK(output.q_to == doctest::Approx(imag(s_t)));
    }

    SUBCASE("Asymmetric results") {
        BranchOutput<asymmetric_t> const output = branch.get_output<asymmetric_t>(uaf, uat);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == 0.0);
        CHECK(output.i_from(0) == doctest::Approx(cabs(i1f)));
        CHECK(output.i_to(1) == doctest::Approx(cabs(i1t)));
        CHECK(output.s_from(2) == doctest::Approx(cabs(s_f) / 3.0));
        CHECK(output.s_to(0) == doctest::Approx(cabs(s_t) / 3.0));
        CHECK(output.p_from(1) == doctest::Approx(real(s_f) / 3.0));
        CHECK(output.p_to(2) == doctest::Approx(real(s_t) / 3.0));
        CHECK(output.q_from(0) == doctest::Approx(imag(s_f) / 3.0));
        CHECK(output.q_to(1) == doctest::Approx(imag(s_t) / 3.0));
    }

    SUBCASE("Short circuit asym results") {
        BranchShortCircuitOutput const asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
        CHECK(asym_output.id == 1);
        CHECK(asym_output.energized);
        CHECK(asym_output.i_from(0) == doctest::Approx(cabs(if_sc) * base_i_from));
        CHECK(asym_output.i_to(1) == doctest::Approx(cabs(it_sc) * base_i_to));
        CHECK(asym_output.i_from_angle(1) == doctest::Approx(pi / 4 - 2 * pi / 3));
        CHECK(asym_output.i_to_angle(2) == doctest::Approx(pi));
    }

    SUBCASE("Short circuit sym results") {
        BranchShortCircuitOutput const sym_output = branch.get_sc_output(if_sc, it_sc);
        BranchShortCircuitOutput const asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
        CHECK(sym_output.id == asym_output.id);
        CHECK(sym_output.energized == asym_output.energized);
        CHECK(sym_output.i_from(0) == doctest::Approx(asym_output.i_from(0)));
        CHECK(sym_output.i_to(1) == doctest::Approx(asym_output.i_to(1)));
        CHECK(sym_output.i_from_angle(1) == doctest::Approx(asym_output.i_from_angle(1)));
        CHECK(sym_output.i_to_angle(2) == doctest::Approx(asym_output.i_to_angle(2)));
    }

    SUBCASE("Update inverse") {
        BranchUpdate branch_update{.id = 1, .from_status = na_IntS, .to_status = na_IntS};
        auto expected = branch_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("From status") {
            SUBCASE("same") { branch_update.from_status = status_to_int(link.from_status()); }
            SUBCASE("different") { branch_update.from_status = IntS{0}; }
            expected.from_status = status_to_int(link.from_status());
        }

        SUBCASE("To status") {
            SUBCASE("same") { branch_update.to_status = status_to_int(link.to_status()); }
            SUBCASE("different") { branch_update.to_status = IntS{0}; }
            expected.to_status = status_to_int(link.to_status());
        }

        SUBCASE("multiple") {
            branch_update.from_status = IntS{0};
            branch_update.to_status = IntS{0};
            expected.from_status = status_to_int(link.from_status());
            expected.to_status = status_to_int(link.to_status());
        }

        auto const inv = link.inverse(branch_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.from_status == expected.from_status);
        CHECK(inv.to_status == expected.to_status);
    }
}

} // namespace power_grid_model
