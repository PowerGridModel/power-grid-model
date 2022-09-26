// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/component/power_sensor.hpp"

namespace power_grid_model {
// TO TEST
// calc_param of Sensor
// get_output of GenericPowerSensor

TEST_CASE("Test power sensor") {
    // ------------------------------------------------------
    // --------------- Symmetric power sensor ---------------
    // ------------------------------------------------------
    SUBCASE("Symmetric Power Sensor - Generator") {
        PowerSensorInput<true> sym_power_sensor_input{};
        sym_power_sensor_input.id = 0;
        sym_power_sensor_input.measured_object = 1;
        sym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::generator;
        sym_power_sensor_input.power_sigma = 1.0 * 1e5;
        sym_power_sensor_input.p_measured = 1.0 * 1e3;
        sym_power_sensor_input.q_measured = 0.8 * 1e3;

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = sym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(1.0 * 1e2));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.0 * 1e2));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.0 * 1e2 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.0 * 1e2 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::generator);

        // -------- Update power sensor --------
        PowerSensorUpdate<true> sym_power_sensor_update{};
        sym_power_sensor_update.power_sigma = 2.0 * 1e5;
        sym_power_sensor_update.p_measured = 3.0 * 1e3;
        sym_power_sensor_update.q_measured = 4.0 * 1e3;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        terminal_type = sym_power_sensor.get_terminal_type();

        sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(3.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.3 * 1e3 / 3.0));

        // Check update nan
        sym_power_sensor_update.power_sigma = nan;
        sym_power_sensor_update.p_measured = nan;
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));
    }

    SUBCASE("Symmetric Power Sensor - Branch_from") {
        PowerSensorInput<true> sym_power_sensor_input{};
        sym_power_sensor_input.id = 0;
        sym_power_sensor_input.measured_object = 1;
        sym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::branch_from;
        sym_power_sensor_input.power_sigma = 1.0 * 1e5;
        sym_power_sensor_input.p_measured = 1.0 * 1e3;
        sym_power_sensor_input.q_measured = 0.8 * 1e3;

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = sym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(1.0 * 1e2));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.0 * 1e2));

        // Check symmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.0 * 1e2 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.0 * 1e2 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::branch_from);

        // -------- Update power sensor --------
        PowerSensorUpdate<true> sym_power_sensor_update{};
        sym_power_sensor_update.power_sigma = 2.0 * 1e5;
        sym_power_sensor_update.p_measured = 3.0 * 1e3;
        sym_power_sensor_update.q_measured = 4.0 * 1e3;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        terminal_type = sym_power_sensor.get_terminal_type();

        sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(3.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.3 * 1e3 / 3.0));

        // Check update nan
        sym_power_sensor_update.power_sigma = nan;
        sym_power_sensor_update.p_measured = nan;
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));
    }

    SUBCASE("Symmetric Power Sensor - Branch_to") {
        PowerSensorInput<true> sym_power_sensor_input{};
        sym_power_sensor_input.id = 0;
        sym_power_sensor_input.measured_object = 1;
        sym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::branch_to;
        sym_power_sensor_input.power_sigma = 1.0 * 1e5;
        sym_power_sensor_input.p_measured = 1.0 * 1e3;
        sym_power_sensor_input.q_measured = 0.8 * 1e3;

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = sym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(1.0 * 1e2));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.0 * 1e2));

        // Check symmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.0 * 1e2 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.0 * 1e2 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::branch_to);

        // -------- Update power sensor --------
        PowerSensorUpdate<true> sym_power_sensor_update{};
        sym_power_sensor_update.power_sigma = 2.0 * 1e5;
        sym_power_sensor_update.p_measured = 3.0 * 1e3;
        sym_power_sensor_update.q_measured = 4.0 * 1e3;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        terminal_type = sym_power_sensor.get_terminal_type();

        sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(3.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.3 * 1e3 / 3.0));

        // Check update nan
        sym_power_sensor_update.power_sigma = nan;
        sym_power_sensor_update.p_measured = nan;
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));
    }

    SUBCASE("Symmetric Power Sensor - Source") {
        PowerSensorInput<true> sym_power_sensor_input{};
        sym_power_sensor_input.id = 0;
        sym_power_sensor_input.measured_object = 1;
        sym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::source;
        sym_power_sensor_input.power_sigma = 1.0 * 1e5;
        sym_power_sensor_input.p_measured = 1.0 * 1e3;
        sym_power_sensor_input.q_measured = 0.8 * 1e3;

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = sym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(1.0 * 1e2));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.0 * 1e2));

        // Check symmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(1.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(8.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.0 * 1e2 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.0 * 1e2 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::source);

        // -------- Update power sensor --------
        PowerSensorUpdate<true> sym_power_sensor_update{};
        sym_power_sensor_update.power_sigma = 2.0 * 1e5;
        sym_power_sensor_update.p_measured = 3.0 * 1e3;
        sym_power_sensor_update.q_measured = 4.0 * 1e3;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        terminal_type = sym_power_sensor.get_terminal_type();

        sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(3.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.3 * 1e3 / 3.0));

        // Check update nan
        sym_power_sensor_update.power_sigma = nan;
        sym_power_sensor_update.p_measured = nan;
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));
    }

    SUBCASE("Symmetric Power Sensor - Shunt") {
        PowerSensorInput<true> sym_power_sensor_input{};
        sym_power_sensor_input.id = 0;
        sym_power_sensor_input.measured_object = 1;
        sym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::shunt;
        sym_power_sensor_input.power_sigma = 1.0 * 1e5;
        sym_power_sensor_input.p_measured = 1.0 * 1e3;
        sym_power_sensor_input.q_measured = 0.8 * 1e3;

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = sym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-1.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-8.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(1.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.5 * 1e3));

        // Check symmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-1.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-8.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.5 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::shunt);

        // -------- Update power sensor --------
        PowerSensorUpdate<true> sym_power_sensor_update{};
        sym_power_sensor_update.power_sigma = 2.0 * 1e5;
        sym_power_sensor_update.p_measured = 3.0 * 1e3;
        sym_power_sensor_update.q_measured = 4.0 * 1e3;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        terminal_type = sym_power_sensor.get_terminal_type();

        sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-4.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(4.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-4.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(3.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(4.7 * 1e3 / 3.0));

        // Check update nan
        sym_power_sensor_update.power_sigma = nan;
        sym_power_sensor_update.p_measured = nan;
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-4.0 * 1e-3));

        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-4.0 * 1e-3));
    }

    SUBCASE("Symmetric Power Sensor - Load") {
        PowerSensorInput<true> sym_power_sensor_input{};
        sym_power_sensor_input.id = 0;
        sym_power_sensor_input.measured_object = 1;
        sym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::load;
        sym_power_sensor_input.power_sigma = 1.0 * 1e5;
        sym_power_sensor_input.p_measured = 1.0 * 1e3;
        sym_power_sensor_input.q_measured = 0.8 * 1e3;

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = sym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-1.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-8.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(1.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.5 * 1e3));

        // Check symmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(1.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-1.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-8.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.5 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::load);

        // -------- Update power sensor --------
        PowerSensorUpdate<true> sym_power_sensor_update{};
        sym_power_sensor_update.power_sigma = 2.0 * 1e5;
        sym_power_sensor_update.p_measured = 3.0 * 1e3;
        sym_power_sensor_update.q_measured = 4.0 * 1e3;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        terminal_type = sym_power_sensor.get_terminal_type();

        sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-4.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(4.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-4.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(3.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(4.7 * 1e3 / 3.0));

        // Check update nan
        sym_power_sensor_update.power_sigma = nan;
        sym_power_sensor_update.p_measured = nan;
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor.update(sym_power_sensor_update);

        sym_sensor_param = sym_power_sensor.calc_param<true>();
        asym_sensor_param = sym_power_sensor.calc_param<false>();

        CHECK(sym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-4.0 * 1e-3));

        CHECK(asym_sensor_param.variance == doctest::Approx(4.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-4.0 * 1e-3));
    }

    // -------------------------------------------------------
    // --------------- Asymmetric power sensor ---------------
    // -------------------------------------------------------
    SUBCASE("Asymmetric Power Sensor - Generator") {
        PowerSensorInput<false> asym_power_sensor_input{};
        asym_power_sensor_input.id = 0;
        asym_power_sensor_input.measured_object = 1;
        asym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::generator;
        asym_power_sensor_input.power_sigma = 1.0 * 1e5;
        asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = asym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.7 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::generator);

        // -------- Update power sensor --------
        PowerSensorUpdate<false> asym_power_sensor_update{};
        asym_power_sensor_update.power_sigma = 2.0 * 1e5;
        asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        terminal_type = asym_power_sensor.get_terminal_type();

        sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(8.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(11.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(8.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(11.3 * 1e3 / 3.0));

        // update with partial nan
        asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
        asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        CHECK(real(sym_sensor_param.value) == doctest::Approx(16.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(21.0 * 1e-3));

        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(18.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[1]) == doctest::Approx(9.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[2]) == doctest::Approx(21.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[0]) == doctest::Approx(24.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(27.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[2]) == doctest::Approx(12.0 * 1e-3));
    }

    SUBCASE("Asymmetric Power Sensor - Branch_from") {
        PowerSensorInput<false> asym_power_sensor_input{};
        asym_power_sensor_input.id = 0;
        asym_power_sensor_input.measured_object = 1;
        asym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::branch_from;
        asym_power_sensor_input.power_sigma = 1.0 * 1e5;
        asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = asym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.7 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::branch_from);

        // -------- Update power sensor --------
        PowerSensorUpdate<false> asym_power_sensor_update{};
        asym_power_sensor_update.power_sigma = 2.0 * 1e5;
        asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        terminal_type = asym_power_sensor.get_terminal_type();

        sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(8.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(11.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(8.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(11.3 * 1e3 / 3.0));

        // update with partial nan
        asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
        asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        CHECK(real(sym_sensor_param.value) == doctest::Approx(16.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(21.0 * 1e-3));

        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(18.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[1]) == doctest::Approx(9.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[2]) == doctest::Approx(21.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[0]) == doctest::Approx(24.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(27.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[2]) == doctest::Approx(12.0 * 1e-3));
    }

    SUBCASE("Asymmetric Power Sensor - Branch_to") {
        PowerSensorInput<false> asym_power_sensor_input{};
        asym_power_sensor_input.id = 0;
        asym_power_sensor_input.measured_object = 1;
        asym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::branch_to;
        asym_power_sensor_input.power_sigma = 1.0 * 1e5;
        asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = asym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.7 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::branch_to);

        // -------- Update power sensor --------
        PowerSensorUpdate<false> asym_power_sensor_update{};
        asym_power_sensor_update.power_sigma = 2.0 * 1e5;
        asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        terminal_type = asym_power_sensor.get_terminal_type();

        sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(8.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(11.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(8.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(11.3 * 1e3 / 3.0));

        // update with partial nan
        asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
        asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        CHECK(real(sym_sensor_param.value) == doctest::Approx(16.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(21.0 * 1e-3));

        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(18.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[1]) == doctest::Approx(9.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[2]) == doctest::Approx(21.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[0]) == doctest::Approx(24.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(27.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[2]) == doctest::Approx(12.0 * 1e-3));
    }

    SUBCASE("Asymmetric Power Sensor - Source") {
        PowerSensorInput<false> asym_power_sensor_input{};
        asym_power_sensor_input.id = 0;
        asym_power_sensor_input.measured_object = 1;
        asym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::source;
        asym_power_sensor_input.power_sigma = 1.0 * 1e5;
        asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = asym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(1.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(24.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.7 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::source);

        // -------- Update power sensor --------
        PowerSensorUpdate<false> asym_power_sensor_update{};
        asym_power_sensor_update.power_sigma = 2.0 * 1e5;
        asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        terminal_type = asym_power_sensor.get_terminal_type();

        sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(8.1 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(11.3 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(9.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(12.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(8.1 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(11.3 * 1e3 / 3.0));

        // update with partial nan
        asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
        asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        CHECK(real(sym_sensor_param.value) == doctest::Approx(16.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(21.0 * 1e-3));

        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(18.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[1]) == doctest::Approx(9.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[2]) == doctest::Approx(21.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[0]) == doctest::Approx(24.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(27.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[2]) == doctest::Approx(12.0 * 1e-3));
    }

    SUBCASE("Asymmetric Power Sensor - Shunt") {
        PowerSensorInput<false> asym_power_sensor_input{};
        asym_power_sensor_input.id = 0;
        asym_power_sensor_input.measured_object = 1;
        asym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::shunt;
        asym_power_sensor_input.power_sigma = 1.0 * 1e5;
        asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = asym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-24.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(3.1 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-24.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(3.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.1 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::shunt);

        // -------- Update power sensor --------
        PowerSensorUpdate<false> asym_power_sensor_update{};
        asym_power_sensor_update.power_sigma = 2.0 * 1e5;
        asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        terminal_type = asym_power_sensor.get_terminal_type();

        sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-9.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-12.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(9.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(12.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-9.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-12.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(9.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(12.7 * 1e3 / 3.0));

        // update with partial nan
        asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
        asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        CHECK(real(sym_sensor_param.value) == doctest::Approx(-16.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-21.0 * 1e-3));

        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-18.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[1]) == doctest::Approx(-9.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[2]) == doctest::Approx(-21.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[0]) == doctest::Approx(-24.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-27.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[2]) == doctest::Approx(-12.0 * 1e-3));
    }

    SUBCASE("Asymmetric Power Sensor - Load") {
        PowerSensorInput<false> asym_power_sensor_input{};
        asym_power_sensor_input.id = 0;
        asym_power_sensor_input.measured_object = 1;
        asym_power_sensor_input.measured_terminal_type = MeasuredTerminalType::load;
        asym_power_sensor_input.power_sigma = 1.0 * 1e5;
        asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};

        ComplexValue<true> s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
        ComplexValue<false> s_asym = s_sym * RealValue<false>{1.0};

        PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

        SensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
        SensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

        MeasuredTerminalType terminal_type = asym_power_sensor.get_terminal_type();

        PowerSensorOutput<true> sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        PowerSensorOutput<false> sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-24.0 * 1e-4));

        CHECK(sym_sensor_output.id == 0);
        CHECK(sym_sensor_output.energized == 1);
        CHECK(sym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(3.1 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(9.0 / 1e2));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-24.0 * 1e-4));

        CHECK(sym_sensor_output_asym_param.id == 0);
        CHECK(sym_sensor_output_asym_param.energized == 1);
        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(3.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.1 * 1e3 / 3.0));

        CHECK(terminal_type == MeasuredTerminalType::load);

        // -------- Update power sensor --------
        PowerSensorUpdate<false> asym_power_sensor_update{};
        asym_power_sensor_update.power_sigma = 2.0 * 1e5;
        asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        terminal_type = asym_power_sensor.get_terminal_type();

        sym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
        sym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

        // Check symmetric output for symmetric parameters
        CHECK(sym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(sym_sensor_param.value) == doctest::Approx(-9.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-12.0 * 1e-3));

        CHECK(sym_sensor_output.p_residual == doctest::Approx(9.9 * 1e3));
        CHECK(sym_sensor_output.q_residual == doctest::Approx(12.7 * 1e3));

        // Check asymmetric output for asymmetric parameters
        CHECK(asym_sensor_param.variance == doctest::Approx(3.6 / 1e1));
        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-9.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-12.0 * 1e-3));

        CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(9.9 * 1e3 / 3.0));
        CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(12.7 * 1e3 / 3.0));

        // update with partial nan
        asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
        asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
        asym_power_sensor.update(asym_power_sensor_update);

        sym_sensor_param = asym_power_sensor.calc_param<true>();
        asym_sensor_param = asym_power_sensor.calc_param<false>();

        CHECK(real(sym_sensor_param.value) == doctest::Approx(-16.0 * 1e-3));
        CHECK(imag(sym_sensor_param.value) == doctest::Approx(-21.0 * 1e-3));

        CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-18.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[1]) == doctest::Approx(-9.0 * 1e-3));
        CHECK(real(asym_sensor_param.value[2]) == doctest::Approx(-21.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[0]) == doctest::Approx(-24.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-27.0 * 1e-3));
        CHECK(imag(asym_sensor_param.value[2]) == doctest::Approx(-12.0 * 1e-3));
    }
}
}  // namespace power_grid_model