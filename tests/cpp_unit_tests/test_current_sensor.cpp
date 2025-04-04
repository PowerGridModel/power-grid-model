// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/current_sensor.hpp>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("test_current_sensor");

namespace power_grid_model {
namespace {
auto const r_nan = RealValue<asymmetric_t>{nan};

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

TEST_CASE("Test current sensor") {
    SUBCASE("Symmetric Current Sensor") {
        for (auto const terminal_type :
             {MeasuredTerminalType::branch_from, MeasuredTerminalType::branch_to, MeasuredTerminalType::branch3_1,
              MeasuredTerminalType::branch3_2, MeasuredTerminalType::branch3_3}) {
            CAPTURE(terminal_type);

            CurrentSensorInput<symmetric_t> sym_current_sensor_input{};
            sym_current_sensor_input.id = 0;
            sym_current_sensor_input.measured_object = 1;
            sym_current_sensor_input.measured_terminal_type = terminal_type;
            sym_current_sensor_input.angle_measurement_type = AngleMeasurementType::local_angle;
            sym_current_sensor_input.i_sigma = 1.0;
            sym_current_sensor_input.i_measured = 1.0 * 1e3;
            sym_current_sensor_input.i_angle_measured = 0.0;
            sym_current_sensor_input.i_angle_sigma = 0.2;

            double const u_rated = 10.0e3;
            double const base_current = base_power_3p / u_rated / sqrt3;
            double const i_pu = 1.0e3 / base_current;
            double const i_sigma_pu = 1.0 / base_current;
            double const i_variance_pu = pow(i_sigma_pu, 2);
            double const i_angle_variance_pu = pow(0.2, 2);

            ComplexValue<symmetric_t> const i_sym = (1.0 * 1e3 + 1i * 0.0) / base_current;
            ComplexValue<asymmetric_t> const i_asym = i_sym * RealValue<asymmetric_t>{1.0};

            CurrentSensor<symmetric_t> const sym_current_sensor{sym_current_sensor_input, u_rated};

            CurrentSensorCalcParam<symmetric_t> sym_sensor_param = sym_current_sensor.calc_param<symmetric_t>();
            CurrentSensorCalcParam<asymmetric_t> asym_sensor_param = sym_current_sensor.calc_param<asymmetric_t>();

            CurrentSensorOutput<symmetric_t> const sym_sensor_output =
                sym_current_sensor.get_output<symmetric_t>(i_sym);
            CurrentSensorOutput<asymmetric_t> sym_sensor_output_asym_param =
                sym_current_sensor.get_output<asymmetric_t>(i_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.angle_measurement_type == AngleMeasurementType::local_angle);
            CHECK(sym_sensor_param.measurement.real_component.variance == doctest::Approx(i_variance_pu));
            CHECK(sym_sensor_param.measurement.imag_component.variance ==
                  doctest::Approx(i_angle_variance_pu * i_pu * i_pu));
            CHECK(real(sym_sensor_param.measurement.value()) == doctest::Approx(i_pu));
            CHECK(imag(sym_sensor_param.measurement.value()) == doctest::Approx(0.0));

            CHECK(sym_sensor_output.id == 0);
            CHECK(sym_sensor_output.energized == 1);
            CHECK(sym_sensor_output.i_residual == doctest::Approx(0.0));
            CHECK(sym_sensor_output.i_angle_residual == doctest::Approx(0.0));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.measurement.real_component.variance[0] == doctest::Approx(i_variance_pu));
            CHECK(asym_sensor_param.measurement.imag_component.variance[1] ==
                  doctest::Approx(i_variance_pu * sin(deg_240) * sin(deg_240) +
                                  i_angle_variance_pu * i_pu * i_pu * cos(deg_240) * cos(deg_240)));
            CHECK(real(asym_sensor_param.measurement.value()[0]) == doctest::Approx(i_pu));
            CHECK(imag(asym_sensor_param.measurement.value()[1]) == doctest::Approx(i_pu * sin(deg_240)));

            CHECK(sym_sensor_output_asym_param.id == 0);
            CHECK(sym_sensor_output_asym_param.energized == 1);
            CHECK(sym_sensor_output_asym_param.i_residual[0] == doctest::Approx(0.0));
            CHECK(sym_sensor_output_asym_param.i_angle_residual[1] == doctest::Approx(0.0));

            CHECK(sym_current_sensor.get_terminal_type() == terminal_type);

            CHECK(sym_current_sensor.get_angle_measurement_type() == AngleMeasurementType::local_angle);
        }
        SUBCASE("Wrong measured terminal type") {
            for (auto const terminal_type :
                 {MeasuredTerminalType::source, MeasuredTerminalType::shunt, MeasuredTerminalType::load,
                  MeasuredTerminalType::generator, MeasuredTerminalType::node}) {
                CHECK_THROWS_AS((CurrentSensor<symmetric_t>{
                                    {1, 1, terminal_type, AngleMeasurementType::local_angle, 1.0, 1.0, 1.0, 1.0}, 1.0}),
                                InvalidMeasuredTerminalType);
            }
        }
        SUBCASE("Symmetric calculation parameters") {
            double const u_rated = 10.0e3;
            double const base_current = base_power_3p / u_rated / sqrt3;

            CurrentSensor<symmetric_t> sym_current_sensor{{.id = 1,
                                                           .measured_object = 1,
                                                           .measured_terminal_type = MeasuredTerminalType::branch3_1,
                                                           .angle_measurement_type = AngleMeasurementType::local_angle},
                                                          u_rated};

            SUBCASE("No phase shift") {
                sym_current_sensor.update(
                    {.id = 1, .i_sigma = 1.0, .i_angle_sigma = 0.2, .i_measured = 1.0, .i_angle_measured = 0.0});
                auto const sym_param = sym_current_sensor.calc_param<symmetric_t>();

                CHECK(sym_param.angle_measurement_type == AngleMeasurementType::local_angle);
                CHECK(sym_param.measurement.real_component.variance == doctest::Approx(pow(1.0 / base_current, 2)));
                CHECK(sym_param.measurement.imag_component.variance == doctest::Approx(pow(0.2 / base_current, 2)));
                CHECK(real(sym_param.measurement.value()) == doctest::Approx(1.0 / base_current));
                CHECK(imag(sym_param.measurement.value()) == doctest::Approx(0.0 / base_current));
            }

            SUBCASE("90deg phase shift") {
                sym_current_sensor.update(
                    {.id = 1, .i_sigma = 1.0, .i_angle_sigma = 0.2, .i_measured = 1.0, .i_angle_measured = pi / 2});
                auto const sym_param = sym_current_sensor.calc_param<symmetric_t>();

                CHECK(sym_param.angle_measurement_type == AngleMeasurementType::local_angle);
                CHECK(sym_param.measurement.real_component.variance == doctest::Approx(pow(0.2 / base_current, 2)));
                CHECK(sym_param.measurement.imag_component.variance == doctest::Approx(pow(1.0 / base_current, 2)));
                CHECK(real(sym_param.measurement.value()) == doctest::Approx(0.0 / base_current));
                CHECK(imag(sym_param.measurement.value()) == doctest::Approx(1.0 / base_current));
            }

            SUBCASE("45deg phase shift") {
                using std::numbers::sqrt2;
                constexpr auto inv_sqrt2 = sqrt2 / 2;

                sym_current_sensor.update(
                    {.id = 1, .i_sigma = 1.0, .i_angle_sigma = 0.2, .i_measured = 1.0, .i_angle_measured = pi / 4});
                auto const sym_param = sym_current_sensor.calc_param<symmetric_t>();

                CHECK(sym_param.angle_measurement_type == AngleMeasurementType::local_angle);
                CHECK(sym_param.measurement.real_component.variance ==
                      doctest::Approx(1.04 / 2.0 / (base_current * base_current)));
                CHECK(sym_param.measurement.imag_component.variance ==
                      doctest::Approx(sym_param.measurement.real_component.variance));
                CHECK(real(sym_param.measurement.value()) == doctest::Approx(inv_sqrt2 / base_current));
                CHECK(imag(sym_param.measurement.value()) == doctest::Approx(real(sym_param.measurement.value())));
            }
        }
    }
    SUBCASE("Update inverse - sym") {
        constexpr auto i_measured = 1.0;
        constexpr auto i_angle_measured = 2.0;
        constexpr auto i_sigma = 3.0;
        constexpr auto i_angle_sigma = 4.0;
        constexpr auto u_rated = 10.0e3;
        CurrentSensor<symmetric_t> const current_sensor{{.id = 1,
                                                         .measured_object = 1,
                                                         .measured_terminal_type = MeasuredTerminalType::branch3_1,
                                                         .angle_measurement_type = AngleMeasurementType::local_angle,
                                                         .i_sigma = i_sigma,
                                                         .i_angle_sigma = i_angle_sigma,
                                                         .i_measured = i_measured,
                                                         .i_angle_measured = i_angle_measured},
                                                        u_rated};

        CurrentSensorUpdate<symmetric_t> cs_update{
            .id = 1, .i_sigma = nan, .i_angle_sigma = nan, .i_measured = nan, .i_angle_measured = nan};
        auto expected = cs_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("i_sigma") {
            SUBCASE("same") { cs_update.i_sigma = i_sigma; }
            SUBCASE("different") { cs_update.i_sigma = 0.0; }
            expected.i_sigma = i_sigma;
        }

        SUBCASE("i_angle_sigma") {
            SUBCASE("same") { cs_update.i_angle_sigma = i_angle_sigma; }
            SUBCASE("different") { cs_update.i_angle_sigma = 0.0; }
            expected.i_angle_sigma = i_angle_sigma;
        }

        SUBCASE("i_measured") {
            SUBCASE("same") { cs_update.i_measured = i_measured; }
            SUBCASE("different") { cs_update.i_measured = 0.0; }
            expected.i_measured = i_measured;
        }

        SUBCASE("i_angle_measured") {
            SUBCASE("same") { cs_update.i_angle_measured = i_angle_measured; }
            SUBCASE("different") { cs_update.i_angle_measured = 0.0; }
            expected.i_angle_measured = i_angle_measured;
        }

        SUBCASE("multiple") {
            cs_update.i_sigma = 0.0;
            cs_update.i_angle_sigma = 0.0;
            cs_update.i_measured = 0.0;
            cs_update.i_angle_measured = 0.0;
            expected.i_sigma = i_sigma;
            expected.i_angle_sigma = i_angle_sigma;
            expected.i_measured = i_measured;
            expected.i_angle_measured = i_angle_measured;
        }

        auto const inv = current_sensor.inverse(cs_update);

        CHECK(inv.id == expected.id);
        check_nan_preserving_equality(inv.i_sigma, expected.i_sigma);
        check_nan_preserving_equality(inv.i_angle_sigma, expected.i_angle_sigma);
        check_nan_preserving_equality(inv.i_measured, expected.i_measured);
        check_nan_preserving_equality(inv.i_angle_measured, expected.i_angle_measured);
    }

    SUBCASE("Update inverse - asym") {
        RealValue<asymmetric_t> i_measured = {1.0, 2.0, 3.0};
        RealValue<asymmetric_t> i_angle_measured = {4.0, 5.0, 6.0};
        constexpr auto i_sigma = 3.0;
        constexpr auto i_angle_sigma = 4.0;
        constexpr auto u_rated = 10.0e3;
        MeasuredTerminalType const measured_terminal_type = MeasuredTerminalType::branch_from;

        CurrentSensorUpdate<asymmetric_t> cs_update{
            .id = 1, .i_sigma = nan, .i_angle_sigma = nan, .i_measured = r_nan, .i_angle_measured = r_nan};
        auto expected = cs_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("i_sigma") {
            SUBCASE("same") { cs_update.i_sigma = i_sigma; }
            SUBCASE("different") { cs_update.i_sigma = 0.0; }
            expected.i_sigma = i_sigma;
        }

        SUBCASE("i_angle_sigma") {
            SUBCASE("same") { cs_update.i_angle_sigma = i_angle_sigma; }
            SUBCASE("different") { cs_update.i_angle_sigma = 0.0; }
            expected.i_angle_sigma = i_angle_sigma;
        }

        SUBCASE("i_measured") {
            SUBCASE("same") { cs_update.i_measured = i_measured; }
            SUBCASE("1 different") {
                cs_update.i_measured = {0.0, nan, nan};
                expected.i_measured = {i_measured(0), nan, nan};
            }
            SUBCASE("all different") {
                cs_update.i_measured = {0.0, 0.1, 0.2};
                expected.i_measured = i_measured;
            }
        }

        SUBCASE("i_angle_measured") {
            SUBCASE("same") { cs_update.i_angle_measured = i_angle_measured; }
            SUBCASE("1 different") {
                cs_update.i_angle_measured = {0.0, nan, nan};
                expected.i_angle_measured = {i_angle_measured(0), nan, nan};
            }
            SUBCASE("all different") {
                cs_update.i_angle_measured = {0.0, 0.1, 0.2};
                expected.i_angle_measured = i_angle_measured;
            }
        }

        SUBCASE("multiple") {
            cs_update.i_sigma = 0.0;
            cs_update.i_angle_sigma = 0.1;
            cs_update.i_measured = {0.0, 0.2, 0.4};
            cs_update.i_angle_measured = {0.0, 0.3, 0.6};
            expected.i_sigma = i_sigma;
            expected.i_angle_sigma = i_angle_sigma;
            expected.i_measured = i_measured;
            expected.i_angle_measured = i_angle_measured;
        }

        CurrentSensor<asymmetric_t> const current_sensor{{.id = 1,
                                                          .measured_object = 1,
                                                          .measured_terminal_type = measured_terminal_type,
                                                          .angle_measurement_type = AngleMeasurementType::local_angle,
                                                          .i_sigma = i_sigma,
                                                          .i_angle_sigma = i_angle_sigma,
                                                          .i_measured = i_measured,
                                                          .i_angle_measured = i_angle_measured},
                                                         u_rated};

        auto const inv = current_sensor.inverse(cs_update);

        CHECK(inv.id == expected.id);
        check_nan_preserving_equality(inv.i_sigma, expected.i_sigma);
        check_nan_preserving_equality(inv.i_angle_sigma, expected.i_angle_sigma);
        check_nan_preserving_equality(inv.i_measured, expected.i_measured);
        check_nan_preserving_equality(inv.i_angle_measured, expected.i_angle_measured);
    }
}

} // namespace power_grid_model

TEST_SUITE_END();
