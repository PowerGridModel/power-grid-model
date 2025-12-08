// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/shunt.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
void check_nan_preserving_equality(std::floating_point auto actual, std::floating_point auto expected) {
    if (is_nan(expected)) {
        is_nan(actual);
    } else {
        CHECK(actual == doctest::Approx(expected));
    }
}
} // namespace

TEST_CASE("Test shunt") {
    ShuntInput const shunt_input{.id = 1, .node = 2, .status = 1, .g1 = 1.0, .b1 = 2.0, .g0 = 3.0, .b0 = 4.0};
    Shunt shunt{shunt_input, 10e3};
    double const base_i = base_power_1p / (10.0e3 / sqrt3);
    double const base_y = base_power_3p / 10e3 / 10e3;
    DoubleComplex const y1 = (1.0 + 2.0i) / base_y;
    DoubleComplex const y0 = (3.0 + 4.0i) / base_y;
    DoubleComplex const u{1.0};
    ComplexValue<asymmetric_t> const ua{1.0};
    double const p = 10e3 * 10e3 * 1.0;
    double const q = -10e3 * 10e3 * 2.0;
    double const s = std::sqrt(p * p + q * q);
    double const i = s / 10e3 / sqrt3;
    double const pf = p / s;

    CHECK(shunt.math_model_type() == ComponentType::shunt);

    SUBCASE("test parameters") {
        ComplexTensor<symmetric_t> const y = shunt.calc_param<symmetric_t>();
        CHECK(cabs(y - y1) < numerical_tolerance);
        ComplexTensor<asymmetric_t> ya = shunt.calc_param<asymmetric_t>();
        CHECK(cabs(ya(0, 0) - (2.0 * y1 + y0) / 3.0) < numerical_tolerance);
        CHECK(cabs(ya(0, 1) - (y0 - y1) / 3.0) < numerical_tolerance);
        // no source
        ya = shunt.calc_param<asymmetric_t>(false);
        CHECK(cabs(ya(0, 0)) < numerical_tolerance);
        CHECK(cabs(ya(0, 1)) < numerical_tolerance);
    }

    SUBCASE("test results; u as input") {
        ApplianceOutput<symmetric_t> const sym_result = shunt.get_output<symmetric_t>(u);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(p));
        CHECK(sym_result.q == doctest::Approx(q));
        CHECK(sym_result.s == doctest::Approx(s));
        CHECK(sym_result.i == doctest::Approx(i));
        CHECK(sym_result.pf == doctest::Approx(pf));
        ApplianceOutput<asymmetric_t> asym_result = shunt.get_output<asymmetric_t>(ua);
        CHECK(asym_result.p(0) == doctest::Approx(p / 3));
        CHECK(asym_result.q(1) == doctest::Approx(q / 3));
        CHECK(asym_result.s(2) == doctest::Approx(s / 3));
        CHECK(asym_result.i(0) == doctest::Approx(i));
        CHECK(asym_result.pf(1) == doctest::Approx(pf));
    }

    SUBCASE("Symmetric test results; s, i as input") {
        ApplianceSolverOutput<symmetric_t> appliance_solver_output_sym;
        appliance_solver_output_sym.i = 1.0 + 2.0i;
        appliance_solver_output_sym.s = 3.0 + 4.0i;
        ApplianceOutput<symmetric_t> const sym_result = shunt.get_output<symmetric_t>(appliance_solver_output_sym);
        CHECK(sym_result.id == 1);
        CHECK(sym_result.energized);
        CHECK(sym_result.p == doctest::Approx(-3.0 * base_power<symmetric_t>));
        CHECK(sym_result.q == doctest::Approx(-4.0 * base_power<symmetric_t>));
        CHECK(sym_result.s == doctest::Approx(cabs(3.0 + 4.0i) * base_power<symmetric_t>));
        CHECK(sym_result.i == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(sym_result.pf == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("Asymmetric test results; s, i as input") {
        ApplianceSolverOutput<asymmetric_t> appliance_solver_output_asym;
        ComplexValue<asymmetric_t> const i_a{1.0 + 2.0i};
        ComplexValue<asymmetric_t> const s_a{3.0 + 4.0i, 3.0 + 4.0i, 3.0 + 4.0i};
        appliance_solver_output_asym.i = i_a;
        appliance_solver_output_asym.s = s_a;
        ApplianceOutput<asymmetric_t> asym_result = shunt.get_output<asymmetric_t>(appliance_solver_output_asym);
        CHECK(asym_result.id == 1);
        CHECK(asym_result.energized);
        CHECK(asym_result.p(0) == doctest::Approx(-3.0 * base_power<asymmetric_t>));
        CHECK(asym_result.q(1) == doctest::Approx(-4.0 * base_power<asymmetric_t>));
        CHECK(asym_result.s(2) == doctest::Approx(5.0 * base_power<asymmetric_t>));
        CHECK(asym_result.i(0) == doctest::Approx(cabs(1.0 + 2.0i) * base_i));
        CHECK(asym_result.pf(1) == doctest::Approx(-3.0 / cabs(3.0 + 4.0i)));
    }

    SUBCASE("test change") {
        SUBCASE("status") {
            auto changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = nan, .b1 = nan, .g0 = nan, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(!changed.param);
            changed = shunt.update(ShuntUpdate{.id = 1, .status = 0, .g1 = nan, .b1 = nan, .g0 = nan, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("g1") {
            auto changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = 1.0, .b1 = nan, .g0 = nan, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(!changed.param);
            changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = 10.0, .b1 = nan, .g0 = nan, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("g1") {
            auto changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = nan, .b1 = 2.0, .g0 = nan, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(!changed.param);
            changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = nan, .b1 = 20.0, .g0 = nan, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("g1") {
            auto changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = nan, .b1 = nan, .g0 = 3.0, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(!changed.param);
            changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = nan, .b1 = nan, .g0 = 30.0, .b0 = nan});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("g1") {
            auto changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = nan, .b1 = nan, .g0 = nan, .b0 = 4.0});
            CHECK(!changed.topo);
            CHECK(!changed.param);
            changed = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = nan, .b1 = nan, .g0 = nan, .b0 = 40.0});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("all or none") {
            auto changed_ = shunt.update(ShuntUpdate{.id = 1, .status = 1, .g1 = 1.0, .b1 = 2.0, .g0 = 3.0, .b0 = 4.0});
            CHECK(!changed_.topo);
            CHECK(!changed_.param);
            changed_ = shunt.update(ShuntUpdate{.id = 1, .status = 0, .g1 = 10.0, .b1 = 20.0, .g0 = 30.0, .b0 = 40.0});
            CHECK(!changed_.topo);
            CHECK(changed_.param);
            changed_ =
                shunt.update(ShuntUpdate{.id = 1, .status = na_IntS, .g1 = nan, .b1 = nan, .g0 = nan, .b0 = nan});
            CHECK(!changed_.topo);
            CHECK(!changed_.param);
        }
    }

    SUBCASE("Update inverse") {
        ShuntUpdate shunt_update{.id = 1, .status = na_IntS, .g1 = nan, .b1 = nan, .g0 = nan, .b0 = nan};
        auto expected = shunt_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("Status") {
            SUBCASE("same") { shunt_update.status = status_to_int(shunt.status()); }
            SUBCASE("different") { shunt_update.status = IntS{0}; }
            expected.status = status_to_int(shunt.status());
        }

        SUBCASE("g1") {
            SUBCASE("same") { shunt_update.g1 = 1.0; }
            SUBCASE("different") { shunt_update.g1 = 0.0; }
            expected.g1 = 1.0;
        }

        SUBCASE("b1") {
            SUBCASE("same") { shunt_update.b1 = 2.0; }
            SUBCASE("different") { shunt_update.b1 = 0.0; }
            expected.b1 = 2.0;
        }

        SUBCASE("g0") {
            SUBCASE("same") { shunt_update.g0 = 3.0; }
            SUBCASE("different") { shunt_update.g0 = 0.0; }
            expected.g0 = 3.0;
        }

        SUBCASE("b0") {
            SUBCASE("same") { shunt_update.b0 = 4.0; }
            SUBCASE("different") { shunt_update.b0 = 0.0; }
            expected.b0 = 4.0;
        }

        SUBCASE("multiple") {
            shunt_update.status = IntS{0};
            shunt_update.g1 = 0.0;
            shunt_update.b1 = 0.1;
            shunt_update.g0 = 0.2;
            shunt_update.b0 = 0.3;
            expected.status = status_to_int(shunt.status());
            expected.g1 = 1.0;
            expected.b1 = 2.0;
            expected.g0 = 3.0;
            expected.b0 = 4.0;
        }

        auto const inv = shunt.inverse(shunt_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.status == expected.status);
        check_nan_preserving_equality(inv.g1, expected.g1);
        check_nan_preserving_equality(inv.b1, expected.b1);
        check_nan_preserving_equality(inv.g0, expected.g0);
        check_nan_preserving_equality(inv.b0, expected.b0);
    }
}

} // namespace power_grid_model
