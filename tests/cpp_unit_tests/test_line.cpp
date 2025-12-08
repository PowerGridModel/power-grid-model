// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/line.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test line") {
    LineInput const input{.id = 1,
                          .from_node = 2,
                          .to_node = 3,
                          .from_status = 1,
                          .to_status = 1,
                          .r1 = 0.3,
                          .x1 = 0.4,
                          .c1 = 2e-4,
                          .tan1 = 0.1,
                          .r0 = 0.1,
                          .x0 = 0.2,
                          .c0 = 1e-4,
                          .tan0 = 0.2,
                          .i_n = 200.0};
    Line line{input, 50.0, 10.0e3, 10.0e3};
    double const base_i = base_power_1p / (10.0e3 / sqrt3);
    double const base_y = base_i * base_i / base_power_1p;
    Branch& branch = line;
    DoubleComplex const y1_series = 1.0 / (0.3 + 0.4i) / base_y;
    DoubleComplex const y1_shunt = (50.0 * 2 * pi * 2e-4) * (0.1 + 1.0i) / base_y;
    DoubleComplex const y0_series = 1.0 / (0.1 + 0.2i) / base_y;
    DoubleComplex const y0_shunt = (50.0 * 2 * pi * 1e-4) * (0.2 + 1.0i) / base_y;
    // symmetric
    DoubleComplex const yff1 = y1_series + 0.5 * y1_shunt;
    DoubleComplex const yft1 = -y1_series;
    DoubleComplex const ys1 = 0.5 * y1_shunt + 1.0 / (1.0 / y1_series + 2.0 / y1_shunt);
    // asymmetric
    DoubleComplex const yff0 = y0_series + 0.5 * y0_shunt;
    DoubleComplex const yft0 = -y0_series;
    DoubleComplex const ys0 = 0.5 * y0_shunt + 1.0 / (1.0 / y0_series + 2.0 / y0_shunt);
    ComplexTensor<asymmetric_t> const yffa{(2.0 * yff1 + yff0) / 3.0, (yff0 - yff1) / 3.0};
    ComplexTensor<asymmetric_t> const yfta{(2.0 * yft1 + yft0) / 3.0, (yft0 - yft1) / 3.0};
    ComplexTensor<asymmetric_t> const ysa{(2.0 * ys1 + ys0) / 3.0, (ys0 - ys1) / 3.0};

    DoubleComplex const u1f = 1.0;
    DoubleComplex const u1t = 0.9;
    ComplexValue<asymmetric_t> const uaf{1.0};
    ComplexValue<asymmetric_t> const uat{0.9};
    DoubleComplex const i1f = (yff1 * u1f + yft1 * u1t) * base_i;
    DoubleComplex const i1t = (yft1 * u1f + yff1 * u1t) * base_i;
    DoubleComplex const s_f = conj(i1f) * u1f * 10e3 * sqrt3;
    DoubleComplex const s_t = conj(i1t) * u1t * 10e3 * sqrt3;
    double const loading = std::max(cabs(i1f), cabs(i1t)) / 200.0;

    // Short circuit results
    DoubleComplex const if_sc{1.0, 1.0};
    DoubleComplex const it_sc{2.0, 2.0 * sqrt3};
    ComplexValue<asymmetric_t> const if_sc_asym{1.0 + 1.0i};
    ComplexValue<asymmetric_t> const it_sc_asym{2.0 + (2.0i * sqrt3)};

    CHECK(line.math_model_type() == ComponentType::branch);

    SUBCASE("Voltge error") { CHECK_THROWS_AS(Line(input, 50.0, 10.0e3, 50.0e3), ConflictVoltage); }

    SUBCASE("General") {
        CHECK(branch.from_node() == 2);
        CHECK(branch.to_node() == 3);
        CHECK(branch.from_status() == true);
        CHECK(branch.to_status() == true);
        CHECK(branch.branch_status() == true);
        CHECK(branch.status(BranchSide::from) == branch.from_status());
        CHECK(branch.status(BranchSide::to) == branch.to_status());
        CHECK(branch.base_i_from() == doctest::Approx(base_i));
        CHECK(branch.base_i_to() == doctest::Approx(base_i));
        CHECK(branch.phase_shift() == 0.0);
        CHECK(!branch.is_param_mutable());
    }

    SUBCASE("Symmetric parameters") {
        // double connected
        BranchCalcParam<symmetric_t> param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytt() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytf() - yft1) < numerical_tolerance);
        CHECK(cabs(param.yft() - yft1) < numerical_tolerance);
        // to connected
        CHECK(branch.update(BranchUpdate{1, false, na_IntS}).topo);
        param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - ys1) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
        // not connected
        CHECK(branch.set_status(na_IntS, false));
        param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
        // not changing
        CHECK(!branch.set_status(false, false));
        // from connected
        CHECK(branch.set_status(true, na_IntS));
        param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - ys1) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
    }

    SUBCASE("Asymmetric parameters") {
        // double connected
        BranchCalcParam<asymmetric_t> param = line.calc_param<asymmetric_t>();
        CHECK((cabs(param.yff() - yffa) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - yffa) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - yfta) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - yfta) < numerical_tolerance).all());
        // no source
        param = branch.calc_param<asymmetric_t>(false);
        CHECK((cabs(param.yff() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - 0.0) < numerical_tolerance).all());
        // from connected
        CHECK(branch.set_status(na_IntS, false));
        param = line.calc_param<asymmetric_t>();
        CHECK((cabs(param.yff() - ysa) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - 0.0) < numerical_tolerance).all());
    }

    SUBCASE("Symmetric results") {
        BranchOutput<symmetric_t> const output = branch.get_output<symmetric_t>(1.0, 0.9);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == doctest::Approx(loading));
        CHECK(output.i_from == doctest::Approx(cabs(i1f)));
        CHECK(output.i_to == doctest::Approx(cabs(i1t)));
        CHECK(output.s_from == doctest::Approx(cabs(s_f)));
        CHECK(output.s_to == doctest::Approx(cabs(s_t)));
        CHECK(output.p_from == doctest::Approx(real(s_f)));
        CHECK(output.p_to == doctest::Approx(real(s_t)));
        CHECK(output.q_from == doctest::Approx(imag(s_f)));
        CHECK(output.q_to == doctest::Approx(imag(s_t)));
    }

    SUBCASE("Symmetric results with direct power and current output") {
        BranchSolverOutput<symmetric_t> branch_solver_output{};
        branch_solver_output.i_f = 1.0 - 2.0i;
        branch_solver_output.i_t = 2.0 - 1.0i;
        branch_solver_output.s_f = 1.0 - 1.5i;
        branch_solver_output.s_t = 1.5 - 1.5i;
        BranchOutput<symmetric_t> const output = branch.get_output<symmetric_t>(branch_solver_output);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == doctest::Approx(cabs(2.0 - 1.0i) * base_i / input.i_n));
        CHECK(output.i_from == doctest::Approx(cabs(1.0 - 2.0i) * base_i));
        CHECK(output.i_to == doctest::Approx(cabs(2.0 - 1.0i) * base_i));
        CHECK(output.s_from == doctest::Approx(cabs(1.0 - 1.5i) * base_power<symmetric_t>));
        CHECK(output.s_to == doctest::Approx(cabs(1.5 - 1.5i) * base_power<symmetric_t>));
        CHECK(output.p_from == doctest::Approx(1.0 * base_power<symmetric_t>));
        CHECK(output.p_to == doctest::Approx(1.5 * base_power<symmetric_t>));
        CHECK(output.q_from == doctest::Approx(-1.5 * base_power<symmetric_t>));
        CHECK(output.q_to == doctest::Approx(-1.5 * base_power<symmetric_t>));
    }

    SUBCASE("No source results") {
        BranchOutput<asymmetric_t> output = branch.get_null_output<asymmetric_t>();
        CHECK(output.id == 1);
        CHECK(!output.energized);
        CHECK(output.loading == 0.0);
        CHECK(output.i_from(0) == 0.0);
        CHECK(output.i_to(1) == 0.0);
        CHECK(output.s_from(2) == 0.0);
        CHECK(output.s_to(0) == 0.0);
        CHECK(output.p_from(1) == 0.0);
        CHECK(output.p_to(2) == 0.0);
        CHECK(output.q_from(0) == 0.0);
        CHECK(output.q_to(1) == 0.0);
    }

    SUBCASE("No source short circuit results") {
        BranchShortCircuitOutput output = branch.get_null_sc_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
        CHECK(output.i_from(0) == 0.0);
        CHECK(output.i_to(1) == 0.0);
        CHECK(output.i_from_angle(0) == 0.0);
        CHECK(output.i_to_angle(1) == 0.0);
    }

    SUBCASE("Asymmetric results") {
        BranchOutput<asymmetric_t> output = branch.get_output<asymmetric_t>(uaf, uat);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == doctest::Approx(loading));
        CHECK(output.i_from(0) == doctest::Approx(cabs(i1f)));
        CHECK(output.i_to(1) == doctest::Approx(cabs(i1t)));
        CHECK(output.s_from(2) == doctest::Approx(cabs(s_f) / 3.0));
        CHECK(output.s_to(0) == doctest::Approx(cabs(s_t) / 3.0));
        CHECK(output.p_from(1) == doctest::Approx(real(s_f) / 3.0));
        CHECK(output.p_to(2) == doctest::Approx(real(s_t) / 3.0));
        CHECK(output.q_from(0) == doctest::Approx(imag(s_f) / 3.0));
        CHECK(output.q_to(1) == doctest::Approx(imag(s_t) / 3.0));
    }

    SUBCASE("Asym short circuit results") {
        BranchShortCircuitOutput asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
        CHECK(asym_output.id == 1);
        CHECK(asym_output.energized);
        CHECK(asym_output.i_from(1) == doctest::Approx(cabs(if_sc) * base_i));
        CHECK(asym_output.i_from(2) == doctest::Approx(cabs(if_sc) * base_i));
        CHECK(asym_output.i_to(0) == doctest::Approx(cabs(it_sc) * base_i));
        CHECK(asym_output.i_to(1) == doctest::Approx(cabs(it_sc) * base_i));
        CHECK(asym_output.i_from_angle(0) == doctest::Approx(pi / 4));
        CHECK(asym_output.i_from_angle(2) == doctest::Approx(pi / 4 + deg_120));
        CHECK(asym_output.i_to_angle(1) == doctest::Approx(pi / 3 - deg_120));
        CHECK(asym_output.i_to_angle(2) == doctest::Approx(pi / 3 + deg_120));
        CHECK(asym_output.id == 1);
    }

    SUBCASE("Sym short circuit results") {
        BranchShortCircuitOutput sym_output = branch.get_sc_output(if_sc, it_sc);
        BranchShortCircuitOutput asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
        CHECK(sym_output.energized == asym_output.energized);
        CHECK(sym_output.i_from(1) == doctest::Approx(asym_output.i_from(1)));
        CHECK(sym_output.i_from(2) == doctest::Approx(asym_output.i_from(2)));
        CHECK(sym_output.i_to(0) == doctest::Approx(asym_output.i_to(0)));
        CHECK(sym_output.i_to(1) == doctest::Approx(asym_output.i_to(1)));
        CHECK(sym_output.i_from_angle(0) == doctest::Approx(asym_output.i_from_angle(0)));
        CHECK(sym_output.i_from_angle(2) == doctest::Approx(asym_output.i_from_angle(2)));
        CHECK(sym_output.i_to_angle(1) == doctest::Approx(asym_output.i_to_angle(1)));
        CHECK(sym_output.i_to_angle(2) == doctest::Approx(asym_output.i_to_angle(2)));
    }

    SUBCASE("Update inverse") {
        BranchUpdate branch_update{.id = 1, .from_status = na_IntS, .to_status = na_IntS};
        auto expected = branch_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("From status") {
            SUBCASE("same") { branch_update.from_status = status_to_int(line.from_status()); }
            SUBCASE("different") { branch_update.from_status = IntS{0}; }
            expected.from_status = status_to_int(line.from_status());
        }

        SUBCASE("To status") {
            SUBCASE("same") { branch_update.to_status = status_to_int(line.to_status()); }
            SUBCASE("different") { branch_update.to_status = IntS{0}; }
            expected.to_status = status_to_int(line.to_status());
        }

        SUBCASE("multiple") {
            branch_update.from_status = IntS{0};
            branch_update.to_status = IntS{0};
            expected.from_status = status_to_int(line.from_status());
            expected.to_status = status_to_int(line.to_status());
        }

        auto const inv = line.inverse(branch_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.from_status == expected.from_status);
        CHECK(inv.to_status == expected.to_status);
    }
}

} // namespace power_grid_model
