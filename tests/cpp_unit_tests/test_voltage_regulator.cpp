// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/voltage_regulator.hpp>

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

TEST_CASE("Test voltage regulator") {
    VoltageRegulatorInput const input{
        .id = 1, .regulated_object = 2, .status = 1, .u_ref = 1.05, .q_min = 1e6, .q_max = 100e6};

    VoltageRegulator voltage_regulator{input, ComponentType::generic_load_gen};

    SUBCASE("Test energized") {
        CHECK(voltage_regulator.is_energized(true));
        CHECK_FALSE(voltage_regulator.is_energized(false));
    }

    SUBCASE("Test regulated object") { CHECK(voltage_regulator.regulated_object() == ID{2}); }

    SUBCASE("Test regulated object type") {
        CHECK(voltage_regulator.regulated_object_type() == ComponentType::generic_load_gen);
    }

    SUBCASE("Test status") { CHECK(voltage_regulator.status()); }

    SUBCASE("Test u_ref") { CHECK(voltage_regulator.u_ref() == 1.05); }

    SUBCASE("Test q limits") {
        CHECK(voltage_regulator.q_min() == 1e6);
        CHECK(voltage_regulator.q_max() == 100e6);
    }

    SUBCASE("Test get_output") {
        VoltageRegulatorOutput const output = voltage_regulator.get_output(
            {.limit_violated = 0, .generator_id = 2, .generator_status = 1});
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.limit_violated == 0);
    }

    SUBCASE("Test calc param") {
        SUBCASE("symmetric") {
            VoltageRegulatorCalcParam<symmetric_t> const param = voltage_regulator.calc_param<symmetric_t>();

            CHECK(param.u_ref == 1.05);
            CHECK(param.q_min == 1);
            CHECK(param.q_max == 100);
            CHECK(param.status);
        }
        SUBCASE("asymmetric") {
            VoltageRegulatorCalcParam<asymmetric_t> const param = voltage_regulator.calc_param<asymmetric_t>();

            CHECK(param.u_ref == 1.05);
            for (size_t i = 0; i != 3; i++) {
                CHECK(param.q_min(i) == 1);
                CHECK(param.q_max(i) == 100);
            }
            CHECK(param.status);
        }
    }

    SUBCASE("Test short circuit output") {
        RegulatorShortCircuitOutput const sc_output = voltage_regulator.get_null_sc_output();
        CHECK(sc_output.id == 1);
        CHECK(sc_output.energized == 0);
    }

    SUBCASE("Test update") {
        SUBCASE("Set all values") {
            VoltageRegulatorUpdate const update{.id = 1, .status = 0, .u_ref = 0.97, .q_min = 10e6, .q_max = 110e6};
            voltage_regulator.update(update);

            SUBCASE("symmetric") {
                VoltageRegulatorCalcParam<symmetric_t> const param = voltage_regulator.calc_param<symmetric_t>();

                CHECK(param.u_ref == 0.97);
                CHECK(param.q_min == 10.0);
                CHECK(param.q_max == 110.0);
                CHECK_FALSE(param.status);
                CHECK_FALSE(voltage_regulator.is_energized(true));
                CHECK_FALSE(voltage_regulator.is_energized(false));
            }
            SUBCASE("asymmetric") {
                VoltageRegulatorCalcParam<asymmetric_t> const param = voltage_regulator.calc_param<asymmetric_t>();

                CHECK(param.u_ref == 0.97);
                for (size_t i = 0; i != 3; i++) {
                    CHECK(param.q_min(i) == 10.0);
                    CHECK(param.q_max(i) == 110.0);
                }
                CHECK_FALSE(param.status);
                CHECK_FALSE(voltage_regulator.is_energized(true));
                CHECK_FALSE(voltage_regulator.is_energized(false));
            }
        }
        SUBCASE("Set nan values") {
            SUBCASE("symmetric") {
                VoltageRegulatorCalcParam<symmetric_t> const before_param = voltage_regulator.calc_param<symmetric_t>();

                VoltageRegulatorUpdate const update{.id = 1};
                voltage_regulator.update(update);

                VoltageRegulatorCalcParam<symmetric_t> const param = voltage_regulator.calc_param<symmetric_t>();

                CHECK(cabs(param.u_ref - before_param.u_ref) < numerical_tolerance);
                CHECK(param.q_min == doctest::Approx(before_param.q_min));
                CHECK(param.q_max == doctest::Approx(before_param.q_max));
                CHECK(param.status == before_param.status);
            }
            SUBCASE("asymmetric") {
                VoltageRegulatorCalcParam<asymmetric_t> const before_param =
                    voltage_regulator.calc_param<asymmetric_t>();

                VoltageRegulatorUpdate const update{.id = 1};
                voltage_regulator.update(update);

                VoltageRegulatorCalcParam<asymmetric_t> const param = voltage_regulator.calc_param<asymmetric_t>();

                CHECK(cabs(param.u_ref - before_param.u_ref) < numerical_tolerance);
                for (size_t i = 0; i != 3; i++) {
                    CHECK(param.q_min(i) == doctest::Approx(before_param.q_min(i)));
                    CHECK(param.q_max(i) == doctest::Approx(before_param.q_max(i)));
                }
                CHECK(param.status == before_param.status);
            }
        }
    }

    SUBCASE("Test update inverse") {
        VoltageRegulatorUpdate update{.id = 1, .status = na_IntS, .u_ref = nan, .q_min = nan, .q_max = nan};
        auto expected = update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("Status") {
            SUBCASE("same") { update.status = status_to_int(voltage_regulator.status()); }
            SUBCASE("different") { update.status = IntS{0}; }
            expected.status = status_to_int(voltage_regulator.status());
        }

        SUBCASE("u_ref") {
            SUBCASE("same") { update.u_ref = input.u_ref; }
            SUBCASE("different") { update.u_ref = 1.1; }
            expected.u_ref = input.u_ref;
        }

        SUBCASE("q_min") {
            SUBCASE("same") { update.q_min = input.q_min; }
            SUBCASE("different") { update.q_min = 30e6; }
            expected.q_min = input.q_min;
        }

        SUBCASE("q_max") {
            SUBCASE("same") { update.q_max = input.q_max; }
            SUBCASE("different") { update.q_max = 300e6; }
            expected.q_max = input.q_max;
        }

        SUBCASE("multiple") {
            update.id = 1;
            update.status = 0;
            update.u_ref = 1.025;
            update.q_min = 40e6;
            update.q_max = 400e6;
            expected.status = status_to_int(voltage_regulator.status());
            expected.u_ref = input.u_ref;
            expected.q_min = input.q_min;
            expected.q_max = input.q_max;
        }

        auto const inv = voltage_regulator.inverse(update);

        CHECK(inv.id == expected.id);
        CHECK(inv.status == expected.status);
        check_nan_preserving_equality(inv.u_ref, expected.u_ref);
        check_nan_preserving_equality(inv.q_min, expected.q_min);
        check_nan_preserving_equality(inv.q_max, expected.q_max);
    }
}

} // namespace power_grid_model
