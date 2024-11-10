// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/asym_line.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test asym line") {
    double system_frequency = 50.0;
    AsymLineInput const input{.id = 1,
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
    AsymLine asym_line{input, system_frequency, 10.0e3, 10.0e3};
    double const base_i = base_power_1p / (10.0e3 / sqrt3);
    double const base_y = base_i * base_i / base_power_1p;
    Branch& branch = asym_line;
    ComplexTensor<asymmetric_t> const y_series = ComplexTensor<asymmetric_t>(0.66303418-0.34266364i, -0.02971114+0.03783535i, 0.04762194+0.00681293i, 0.04762194+0.00681293i, 2.48612768-0.46271628i, 0.05942228-0.07567069i, -0.02971114+0.03783535i, -0.09524388-0.01362585i, 2.48612768-0.46271628i);

    ComplexTensor<asymmetric_t> const y_shunt = 2 * pi * system_frequency * ComplexTensor<asymmetric_t>(input.c0 + input.c1, -input.c1) * 1.0i;

    DoubleComplex const y1_series = (y_series(0,0) + y_series(1,1) + y_series(2,2)) / 3.0 - (y_series(0,2) + y_series(1,1) + y_series(2,0)) / 3.0;
    DoubleComplex const y1_shunt = (y_shunt(0,0) + y_shunt(1,1) + y_shunt(2,2)) / 3.0 - (y_shunt(0,2) + y_shunt(1,1) + y_shunt(2,0)) / 3.0;

    // symmetric
    DoubleComplex const yff1 = y1_series + 0.5 * y1_shunt;
    DoubleComplex const yft1 = -y1_series;
    DoubleComplex const ys1 = 0.5 * y1_shunt + 1.0 / (1.0 / y1_series + 2.0 / y1_shunt);

    // asymmetric
    ComplexTensor<asymmetric_t> ytt = y_series + 0.5 * y_shunt;
    DoubleComplex const yff0 = ytt(0,0);
    DoubleComplex const yft0 = -y_series(0,0);
    DoubleComplex const ys0 = 0.5 * y_shunt(0,0) + 1.0 / (1.0 / y_series(0,0) + 2.0 / y_shunt(0,0));
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
    DoubleComplex const it_sc{2.0, 2.0 * sqrt(3)};
    ComplexValue<asymmetric_t> const if_sc_asym{1.0 + 1.0i};
    ComplexValue<asymmetric_t> const it_sc_asym{2.0 + (2.0i * sqrt(3))};

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
        param = asym_line.calc_param<asymmetric_t>();
        CHECK((cabs(param.yff() - ysa) < numerical_tolerance).all());
        CHECK((cabs(param.ytt() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.ytf() - 0.0) < numerical_tolerance).all());
        CHECK((cabs(param.yft() - 0.0) < numerical_tolerance).all());
    }
}

} // namespace power_grid_model