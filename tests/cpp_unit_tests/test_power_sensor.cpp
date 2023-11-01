// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/power_sensor.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
const auto r_nan = RealValue<false>{nan};
}

// TO TEST
// calc_param of Sensor
// get_output of GenericPowerSensor

TEST_CASE("Test power sensor") {
    // ------------------------------------------------------
    // --------------- Symmetric power sensor ---------------
    // ------------------------------------------------------
    SUBCASE("Symmetric Power Sensor - generator, branch_from, branch_to, source") {
        for (auto const terminal_type : {MeasuredTerminalType::generator, MeasuredTerminalType::branch_from,
                                         MeasuredTerminalType::branch_to, MeasuredTerminalType::source}) {
            CAPTURE(terminal_type);

            PowerSensorInput<true> sym_power_sensor_input{};
            sym_power_sensor_input.id = 0;
            sym_power_sensor_input.measured_object = 1;
            sym_power_sensor_input.measured_terminal_type = terminal_type;
            sym_power_sensor_input.power_sigma = 1.0 * 1e5;
            sym_power_sensor_input.p_measured = 1.0 * 1e3;
            sym_power_sensor_input.q_measured = 0.8 * 1e3;
            sym_power_sensor_input.p_sigma = nan;
            sym_power_sensor_input.q_sigma = nan;

            ComplexValue<true> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<false> const s_asym = s_sym * RealValue<false>{1.0};

            PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

            PowerSensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
            PowerSensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

            PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
            PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(1.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(8.0 * 1e-4));

            CHECK(sym_sensor_output.id == 0);
            CHECK(sym_sensor_output.energized == 1);
            CHECK(sym_sensor_output.p_residual == doctest::Approx(1.0 * 1e2));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(1.0 * 1e2));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(1.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(8.0 * 1e-4));

            CHECK(sym_sensor_output_asym_param.id == 0);
            CHECK(sym_sensor_output_asym_param.energized == 1);
            CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.0 * 1e2 / 3.0));
            CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.0 * 1e2 / 3.0));

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<true> sym_power_sensor_update{};
            sym_power_sensor_update.power_sigma = 2.0 * 1e5;
            sym_power_sensor_update.p_measured = 3.0 * 1e3;
            sym_power_sensor_update.q_measured = 4.0 * 1e3;
            sym_power_sensor_update.p_sigma = nan;
            sym_power_sensor_update.q_sigma = nan;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<true>();
            asym_sensor_param = sym_power_sensor.calc_param<false>();

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
            sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

            CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(3.3 * 1e3));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(4.0 / 1e2 / 2));
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

            CHECK(sym_sensor_param.p_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(4.0 * 1e-3));

            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(4.0 * 1e-3));
        }
    }

    SUBCASE("Symmetric Power Sensor - shunt, load") {
        for (auto const terminal_type : {MeasuredTerminalType::shunt, MeasuredTerminalType::load}) {
            CAPTURE(terminal_type);

            PowerSensorInput<true> sym_power_sensor_input{};
            sym_power_sensor_input.id = 0;
            sym_power_sensor_input.measured_object = 1;
            sym_power_sensor_input.measured_terminal_type = terminal_type;
            sym_power_sensor_input.power_sigma = 1.0 * 1e5;
            sym_power_sensor_input.p_measured = 1.0 * 1e3;
            sym_power_sensor_input.q_measured = 0.8 * 1e3;
            sym_power_sensor_input.p_sigma = nan;
            sym_power_sensor_input.q_sigma = nan;

            ComplexValue<true> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<false> const s_asym = s_sym * RealValue<false>{1.0};

            PowerSensor<true> sym_power_sensor{sym_power_sensor_input};

            PowerSensorCalcParam<true> sym_sensor_param = sym_power_sensor.calc_param<true>();
            PowerSensorCalcParam<false> asym_sensor_param = sym_power_sensor.calc_param<false>();

            PowerSensorOutput<true> sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
            PowerSensorOutput<false> sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(-1.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(-8.0 * 1e-4));

            CHECK(sym_sensor_output.id == 0);
            CHECK(sym_sensor_output.energized == 1);
            CHECK(sym_sensor_output.p_residual == doctest::Approx(1.9 * 1e3));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(1.5 * 1e3));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-1.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-8.0 * 1e-4));

            CHECK(sym_sensor_output_asym_param.id == 0);
            CHECK(sym_sensor_output_asym_param.energized == 1);
            CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.9 * 1e3 / 3.0));
            CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.5 * 1e3 / 3.0));

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<true> sym_power_sensor_update{};
            sym_power_sensor_update.power_sigma = 2.0 * 1e5;
            sym_power_sensor_update.p_measured = 3.0 * 1e3;
            sym_power_sensor_update.q_measured = 4.0 * 1e3;
            sym_power_sensor_update.p_sigma = nan;
            sym_power_sensor_update.q_sigma = nan;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<true>();
            asym_sensor_param = sym_power_sensor.calc_param<false>();

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            sym_sensor_output = sym_power_sensor.get_output<true>(s_sym);
            sym_sensor_output_asym_param = sym_power_sensor.get_output<false>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(-4.0 * 1e-3));

            CHECK(sym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(4.7 * 1e3));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(4.0 / 1e2 / 2));
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

            CHECK(sym_sensor_param.p_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(-4.0 * 1e-3));

            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-4.0 * 1e-3));
        }
    }

    SUBCASE("Symmetric Power Sensor - Partial initialization and full update") {
        PowerSensorInput<true> sym_power_sensor_input{};
        sym_power_sensor_input.p_measured = nan;
        sym_power_sensor_input.q_measured = RealValue<true>{1.0};
        sym_power_sensor_input.p_sigma = nan;
        sym_power_sensor_input.q_sigma = nan;

        PowerSensorUpdate<true> sym_power_sensor_update{};
        sym_power_sensor_update.p_measured = RealValue<true>{1.0};
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor_update.p_sigma = nan;
        sym_power_sensor_update.q_sigma = nan;

        PowerSensor<true> sym_power_sensor{sym_power_sensor_input};
        sym_power_sensor.update(sym_power_sensor_update);

        auto const result = sym_power_sensor.get_output<true>({});
        CHECK_FALSE(std::isnan(result.p_residual));
        CHECK_FALSE(std::isnan(result.q_residual));
    }

    // -------------------------------------------------------
    // --------------- Asymmetric power sensor ---------------
    // -------------------------------------------------------
    SUBCASE("Asymmetric Power Sensor - generator, branch_from, branch_to, source") {
        for (auto const terminal_type : {MeasuredTerminalType::generator, MeasuredTerminalType::branch_from,
                                         MeasuredTerminalType::branch_to, MeasuredTerminalType::source}) {
            CAPTURE(terminal_type);

            PowerSensorInput<false> asym_power_sensor_input{};
            asym_power_sensor_input.id = 0;
            asym_power_sensor_input.measured_object = 1;
            asym_power_sensor_input.measured_terminal_type = terminal_type;
            asym_power_sensor_input.power_sigma = 1.0 * 1e5;
            asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_input.p_sigma = r_nan;
            asym_power_sensor_input.q_sigma = r_nan;

            ComplexValue<true> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<false> const s_asym = s_sym * RealValue<false>{1.0};

            PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

            PowerSensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
            PowerSensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

            PowerSensorOutput<true> asym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
            PowerSensorOutput<false> asym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(24.0 * 1e-4));

            CHECK(asym_sensor_output.id == 0);
            CHECK(asym_sensor_output.energized == 1);
            CHECK(asym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(1.7 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(24.0 * 1e-4));

            CHECK(asym_sensor_output_asym_param.id == 0);
            CHECK(asym_sensor_output_asym_param.energized == 1);
            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.7 * 1e3 / 3.0));

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<false> asym_power_sensor_update{};
            asym_power_sensor_update.power_sigma = 2.0 * 1e5;
            asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_update.p_sigma = r_nan;
            asym_power_sensor_update.q_sigma = r_nan;
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<true>();
            asym_sensor_param = asym_power_sensor.calc_param<false>();

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            asym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
            asym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(9.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(12.0 * 1e-3));

            CHECK(asym_sensor_output.p_residual == doctest::Approx(8.1 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(11.3 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(9.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(12.0 * 1e-3));

            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(8.1 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(11.3 * 1e3 / 3.0));

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
    }

    SUBCASE("Asymmetric Power Sensor - shunt, load") {
        for (auto const terminal_type : {MeasuredTerminalType::shunt, MeasuredTerminalType::load}) {
            CAPTURE(terminal_type);

            PowerSensorInput<false> asym_power_sensor_input{};
            asym_power_sensor_input.id = 0;
            asym_power_sensor_input.measured_object = 1;
            asym_power_sensor_input.measured_terminal_type = terminal_type;
            asym_power_sensor_input.power_sigma = 1.0 * 1e5;
            asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_input.p_sigma = r_nan;
            asym_power_sensor_input.q_sigma = r_nan;

            ComplexValue<true> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<false> const s_asym = s_sym * RealValue<false>{1.0};

            PowerSensor<false> asym_power_sensor{asym_power_sensor_input};

            PowerSensorCalcParam<true> sym_sensor_param = asym_power_sensor.calc_param<true>();
            PowerSensorCalcParam<false> asym_sensor_param = asym_power_sensor.calc_param<false>();

            PowerSensorOutput<true> asym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
            PowerSensorOutput<false> asym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(-24.0 * 1e-4));

            CHECK(asym_sensor_output.id == 0);
            CHECK(asym_sensor_output.energized == 1);
            CHECK(asym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(3.1 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-24.0 * 1e-4));

            CHECK(asym_sensor_output_asym_param.id == 0);
            CHECK(asym_sensor_output_asym_param.energized == 1);
            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(3.9 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.1 * 1e3 / 3.0));

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<false> asym_power_sensor_update{};
            asym_power_sensor_update.power_sigma = 2.0 * 1e5;
            asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<false>{1.0};
            asym_power_sensor_update.p_sigma = r_nan;
            asym_power_sensor_update.q_sigma = r_nan;
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<true>();
            asym_sensor_param = asym_power_sensor.calc_param<false>();

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            asym_sensor_output = asym_power_sensor.get_output<true>(s_sym);
            asym_sensor_output_asym_param = asym_power_sensor.get_output<false>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.p_variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(sym_sensor_param.q_variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(-9.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(-12.0 * 1e-3));

            CHECK(asym_sensor_output.p_residual == doctest::Approx(9.9 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(12.7 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.p_variance[0] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(asym_sensor_param.q_variance[1] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(-9.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(-12.0 * 1e-3));

            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(9.9 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(12.7 * 1e3 / 3.0));

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

    SUBCASE("Asymmetric Power Sensor - Partial initialization and full update") {
        PowerSensorInput<false> asym_power_sensor_input{};
        asym_power_sensor_input.p_measured = r_nan;
        asym_power_sensor_input.q_measured = RealValue<false>{1.0};
        asym_power_sensor_input.p_sigma = r_nan;
        asym_power_sensor_input.q_sigma = r_nan;

        PowerSensorUpdate<false> asym_power_sensor_update{};
        asym_power_sensor_update.p_measured = RealValue<false>{1.0};
        asym_power_sensor_update.q_measured = r_nan;
        asym_power_sensor_update.p_sigma = r_nan;
        asym_power_sensor_update.q_sigma = r_nan;

        PowerSensor<false> asym_power_sensor{asym_power_sensor_input};
        asym_power_sensor.update(asym_power_sensor_update);

        auto const result = asym_power_sensor.get_output<false>({});
        CHECK(result.p_residual[0] != r_nan[0]);
        CHECK(result.p_residual[1] != r_nan[1]);
        CHECK(result.p_residual[2] != r_nan[2]);
        CHECK(result.q_residual[0] != r_nan[0]);
        CHECK(result.q_residual[1] != r_nan[1]);
        CHECK(result.q_residual[2] != r_nan[2]);
    }
}
} // namespace power_grid_model