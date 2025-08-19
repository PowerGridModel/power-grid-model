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
using TerminalAndAngleTypePair = std::pair<MeasuredTerminalType, AngleMeasurementType>;

auto const terminal_and_angle_measurement_types = [] {
    std::vector<TerminalAndAngleTypePair> result;
    for (auto const terminal_type :
         {MeasuredTerminalType::branch_from, MeasuredTerminalType::branch_to, MeasuredTerminalType::branch3_1,
          MeasuredTerminalType::branch3_2, MeasuredTerminalType::branch3_3}) {
        for (auto const angle_measurement_type :
             {AngleMeasurementType::global_angle, AngleMeasurementType::local_angle}) {
            result.emplace_back(terminal_type, angle_measurement_type);
        }
    }
    return result;
}();
} // namespace

TEST_CASE("Test current sensor") {
    SUBCASE("Symmetric Current Sensor") {
        for (auto const& [terminal_type, angle_measurement_type] : terminal_and_angle_measurement_types) {
            CAPTURE(terminal_type);
            CAPTURE(angle_measurement_type);

            CurrentSensorInput<symmetric_t> sym_current_sensor_input{};
            sym_current_sensor_input.id = 0;
            sym_current_sensor_input.measured_object = 1;
            sym_current_sensor_input.measured_terminal_type = terminal_type;
            sym_current_sensor_input.angle_measurement_type = angle_measurement_type;
            sym_current_sensor_input.i_sigma = 1.0;
            sym_current_sensor_input.i_measured = 1.0 * 1e3;
            sym_current_sensor_input.i_angle_measured = pi / 4.;
            sym_current_sensor_input.i_angle_sigma = 0.2;

            double const u_rated = 10.0e3;
            double const base_current = base_power_3p / u_rated / sqrt3;
            double const i_pu = 1.0e3 / base_current;
            double const i_sigma_pu = 1.0 / base_current;
            double const i_variance_pu = i_sigma_pu * i_sigma_pu;
            double const i_angle = pi / 4.;
            double const i_angle_sigma_pi = 0.2;
            double const i_angle_variance_pu = i_angle_sigma_pi * i_angle_sigma_pi;

            CurrentSensor<symmetric_t> const sym_current_sensor{sym_current_sensor_input, u_rated};
            CHECK(sym_current_sensor.get_terminal_type() == terminal_type);
            CHECK(sym_current_sensor.get_angle_measurement_type() == angle_measurement_type);

            SUBCASE("Output for symmetric parameters") {
                auto const i_sym = ComplexValue<symmetric_t>{(1e3 * exp(1.0i * i_angle)) / base_current};
                CurrentSensorCalcParam<symmetric_t> const sym_sensor_param =
                    sym_current_sensor.calc_param<symmetric_t>();
                CurrentSensorOutput<symmetric_t> const sym_sensor_output =
                    (angle_measurement_type == AngleMeasurementType::global_angle)
                        ? sym_current_sensor.get_output<symmetric_t>(i_sym, ComplexValue<symmetric_t>{1.0})
                        : sym_current_sensor.get_output<symmetric_t>(conj(i_sym), ComplexValue<symmetric_t>{1.0});

                // Check symmetric sensor output for symmetric parameters
                CHECK(sym_sensor_param.angle_measurement_type == angle_measurement_type);
                // Var(I_Re) ≈ Var(I) * cos^2(pi/4) + Var(θ) * I^2 * sin^2(pi/4)
                CHECK(sym_sensor_param.measurement.real_component.variance ==
                      doctest::Approx(0.5 * (i_variance_pu + i_angle_variance_pu * i_pu * i_pu)));
                // Var(I_Im) ≈ Var(I) * sin^2(pi/4) + Var(θ) * I^2 * cos^2(pi/4)
                CHECK(sym_sensor_param.measurement.imag_component.variance ==
                      doctest::Approx(0.5 * (i_variance_pu + i_angle_variance_pu * i_pu * i_pu)));
                CHECK(real(sym_sensor_param.measurement.value()) == doctest::Approx(i_pu * cos(i_angle)));
                CHECK(imag(sym_sensor_param.measurement.value()) == doctest::Approx(i_pu * sin(i_angle)));

                CHECK(sym_sensor_output.id == 0);
                CHECK(sym_sensor_output.energized == 1);
                CHECK(sym_sensor_output.i_residual == doctest::Approx(0.0));
                CHECK(sym_sensor_output.i_angle_residual == doctest::Approx(0.0));
            }

            SUBCASE("Output for asymmetric parameters") {
                auto const i_asym = ComplexValue<asymmetric_t>{(1e3 * exp(1.0i * i_angle)) / base_current,
                                                               (1e3 * exp(1.0i * (i_angle + deg_240))) / base_current,
                                                               (1e3 * exp(1.0i * (i_angle + deg_120))) / base_current};
                auto const i_asym_local = ComplexValue<asymmetric_t>{(1e3 * exp(1.0i * i_angle)) / base_current,
                                                                     (1e3 * exp(1.0i * i_angle)) / base_current,
                                                                     (1e3 * exp(1.0i * i_angle)) / base_current};
                CurrentSensorCalcParam<asymmetric_t> asym_sensor_param = sym_current_sensor.calc_param<asymmetric_t>();
                CurrentSensorOutput<asymmetric_t> const sym_sensor_output_asym_param =
                    (angle_measurement_type == AngleMeasurementType::global_angle)
                        ? sym_current_sensor.get_output<asymmetric_t>(i_asym, ComplexValue<asymmetric_t>{1.0})
                        : sym_current_sensor.get_output<asymmetric_t>(conj(i_asym_local),
                                                                      ComplexValue<asymmetric_t>{1.0});

                // Check symmetric sensor output for asymmetric parameters
                CHECK(asym_sensor_param.angle_measurement_type == angle_measurement_type);

                CHECK(asym_sensor_param.measurement.real_component.variance[0] ==
                      doctest::Approx(0.5 * (i_variance_pu + i_angle_variance_pu * i_pu * i_pu)));
                auto const shifted_i_angle = i_angle + deg_240;
                CHECK(asym_sensor_param.measurement.imag_component.variance[1] ==
                      doctest::Approx(i_variance_pu * sin(shifted_i_angle) * sin(shifted_i_angle) +
                                      i_angle_variance_pu * i_pu * i_pu * cos(shifted_i_angle) * cos(shifted_i_angle)));
                CHECK(real(asym_sensor_param.measurement.value()[0]) == doctest::Approx(i_pu * cos(i_angle)));
                CHECK(imag(asym_sensor_param.measurement.value()[1]) == doctest::Approx(i_pu * sin(shifted_i_angle)));

                CHECK(sym_sensor_output_asym_param.id == 0);
                CHECK(sym_sensor_output_asym_param.energized == 1);
                for (auto phase : IdxRange{3}) {
                    CAPTURE(phase);
                    CHECK(sym_sensor_output_asym_param.i_residual[phase] == doctest::Approx(0.0));
                    CHECK(sym_sensor_output_asym_param.i_angle_residual[phase] == doctest::Approx(0.0));
                }
            }
        }
        SUBCASE("Wrong measured terminal type") {
            for (auto const& terminal_type :
                 {MeasuredTerminalType::source, MeasuredTerminalType::shunt, MeasuredTerminalType::load,
                  MeasuredTerminalType::generator, MeasuredTerminalType::node}) {
                for (auto const angle_measurement_type :
                     {AngleMeasurementType::global_angle, AngleMeasurementType::local_angle}) {
                    CAPTURE(terminal_type);
                    CAPTURE(angle_measurement_type);

                    CHECK_THROWS_AS((CurrentSensor<symmetric_t>{
                                        {1, 1, terminal_type, angle_measurement_type, 1.0, 1.0, 1.0, 1.0}, 1.0}),
                                    InvalidMeasuredTerminalType);
                }
            }
        }
        SUBCASE("Symmetric calculation parameters") {
            double const u_rated = 10.0e3;
            double const base_current = base_power_3p / u_rated / sqrt3;
            for (auto const& [terminal_type, angle_measurement_type] : terminal_and_angle_measurement_types) {
                CurrentSensor<symmetric_t> sym_current_sensor{{.id = 1,
                                                               .measured_object = 1,
                                                               .measured_terminal_type = terminal_type,
                                                               .angle_measurement_type = angle_measurement_type},
                                                              u_rated};

                SUBCASE("No phase shift") {
                    sym_current_sensor.update(
                        {.id = 1, .i_sigma = 1.0, .i_angle_sigma = 0.2, .i_measured = 1.0, .i_angle_measured = 0.0});
                    auto const sym_param = sym_current_sensor.calc_param<symmetric_t>();

                    CHECK(sym_param.angle_measurement_type == angle_measurement_type);
                    CHECK(sym_param.measurement.real_component.variance == doctest::Approx(pow(1.0 / base_current, 2)));
                    CHECK(sym_param.measurement.imag_component.variance == doctest::Approx(pow(0.2 / base_current, 2)));
                    CHECK(real(sym_param.measurement.value()) == doctest::Approx(1.0 / base_current));
                    CHECK(imag(sym_param.measurement.value()) == doctest::Approx(0.0 / base_current));
                }

                SUBCASE("90deg phase shift") {
                    sym_current_sensor.update(
                        {.id = 1, .i_sigma = 1.0, .i_angle_sigma = 0.2, .i_measured = 1.0, .i_angle_measured = pi / 2});
                    auto const sym_param = sym_current_sensor.calc_param<symmetric_t>();

                    CHECK(sym_param.angle_measurement_type == angle_measurement_type);
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

                    CHECK(sym_param.angle_measurement_type == angle_measurement_type);
                    CHECK(sym_param.measurement.real_component.variance ==
                          doctest::Approx(1.04 / 2.0 / (base_current * base_current)));
                    CHECK(sym_param.measurement.imag_component.variance ==
                          doctest::Approx(sym_param.measurement.real_component.variance));
                    CHECK(real(sym_param.measurement.value()) == doctest::Approx(inv_sqrt2 / base_current));
                    CHECK(imag(sym_param.measurement.value()) == doctest::Approx(real(sym_param.measurement.value())));
                }
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
                                                         .angle_measurement_type = AngleMeasurementType::global_angle,
                                                         .i_sigma = i_sigma,
                                                         .i_angle_sigma = i_angle_sigma,
                                                         .i_measured = i_measured,
                                                         .i_angle_measured = i_angle_measured},
                                                        u_rated};
        CurrentSensorUpdate<symmetric_t> cs_update{
            .id = 1, .i_sigma = nan, .i_angle_sigma = nan, .i_measured = nan, .i_angle_measured = nan};
        auto expected = cs_update;
        CAPTURE(expected.id);

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
                                                          .measured_terminal_type = MeasuredTerminalType::branch_from,
                                                          .angle_measurement_type = AngleMeasurementType::global_angle,
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
