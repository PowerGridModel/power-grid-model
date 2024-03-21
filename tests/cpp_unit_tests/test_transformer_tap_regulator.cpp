// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/transformer_tap_regulator.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

TEST_CASE("Test transformer tap regulator") {
    TransformerTapRegulatorInput input{.id = 1,
                                       .regulated_object = 2,
                                       .control_side = ControlSide::from,
                                       .u_set = 10.0e3,
                                       .u_band = 1.0e3,
                                       .status = true,
                                       .line_drop_compensation_r = 1.0,
                                       .line_drop_compensation_x = 2.0};

    double u_rated{10.0e3};

    TransformerTapRegulator transformer_tap_regulator{input, u_rated};

    SUBCASE("Test energized") {
        CHECK(transformer_tap_regulator.energized(true));
        CHECK(transformer_tap_regulator.energized(false));
    }

    SUBCASE("Test math model type") { CHECK(transformer_tap_regulator.math_model_type() == ComponentType::regulator); }

    SUBCASE("Test control side") { CHECK(transformer_tap_regulator.control_side() == ControlSide::from); }

    SUBCASE("Test get_output") {
        TransformerTapRegulatorOutput output = transformer_tap_regulator.get_output(10);
        CHECK(output.id == 1);
        CHECK(output.tap_pos == 10);
    }

    SUBCASE("Test update") {
        TransformerTapRegulatorUpdate update{.id = 1,
                                             .u_set = 11.0e3,
                                             .u_band = 2.0e3,
                                             .status = false,
                                             .line_drop_compensation_r = 2.0,
                                             .line_drop_compensation_x = 4.0};

        transformer_tap_regulator.update(update);

        TransformerTapRegulatorCalcParam param = transformer_tap_regulator.calc_param<symmetric_t>();
        double u_set_expected{1.1};
        double u_band_expected{0.2};
        double z_base = u_rated * u_rated / 1e6;
        double r_pu = 2.0 / z_base;
        double x_pu = 4.0 / z_base;
        DoubleComplex z_compensation_expected{r_pu, x_pu};

        CHECK(param.u_set == doctest::Approx(u_set_expected));
        CHECK(param.u_band == doctest::Approx(u_band_expected));
        CHECK(cabs(param.z_compensation - z_compensation_expected) < numerical_tolerance);
        CHECK_FALSE(param.status);
    }

    SUBCASE("Test calc param - sym") {
        TransformerTapRegulatorCalcParam param = transformer_tap_regulator.calc_param<symmetric_t>();

        double u_set_expected{1.0};
        double u_band_expected{0.1};
        double z_base = u_rated * u_rated / 1e6;
        double r_pu = 1.0 / z_base;
        double x_pu = 2.0 / z_base;
        DoubleComplex z_compensation_expected{r_pu, x_pu};

        CHECK(param.u_set == doctest::Approx(u_set_expected));
        CHECK(param.u_band == doctest::Approx(u_band_expected));
        CHECK(cabs(param.z_compensation - z_compensation_expected) < numerical_tolerance);
        CHECK(param.status);
    }

    SUBCASE("Test calc param - asym") {
        TransformerTapRegulatorCalcParam param = transformer_tap_regulator.calc_param<asymmetric_t>();

        double u_set_expected{1.0};
        double u_band_expected{0.1};
        double z_base = u_rated * u_rated / (1e6 / 3.0);
        double r_pu = 1.0 / z_base;
        double x_pu = 2.0 / z_base;
        DoubleComplex z_compensation_expected{r_pu, x_pu};

        CHECK(param.u_set == doctest::Approx(u_set_expected));
        CHECK(param.u_band == doctest::Approx(u_band_expected));
        CHECK(cabs(param.z_compensation - z_compensation_expected) < numerical_tolerance);
        CHECK(param.status);
    }
}

} // namespace power_grid_model