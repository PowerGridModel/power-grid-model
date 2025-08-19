// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/component/sensor.hpp>
#include <power_grid_model/component/voltage_sensor.hpp>

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

void check_nan_preserving_equality(RealValue<asymmetric_t> const& actual, RealValue<asymmetric_t> const& expected) {
    for (auto i : {0, 1, 2}) {
        CAPTURE(i);
        check_nan_preserving_equality(actual(i), expected(i));
    }
}
} // namespace

TEST_CASE("Test voltage sensor") {
    SUBCASE("Test Sensor energized function") {
        VoltageSensorInput<symmetric_t> const voltage_sensor_input{.id = 0, .measured_object = 1};
        double const u_rated = 10.0e3;
        VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

        CHECK(voltage_sensor.energized(true) == true);
        CHECK(voltage_sensor.energized(false) == true);
    }

    SUBCASE("Test Sensor math_model_type") {
        VoltageSensorInput<symmetric_t> const voltage_sensor_input{.id = 0, .measured_object = 1};
        double const u_rated = 10.0e3;
        VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

        CHECK(voltage_sensor.math_model_type() == ComponentType::sensor);
    }

    SUBCASE("Test get_null_output") {
        VoltageSensorInput<symmetric_t> voltage_sensor_input{.id = 0, .measured_object = 1};
        voltage_sensor_input.id = 12;
        double const u_rated = 10.0e3;
        VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

        VoltageSensorOutput<symmetric_t> const vs_output = voltage_sensor.get_null_output<symmetric_t>();
        CHECK(vs_output.id == 12);
        CHECK(vs_output.energized == 0);
        CHECK(vs_output.u_residual == doctest::Approx(0.0));
        CHECK(vs_output.u_angle_residual == doctest::Approx(0.0));

        SensorShortCircuitOutput const vs_sc_output = voltage_sensor.get_null_sc_output();
        CHECK(vs_sc_output.id == 12);
        CHECK(vs_sc_output.energized == 0);
    }

    SUBCASE("Test voltage sensor update - sym") {
        VoltageSensorInput<symmetric_t> const voltage_sensor_input{.id = 0, .measured_object = 1};
        double const u_rated = 2.0;
        VoltageSensor<symmetric_t> voltage_sensor{voltage_sensor_input, u_rated};

        VoltageSensorUpdate<symmetric_t> vs_update;
        vs_update.id = 0;
        vs_update.u_measured = 1.0;
        vs_update.u_angle_measured = 2.0;
        vs_update.u_sigma = 3.0;

        UpdateChange const update = voltage_sensor.update(vs_update);

        CHECK(update.param == false);
        CHECK(update.topo == false);

        ComplexValue<symmetric_t> const expected_param_value{0.5 * exp(1i * 2.0)};
        VoltageSensorCalcParam<symmetric_t> param = voltage_sensor.calc_param<symmetric_t>();
        CHECK(param.variance == doctest::Approx(2.25));
        CHECK(param.value == expected_param_value);

        // update with nan
        vs_update.u_measured = nan;
        vs_update.u_angle_measured = nan;
        vs_update.u_sigma = nan;

        voltage_sensor.update(vs_update);
        param = voltage_sensor.calc_param<symmetric_t>();
        CHECK(param.variance == doctest::Approx(2.25));
        CHECK(param.value == expected_param_value);
    }

    SUBCASE("Test voltage sensor update - asym") {
        VoltageSensorInput<asymmetric_t> const voltage_sensor_input{.id = 0, .measured_object = 1};
        double const u_rated = 2.0;
        VoltageSensor<asymmetric_t> voltage_sensor{voltage_sensor_input, u_rated};

        VoltageSensorUpdate<asymmetric_t> vs_update;
        vs_update.id = 0;
        vs_update.u_measured = {1.0, 1.1, 1.2};
        vs_update.u_angle_measured = {2.0, 2.1, 2.2};
        vs_update.u_sigma = 3.0;

        UpdateChange update = voltage_sensor.update(vs_update);
        CHECK(update.param == false);
        CHECK(update.topo == false);

        VoltageSensorCalcParam<asymmetric_t> param = voltage_sensor.calc_param<asymmetric_t>();
        CHECK(param.variance == doctest::Approx(6.75));

        ComplexValue<asymmetric_t> expected_param_value{0.5 * sqrt3 * exp(1i * 2.0), 0.55 * sqrt3 * exp(1i * 2.1),
                                                        0.6 * sqrt3 * exp(1i * 2.2)};
        CHECK(cabs(param.value[0]) == doctest::Approx(cabs(expected_param_value[0])));
        CHECK(cabs(param.value[1]) == doctest::Approx(cabs(expected_param_value[1])));
        CHECK(cabs(param.value[2]) == doctest::Approx(cabs(expected_param_value[2])));

        // update with nan's
        vs_update.u_measured = {3.0, nan, 3.2};
        vs_update.u_angle_measured = {4.0, 4.1, nan};

        update = voltage_sensor.update(vs_update);
        param = voltage_sensor.calc_param<asymmetric_t>();
        expected_param_value = {1.5 * sqrt3 * exp(1i * 4.0), 0.55 * sqrt3 * exp(1i * 4.1), 1.6 * sqrt3 * exp(1i * 2.2)};

        CHECK(cabs(param.value[0]) == doctest::Approx(cabs(expected_param_value[0])));
        CHECK(cabs(param.value[1]) == doctest::Approx(cabs(expected_param_value[1])));
        CHECK(cabs(param.value[2]) == doctest::Approx(cabs(expected_param_value[2])));
    }

    SUBCASE("Test sym/asym calc_param for symmetric voltage sensor, angle = 0") {
        RealValue<symmetric_t> const u_measured{10.1e3};
        RealValue<symmetric_t> const u_angle_measured{0};
        double const u_sigma = 1.0;
        double const u_rated = 10.0e3;

        VoltageSensorInput<symmetric_t> voltage_sensor_input{};
        voltage_sensor_input.id = 0;
        voltage_sensor_input.measured_object = 1;
        voltage_sensor_input.u_sigma = u_sigma;
        voltage_sensor_input.u_measured = u_measured;
        voltage_sensor_input.u_angle_measured = u_angle_measured;

        VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

        VoltageSensorCalcParam<symmetric_t> const sym_sensor_sym_param = voltage_sensor.calc_param<symmetric_t>();
        VoltageSensorCalcParam<asymmetric_t> const sym_sensor_asym_param = voltage_sensor.calc_param<asymmetric_t>();

        // Test sym voltage sensor with sym param calculation
        CHECK(real(sym_sensor_sym_param.value) == doctest::Approx(1.01));
        CHECK(imag(sym_sensor_sym_param.value) == doctest::Approx(0.0));
        CHECK(sym_sensor_sym_param.variance == doctest::Approx(1.0e-8));

        // Test sym voltage sensor with asym param calculation
        CHECK(real(sym_sensor_asym_param.value[0]) == doctest::Approx(1.01));
        CHECK(imag(sym_sensor_asym_param.value[0]) == doctest::Approx(0.0));

        CHECK(cabs(sym_sensor_asym_param.value[1]) == doctest::Approx(1.01));
        CHECK(arg(sym_sensor_asym_param.value[1]) == doctest::Approx(-2 * pi / 3));

        CHECK(cabs(sym_sensor_asym_param.value[2]) == doctest::Approx(1.01));
        CHECK(arg(sym_sensor_asym_param.value[2]) == doctest::Approx(2 * pi / 3));

        CHECK(sym_sensor_asym_param.variance == doctest::Approx(1.0e-8));
    }

    SUBCASE("Test sym/asym calc_param for symmetric voltage sensor, angle = nan") {
        RealValue<symmetric_t> const u_measured{10.1e3};
        RealValue<symmetric_t> const u_angle_measured{nan};
        double const u_sigma = 1.0;
        double const u_rated = 10.0e3;

        VoltageSensorInput<symmetric_t> voltage_sensor_input{};
        voltage_sensor_input.id = 0;
        voltage_sensor_input.measured_object = 1;
        voltage_sensor_input.u_sigma = u_sigma;
        voltage_sensor_input.u_measured = u_measured;
        voltage_sensor_input.u_angle_measured = u_angle_measured;

        VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

        VoltageSensorCalcParam<symmetric_t> const sym_sensor_sym_param = voltage_sensor.calc_param<symmetric_t>();
        VoltageSensorCalcParam<asymmetric_t> const sym_sensor_asym_param = voltage_sensor.calc_param<asymmetric_t>();

        // Test sym voltage sensor with sym param calculation
        CHECK(real(sym_sensor_sym_param.value) == doctest::Approx(1.01));
        CHECK(is_nan(imag(sym_sensor_sym_param.value)));
        CHECK(sym_sensor_sym_param.variance == doctest::Approx(1.0e-8));

        // Test sym voltage sensor with asym param calculation
        CHECK(real(sym_sensor_asym_param.value[0]) == doctest::Approx(1.01));
        CHECK(is_nan(imag(sym_sensor_asym_param.value[0])));

        CHECK(real(sym_sensor_asym_param.value[1]) == doctest::Approx(1.01));
        CHECK(is_nan(imag(sym_sensor_asym_param.value[1])));

        CHECK(real(sym_sensor_asym_param.value[2]) == doctest::Approx(1.01));
        CHECK(is_nan(imag(sym_sensor_asym_param.value[2])));

        CHECK(sym_sensor_asym_param.variance == doctest::Approx(1.0e-8));
    }

    SUBCASE("Test sym/asym calc_param for asymmetric voltage sensor, angle") {
        RealValue<asymmetric_t> const u_measured{10.1e3 / sqrt3, 10.2e3 / sqrt3, 10.3e3 / sqrt3};
        RealValue<asymmetric_t> const u_angle_measured{0.1, (-deg_120 + 0.2), (-deg_240 + 0.3)};
        double const u_sigma = 1.0;
        double const u_rated = 10.0e3;

        VoltageSensorInput<asymmetric_t> voltage_sensor_input{};
        voltage_sensor_input.id = 0;
        voltage_sensor_input.measured_object = 1;
        voltage_sensor_input.u_sigma = u_sigma;
        voltage_sensor_input.u_measured = u_measured;
        voltage_sensor_input.u_angle_measured = u_angle_measured;

        VoltageSensor<asymmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

        VoltageSensorCalcParam<symmetric_t> const asym_sensor_sym_param = voltage_sensor.calc_param<symmetric_t>();
        VoltageSensorCalcParam<asymmetric_t> const asym_sensor_asym_param = voltage_sensor.calc_param<asymmetric_t>();

        // Test asym voltage sensor with sym param calculation
        CHECK(real(asym_sensor_sym_param.value) ==
              doctest::Approx((1.01 * cos(0.1) + 1.02 * cos(0.2) + 1.03 * cos(0.3)) / 3));
        CHECK(imag(asym_sensor_sym_param.value) ==
              doctest::Approx((1.01 * sin(0.1) + 1.02 * sin(0.2) + 1.03 * sin(0.3)) / 3));
        CHECK(asym_sensor_sym_param.variance == doctest::Approx(3.0e-8));

        // Test asym voltage sensor with asym param calculation
        CHECK(cabs(asym_sensor_asym_param.value[0]) == doctest::Approx(1.01));
        CHECK(arg(asym_sensor_asym_param.value[0]) == doctest::Approx(0.1));

        CHECK(cabs(asym_sensor_asym_param.value[1]) == doctest::Approx(1.02));
        CHECK(arg(asym_sensor_asym_param.value[1]) == doctest::Approx(-deg_120 + 0.2));

        CHECK(cabs(asym_sensor_asym_param.value[2]) == doctest::Approx(1.03));
        CHECK(arg(asym_sensor_asym_param.value[2]) == doctest::Approx(deg_120 + 0.3));

        CHECK(asym_sensor_asym_param.variance == doctest::Approx(3.0e-8));
    }

    SUBCASE("Test sym/asym calc_param for asymmetric voltage sensor, angle = nan") {
        RealValue<asymmetric_t> const u_measured{10.1e3 / sqrt3, 10.2e3 / sqrt3, 10.3e3 / sqrt3};
        // if one of the angle is nan, the whole measurment is treated as no angle value
        RealValue<asymmetric_t> const u_angle_measured{1.0, 2.0, nan};
        double const u_sigma = 1.0;
        double const u_rated = 10.0e3;

        VoltageSensorInput<asymmetric_t> voltage_sensor_input{};
        voltage_sensor_input.id = 0;
        voltage_sensor_input.measured_object = 1;
        voltage_sensor_input.u_sigma = u_sigma;
        voltage_sensor_input.u_measured = u_measured;
        voltage_sensor_input.u_angle_measured = u_angle_measured;

        VoltageSensor<asymmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

        VoltageSensorCalcParam<symmetric_t> const asym_sensor_sym_param = voltage_sensor.calc_param<symmetric_t>();
        VoltageSensorCalcParam<asymmetric_t> const asym_sensor_asym_param = voltage_sensor.calc_param<asymmetric_t>();

        // Test asym voltage sensor with sym param calculation
        CHECK(real(asym_sensor_sym_param.value) == doctest::Approx((1.01 + 1.02 + 1.03) / 3));
        CHECK(is_nan(imag(asym_sensor_sym_param.value)));
        CHECK(asym_sensor_sym_param.variance == doctest::Approx(3.0e-8));

        // Test asym voltage sensor with asym param calculation
        CHECK(real(asym_sensor_asym_param.value[0]) == doctest::Approx(1.01));
        CHECK(is_nan(imag(asym_sensor_asym_param.value[0])));

        CHECK(real(asym_sensor_asym_param.value[1]) == doctest::Approx(1.02));
        CHECK(is_nan(imag(asym_sensor_asym_param.value[1])));

        CHECK(real(asym_sensor_asym_param.value[2]) == doctest::Approx(1.03));
        CHECK(is_nan(imag(asym_sensor_asym_param.value[2])));

        CHECK(asym_sensor_asym_param.variance == doctest::Approx(3.0e-8));
    }

    SUBCASE("Test get_output sym/asym for symmetric voltage sensor") {
        SUBCASE("Angle = 0") {
            RealValue<symmetric_t> const u_measured{10.1e3};
            RealValue<symmetric_t> const u_angle_measured{0};
            double const u_sigma = 1.0;
            double const u_rated = 10.0e3;

            VoltageSensorInput<symmetric_t> voltage_sensor_input{};
            voltage_sensor_input.id = 0;
            voltage_sensor_input.measured_object = 1;
            voltage_sensor_input.u_sigma = u_sigma;
            voltage_sensor_input.u_measured = u_measured;
            voltage_sensor_input.u_angle_measured = u_angle_measured;

            VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

            ComplexValue<symmetric_t> const u_calc_sym{1.02 * exp(1i * 0.2)};
            VoltageSensorOutput<symmetric_t> const sym_voltage_sensor_sym_output =
                voltage_sensor.get_output<symmetric_t>(u_calc_sym);

            ComplexValue<asymmetric_t> const u_calc_asym{1.02 * exp(1i * 0.2), 1.03 * exp(1i * 0.3),
                                                         1.04 * exp(1i * 0.4)};
            VoltageSensorOutput<asymmetric_t> sym_voltage_sensor_asym_output =
                voltage_sensor.get_output<asymmetric_t>(u_calc_asym);

            // Check sym output
            CHECK(sym_voltage_sensor_sym_output.id == 0);
            CHECK(sym_voltage_sensor_sym_output.energized == 1);
            CHECK(sym_voltage_sensor_sym_output.u_residual == doctest::Approx(-100.0));
            CHECK(sym_voltage_sensor_sym_output.u_angle_residual == doctest::Approx(-0.2));

            // Check asym output
            CHECK(sym_voltage_sensor_asym_output.id == 0);
            CHECK(sym_voltage_sensor_asym_output.energized == 1);
            CHECK(sym_voltage_sensor_asym_output.u_residual[0] == doctest::Approx(-100.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[1] == doctest::Approx(-200.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[2] == doctest::Approx(-300.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[0] == doctest::Approx(-0.2));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[1] == doctest::Approx(-0.3));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[2] == doctest::Approx(-0.4));
        }

        SUBCASE("Angle = 0.2") {
            RealValue<symmetric_t> const u_measured{10.1e3};
            RealValue<symmetric_t> const u_angle_measured{0.2};
            double const u_sigma = 1.0;
            double const u_rated = 10.0e3;

            VoltageSensorInput<symmetric_t> voltage_sensor_input{};
            voltage_sensor_input.id = 0;
            voltage_sensor_input.measured_object = 1;
            voltage_sensor_input.u_sigma = u_sigma;
            voltage_sensor_input.u_measured = u_measured;
            voltage_sensor_input.u_angle_measured = u_angle_measured;

            VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

            ComplexValue<symmetric_t> const u_calc_sym{1.02 * exp(1i * 0.2)};
            VoltageSensorOutput<symmetric_t> const sym_voltage_sensor_sym_output =
                voltage_sensor.get_output<symmetric_t>(u_calc_sym);

            ComplexValue<asymmetric_t> const u_calc_asym{1.02 * exp(1i * 0.2), 1.03 * exp(1i * 0.3),
                                                         1.04 * exp(1i * 0.4)};
            VoltageSensorOutput<asymmetric_t> sym_voltage_sensor_asym_output =
                voltage_sensor.get_output<asymmetric_t>(u_calc_asym);

            // Check sym output
            CHECK(sym_voltage_sensor_sym_output.id == 0);
            CHECK(sym_voltage_sensor_sym_output.energized == 1);
            CHECK(sym_voltage_sensor_sym_output.u_residual == doctest::Approx(-100.0));
            CHECK(sym_voltage_sensor_sym_output.u_angle_residual == doctest::Approx(0.0).epsilon(1e-12));

            // Check asym output
            CHECK(sym_voltage_sensor_asym_output.id == 0);
            CHECK(sym_voltage_sensor_asym_output.energized == 1);
            CHECK(sym_voltage_sensor_asym_output.u_residual[0] == doctest::Approx(-100.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[1] == doctest::Approx(-200.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[2] == doctest::Approx(-300.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[0] == doctest::Approx(0.0).epsilon(1e-12));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[1] == doctest::Approx(-0.1));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[2] == doctest::Approx(-0.2));
        }

        SUBCASE("Angle = ± pi ∓ 0.1") {
            RealValue<symmetric_t> const u_measured{10.1e3};
            RealValue<symmetric_t> const u_angle_measured{pi - 0.1};
            double const u_sigma = 1.0;
            double const u_rated = 10.0e3;

            VoltageSensorInput<symmetric_t> voltage_sensor_input{};
            voltage_sensor_input.id = 0;
            voltage_sensor_input.measured_object = 1;
            voltage_sensor_input.u_sigma = u_sigma;
            voltage_sensor_input.u_measured = u_measured;
            voltage_sensor_input.u_angle_measured = u_angle_measured;

            VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

            ComplexValue<symmetric_t> const u_calc_sym{1.02 * exp(1i * (-pi + 0.1))};
            VoltageSensorOutput<symmetric_t> const sym_voltage_sensor_sym_output =
                voltage_sensor.get_output<symmetric_t>(u_calc_sym);

            ComplexValue<asymmetric_t> const u_calc_asym{1.02 * exp(1i * (-pi + 0.1)), 1.03 * exp(1i * (-pi + 0.2)),
                                                         1.04 * exp(1i * (-pi + 0.3))};
            VoltageSensorOutput<asymmetric_t> sym_voltage_sensor_asym_output =
                voltage_sensor.get_output<asymmetric_t>(u_calc_asym);

            // Check sym output
            CHECK(sym_voltage_sensor_sym_output.id == 0);
            CHECK(sym_voltage_sensor_sym_output.energized == 1);
            CHECK(sym_voltage_sensor_sym_output.u_residual == doctest::Approx(-100.0));
            CHECK(sym_voltage_sensor_sym_output.u_angle_residual == doctest::Approx(-0.2).epsilon(1e-12));

            // Check asym output
            CHECK(sym_voltage_sensor_asym_output.id == 0);
            CHECK(sym_voltage_sensor_asym_output.energized == 1);
            CHECK(sym_voltage_sensor_asym_output.u_residual[0] == doctest::Approx(-100.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[1] == doctest::Approx(-200.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[2] == doctest::Approx(-300.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[0] == doctest::Approx(-0.2).epsilon(1e-12));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[1] == doctest::Approx(-0.3));
            CHECK(sym_voltage_sensor_asym_output.u_angle_residual[2] == doctest::Approx(-0.4));
        }

        SUBCASE("Angle = nan") {
            RealValue<symmetric_t> const u_measured{10.1e3};
            RealValue<symmetric_t> const u_angle_measured{nan};
            double const u_sigma = 1.0;
            double const u_rated = 10.0e3;

            VoltageSensorInput<symmetric_t> voltage_sensor_input{};
            voltage_sensor_input.id = 0;
            voltage_sensor_input.measured_object = 1;
            voltage_sensor_input.u_sigma = u_sigma;
            voltage_sensor_input.u_measured = u_measured;
            voltage_sensor_input.u_angle_measured = u_angle_measured;

            VoltageSensor<symmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

            ComplexValue<symmetric_t> const u_calc_sym{1.02 * exp(1i * 0.2)};
            VoltageSensorOutput<symmetric_t> const sym_voltage_sensor_sym_output =
                voltage_sensor.get_output<symmetric_t>(u_calc_sym);

            ComplexValue<asymmetric_t> const u_calc_asym{1.02 * exp(1i * 0.2), 1.03 * exp(1i * 0.3),
                                                         1.04 * exp(1i * 0.4)};
            VoltageSensorOutput<asymmetric_t> const sym_voltage_sensor_asym_output =
                voltage_sensor.get_output<asymmetric_t>(u_calc_asym);

            // Check sym output
            CHECK(sym_voltage_sensor_sym_output.id == 0);
            CHECK(sym_voltage_sensor_sym_output.energized == 1);
            CHECK(sym_voltage_sensor_sym_output.u_residual == doctest::Approx(-100.0));
            CHECK(is_nan(sym_voltage_sensor_sym_output.u_angle_residual));

            // Check asym output
            CHECK(sym_voltage_sensor_asym_output.id == 0);
            CHECK(sym_voltage_sensor_asym_output.energized == 1);
            CHECK(sym_voltage_sensor_asym_output.u_residual[0] == doctest::Approx(-100.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[1] == doctest::Approx(-200.0 / sqrt3));
            CHECK(sym_voltage_sensor_asym_output.u_residual[2] == doctest::Approx(-300.0 / sqrt3));
            CHECK(is_nan(sym_voltage_sensor_asym_output.u_angle_residual[0]));
            CHECK(is_nan(sym_voltage_sensor_asym_output.u_angle_residual[1]));
            CHECK(is_nan(sym_voltage_sensor_asym_output.u_angle_residual[2]));
        }
    }

    SUBCASE("Test get_output sym/asym for asymmetric voltage sensor") {
        SUBCASE("With angle") {
            RealValue<asymmetric_t> const u_measured{
                10.1e3 / sqrt3, 10.2e3 / sqrt3,
                10.3e3 / sqrt3}; // Asym voltage sensor measures line-ground voltage, hence /sqrt3
            RealValue<asymmetric_t> const u_angle_measured{0.1, 0.2, 0.3};
            double const u_sigma = 1.0;
            double const u_rated = 10.0e3;

            VoltageSensorInput<asymmetric_t> voltage_sensor_input{};
            voltage_sensor_input.id = 0;
            voltage_sensor_input.measured_object = 1;
            voltage_sensor_input.u_sigma = u_sigma;
            voltage_sensor_input.u_measured = u_measured;
            voltage_sensor_input.u_angle_measured = u_angle_measured;

            VoltageSensor<asymmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

            ComplexValue<symmetric_t> const u_calc_sym{1.02 * exp(1i * 0.2)};
            VoltageSensorOutput<symmetric_t> const asym_voltage_sensor_sym_output =
                voltage_sensor.get_output<symmetric_t>(u_calc_sym);
            DoubleComplex const u1_measured = voltage_sensor.calc_param<symmetric_t>().value;

            ComplexValue<asymmetric_t> const u_calc_asym{1.02 * exp(1i * 0.2), 1.04 * exp(1i * 0.4),
                                                         1.06 * exp(1i * 0.6)};
            VoltageSensorOutput<asymmetric_t> asym_voltage_sensor_asym_output =
                voltage_sensor.get_output<asymmetric_t>(u_calc_asym);

            // Check sym output
            CHECK(asym_voltage_sensor_sym_output.id == 0);
            CHECK(asym_voltage_sensor_sym_output.energized == 1);
            CHECK(asym_voltage_sensor_sym_output.u_residual ==
                  doctest::Approx((cabs(u1_measured) - cabs(u_calc_sym)) * u_rated));
            CHECK(asym_voltage_sensor_sym_output.u_angle_residual == doctest::Approx(arg(u1_measured) - 0.2));

            // Check asym output
            CHECK(asym_voltage_sensor_asym_output.id == 0);
            CHECK(asym_voltage_sensor_asym_output.energized == 1);
            CHECK(asym_voltage_sensor_asym_output.u_residual[0] == doctest::Approx(-100.0 / sqrt3));
            CHECK(asym_voltage_sensor_asym_output.u_residual[1] == doctest::Approx(-200.0 / sqrt3));
            CHECK(asym_voltage_sensor_asym_output.u_residual[2] == doctest::Approx(-300.0 / sqrt3));
            CHECK(asym_voltage_sensor_asym_output.u_angle_residual[0] == doctest::Approx(-0.1));
            CHECK(asym_voltage_sensor_asym_output.u_angle_residual[1] == doctest::Approx(-0.2));
            CHECK(asym_voltage_sensor_asym_output.u_angle_residual[2] == doctest::Approx(-0.3));
        }

        SUBCASE("Angle = nan") {
            RealValue<asymmetric_t> const u_measured{
                10.1e3 / sqrt3, 10.2e3 / sqrt3,
                10.3e3 / sqrt3}; // Asym voltage sensor measures line-ground voltage, hence /sqrt3
            RealValue<asymmetric_t> const u_angle_measured{nan, nan, nan};
            double const u_sigma = 1.0;
            double const u_rated = 10.0e3;

            VoltageSensorInput<asymmetric_t> voltage_sensor_input{};
            voltage_sensor_input.id = 0;
            voltage_sensor_input.measured_object = 1;
            voltage_sensor_input.u_sigma = u_sigma;
            voltage_sensor_input.u_measured = u_measured;
            voltage_sensor_input.u_angle_measured = u_angle_measured;

            VoltageSensor<asymmetric_t> const voltage_sensor{voltage_sensor_input, u_rated};

            ComplexValue<symmetric_t> const u_calc_sym{1.02 * exp(1i * 0.2)};
            VoltageSensorOutput<symmetric_t> const asym_voltage_sensor_sym_output =
                voltage_sensor.get_output<symmetric_t>(u_calc_sym);

            ComplexValue<asymmetric_t> const u_calc_asym{1.02 * exp(1i * 0.2), 1.04 * exp(1i * 0.4),
                                                         1.06 * exp(1i * 0.6)};
            VoltageSensorOutput<asymmetric_t> asym_voltage_sensor_asym_output =
                voltage_sensor.get_output<asymmetric_t>(u_calc_asym);

            // Check sym output
            CHECK(asym_voltage_sensor_sym_output.id == 0);
            CHECK(asym_voltage_sensor_sym_output.energized == 1);
            CHECK(asym_voltage_sensor_sym_output.u_residual == doctest::Approx(0.0).epsilon(1e-6));
            CHECK(is_nan(asym_voltage_sensor_sym_output.u_angle_residual));

            // Check asym output
            CHECK(asym_voltage_sensor_asym_output.id == 0);
            CHECK(asym_voltage_sensor_asym_output.energized == 1);
            CHECK(asym_voltage_sensor_asym_output.u_residual[0] == doctest::Approx(-100.0 / sqrt3));
            CHECK(asym_voltage_sensor_asym_output.u_residual[1] == doctest::Approx(-200.0 / sqrt3));
            CHECK(asym_voltage_sensor_asym_output.u_residual[2] == doctest::Approx(-300.0 / sqrt3));
            CHECK(is_nan(asym_voltage_sensor_asym_output.u_angle_residual[0]));
            CHECK(is_nan(asym_voltage_sensor_asym_output.u_angle_residual[1]));
            CHECK(is_nan(asym_voltage_sensor_asym_output.u_angle_residual[2]));
        }
    }

    SUBCASE("Construction and update") {
        VoltageSensorInput<symmetric_t> const sym_voltage_sensor_input{
            .id = 7, .measured_object = 3, .u_sigma = 1.0, .u_measured = 25000, .u_angle_measured = -0.2};
        VoltageSensorUpdate<symmetric_t> const sym_voltage_sensor_update{
            .id = 7,
            .u_sigma = sym_voltage_sensor_input.u_sigma,
            .u_measured = sym_voltage_sensor_input.u_measured,
            .u_angle_measured = sym_voltage_sensor_input.u_angle_measured};

        SymVoltageSensor sym_voltage_sensor{sym_voltage_sensor_input, 31250};
        auto const orig_calc_param = sym_voltage_sensor.calc_param<symmetric_t>();

        sym_voltage_sensor.update(sym_voltage_sensor_update);
        auto const updated_calc_param = sym_voltage_sensor.calc_param<symmetric_t>();

        CHECK(orig_calc_param.value == updated_calc_param.value);
        CHECK(orig_calc_param.variance == updated_calc_param.variance);
    }

    SUBCASE("Update inverse - sym") {
        constexpr auto u_sigma = 1.0;
        constexpr auto u_measured = 2.0;
        constexpr auto u_angle_measured = 3.0;
        constexpr auto u_rated = 10.0e3;
        VoltageSensor<symmetric_t> const voltage_sensor{{.id = 1,
                                                         .measured_object = 2,
                                                         .u_sigma = u_sigma,
                                                         .u_measured = u_measured,
                                                         .u_angle_measured = u_angle_measured},
                                                        u_rated};

        VoltageSensorUpdate<symmetric_t> vs_update{.id = 1, .u_sigma = nan, .u_measured = nan, .u_angle_measured = nan};
        auto expected = vs_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("u_sigma") {
            SUBCASE("same") { vs_update.u_sigma = u_sigma; }
            SUBCASE("different") { vs_update.u_sigma = 0.0; }
            expected.u_sigma = u_sigma;
        }

        SUBCASE("u_measured") {
            SUBCASE("same") { vs_update.u_measured = u_measured; }
            SUBCASE("different") { vs_update.u_measured = 0.0; }
            expected.u_measured = u_measured;
        }

        SUBCASE("u_angle_measured") {
            SUBCASE("same") { vs_update.u_angle_measured = u_angle_measured; }
            SUBCASE("different") { vs_update.u_angle_measured = 0.0; }
            expected.u_angle_measured = u_angle_measured;
        }

        SUBCASE("multiple") {
            vs_update.u_sigma = 0.0;
            vs_update.u_measured = 0.0;
            vs_update.u_angle_measured = 0.0;
            expected.u_sigma = u_sigma;
            expected.u_measured = u_measured;
            expected.u_angle_measured = u_angle_measured;
        }

        auto const inv = voltage_sensor.inverse(vs_update);

        CHECK(inv.id == expected.id);
        check_nan_preserving_equality(inv.u_sigma, expected.u_sigma);
        check_nan_preserving_equality(inv.u_measured, expected.u_measured);
        check_nan_preserving_equality(inv.u_angle_measured, expected.u_angle_measured);
    }

    SUBCASE("Update inverse - asym") {
        constexpr double u_sigma = 1.0;
        RealValue<asymmetric_t> const u_measured{2.0, 3.0, 4.0};
        RealValue<asymmetric_t> const u_angle_measured{5.0, 6.0, 7.0};
        constexpr double u_rated = 10.0e3;
        VoltageSensor<asymmetric_t> const voltage_sensor{{.id = 1,
                                                          .measured_object = 2,
                                                          .u_sigma = u_sigma,
                                                          .u_measured = u_measured,
                                                          .u_angle_measured = u_angle_measured},
                                                         u_rated};

        VoltageSensorUpdate<asymmetric_t> vs_update{
            .id = 1, .u_sigma = nan, .u_measured = {nan, nan, nan}, .u_angle_measured = {nan, nan, nan}};
        auto expected = vs_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("u_sigma") {
            SUBCASE("same") { vs_update.u_sigma = u_sigma; }
            SUBCASE("different") { vs_update.u_sigma = 0.0; }
            expected.u_sigma = u_sigma;
        }

        SUBCASE("u_measured") {
            SUBCASE("same") { vs_update.u_measured = u_measured; }
            SUBCASE("1 different") {
                vs_update.u_measured = {0.0, nan, nan};
                expected.u_measured = {u_measured(0), nan, nan};
            }
            SUBCASE("all different") {
                vs_update.u_measured = {0.0, 0.1, 0.2};
                expected.u_measured = u_measured;
            }
        }

        SUBCASE("u_angle_measured") {
            SUBCASE("same") { vs_update.u_angle_measured = u_angle_measured; }
            SUBCASE("1 different") {
                vs_update.u_angle_measured = {0.0, nan, nan};
                expected.u_angle_measured = {u_angle_measured(0), nan, nan};
            }
            SUBCASE("all different") {
                vs_update.u_angle_measured = {0.0, 0.4, 0.6};
                expected.u_angle_measured = u_angle_measured;
            }
        }

        SUBCASE("multiple") {
            vs_update.u_sigma = 0.0;
            vs_update.u_measured = {0.0, 0.1, 0.2};
            vs_update.u_angle_measured = {0.0, 0.4, 0.6};
            expected.u_sigma = u_sigma;
            expected.u_measured = u_measured;
            expected.u_angle_measured = u_angle_measured;
        }

        auto const inv = voltage_sensor.inverse(vs_update);

        CHECK(inv.id == expected.id);
        check_nan_preserving_equality(inv.u_sigma, expected.u_sigma);
        check_nan_preserving_equality(inv.u_measured, expected.u_measured);
        check_nan_preserving_equality(inv.u_angle_measured, expected.u_angle_measured);
    }
}

} // namespace power_grid_model
