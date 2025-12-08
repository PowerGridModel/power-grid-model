// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/transformer_tap_regulator.hpp>

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

TEST_CASE("Test transformer tap regulator") {
    TransformerTapRegulatorInput const input{.id = 1,
                                             .regulated_object = 2,
                                             .status = 1,
                                             .control_side = ControlSide::from,
                                             .u_set = 10.0e3,
                                             .u_band = 1.0e3,
                                             .line_drop_compensation_r = 1.0,
                                             .line_drop_compensation_x = 2.0};

    double const u_rated{10.0e3};

    TransformerTapRegulator transformer_tap_regulator{input, ComponentType::branch, u_rated};

    SUBCASE("Test energized") {
        CHECK(transformer_tap_regulator.energized(true));
        CHECK(transformer_tap_regulator.energized(false));
    }

    SUBCASE("Test regulated object") { CHECK(transformer_tap_regulator.regulated_object() == ID{2}); }

    SUBCASE("Test regulated object type") {
        CHECK(transformer_tap_regulator.regulated_object_type() == ComponentType::branch);
    }

    SUBCASE("Test math model type") { CHECK(transformer_tap_regulator.math_model_type() == ComponentType::regulator); }

    SUBCASE("Test control side") { CHECK(transformer_tap_regulator.control_side() == ControlSide::from); }

    SUBCASE("Test status") { CHECK(transformer_tap_regulator.status()); }

    SUBCASE("Test get_output") {
        TransformerTapRegulatorOutput const output = transformer_tap_regulator.get_output(10);
        CHECK(output.id == 1);
        CHECK(output.tap_pos == 10);
    }

    SUBCASE("Test short circuit output") {
        RegulatorShortCircuitOutput const sc_output = transformer_tap_regulator.get_null_sc_output();
        CHECK(sc_output.id == 1);
        CHECK(sc_output.energized == 0);
    }

    SUBCASE("Test update") {
        SUBCASE("Set all values") {
            TransformerTapRegulatorUpdate const update{.id = 1,
                                                       .status = 0,
                                                       .u_set = 11.0e3,
                                                       .u_band = 2.0e3,
                                                       .line_drop_compensation_r = 2.0,
                                                       .line_drop_compensation_x = 4.0};

            transformer_tap_regulator.update(update);

            TransformerTapRegulatorCalcParam const param = transformer_tap_regulator.calc_param<symmetric_t>();
            double const u_set_expected{1.1};
            double const u_band_expected{0.2};
            double const z_base = u_rated * u_rated / 1e6;
            double const r_pu = 2.0 / z_base;
            double const x_pu = 4.0 / z_base;
            DoubleComplex const z_compensation_expected{r_pu, x_pu};

            CHECK(param.u_set == doctest::Approx(u_set_expected));
            CHECK(param.u_band == doctest::Approx(u_band_expected));
            CHECK(cabs(param.z_compensation - z_compensation_expected) < numerical_tolerance);
            CHECK_FALSE(param.status);
        }
        SUBCASE("Set nan values") {
            TransformerTapRegulatorCalcParam const before_param = transformer_tap_regulator.calc_param<symmetric_t>();

            TransformerTapRegulatorUpdate const update{.id = 1};
            transformer_tap_regulator.update(update);

            TransformerTapRegulatorCalcParam const param = transformer_tap_regulator.calc_param<symmetric_t>();

            CHECK(param.u_set == doctest::Approx(before_param.u_set));
            CHECK(param.u_band == doctest::Approx(before_param.u_band));
            CHECK(cabs(param.z_compensation - before_param.z_compensation) < numerical_tolerance);
            CHECK(param.status == before_param.status);
        }
    }

    SUBCASE("Test update inverse") {
        TransformerTapRegulatorUpdate update{.id = 1,
                                             .status = na_IntS,
                                             .u_set = nan,
                                             .u_band = nan,
                                             .line_drop_compensation_r = nan,
                                             .line_drop_compensation_x = nan};
        auto expected = update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("Status") {
            SUBCASE("same") { update.status = status_to_int(transformer_tap_regulator.status()); }
            SUBCASE("different") { update.status = IntS{0}; }
            expected.status = status_to_int(transformer_tap_regulator.status());
        }

        SUBCASE("u_set") {
            SUBCASE("same") { update.u_set = input.u_set; }
            SUBCASE("different") { update.u_set = 11.0e3; }
            expected.u_set = input.u_set;
        }

        SUBCASE("u_band") {
            SUBCASE("same") { update.u_band = input.u_band; }
            SUBCASE("different") { update.u_band = 2.0e3; }
            expected.u_band = input.u_band;
        }

        SUBCASE("line_drop_compensation_r") {
            SUBCASE("same") { update.line_drop_compensation_r = input.line_drop_compensation_r; }
            SUBCASE("different") { update.line_drop_compensation_r = 0.0; }
            expected.line_drop_compensation_r = input.line_drop_compensation_r;
        }

        SUBCASE("line_drop_compensation_x") {
            SUBCASE("same") { update.line_drop_compensation_x = input.line_drop_compensation_x; }
            SUBCASE("different") { update.line_drop_compensation_x = 0.0; }
            expected.line_drop_compensation_x = input.line_drop_compensation_x;
        }

        SUBCASE("multiple") {
            update.id = 1;
            update.status = 0;
            update.u_set = 11.0e3;
            update.u_band = 2.0e3;
            update.line_drop_compensation_r = 2.0;
            update.line_drop_compensation_x = 4.0;
            expected.status = status_to_int(transformer_tap_regulator.status());
            expected.u_set = input.u_set;
            expected.u_band = input.u_band;
            expected.line_drop_compensation_r = input.line_drop_compensation_r;
            expected.line_drop_compensation_x = input.line_drop_compensation_x;
        }

        auto const inv = transformer_tap_regulator.inverse(update);

        CHECK(inv.id == expected.id);
        CHECK(inv.status == expected.status);
        check_nan_preserving_equality(inv.u_set, expected.u_set);
        check_nan_preserving_equality(inv.u_band, expected.u_band);
        check_nan_preserving_equality(inv.line_drop_compensation_r, expected.line_drop_compensation_r);
        check_nan_preserving_equality(inv.line_drop_compensation_x, expected.line_drop_compensation_x);
    }

    SUBCASE("Test calc param - sym") {
        TransformerTapRegulatorCalcParam const param = transformer_tap_regulator.calc_param<symmetric_t>();

        double const u_set_expected{1.0};
        double const u_band_expected{0.1};
        double const z_base = u_rated * u_rated / 1e6;
        double const r_pu = 1.0 / z_base;
        double const x_pu = 2.0 / z_base;
        DoubleComplex const z_compensation_expected{r_pu, x_pu};

        CHECK(param.u_set == doctest::Approx(u_set_expected));
        CHECK(param.u_band == doctest::Approx(u_band_expected));
        CHECK(cabs(param.z_compensation - z_compensation_expected) < numerical_tolerance);
        CHECK(param.status);
    }

    SUBCASE("Test calc param - asym") {
        TransformerTapRegulatorCalcParam const param = transformer_tap_regulator.calc_param<asymmetric_t>();

        double const u_set_expected{1.0};
        double const u_band_expected{0.1};
        double const z_base = u_rated * u_rated / (1e6 / 3.0);
        double const r_pu = 1.0 / z_base;
        double const x_pu = 2.0 / z_base;
        DoubleComplex const z_compensation_expected{r_pu, x_pu};

        CHECK(param.u_set == doctest::Approx(u_set_expected));
        CHECK(param.u_band == doctest::Approx(u_band_expected));
        CHECK(cabs(param.z_compensation - z_compensation_expected) < numerical_tolerance);
        CHECK(param.status);
    }

    SUBCASE("Test default line drop compensation") {
        TransformerTapRegulator const regulator{{.id = 1,
                                                 .regulated_object = 2,
                                                 .status = 1,
                                                 .control_side = ControlSide::from,
                                                 .u_set = 10.0e3,
                                                 .u_band = 1.0e3},
                                                ComponentType::branch,
                                                u_rated};

        TransformerTapRegulatorCalcParam const param = regulator.calc_param<symmetric_t>();
        CHECK(!is_nan(param.z_compensation));
        CHECK(param.z_compensation.real() == doctest::Approx(0.0));
        CHECK(param.z_compensation.imag() == doctest::Approx(0.0));
    }
}

} // namespace power_grid_model
