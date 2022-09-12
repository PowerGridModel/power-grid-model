// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/component/shunt.hpp"

namespace power_grid_model {

TEST_CASE("Test shunt") {
    ShuntInput shunt_input{{{1}, 2, true}, 1.0, 2.0, 3.0, 4.0};
    Shunt shunt{shunt_input, 10e3};
    double const base_i = base_power_1p / (10.0e3 / sqrt3);
    double const base_y = base_power_3p / 10e3 / 10e3;
    DoubleComplex const y1 = (1.0 + 2.0i) / base_y;
    DoubleComplex const y0 = (3.0 + 4.0i) / base_y;
    DoubleComplex const u{1.0};
    ComplexValue<false> ua{1.0};
    double const p = 10e3 * 10e3 * 1.0;
    double const q = -10e3 * 10e3 * 2.0;
    double const s = std::sqrt(p * p + q * q);
    double const i = s / 10e3 / sqrt3;
    double const pf = p / s;

    CHECK(shunt.math_model_type() == ComponentType::shunt);

    SUBCASE("test parameters") {
        ComplexTensor<true> y = shunt.calc_param<true>();
        CHECK(cabs(y - y1) < numerical_tolerance);
        ComplexTensor<false> ya = shunt.calc_param<false>();
        CHECK(cabs(ya(0, 0) - (2.0 * y1 + y0) / 3.0) < numerical_tolerance);
        CHECK(cabs(ya(0, 1) - (y0 - y1) / 3.0) < numerical_tolerance);
        // no source
        ya = shunt.calc_param<false>(false);
        CHECK(cabs(ya(0, 0)) < numerical_tolerance);
        CHECK(cabs(ya(0, 1)) < numerical_tolerance);
    }

    SUBCASE("test results; u as input") {
        ApplianceOutput<true> sym_result = shunt.get_output<true>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(p));
        CHECK(sym_result.q == doctest::Approx(q));
        CHECK(sym_result.s == doctest::Approx(s));
        CHECK(sym_result.i == doctest::Approx(i));
        CHECK(sym_result.pf == doctest::Approx(pf));
        ApplianceOutput<false> asym_result = shunt.get_output<false>(ua);
        CHECK(asym_result.p(0) == doctest::Approx(p / 3));
        CHECK(asym_result.q(1) == doctest::Approx(q / 3));
        CHECK(asym_result.s(2) == doctest::Approx(s / 3));
        CHECK(asym_result.i(0) == doctest::Approx(i));
        CHECK(asym_result.pf(1) == doctest::Approx(pf));
    }

    SUBCASE("Symmetric test results; s, i as input") {
        ApplianceMathOutput<true> appliance_math_output_sym;
        appliance_math_output_sym.i = 1.0 + 2.0i;
        appliance_math_output_sym.s = 3.0 + 4.0i;
        ApplianceOutput<true> sym_result = shunt.get_output<true>(appliance_math_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(-3.0 * base_power<true>));
        CHECK(sym_result.q == doctest::Approx(-4.0 * base_power<true>));
        CHECK(sym_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<true>));
        CHECK(sym_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("Asymmetric test results; s, i as input") {
        ApplianceMathOutput<false> appliance_math_output_asym;
        ComplexValue<false> const i_a{1.0 + 2.0i};
        ComplexValue<false> const s_a{3.0 + 4.0i, 3.0 + 4.0i, 3.0 + 4.0i};
        appliance_math_output_asym.i = i_a;
        appliance_math_output_asym.s = s_a;
        ApplianceOutput<false> asym_result = shunt.get_output<false>(appliance_math_output_asym);
        CHECK(asym_result.id == 1);
        CHECK(asym_result.energized);
        CHECK(asym_result.p(0) == doctest::Approx(-3.0 * base_power<false>));
        CHECK(asym_result.q(1) == doctest::Approx(-4.0 * base_power<false>));
        CHECK(asym_result.s(2) == doctest::Approx(5.0 * base_power<false>));
        CHECK(asym_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("test change") {
        auto changed = shunt.update(ApplianceUpdate{{1}, true});
        CHECK(!changed.topo);
        CHECK(!changed.param);
        changed = shunt.update(ApplianceUpdate{{1}, false});
        CHECK(!changed.topo);
        CHECK(changed.param);
    }
}

}  // namespace power_grid_model