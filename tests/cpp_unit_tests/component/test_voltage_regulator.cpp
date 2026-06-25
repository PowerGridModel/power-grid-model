// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/voltage_regulator.hpp>

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/output.hpp>
#include <power_grid_model/auxiliary/update.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/component/base.hpp>

#include <doctest/doctest.h>

#include <complex>
#include <concepts>
#include <cstddef>

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
        VoltageRegulatorOutput const output =
            voltage_regulator.get_output({.limit_violated = 0, .generator_id = 2, .generator_status = 1});
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.limit_violated == 0);
    }

    SUBCASE("Test calc param") {
        auto test_calc_param = []<symmetry_tag sym>(VoltageRegulator const& vr) {
            auto const param = vr.calc_param<sym>();
            CHECK(param.u_ref == 1.05);
            CHECK(param.q_min == 1);
            CHECK(param.q_max == 100.0);
            CHECK(param.status);
        };
        SUBCASE("symmetric") { test_calc_param.operator()<symmetric_t>(voltage_regulator); }
        SUBCASE("asymmetric") { test_calc_param.operator()<asymmetric_t>(voltage_regulator); }
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

            auto test_updated_values = []<symmetry_tag sym>(VoltageRegulator const& vr) {
                auto const param = vr.calc_param<sym>();
                CHECK(param.u_ref == 0.97);
                CHECK(param.q_min == 10.0);
                CHECK(param.q_max == 110.0);
                CHECK_FALSE(param.status);
                CHECK_FALSE(vr.is_energized(true));
                CHECK_FALSE(vr.is_energized(false));
            };
            SUBCASE("symmetric") { test_updated_values.operator()<symmetric_t>(voltage_regulator); }
            SUBCASE("asymmetric") { test_updated_values.operator()<asymmetric_t>(voltage_regulator); }
        }
        SUBCASE("Set nan values") {
            auto test_nan_update = []<symmetry_tag sym>(VoltageRegulator& vr) {
                auto const before_param = vr.calc_param<sym>();
                VoltageRegulatorUpdate const update{.id = 1};
                vr.update(update);
                auto const param = vr.calc_param<sym>();

                CHECK(cabs(param.u_ref - before_param.u_ref) < numerical_tolerance);
                CHECK(param.q_min == doctest::Approx(before_param.q_min));
                CHECK(param.q_max == doctest::Approx(before_param.q_max));
                CHECK(param.status == before_param.status);
            };
            SUBCASE("symmetric") { test_nan_update.operator()<symmetric_t>(voltage_regulator); }
            SUBCASE("asymmetric") { test_nan_update.operator()<asymmetric_t>(voltage_regulator); }
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
