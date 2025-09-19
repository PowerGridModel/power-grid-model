// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/asym_line.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::complex_literals;

namespace {
// this function exceeds the reasonable threshold of 800 statements
// NOLINTNEXTLINE(readability-function-size,google-readability-function-size,hicpp-function-size)
void execute_subcases(const AsymLineInput& input, const ComplexTensor<asymmetric_t>& y_series,
                      const ComplexTensor<asymmetric_t>& c_matrix, const double base_i, const double base_y,
                      const double system_frequency, const double voltage_lvl) {
    CAPTURE(input.id);
    CAPTURE(y_series);
    AsymLine asym_line{input, system_frequency, voltage_lvl, voltage_lvl};
    Branch& branch = asym_line;

    ComplexTensor<asymmetric_t> const y_shunt = 1 / base_y * (2 * pi * system_frequency * c_matrix * 1.0i);

    DoubleComplex const y1_series =
        (y_series(0, 0) + y_series(1, 1) + y_series(2, 2)) / 3.0 -
        (y_series(0, 1) + y_series(1, 2) + y_series(1, 0) + y_series(1, 2) + y_series(2, 0) + y_series(2, 1)) / 6.0;
    DoubleComplex const y1_shunt =
        (y_shunt(0, 0) + y_shunt(1, 1) + y_shunt(2, 2)) / 3.0 -
        (y_shunt(0, 1) + y_shunt(1, 2) + y_shunt(1, 0) + y_shunt(1, 2) + y_shunt(2, 0) + y_shunt(2, 1)) / 6.0;

    // symmetric
    DoubleComplex const yff1 = y1_series + 0.5 * y1_shunt;
    DoubleComplex const yft1 = -y1_series;
    DoubleComplex const ys1 = 0.5 * y1_shunt + 1.0 / (1.0 / y1_series + 2.0 / y1_shunt);

    // asymmetric
    ComplexTensor<asymmetric_t> const ytt = y_series + 0.5 * y_shunt;
    ComplexTensor<asymmetric_t> const branch_shunt = 0.5 * y_shunt + inv(inv(y_series) + 2.0 * inv(y_shunt));

    double constexpr nominal_current = 216.0;
    DoubleComplex const u1f = 1.0;
    DoubleComplex const u1t = 0.9;
    ComplexValue<asymmetric_t> const uaf{1.0};
    ComplexValue<asymmetric_t> const uat{0.9};

    // Symmetric results
    DoubleComplex const i1f = (yff1 * u1f + yft1 * u1t) * base_i;
    DoubleComplex const i1t = (yft1 * u1f + yff1 * u1t) * base_i;
    DoubleComplex const s_f = conj(i1f) * u1f * 10e3 * sqrt3;
    DoubleComplex const s_t = conj(i1t) * u1t * 10e3 * sqrt3;
    double const loading_sym = std::max(cabs(i1f), cabs(i1t)) / nominal_current;

    // Asymmetric results
    ComplexValue<asymmetric_t> const i_f = dot(ytt, uaf) + dot(-y_series, uat);
    ComplexValue<asymmetric_t> const i_t = dot(-y_series, uaf) + dot(ytt, uat);
    ComplexValue<asymmetric_t> const i3pf = base_i * cabs(i_f);
    ComplexValue<asymmetric_t> const i3pt = base_i * cabs(i_t);

    ComplexValue<asymmetric_t> const s_f_asym = uaf * conj(i_f);
    ComplexValue<asymmetric_t> const s_t_asym = uat * conj(i_t);

    RealValue<asymmetric_t> const i_from_asym = base_i * cabs(i_f);
    RealValue<asymmetric_t> const i_to_asym = base_i * cabs(i_t);

    ComplexValue<asymmetric_t> const p3pf = base_power<asymmetric_t> * real(s_f_asym);
    ComplexValue<asymmetric_t> const p3pt = base_power<asymmetric_t> * real(s_t_asym);
    ComplexValue<asymmetric_t> const q3pf = base_power<asymmetric_t> * imag(s_f_asym);
    ComplexValue<asymmetric_t> const q3pt = base_power<asymmetric_t> * imag(s_t_asym);

    double const max_i = std::max(max_val(i_from_asym), max_val(i_to_asym));
    double const loading_asym = max_i / nominal_current;
    // Short circuit results
    DoubleComplex const if_sc{1.0, 1.0};
    DoubleComplex const it_sc{2.0, 2.0 * sqrt3};
    ComplexValue<asymmetric_t> const if_sc_asym{1.0 + 1.0i};
    ComplexValue<asymmetric_t> const it_sc_asym{2.0 + (2.0i * sqrt3)};

    CHECK(asym_line.math_model_type() == ComponentType::branch);

    SUBCASE("Voltge error") { CHECK_THROWS_AS(AsymLine(input, 50.0, 10.0e3, 50.0e3), ConflictVoltage); }

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
        BranchCalcParam<asymmetric_t> param = asym_line.calc_param<asymmetric_t>();
        CHECK((cabs(param.yff() - ytt) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - ytt) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - (-y_series)) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - (-y_series)) < numerical_tolerance).all());
        // no source
        param = branch.calc_param<asymmetric_t>(false);
        CHECK((cabs(param.yff() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - 0.0) < numerical_tolerance).all());
        // from connected
        CHECK(branch.set_status(na_IntS, false));
        param = asym_line.calc_param<asymmetric_t>();
        CHECK((cabs(param.yff() - branch_shunt) < numerical_tolerance).all()); // Fail
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - 0.0) < numerical_tolerance).all());
    }

    SUBCASE("Symmetric results") {
        BranchOutput<symmetric_t> const output = branch.get_output<symmetric_t>(1.0, 0.9);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == doctest::Approx(loading_sym));
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
        BranchOutput<asymmetric_t> const output = branch.get_null_output<asymmetric_t>();
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
        BranchShortCircuitOutput const output = branch.get_null_sc_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
        CHECK(output.i_from(0) == 0.0);
        CHECK(output.i_to(1) == 0.0);
        CHECK(output.i_from_angle(0) == 0.0);
        CHECK(output.i_to_angle(1) == 0.0);
    }

    SUBCASE("Asymmetric results") {
        BranchOutput<asymmetric_t> const output = branch.get_output<asymmetric_t>(uaf, uat);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == doctest::Approx(loading_asym));
        CHECK((cabs(output.i_from - i3pf) < numerical_tolerance).all());
        CHECK((cabs(output.i_to - i3pt) < numerical_tolerance).all());
        CHECK((cabs(output.p_from - p3pf) < numerical_tolerance).all());
        CHECK((cabs(output.p_to - p3pt) < numerical_tolerance).all());
        CHECK((cabs(output.q_from - q3pf) < numerical_tolerance).all());
        CHECK((cabs(output.q_to - q3pt) < numerical_tolerance).all());
    }

    SUBCASE("Asym short circuit results") {
        BranchShortCircuitOutput const asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
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
        BranchShortCircuitOutput const sym_output = branch.get_sc_output(if_sc, it_sc);
        BranchShortCircuitOutput const asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
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
            SUBCASE("same") { branch_update.from_status = status_to_int(asym_line.from_status()); }
            SUBCASE("different") { branch_update.from_status = IntS{0}; }
            expected.from_status = status_to_int(asym_line.from_status());
        }

        SUBCASE("To status") {
            SUBCASE("same") { branch_update.to_status = status_to_int(asym_line.to_status()); }
            SUBCASE("different") { branch_update.to_status = IntS{0}; }
            expected.to_status = status_to_int(asym_line.to_status());
        }

        SUBCASE("multiple") {
            branch_update.from_status = IntS{0};
            branch_update.to_status = IntS{0};
            expected.from_status = status_to_int(asym_line.from_status());
            expected.to_status = status_to_int(asym_line.to_status());
        }

        auto const inv = asym_line.inverse(branch_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.from_status == expected.from_status);
        CHECK(inv.to_status == expected.to_status);
    }
}
} // namespace

TEST_CASE("Test asym line") {
    double const system_frequency = 50.0;
    double const voltage_lvl = 10.0e3;
    double const base_i = base_power_1p / (voltage_lvl / sqrt3);
    double const base_y = base_i * base_i / base_power_1p;

    SUBCASE("R and X matrix c0, c1 including neutral") {
        AsymLineInput const input = {.id = 1,
                                     .from_node = 2,
                                     .to_node = 3,
                                     .from_status = 1,
                                     .to_status = 1,
                                     .r_aa = 0.4369,
                                     .r_ba = 0.0496,
                                     .r_bb = 0.4369,
                                     .r_ca = 0.0485,
                                     .r_cb = 0.0496,
                                     .r_cc = 0.4369,
                                     .r_na = 0.0496,
                                     .r_nb = 0.0485,
                                     .r_nc = 0.0496,
                                     .r_nn = 0.4369,
                                     .x_aa = 0.8538,
                                     .x_ba = 0.7886,
                                     .x_bb = 0.8538,
                                     .x_ca = 0.7663,
                                     .x_cb = 0.7886,
                                     .x_cc = 0.8538,
                                     .x_na = 0.7886,
                                     .x_nb = 0.7663,
                                     .x_nc = 0.7886,
                                     .x_nn = 0.8538,
                                     .c0 = 0.18,
                                     .c1 = 0.308,
                                     .i_n = 216.0};

        ComplexTensor4 const r_matrix{input.r_aa, input.r_bb, input.r_cc, input.r_nn, input.r_ba,
                                      input.r_ca, input.r_na, input.r_cb, input.r_nb, input.r_nc};
        ComplexTensor4 const x_matrix{input.x_aa, input.x_bb, input.x_cc, input.x_nn, input.x_ba,
                                      input.x_ca, input.x_na, input.x_cb, input.x_nb, input.x_nc};
        ComplexTensor<asymmetric_t> const c_matrix =
            ComplexTensor<asymmetric_t>{(2.0 * input.c1 + input.c0) / 3.0, (input.c0 - input.c1) / 3.0};
        ComplexTensor4 const z = r_matrix + 1.0i * x_matrix;

        ComplexTensor<asymmetric_t> const y_series = 1 / base_y * inv(kron_reduction(z));
        execute_subcases(input, y_series, c_matrix, base_i, base_y, system_frequency, voltage_lvl);
    }

    SUBCASE("R and X matrix, c0, c1 excluding neutral") {
        AsymLineInput const input = {.id = 1,
                                     .from_node = 2,
                                     .to_node = 3,
                                     .from_status = 1,
                                     .to_status = 1,
                                     .r_aa = 0.4369,
                                     .r_ba = 0.0496,
                                     .r_bb = 0.4369,
                                     .r_ca = 0.0485,
                                     .r_cb = 0.0496,
                                     .r_cc = 0.4369,
                                     .x_aa = 0.8538,
                                     .x_ba = 0.7886,
                                     .x_bb = 0.8538,
                                     .x_ca = 0.7663,
                                     .x_cb = 0.7886,
                                     .x_cc = 0.8538,
                                     .c0 = 0.18,
                                     .c1 = 0.308,
                                     .i_n = 216.0};
        ComplexTensor<asymmetric_t> const r_matrix{input.r_aa, input.r_bb, input.r_cc,
                                                   input.r_ba, input.r_ca, input.r_cb};
        ComplexTensor<asymmetric_t> const x_matrix{input.x_aa, input.x_bb, input.x_cc,
                                                   input.x_ba, input.x_ca, input.x_cb};
        ComplexTensor<asymmetric_t> const y_series = 1 / base_y * inv(r_matrix + 1.0i * x_matrix);
        ComplexTensor<asymmetric_t> const c_matrix =
            ComplexTensor<asymmetric_t>{(2.0 * input.c1 + input.c0) / 3.0, (input.c0 - input.c1) / 3.0};
        execute_subcases(input, y_series, c_matrix, base_i, base_y, system_frequency, voltage_lvl);
    }

    SUBCASE("R, X excluding neutral C matrix") {
        AsymLineInput const input = {.id = 1,
                                     .from_node = 2,
                                     .to_node = 3,
                                     .from_status = 1,
                                     .to_status = 1,
                                     .r_aa = 0.4369,
                                     .r_ba = 0.0496,
                                     .r_bb = 0.4369,
                                     .r_ca = 0.0485,
                                     .r_cb = 0.0496,
                                     .r_cc = 0.4369,
                                     .x_aa = 0.8538,
                                     .x_ba = 0.7886,
                                     .x_bb = 0.8538,
                                     .x_ca = 0.7663,
                                     .x_cb = 0.7886,
                                     .x_cc = 0.8538,
                                     .c_aa = 0.3200,
                                     .c_ba = 0.5400,
                                     .c_bb = 0.3200,
                                     .c_ca = 0.7600,
                                     .c_cb = 0.5400,
                                     .c_cc = 0.3200,
                                     .i_n = 216.0};
        ComplexTensor<asymmetric_t> const r_matrix{input.r_aa, input.r_bb, input.r_cc,
                                                   input.r_ba, input.r_ca, input.r_cb};
        ComplexTensor<asymmetric_t> const x_matrix{input.x_aa, input.x_bb, input.x_cc,
                                                   input.x_ba, input.x_ca, input.x_cb};
        ComplexTensor<asymmetric_t> const y_series = 1 / base_y * inv(r_matrix + 1.0i * x_matrix);
        ComplexTensor<asymmetric_t> const c_matrix =
            ComplexTensor<asymmetric_t>(input.c_aa, input.c_bb, input.c_cc, input.c_ba, input.c_ca, input.c_cb);
        execute_subcases(input, y_series, c_matrix, base_i, base_y, system_frequency, voltage_lvl);
    }
}

} // namespace power_grid_model
