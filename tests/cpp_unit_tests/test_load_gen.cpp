// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/load_gen.hpp>

#include <doctest/doctest.h>

TYPE_TO_STRING_AS("SymGenerator", power_grid_model::SymGenerator);
TYPE_TO_STRING_AS("AsymGenerator", power_grid_model::AsymGenerator);
TYPE_TO_STRING_AS("SymLoad", power_grid_model::SymLoad);
TYPE_TO_STRING_AS("AsymLoad", power_grid_model::AsymLoad);

namespace power_grid_model {
namespace {
using std::numbers::sqrt2;

void check_nan_preserving_equality(std::floating_point auto actual, std::floating_point auto expected) {
    if (is_nan(expected)) {
        is_nan(actual);
    } else {
        CHECK(actual == doctest::Approx(expected));
    }
}

void check_nan_preserving_equality(RealValue<asymmetric_t> const& actual, RealValue<asymmetric_t> const& expected) {
    for (auto i : {0, 1, 2}) {
        CAPTURE(i);
        check_nan_preserving_equality(actual(i), expected(i));
    }
}
} // namespace

TEST_CASE("Test load generator") {
    LoadGenInput<symmetric_t> sym_load_gen_input{
        .id = 1, .node = 2, .status = 1, .type = LoadGenType::const_pq, .p_specified = 3e6, .q_specified = 3e6};
    LoadGenInput<asymmetric_t> asym_load_gen_input{.id = 1,
                                                   .node = 2,
                                                   .status = 1,
                                                   .type = LoadGenType::const_pq,
                                                   .p_specified = RealValue<asymmetric_t>{1e6},
                                                   .q_specified = RealValue<asymmetric_t>{1e6}};
    SymGenerator sym_gen_pq{sym_load_gen_input, 10e3};
    AsymLoad asym_load_pq{asym_load_gen_input, 10e3};
    sym_load_gen_input.type = LoadGenType::const_i;
    asym_load_gen_input.type = LoadGenType::const_y;
    SymLoad const sym_load_i{sym_load_gen_input, 10e3};
    AsymGenerator const asym_gen_y{asym_load_gen_input, 10e3};

    double const base_i = base_power_1p / (10e3 / sqrt3);
    DoubleComplex const u{1.1 * std::exp(1.0i * 10.0)};
    ComplexValue<asymmetric_t> const ua{1.1 * std::exp(1.0i * 10.0)};
    double const pf = 1 / sqrt2;
    double const s_pq = sqrt2 * 3e6;
    double const p_pq = 3e6;
    double const q_pq = 3e6;
    double const i_pq = s_pq / (1.1 * 10e3) / sqrt3;
    double const s_y = sqrt2 * 3e6 * 1.1 * 1.1;
    double const p_y = 3e6 * 1.1 * 1.1;
    double const q_y = 3e6 * 1.1 * 1.1;
    double const i_y = s_y / (1.1 * 10e3) / sqrt3;
    double const s_i = sqrt2 * 3e6 * 1.1;
    double const p_i = 3e6 * 1.1;
    double const q_i = 3e6 * 1.1;
    double const i_i = s_i / (1.1 * 10e3) / sqrt3;
    double const p_pu = 3e6 / base_power_3p;

    ApplianceSolverOutput<symmetric_t> appliance_solver_output_sym;
    appliance_solver_output_sym.i = 1.0 + 2.0i;
    appliance_solver_output_sym.s = 3.0 + 4.0i;

    ApplianceSolverOutput<symmetric_t> appliance_solver_output_sym_reverse;
    appliance_solver_output_sym_reverse.i = -1.0 - 2.0i;
    appliance_solver_output_sym_reverse.s = -3.0 - 4.0i;

    ApplianceSolverOutput<asymmetric_t> appliance_solver_output_asym;
    ComplexValue<asymmetric_t> const i_a{1.0 + 2.0i};
    ComplexValue<asymmetric_t> const s_a{3.0 + 4.0i, 3.0 + 4.0i, 3.0 + 4.0i};
    appliance_solver_output_asym.i = i_a;
    appliance_solver_output_asym.s = s_a;

    CHECK(sym_gen_pq.math_model_type() == ComponentType::generic_load_gen);

    SUBCASE("Test appliance property") {
        Appliance& appliance = sym_gen_pq;
        CHECK(appliance.base_i() == doctest::Approx(base_i));
        CHECK(appliance.node() == 2);
        CHECK(appliance.status());
        CHECK(appliance.set_status(false));
        CHECK(!appliance.status());
    }

    SUBCASE("Test symmetric generator with constant power; u as input") {
        GenericLoadGen const& load_gen = sym_gen_pq;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(p_pq));
        CHECK(sym_result.q == doctest::Approx(q_pq));
        CHECK(sym_result.s == doctest::Approx(s_pq));
        CHECK(sym_result.i == doctest::Approx(i_pq));
        CHECK(sym_result.pf == doctest::Approx(pf));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result = load_gen.get_output<asymmetric_t>(ua);
        CHECK(asym_result.p(0) == doctest::Approx(p_pq / 3));
        CHECK(asym_result.q(1) == doctest::Approx(q_pq / 3));
        CHECK(asym_result.s(2) == doctest::Approx(s_pq / 3));
        CHECK(asym_result.i(0) == doctest::Approx(i_pq));
        CHECK(asym_result.pf(1) == doctest::Approx(pf));
        // test sym power injection
        ComplexValue<symmetric_t> const s_inj = load_gen.calc_param<symmetric_t>();
        CHECK(real(s_inj) == doctest::Approx(p_pu));
        CHECK(imag(s_inj) == doctest::Approx(p_pu));
        // test asym power injection
        ComplexValue<asymmetric_t> const s_inj_a = load_gen.calc_param<asymmetric_t>();
        CHECK(real(s_inj_a(0)) == doctest::Approx(p_pu));
        CHECK(imag(s_inj_a(1)) == doctest::Approx(p_pu));
    }

    SUBCASE("Test symmetric generator with constant power; s,i as input") {
        GenericLoadGen const& load_gen = sym_gen_pq;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(appliance_solver_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(3.0 * base_power<symmetric_t>));
        CHECK(sym_result.q == doctest::Approx(4.0 * base_power<symmetric_t>));
        CHECK(sym_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<symmetric_t>));
        CHECK(sym_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == doctest::Approx(3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result =
            load_gen.get_output<asymmetric_t>(appliance_solver_output_asym);
        CHECK(asym_result.p(0) == doctest::Approx(3.0 * base_power<asymmetric_t>));
        CHECK(asym_result.q(1) == doctest::Approx(4.0 * base_power<asymmetric_t>));
        CHECK(asym_result.s(2) == doctest::Approx(5.0 * base_power<asymmetric_t>));
        CHECK(asym_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == doctest::Approx(3.0 / cabs(3.0 + 4.0i)));
        // reverse result
        ApplianceOutput<symmetric_t> const reverse_result =
            load_gen.get_output<symmetric_t>(appliance_solver_output_sym_reverse);
        CHECK(reverse_result.id == 1);
        CHECK(reverse_result.energized);
        CHECK(reverse_result.p == doctest::Approx(-3.0 * base_power<symmetric_t>));
        CHECK(reverse_result.q == doctest::Approx(-4.0 * base_power<symmetric_t>));
        CHECK(reverse_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<symmetric_t>));
        CHECK(reverse_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(reverse_result.pf == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("Test asymmetric load with constant power; u as input") {
        GenericLoadGen const& load_gen = asym_load_pq;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(p_pq));
        CHECK(sym_result.q == doctest::Approx(q_pq));
        CHECK(sym_result.s == doctest::Approx(s_pq));
        CHECK(sym_result.i == doctest::Approx(i_pq));
        CHECK(sym_result.pf == doctest::Approx(pf));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result = load_gen.get_output<asymmetric_t>(ua);
        CHECK(asym_result.p(0) == doctest::Approx(p_pq / 3));
        CHECK(asym_result.q(1) == doctest::Approx(q_pq / 3));
        CHECK(asym_result.s(2) == doctest::Approx(s_pq / 3));
        CHECK(asym_result.i(0) == doctest::Approx(i_pq));
        CHECK(asym_result.pf(1) == doctest::Approx(pf));
        // test sym power injection
        ComplexValue<symmetric_t> const s_inj = load_gen.calc_param<symmetric_t>();
        CHECK(real(s_inj) == doctest::Approx(-p_pu));
        CHECK(imag(s_inj) == doctest::Approx(-p_pu));
        ComplexValue<asymmetric_t> const s_inj_a = load_gen.calc_param<asymmetric_t>();
        CHECK(real(s_inj_a(0)) == doctest::Approx(-p_pu));
        CHECK(imag(s_inj_a(1)) == doctest::Approx(-p_pu));
    }

    SUBCASE("Test asymmetric load with constant power; s, i as input") {
        GenericLoadGen const& load_gen = asym_load_pq;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(appliance_solver_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(-3.0 * base_power<symmetric_t>));
        CHECK(sym_result.q == doctest::Approx(-4.0 * base_power<symmetric_t>));
        CHECK(sym_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<symmetric_t>));
        CHECK(sym_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result =
            load_gen.get_output<asymmetric_t>(appliance_solver_output_asym);
        CHECK(asym_result.p(0) == doctest::Approx(-3.0 * base_power<asymmetric_t>));
        CHECK(asym_result.q(1) == doctest::Approx(-4.0 * base_power<asymmetric_t>));
        CHECK(asym_result.s(2) == doctest::Approx(5.0 * base_power<asymmetric_t>));
        CHECK(asym_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("Test symmetric load with constant current; u as input") {
        GenericLoadGen const& load_gen = sym_load_i;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(p_i));
        CHECK(sym_result.q == doctest::Approx(q_i));
        CHECK(sym_result.s == doctest::Approx(s_i));
        CHECK(sym_result.i == doctest::Approx(i_i));
        CHECK(sym_result.pf == doctest::Approx(pf));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result = load_gen.get_output<asymmetric_t>(ua);
        CHECK(asym_result.p(0) == doctest::Approx(p_i / 3));
        CHECK(asym_result.q(1) == doctest::Approx(q_i / 3));
        CHECK(asym_result.s(2) == doctest::Approx(s_i / 3));
        CHECK(asym_result.i(0) == doctest::Approx(i_i));
        CHECK(asym_result.pf(1) == doctest::Approx(pf));
    }

    SUBCASE("Test symmetric load with constant current; s, i as input") {
        GenericLoadGen const& load_gen = sym_load_i;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(appliance_solver_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(-3.0 * base_power<symmetric_t>));
        CHECK(sym_result.q == doctest::Approx(-4.0 * base_power<symmetric_t>));
        CHECK(sym_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<symmetric_t>));
        CHECK(sym_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result =
            load_gen.get_output<asymmetric_t>(appliance_solver_output_asym);
        CHECK(asym_result.p(0) == doctest::Approx(-3.0 * base_power<asymmetric_t>));
        CHECK(asym_result.q(1) == doctest::Approx(-4.0 * base_power<asymmetric_t>));
        CHECK(asym_result.s(2) == doctest::Approx(5.0 * base_power<asymmetric_t>));
        CHECK(asym_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
        // reverse direction
        ApplianceOutput<symmetric_t> const reverse_result =
            load_gen.get_output<symmetric_t>(appliance_solver_output_sym_reverse);
        CHECK(reverse_result.id == 1);
        CHECK(reverse_result.energized);
        CHECK(reverse_result.p == doctest::Approx(3.0 * base_power<symmetric_t>));
        CHECK(reverse_result.q == doctest::Approx(4.0 * base_power<symmetric_t>));
        CHECK(reverse_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<symmetric_t>));
        CHECK(reverse_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(reverse_result.pf == doctest::Approx(3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("Test asymmetric generator with constant addmittance; u as input ") {
        GenericLoadGen const& load_gen = asym_gen_y;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(p_y));
        CHECK(sym_result.q == doctest::Approx(q_y));
        CHECK(sym_result.s == doctest::Approx(s_y));
        CHECK(sym_result.i == doctest::Approx(i_y));
        CHECK(sym_result.pf == doctest::Approx(pf));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result = load_gen.get_output<asymmetric_t>(ua);
        CHECK(asym_result.p(0) == doctest::Approx(p_y / 3));
        CHECK(asym_result.q(1) == doctest::Approx(q_y / 3));
        CHECK(asym_result.s(2) == doctest::Approx(s_y / 3));
        CHECK(asym_result.i(0) == doctest::Approx(i_y));
        CHECK(asym_result.pf(1) == doctest::Approx(pf));
    }

    SUBCASE("Test asymmetric generator with constant addmittance; s, i as input") {
        GenericLoadGen const& load_gen = asym_gen_y;
        // sym result
        ApplianceOutput<symmetric_t> const sym_result = load_gen.get_output<symmetric_t>(appliance_solver_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(3.0 * base_power<symmetric_t>));
        CHECK(sym_result.q == doctest::Approx(4.0 * base_power<symmetric_t>));
        CHECK(sym_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<symmetric_t>));
        CHECK(sym_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == doctest::Approx(3.0 / cabs(3.0 + 4.0i)));
        // asym result
        ApplianceOutput<asymmetric_t> const asym_result =
            load_gen.get_output<asymmetric_t>(appliance_solver_output_asym);
        CHECK(asym_result.p(0) == doctest::Approx(3.0 * base_power<asymmetric_t>));
        CHECK(asym_result.q(1) == doctest::Approx(4.0 * base_power<asymmetric_t>));
        CHECK(asym_result.s(2) == doctest::Approx(5.0 * base_power<asymmetric_t>));
        CHECK(asym_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == doctest::Approx(3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("Test update load") {
        auto changed =
            sym_gen_pq.update(SymLoadGenUpdate{.id = 1, .status = na_IntS, .p_specified = 1e6, .q_specified = nan});
        CHECK(!changed.topo);
        CHECK(!changed.param);
        ApplianceOutput<symmetric_t> const sym_result = sym_gen_pq.get_output<symmetric_t>(u);
        CHECK(sym_result.p == doctest::Approx(1e6));
        CHECK(sym_result.q == doctest::Approx(q_pq));
        asym_load_pq.set_power(RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{1e5});
        ApplianceOutput<asymmetric_t> const asym_result = asym_load_pq.get_output<asymmetric_t>(ua);
        CHECK(asym_result.p(0) == doctest::Approx(p_pq / 3));
        CHECK(asym_result.q(1) == doctest::Approx(1e5));
    }

    SUBCASE("Test set_power - sym") {
        // update with nan, nothing happens
        sym_gen_pq.set_power(RealValue<symmetric_t>{nan}, RealValue<symmetric_t>{nan});
        ComplexValue<symmetric_t> const s_1 = sym_gen_pq.calc_param<symmetric_t>();
        CHECK(real(s_1) == 3.0);
        CHECK(imag(s_1) == 3.0);

        // update with values, s changes
        sym_gen_pq.set_power(RealValue<symmetric_t>{4.0e6}, RealValue<symmetric_t>{5.0e6});
        ComplexValue<symmetric_t> const s_2 = sym_gen_pq.calc_param<symmetric_t>();
        CHECK(real(s_2) == 4.0);
        CHECK(imag(s_2) == 5.0);
    }

    SUBCASE("Test set_power - asym") {
        // update with {nan, nan, nan}, nothing happens
        asym_load_pq.set_power(RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan});
        ComplexValue<asymmetric_t> s_1 = asym_load_pq.calc_param<asymmetric_t>();
        for (size_t i = 0; i != 3; i++) {
            CHECK(real(s_1(i)) == -3.0);
            CHECK(imag(s_1(i)) == -3.0);
        }

        // update some with nan, some with values
        asym_load_pq.set_power(RealValue<asymmetric_t>{2.0e6, nan, 3.0e6}, RealValue<asymmetric_t>{nan, 4.0e6, nan});
        ComplexValue<asymmetric_t> s_2 = asym_load_pq.calc_param<asymmetric_t>();
        CHECK(real(s_2(0)) == -6.0);
        CHECK(real(s_2(1)) == -3.0); // not updated
        CHECK(real(s_2(2)) == -9.0);
        CHECK(imag(s_2(0)) == -3.0); // not updated
        CHECK(imag(s_2(1)) == -12.0);
        CHECK(imag(s_2(2)) == -3.0); // not updated
    }

    SUBCASE("Test no source") {
        auto const s = sym_gen_pq.calc_param<asymmetric_t>(false);
        CHECK(real(s)(0) == doctest::Approx(0.0));
        CHECK(imag(s)(1) == doctest::Approx(0.0));
        auto const asym_result = sym_gen_pq.get_null_output<asymmetric_t>();
        CHECK(!asym_result.energized);
        CHECK(asym_result.p(0) == doctest::Approx(0.0));
        CHECK(asym_result.q(1) == doctest::Approx(0.0));
        CHECK(asym_result.s(2) == doctest::Approx(0.0));
        CHECK(asym_result.i(0) == doctest::Approx(0.0));
        CHECK(asym_result.pf(1) == doctest::Approx(0.0));
    }
}

TEST_CASE_TEMPLATE("Test load generator", LoadGeneratorType, SymLoad, AsymLoad, SymGenerator, AsymGenerator) {
    using InputType = typename LoadGeneratorType::InputType;
    using UpdateType = typename LoadGeneratorType::UpdateType;
    using RealValueType = decltype(InputType::p_specified);

    auto const r_nan = RealValueType{nan};

    SUBCASE("Partial initialization and full update") {
        InputType input{};
        UpdateType update{};

        input.status = 1;

        SUBCASE("p_specified not provided") {
            input.p_specified = r_nan;
            input.q_specified = RealValueType{1.0};

            update.p_specified = RealValueType{1.0};
            update.q_specified = r_nan;
        }

        SUBCASE("q_specified not provided") {
            input.p_specified = RealValueType{1.0};
            input.q_specified = r_nan;

            update.p_specified = r_nan;
            update.q_specified = RealValueType{1.0};
        }

        SUBCASE("both not provided") {
            input.p_specified = r_nan;
            input.q_specified = r_nan;

            update.p_specified = RealValueType{1.0};
            update.q_specified = RealValueType{1.0};
        }

        LoadGeneratorType load_gen{input, 1.0};

        auto const result_incomplete = load_gen.template calc_param<symmetric_t>(true);
        CHECK(std::isnan(result_incomplete.real()));
        CHECK(std::isnan(result_incomplete.imag()));

        load_gen.update(update);

        auto const result_complete = load_gen.template calc_param<symmetric_t>(true);
        CHECK_FALSE(std::isnan(result_complete.real()));
        CHECK_FALSE(std::isnan(result_complete.imag()));
    }

    SUBCASE("Update inverse") {
        auto const status = IntS{1};
        auto const p_specified = RealValueType{1.0};
        auto const q_specified = RealValueType{2.0};

        UpdateType update{.id = 1, .status = na_IntS, .p_specified = r_nan, .q_specified = r_nan};
        auto expected = update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("Status") {
            SUBCASE("same") { update.status = status; }
            SUBCASE("different") { update.status = IntS{0}; }
            expected.status = status;
        }

        SUBCASE("p_specified") {
            SUBCASE("same") { update.p_specified = p_specified; }
            SUBCASE("different") { update.p_specified = RealValueType{0.0}; }
            expected.p_specified = p_specified;
        }

        SUBCASE("q_specified") {
            SUBCASE("same") { update.q_specified = q_specified; }
            SUBCASE("different") { update.q_specified = RealValueType{0.0}; }
            expected.q_specified = q_specified;
        }

        SUBCASE("multiple") {
            update.status = IntS{0};
            update.p_specified = RealValueType{0.0};
            update.q_specified = RealValueType{0.1};
            expected.status = status;
            expected.p_specified = p_specified;
            expected.q_specified = q_specified;
        }

        for (auto const load_gen_type : {LoadGenType::const_pq, LoadGenType::const_y, LoadGenType::const_i}) {
            LoadGeneratorType const load_gen{{.id = 1,
                                              .node = 2,
                                              .status = status,
                                              .type = load_gen_type,
                                              .p_specified = p_specified,
                                              .q_specified = q_specified},
                                             1.0};
            auto const inv = load_gen.inverse(update);

            CHECK(inv.id == expected.id);
            CHECK(inv.status == expected.status);
            check_nan_preserving_equality(inv.p_specified, expected.p_specified);
            check_nan_preserving_equality(inv.q_specified, expected.q_specified);
        }
    }
}
} // namespace power_grid_model
