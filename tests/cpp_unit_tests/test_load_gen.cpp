// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "catch2/catch.hpp"
#include "power_grid_model/component/load_gen.hpp"

namespace power_grid_model {

TEST_CASE("Test load generator") {
    LoadGenInput<true> sym_load_gen_input{{{{1}, 2, true}, LoadGenType::const_pq}, 3e6, 3e6};
    LoadGenInput<false> asym_load_gen_input{
        {{{1}, 2, true}, LoadGenType::const_pq}, RealValue<false>{1e6}, RealValue<false>{1e6}};
    SymGenerator sym_gen_pq{sym_load_gen_input, 10e3};
    AsymLoad asym_load_pq{asym_load_gen_input, 10e3};
    sym_load_gen_input.type = LoadGenType::const_i;
    asym_load_gen_input.type = LoadGenType::const_y;
    SymLoad sym_load_i{sym_load_gen_input, 10e3};
    AsymGenerator asym_gen_y{asym_load_gen_input, 10e3};

    double const base_i = base_power_1p / (10e3 / sqrt3);
    DoubleComplex const u{1.1 * std::exp(1.0i * 10.0)};
    ComplexValue<false> const ua{1.1 * std::exp(1.0i * 10.0)};
    double const pf = 1 / sqrt(2.0);
    double const s_pq = sqrt(2.0) * 3e6;
    double const p_pq = 3e6;
    double const q_pq = 3e6;
    double const i_pq = s_pq / (1.1 * 10e3) / sqrt3;
    double const s_y = sqrt(2.0) * 3e6 * 1.1 * 1.1;
    double const p_y = 3e6 * 1.1 * 1.1;
    double const q_y = 3e6 * 1.1 * 1.1;
    double const i_y = s_y / (1.1 * 10e3) / sqrt3;
    double const s_i = sqrt(2.0) * 3e6 * 1.1;
    double const p_i = 3e6 * 1.1;
    double const q_i = 3e6 * 1.1;
    double const i_i = s_i / (1.1 * 10e3) / sqrt3;
    double const p_pu = 3e6 / base_power_3p;

    ApplianceMathOutput<true> appliance_math_output_sym;
    appliance_math_output_sym.i = 1.0 + 2.0i;
    appliance_math_output_sym.s = 3.0 + 4.0i;

    ApplianceMathOutput<false> appliance_math_output_asym;
    ComplexValue<false> const i_a{1.0 + 2.0i};
    ComplexValue<false> const s_a{3.0 + 4.0i, 3.0 + 4.0i, 3.0 + 4.0i};
    appliance_math_output_asym.i = i_a;
    appliance_math_output_asym.s = s_a;

    CHECK(sym_gen_pq.math_model_type() == ComponentType::generic_load_gen);

    SECTION("Test appliance property") {
        Appliance& appliance = sym_gen_pq;
        CHECK(appliance.base_i() == Approx(base_i));
        CHECK(appliance.node() == 2);
        CHECK(appliance.status());
        CHECK(appliance.set_status(false));
        CHECK(!appliance.status());
    }

    SECTION("Test symmetric generator with constant power; u as input") {
        GenericLoadGen const& load_gen = sym_gen_pq;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(p_pq));
        CHECK(sym_result.q == Approx(q_pq));
        CHECK(sym_result.s == Approx(s_pq));
        CHECK(sym_result.i == Approx(i_pq));
        CHECK(sym_result.pf == Approx(pf));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(ua);
        CHECK(asym_result.p(0) == Approx(p_pq / 3));
        CHECK(asym_result.q(1) == Approx(q_pq / 3));
        CHECK(asym_result.s(2) == Approx(s_pq / 3));
        CHECK(asym_result.i(0) == Approx(i_pq));
        CHECK(asym_result.pf(1) == Approx(pf));
        // test sym power injection
        ComplexValue<true> const s_inj = load_gen.calc_param<true>();
        CHECK(real(s_inj) == Approx(p_pu));
        CHECK(imag(s_inj) == Approx(p_pu));
        // test asym power injection
        ComplexValue<false> const s_inj_a = load_gen.calc_param<false>();
        CHECK(real(s_inj_a(0)) == Approx(p_pu));
        CHECK(imag(s_inj_a(1)) == Approx(p_pu));
    }

    SECTION("Test symmetric generator with constant power; s,i as input") {
        GenericLoadGen const& load_gen = sym_gen_pq;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(appliance_math_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(3.0 * base_power<true>));
        CHECK(sym_result.q == Approx(4.0 * base_power<true>));
        CHECK(sym_result.s == Approx(cabs(3.0 + 4.0i) * base_power<true>));
        CHECK(sym_result.i == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == Approx(3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(appliance_math_output_asym);
        CHECK(asym_result.p(0) == Approx(3.0 * base_power<false>));
        CHECK(asym_result.q(1) == Approx(4.0 * base_power<false>));
        CHECK(asym_result.s(2) == Approx(5.0 * base_power<false>));
        CHECK(asym_result.i(0) == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == Approx(3.0 / cabs(3.0 + 4.0i)));
    }

    SECTION("Test asymmetric load with constant power; u as input") {
        GenericLoadGen const& load_gen = asym_load_pq;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(p_pq));
        CHECK(sym_result.q == Approx(q_pq));
        CHECK(sym_result.s == Approx(s_pq));
        CHECK(sym_result.i == Approx(i_pq));
        CHECK(sym_result.pf == Approx(pf));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(ua);
        CHECK(asym_result.p(0) == Approx(p_pq / 3));
        CHECK(asym_result.q(1) == Approx(q_pq / 3));
        CHECK(asym_result.s(2) == Approx(s_pq / 3));
        CHECK(asym_result.i(0) == Approx(i_pq));
        CHECK(asym_result.pf(1) == Approx(pf));
        // test sym power injection
        ComplexValue<true> const s_inj = load_gen.calc_param<true>();
        CHECK(real(s_inj) == Approx(-p_pu));
        CHECK(imag(s_inj) == Approx(-p_pu));
        ComplexValue<false> const s_inj_a = load_gen.calc_param<false>();
        CHECK(real(s_inj_a(0)) == Approx(-p_pu));
        CHECK(imag(s_inj_a(1)) == Approx(-p_pu));
    }

    SECTION("Test asymmetric load with constant power; s, i as input") {
        GenericLoadGen const& load_gen = asym_load_pq;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(appliance_math_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(-3.0 * base_power<true>));
        CHECK(sym_result.q == Approx(-4.0 * base_power<true>));
        CHECK(sym_result.s == Approx(cabs(3.0 + 4.0i) * base_power<true>));
        CHECK(sym_result.i == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == Approx(-3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(appliance_math_output_asym);
        CHECK(asym_result.p(0) == Approx(-3.0 * base_power<false>));
        CHECK(asym_result.q(1) == Approx(-4.0 * base_power<false>));
        CHECK(asym_result.s(2) == Approx(5.0 * base_power<false>));
        CHECK(asym_result.i(0) == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SECTION("Test symmetric load with constant current; u as input") {
        GenericLoadGen const& load_gen = sym_load_i;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(p_i));
        CHECK(sym_result.q == Approx(q_i));
        CHECK(sym_result.s == Approx(s_i));
        CHECK(sym_result.i == Approx(i_i));
        CHECK(sym_result.pf == Approx(pf));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(ua);
        CHECK(asym_result.p(0) == Approx(p_i / 3));
        CHECK(asym_result.q(1) == Approx(q_i / 3));
        CHECK(asym_result.s(2) == Approx(s_i / 3));
        CHECK(asym_result.i(0) == Approx(i_i));
        CHECK(asym_result.pf(1) == Approx(pf));
    }

    SECTION("Test symmetric load with constant current; s, i as input") {
        GenericLoadGen const& load_gen = sym_load_i;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(appliance_math_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(-3.0 * base_power<true>));
        CHECK(sym_result.q == Approx(-4.0 * base_power<true>));
        CHECK(sym_result.s == Approx(cabs(3.0 + 4.0i) * base_power<true>));
        CHECK(sym_result.i == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == Approx(-3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(appliance_math_output_asym);
        CHECK(asym_result.p(0) == Approx(-3.0 * base_power<false>));
        CHECK(asym_result.q(1) == Approx(-4.0 * base_power<false>));
        CHECK(asym_result.s(2) == Approx(5.0 * base_power<false>));
        CHECK(asym_result.i(0) == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SECTION("Test asymmetric generator with constant addmittance; u as input ") {
        GenericLoadGen const& load_gen = asym_gen_y;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(p_y));
        CHECK(sym_result.q == Approx(q_y));
        CHECK(sym_result.s == Approx(s_y));
        CHECK(sym_result.i == Approx(i_y));
        CHECK(sym_result.pf == Approx(pf));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(ua);
        CHECK(asym_result.p(0) == Approx(p_y / 3));
        CHECK(asym_result.q(1) == Approx(q_y / 3));
        CHECK(asym_result.s(2) == Approx(s_y / 3));
        CHECK(asym_result.i(0) == Approx(i_y));
        CHECK(asym_result.pf(1) == Approx(pf));
    }

    SECTION("Test asymmetric generator with constant addmittance; s, i as input") {
        GenericLoadGen const& load_gen = asym_gen_y;
        // sym result
        ApplianceOutput<true> const sym_result = load_gen.get_output<true>(appliance_math_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == Approx(3.0 * base_power<true>));
        CHECK(sym_result.q == Approx(4.0 * base_power<true>));
        CHECK(sym_result.s == Approx(cabs(3.0 + 4.0i) * base_power<true>));
        CHECK(sym_result.i == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == Approx(3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<false> const asym_result = load_gen.get_output<false>(appliance_math_output_asym);
        CHECK(asym_result.p(0) == Approx(3.0 * base_power<false>));
        CHECK(asym_result.q(1) == Approx(4.0 * base_power<false>));
        CHECK(asym_result.s(2) == Approx(5.0 * base_power<false>));
        CHECK(asym_result.i(0) == Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == Approx(3.0 / cabs(3.0 + 4.0i)));
    }

    SECTION("Test update load") {
        auto changed = sym_gen_pq.update(SymLoadGenUpdate{{{1}, na_IntS}, 1e6, nan});
        CHECK(!changed.topo);
        CHECK(!changed.param);
        ApplianceOutput<true> const sym_result = sym_gen_pq.get_output<true>(u);
        CHECK(sym_result.p == Approx(1e6));
        CHECK(sym_result.q == Approx(q_pq));
        asym_load_pq.set_power(RealValue<false>{nan}, RealValue<false>{1e5});
        ApplianceOutput<false> const asym_result = asym_load_pq.get_output<false>(ua);
        CHECK(asym_result.p(0) == Approx(p_pq / 3));
        CHECK(asym_result.q(1) == Approx(1e5));
    }

    SECTION("Test no source") {
        auto const s = sym_gen_pq.calc_param<false>(false);
        CHECK(real(s)(0) == Approx(0.0));
        CHECK(imag(s)(1) == Approx(0.0));
        auto const asym_result = sym_gen_pq.get_null_output<false>();
        CHECK(!asym_result.energized);
        CHECK(asym_result.p(0) == Approx(0.0));
        CHECK(asym_result.q(1) == Approx(0.0));
        CHECK(asym_result.s(2) == Approx(0.0));
        CHECK(asym_result.i(0) == Approx(0.0));
        CHECK(asym_result.pf(1) == Approx(0.0));
    }
}

}  // namespace power_grid_model