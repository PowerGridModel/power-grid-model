// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/component/line.hpp"

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test line") {
    LineInput input{{{1}, 2, 3, true, true}, 0.3, 0.4, 2e-4, 0.1, 0.1, 0.2, 1e-4, 0.2, 200.0};
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
    ComplexTensor<false> yffa{(2.0 * yff1 + yff0) / 3.0, (yff0 - yff1) / 3.0};
    ComplexTensor<false> yfta{(2.0 * yft1 + yft0) / 3.0, (yft0 - yft1) / 3.0};
    ComplexTensor<false> ysa{(2.0 * ys1 + ys0) / 3.0, (ys0 - ys1) / 3.0};

    DoubleComplex const u1f = 1.0;
    DoubleComplex const u1t = 0.9;
    ComplexValue<false> const uaf{1.0};
    ComplexValue<false> const uat{0.9};
    DoubleComplex const i1f = (yff1 * u1f + yft1 * u1t) * base_i;
    DoubleComplex const i1t = (yft1 * u1f + yff1 * u1t) * base_i;
    DoubleComplex const s_f = conj(i1f) * u1f * 10e3 * sqrt3;
    DoubleComplex const s_t = conj(i1t) * u1t * 10e3 * sqrt3;
    double loading = std::max(cabs(i1f), cabs(i1t)) / 200.0;

    CHECK(line.math_model_type() == ComponentType::branch);

    SUBCASE("Voltge error") {
        CHECK_THROWS_AS(Line(input, 50.0, 10.0e3, 50.0e3), ConflictVoltage);
    }

    SUBCASE("General") {
        CHECK(branch.from_node() == 2);
        CHECK(branch.to_node() == 3);
        CHECK(branch.from_status() == true);
        CHECK(branch.to_status() == true);
        CHECK(branch.branch_status() == true);
        CHECK(branch.base_i_from() == doctest::Approx(base_i));
        CHECK(branch.base_i_to() == doctest::Approx(base_i));
        CHECK(branch.phase_shift() == 0.0);
        CHECK(!branch.is_param_mutable());
    }

    SUBCASE("Symmetric parameters") {
        // double connected
        BranchCalcParam<true> param = branch.calc_param<true>();
        CHECK(cabs(param.yff() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytt() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytf() - yft1) < numerical_tolerance);
        CHECK(cabs(param.yft() - yft1) < numerical_tolerance);
        // to connected
        CHECK(branch.update(BranchUpdate{{1}, false, na_IntS}).topo);
        param = branch.calc_param<true>();
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - ys1) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
        // not connected
        CHECK(branch.set_status(na_IntS, false));
        param = branch.calc_param<true>();
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
        // not changing
        CHECK(!branch.set_status(false, false));
        // from connected
        CHECK(branch.set_status(true, na_IntS));
        param = branch.calc_param<true>();
        CHECK(cabs(param.yff() - ys1) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
    }

    SUBCASE("Asymmetric parameters") {
        // double connected
        BranchCalcParam<false> param = line.calc_param<false>();
        CHECK((cabs(param.yff() - yffa) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - yffa) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - yfta) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - yfta) < numerical_tolerance).all());
        // no source
        param = branch.calc_param<false>(false);
        CHECK((cabs(param.yff() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - 0.0) < numerical_tolerance).all());
        // from connected
        CHECK(branch.set_status(na_IntS, false));
        param = line.calc_param<false>();
        CHECK((cabs(param.yff() - ysa) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - 0.0) < numerical_tolerance).all());
    }

    SUBCASE("Symmetric results") {
        BranchOutput<true> output = branch.get_output<true>(1.0, 0.9);
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
        BranchMathOutput<true> branch_math_output{};
        branch_math_output.i_f = 1.0 - 2.0i;
        branch_math_output.i_t = 2.0 - 1.0i;
        branch_math_output.s_f = 1.0 - 1.5i;
        branch_math_output.s_t = 1.5 - 1.5i;
        BranchOutput<true> output = branch.get_output<true>(branch_math_output);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.loading == doctest::Approx(cabs(2.0 - 1.0i) * base_i / input.i_n));
        CHECK(output.i_from == doctest::Approx(cabs(1.0 - 2.0i) * base_i));
        CHECK(output.i_to == doctest::Approx(cabs(2.0 - 1.0i) * base_i));
        CHECK(output.s_from == doctest::Approx(cabs(1.0 - 1.5i) * base_power<true>));
        CHECK(output.s_to == doctest::Approx(cabs(1.5 - 1.5i) * base_power<true>));
        CHECK(output.p_from == doctest::Approx(1.0 * base_power<true>));
        CHECK(output.p_to == doctest::Approx(1.5 * base_power<true>));
        CHECK(output.q_from == doctest::Approx(-1.5 * base_power<true>));
        CHECK(output.q_to == doctest::Approx(-1.5 * base_power<true>));
    }

    SUBCASE("No source results") {
        BranchOutput<false> output = branch.get_null_output<false>();
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

    SUBCASE("Asymmetric results") {
        BranchOutput<false> output = branch.get_output<false>(uaf, uat);
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
}

}  // namespace power_grid_model