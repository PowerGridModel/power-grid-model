// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/source.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

TEST_CASE("Test source") {
    double const sk = 10e6;
    double const rx_ratio = 0.1;
    double const z01_ratio = 3;
    double const un = 10e3;
    double const zb = un * un / base_power_3p;
    double const z_abs = un * un / sk;
    double const x1 = z_abs / sqrt(rx_ratio * rx_ratio + 1.0);
    double const r1 = rx_ratio * x1;
    double const base_i = base_power_1p / (10.0e3 / sqrt3);
    DoubleComplex const z1{r1 / zb, x1 / zb};
    DoubleComplex const y1 = 1.0 / z1;
    DoubleComplex const z0 = z1 * z01_ratio;
    DoubleComplex const y0 = 1.0 / z0;
    DoubleComplex const y_ref_sym = y1;

    // calculation
    double const u_input = 1.1;
    double const u = 0.9;
    double const i = cabs(y1 * (u_input - u)) * base_power_3p / sqrt3 / un;

    // asym
    ComplexTensor<false> const sym_matrix = get_sym_matrix();
    ComplexTensor<false> const sym_matrix_inv = get_sym_matrix_inv();
    ComplexTensor<false> y012;
    y012 << y1, 0.0, 0.0, 0.0, y1, 0.0, 0.0, 0.0, y0;
    ComplexTensor<false> const y_ref_asym = dot(sym_matrix, y012, sym_matrix_inv);

    // construct
    SourceInput const source_input{{{1}, 2, 1}, u_input, nan, sk, rx_ratio, z01_ratio};
    Source source{source_input, un};

    CHECK(source.math_model_type() == ComponentType::source);

    SUBCASE("Test source parameters") {
        // uref
        DoubleComplex u_ref = source.calc_param();
        CHECK(cabs(u_ref - u_input) < numerical_tolerance);
        source.set_u_ref(nan, nan);
        u_ref = source.calc_param();
        CHECK(cabs(u_ref - u_input) < numerical_tolerance);
        source.set_u_ref(1.0, nan);
        u_ref = source.calc_param();
        CHECK(cabs(u_ref - 1.0) < numerical_tolerance);

        // uref with angle
        source.set_u_ref(nan, 2.5);
        u_ref = source.calc_param();
        CHECK(cabs(u_ref - 1.0 * std::exp(2.5i)) < numerical_tolerance);

        // yref
        DoubleComplex const y_ref_sym_cal = source.math_param<true>();
        CHECK(cabs(y_ref_sym_cal - y_ref_sym) < numerical_tolerance);
        ComplexTensor<false> const y_ref_asym_cal = source.math_param<false>();
        CHECK((cabs(y_ref_asym_cal - y_ref_asym) < numerical_tolerance).all());
    }

    SUBCASE("Test calc_param for short circuit") {
        source.set_u_ref(2.0, 2.5);
        auto voltage_scaling_parameters = std::pair{1000.0, ShortCircuitVoltageScaling::minimum};
        ComplexValue<true> u_ref = source.calc_param(voltage_scaling_parameters);
        CHECK(cabs(u_ref - 0.95 * std::exp(2.5i)) < numerical_tolerance);

        voltage_scaling_parameters.first = 1001.0;
        u_ref = source.calc_param(voltage_scaling_parameters);
        CHECK(cabs(u_ref - 1.0 * std::exp(2.5i)) < numerical_tolerance);

        voltage_scaling_parameters.second = ShortCircuitVoltageScaling::maximum;
        u_ref = source.calc_param(voltage_scaling_parameters);
        CHECK(cabs(u_ref - 1.1 * std::exp(2.5i)) < numerical_tolerance);
    }

    SUBCASE("test source sym results; u as input") {
        ApplianceOutput<true> const sym_result = source.get_output<true>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.i == doctest::Approx(i));
    }

    SUBCASE("test source sym results; s, i as input") {
        ApplianceMathOutput<true> appliance_math_output_sym;
        appliance_math_output_sym.i = 1.0 + 2.0i;
        appliance_math_output_sym.s = 3.0 + 4.0i;
        ApplianceOutput<true> const sym_result = source.get_output<true>(appliance_math_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(3.0 * base_power<true>));
        CHECK(sym_result.q == doctest::Approx(4.0 * base_power<true>));
        CHECK(sym_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<true>));
        CHECK(sym_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == doctest::Approx(3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("test source asym results; u as input") {
        ApplianceOutput<false> const asym_result = source.get_output<false>(ComplexValue<false>{u});
        CHECK(asym_result.id == 1);
        CHECK(asym_result.energized);
        CHECK(asym_result.i(0) == doctest::Approx(i));
    }

    SUBCASE("test source asym results; s, i as input") {
        ApplianceMathOutput<false> appliance_math_output_asym;
        ComplexValue<false> const i_a{1.0 + 2.0i};
        ComplexValue<false> const s_a{3.0 + 4.0i, 3.0 + 4.0i, 3.0 + 4.0i};
        appliance_math_output_asym.i = i_a;
        appliance_math_output_asym.s = s_a;
        ApplianceOutput<false> const asym_result = source.get_output<false>(appliance_math_output_asym);
        CHECK(asym_result.id == 1);
        CHECK(asym_result.energized);
        CHECK(asym_result.p(0) == doctest::Approx(3.0 * base_power<false>));
        CHECK(asym_result.q(1) == doctest::Approx(4.0 * base_power<false>));
        CHECK(asym_result.s(2) == doctest::Approx(5.0 * base_power<false>));
        CHECK(asym_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == doctest::Approx(3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("test asym source short circuit results") {
        ComplexValue<false> const i_asym{1.0 + 2.0i};
        ApplianceShortCircuitOutput const asym_sc_result =
            source.get_sc_output(ApplianceShortCircuitMathOutput<false>{i_asym});
        CHECK(asym_sc_result.id == 1);
        CHECK(asym_sc_result.energized == 1);
        CHECK(asym_sc_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_sc_result.i(2) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_sc_result.i_angle(1) == doctest::Approx(arg(1.0 + 2.0i) - deg_120));
        CHECK(asym_sc_result.i_angle(2) == doctest::Approx(arg(1.0 + 2.0i) - deg_240));
    }

    SUBCASE("test sym source short circuit results") {
        // Sym and asym results should be the same
        DoubleComplex const i_sym = 1.0 + 2.0i;
        ComplexValue<false> const i_asym{1.0 + 2.0i};
        ApplianceShortCircuitOutput const sym_sc_result =
            source.get_sc_output(ApplianceShortCircuitMathOutput<true>{i_sym});
        ApplianceShortCircuitOutput const asym_sc_result =
            source.get_sc_output(ApplianceShortCircuitMathOutput<false>{i_asym});
        CHECK(sym_sc_result.id == asym_sc_result.id);
        CHECK(sym_sc_result.energized == asym_sc_result.energized);
        CHECK(sym_sc_result.i(0) == doctest::Approx(asym_sc_result.i(0)));
        CHECK(sym_sc_result.i(2) == doctest::Approx(asym_sc_result.i(2)));
        CHECK(sym_sc_result.i_angle(1) == doctest::Approx(asym_sc_result.i_angle(1)));
        CHECK(sym_sc_result.i_angle(2) == doctest::Approx(asym_sc_result.i_angle(2)));
    }

    SUBCASE("test no source") {
        ApplianceOutput<false> const asym_result = source.get_null_output<false>();
        CHECK(asym_result.id == 1);
        CHECK(!asym_result.energized);
        CHECK(asym_result.p(0) == doctest::Approx(0.0));
        CHECK(asym_result.q(1) == doctest::Approx(0.0));
        CHECK(asym_result.s(2) == doctest::Approx(0.0));
        CHECK(asym_result.i(0) == doctest::Approx(0.0));
        CHECK(asym_result.pf(1) == doctest::Approx(0.0));
    }

    SUBCASE("test no source for short circuit") {
        ApplianceShortCircuitOutput const sc_result = source.get_null_sc_output();
        CHECK(sc_result.id == 1);
        CHECK(!sc_result.energized);
        CHECK(sc_result.i(1) == doctest::Approx(0.0));
        CHECK(sc_result.i(2) == doctest::Approx(0.0));
        CHECK(sc_result.i_angle(0) == doctest::Approx(0.0));
        CHECK(sc_result.i_angle(1) == doctest::Approx(0.0));
    }

    SUBCASE("test update") {
        auto changed = source.update(SourceUpdate{{{1}, 1}, 1.05, nan});
        CHECK(!changed.topo);
        CHECK(changed.param);
        changed = source.update(SourceUpdate{{{1}, 0}, 1.05, nan});
        CHECK(changed.topo);
        CHECK(changed.param);
        changed = source.update(SourceUpdate{{{1}, 0}, nan, nan});
        CHECK(!changed.topo);
        CHECK(!changed.param);
    }
}

} // namespace power_grid_model
