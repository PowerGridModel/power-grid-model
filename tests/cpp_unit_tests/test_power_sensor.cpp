// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/power_sensor.hpp>

#include <doctest/doctest.h>

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

            PowerSensorInput<symmetric_t> sym_power_sensor_input{};
            sym_power_sensor_input.id = 0;
            sym_power_sensor_input.measured_object = 1;
            sym_power_sensor_input.measured_terminal_type = terminal_type;
            sym_power_sensor_input.power_sigma = 1.0 * 1e5;
            sym_power_sensor_input.p_measured = 1.0 * 1e3;
            sym_power_sensor_input.q_measured = 0.8 * 1e3;
            sym_power_sensor_input.p_sigma = nan;
            sym_power_sensor_input.q_sigma = nan;

            ComplexValue<symmetric_t> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<asymmetric_t> const s_asym = s_sym * RealValue<asymmetric_t>{1.0};

            PowerSensor<symmetric_t> sym_power_sensor{sym_power_sensor_input};

            PowerSensorCalcParam<symmetric_t> sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            PowerSensorCalcParam<asymmetric_t> asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            PowerSensorOutput<symmetric_t> sym_sensor_output = sym_power_sensor.get_output<symmetric_t>(s_sym);
            PowerSensorOutput<asymmetric_t> sym_sensor_output_asym_param =
                sym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(1.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(8.0 * 1e-4));

            CHECK(sym_sensor_output.id == 0);
            CHECK(sym_sensor_output.energized == 1);
            CHECK(sym_sensor_output.p_residual == doctest::Approx(1.0 * 1e2));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(1.0 * 1e2));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(1.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(8.0 * 1e-4));

            CHECK(sym_sensor_output_asym_param.id == 0);
            CHECK(sym_sensor_output_asym_param.energized == 1);
            CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.0 * 1e2 / 3.0));
            CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.0 * 1e2 / 3.0));

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<symmetric_t> sym_power_sensor_update{};
            sym_power_sensor_update.power_sigma = 2.0 * 1e5;
            sym_power_sensor_update.p_measured = 3.0 * 1e3;
            sym_power_sensor_update.q_measured = 4.0 * 1e3;
            sym_power_sensor_update.p_sigma = nan;
            sym_power_sensor_update.q_sigma = nan;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            sym_sensor_output = sym_power_sensor.get_output<symmetric_t>(s_sym);
            sym_sensor_output_asym_param = sym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(4.0 * 1e-3));

            CHECK(sym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(3.3 * 1e3));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(4.0 * 1e-3));

            CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
            CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.3 * 1e3 / 3.0));

            // Check update nan
            sym_power_sensor_update.power_sigma = nan;
            sym_power_sensor_update.p_measured = nan;
            sym_power_sensor_update.q_measured = nan;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(4.0 * 1e-3));

            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(4.0 * 1e-3));

            // Check update with different p and q sigma variances
            sym_power_sensor_update.p_sigma = 1.0e5;
            sym_power_sensor_update.q_sigma = 3.0e5;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(1.0 / 1e2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(9.0 / 1e2));

            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(1.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(9.0 / 1e2));
        }
    }

    SUBCASE("Symmetric Power Sensor - shunt, load") {
        for (auto const terminal_type : {MeasuredTerminalType::shunt, MeasuredTerminalType::load}) {
            CAPTURE(terminal_type);

            PowerSensorInput<symmetric_t> sym_power_sensor_input{};
            sym_power_sensor_input.id = 0;
            sym_power_sensor_input.measured_object = 1;
            sym_power_sensor_input.measured_terminal_type = terminal_type;
            sym_power_sensor_input.power_sigma = 1.0 * 1e5;
            sym_power_sensor_input.p_measured = 1.0 * 1e3;
            sym_power_sensor_input.q_measured = 0.8 * 1e3;
            sym_power_sensor_input.p_sigma = nan;
            sym_power_sensor_input.q_sigma = nan;

            ComplexValue<symmetric_t> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<asymmetric_t> const s_asym = s_sym * RealValue<asymmetric_t>{1.0};

            PowerSensor<symmetric_t> sym_power_sensor{sym_power_sensor_input};

            PowerSensorCalcParam<symmetric_t> sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            PowerSensorCalcParam<asymmetric_t> asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            PowerSensorOutput<symmetric_t> sym_sensor_output = sym_power_sensor.get_output<symmetric_t>(s_sym);
            PowerSensorOutput<asymmetric_t> sym_sensor_output_asym_param =
                sym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(-1.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(-8.0 * 1e-4));

            CHECK(sym_sensor_output.id == 0);
            CHECK(sym_sensor_output.energized == 1);
            CHECK(sym_sensor_output.p_residual == doctest::Approx(1.9 * 1e3));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(1.5 * 1e3));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(1.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(-1.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(-8.0 * 1e-4));

            CHECK(sym_sensor_output_asym_param.id == 0);
            CHECK(sym_sensor_output_asym_param.energized == 1);
            CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(1.9 * 1e3 / 3.0));
            CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.5 * 1e3 / 3.0));

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<symmetric_t> sym_power_sensor_update{};
            sym_power_sensor_update.power_sigma = 2.0 * 1e5;
            sym_power_sensor_update.p_measured = 3.0 * 1e3;
            sym_power_sensor_update.q_measured = 4.0 * 1e3;
            sym_power_sensor_update.p_sigma = nan;
            sym_power_sensor_update.q_sigma = nan;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_power_sensor.get_terminal_type() == terminal_type);

            sym_sensor_output = sym_power_sensor.get_output<symmetric_t>(s_sym);
            sym_sensor_output_asym_param = sym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(-4.0 * 1e-3));

            CHECK(sym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
            CHECK(sym_sensor_output.q_residual == doctest::Approx(4.7 * 1e3));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(-4.0 * 1e-3));

            CHECK(sym_sensor_output_asym_param.p_residual[0] == doctest::Approx(3.9 * 1e3 / 3.0));
            CHECK(sym_sensor_output_asym_param.q_residual[1] == doctest::Approx(4.7 * 1e3 / 3.0));

            // Check update nan
            sym_power_sensor_update.power_sigma = nan;
            sym_power_sensor_update.p_measured = nan;
            sym_power_sensor_update.q_measured = nan;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(-4.0 * 1e-3));

            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(4.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(-4.0 * 1e-3));

            // Check update with different p and q sigma variances
            sym_power_sensor_update.p_sigma = 1.0e5;
            sym_power_sensor_update.q_sigma = 3.0e5;
            sym_power_sensor.update(sym_power_sensor_update);

            sym_sensor_param = sym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = sym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(1.0 / 1e2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(9.0 / 1e2));

            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(1.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(9.0 / 1e2));
        }
    }

    SUBCASE("Symmetric Power Sensor - Partial initialization and full update") {
        PowerSensorInput<symmetric_t> sym_power_sensor_input{};
        sym_power_sensor_input.p_measured = nan;
        sym_power_sensor_input.q_measured = RealValue<symmetric_t>{1.0};
        sym_power_sensor_input.p_sigma = nan;
        sym_power_sensor_input.q_sigma = nan;

        PowerSensorUpdate<symmetric_t> sym_power_sensor_update{};
        sym_power_sensor_update.p_measured = RealValue<symmetric_t>{1.0};
        sym_power_sensor_update.q_measured = nan;
        sym_power_sensor_update.p_sigma = nan;
        sym_power_sensor_update.q_sigma = nan;

        PowerSensor<symmetric_t> sym_power_sensor{sym_power_sensor_input};
        sym_power_sensor.update(sym_power_sensor_update);

        auto const result = sym_power_sensor.get_output<symmetric_t>({});
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

            PowerSensorInput<asymmetric_t> asym_power_sensor_input{};
            asym_power_sensor_input.id = 0;
            asym_power_sensor_input.measured_object = 1;
            asym_power_sensor_input.measured_terminal_type = terminal_type;
            asym_power_sensor_input.power_sigma = 1.0 * 1e5;
            asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_input.p_sigma = r_nan;
            asym_power_sensor_input.q_sigma = r_nan;

            ComplexValue<symmetric_t> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<asymmetric_t> const s_asym = s_sym * RealValue<asymmetric_t>{1.0};

            PowerSensor<asymmetric_t> asym_power_sensor{asym_power_sensor_input};

            PowerSensorCalcParam<symmetric_t> sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            PowerSensorCalcParam<asymmetric_t> asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            PowerSensorOutput<symmetric_t> asym_sensor_output = asym_power_sensor.get_output<symmetric_t>(s_sym);
            PowerSensorOutput<asymmetric_t> asym_sensor_output_asym_param =
                asym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(24.0 * 1e-4));

            CHECK(asym_sensor_output.id == 0);
            CHECK(asym_sensor_output.energized == 1);
            CHECK(asym_sensor_output.p_residual == doctest::Approx(2.1 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(1.7 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(24.0 * 1e-4));

            CHECK(asym_sensor_output_asym_param.id == 0);
            CHECK(asym_sensor_output_asym_param.energized == 1);
            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(2.1 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(1.7 * 1e3 / 3.0));

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<asymmetric_t> asym_power_sensor_update{};
            asym_power_sensor_update.power_sigma = 2.0 * 1e5;
            asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_update.p_sigma = r_nan;
            asym_power_sensor_update.q_sigma = r_nan;
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            asym_sensor_output = asym_power_sensor.get_output<symmetric_t>(s_sym);
            asym_sensor_output_asym_param = asym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(9.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(12.0 * 1e-3));

            CHECK(asym_sensor_output.p_residual == doctest::Approx(8.1 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(11.3 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(9.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(12.0 * 1e-3));

            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(8.1 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(11.3 * 1e3 / 3.0));

            // update with partial nan
            asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
            asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            CHECK(real(sym_sensor_param.value()) == doctest::Approx(16.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(21.0 * 1e-3));

            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(18.0 * 1e-3));
            CHECK(real(asym_sensor_param.value()[1]) == doctest::Approx(9.0 * 1e-3));
            CHECK(real(asym_sensor_param.value()[2]) == doctest::Approx(21.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[0]) == doctest::Approx(24.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(27.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[2]) == doctest::Approx(12.0 * 1e-3));

            // Check update with different p and q sigma variances
            asym_power_sensor_update.p_sigma = {1.0e5, 0.5e5, 2.0e5};
            asym_power_sensor_update.q_sigma = {3.0e5, 2.0e5, 4.0e5};
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(15.75 / 1e2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(87.0 / 1e2));

            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(9.0 / 1e2));
            CHECK(asym_sensor_param.real_component.variance[1] == doctest::Approx(2.25 / 1e2));
            CHECK(asym_sensor_param.real_component.variance[2] == doctest::Approx(36.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[0] == doctest::Approx(81.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(36.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[2] == doctest::Approx(144.0 / 1e2));
        }
    }

    SUBCASE("Asymmetric Power Sensor - shunt, load") {
        for (auto const terminal_type : {MeasuredTerminalType::shunt, MeasuredTerminalType::load}) {
            CAPTURE(terminal_type);

            PowerSensorInput<asymmetric_t> asym_power_sensor_input{};
            asym_power_sensor_input.id = 0;
            asym_power_sensor_input.measured_object = 1;
            asym_power_sensor_input.measured_terminal_type = terminal_type;
            asym_power_sensor_input.power_sigma = 1.0 * 1e5;
            asym_power_sensor_input.p_measured = 1.0 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_input.q_measured = 0.8 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_input.p_sigma = r_nan;
            asym_power_sensor_input.q_sigma = r_nan;

            ComplexValue<symmetric_t> const s_sym = (0.9 * 1e3 + 1i * 0.7 * 1e3) / 1e6;
            ComplexValue<asymmetric_t> const s_asym = s_sym * RealValue<asymmetric_t>{1.0};

            PowerSensor<asymmetric_t> asym_power_sensor{asym_power_sensor_input};

            PowerSensorCalcParam<symmetric_t> sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            PowerSensorCalcParam<asymmetric_t> asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            PowerSensorOutput<symmetric_t> asym_sensor_output = asym_power_sensor.get_output<symmetric_t>(s_sym);
            PowerSensorOutput<asymmetric_t> asym_sensor_output_asym_param =
                asym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(-24.0 * 1e-4));

            CHECK(asym_sensor_output.id == 0);
            CHECK(asym_sensor_output.energized == 1);
            CHECK(asym_sensor_output.p_residual == doctest::Approx(3.9 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(3.1 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(9.0 / 1e2 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(-3.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(-24.0 * 1e-4));

            CHECK(asym_sensor_output_asym_param.id == 0);
            CHECK(asym_sensor_output_asym_param.energized == 1);
            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(3.9 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(3.1 * 1e3 / 3.0));

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            // -------- Update power sensor --------
            PowerSensorUpdate<asymmetric_t> asym_power_sensor_update{};
            asym_power_sensor_update.power_sigma = 2.0 * 1e5;
            asym_power_sensor_update.p_measured = 3.0 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_update.q_measured = 4.0 * 1e3 * RealValue<asymmetric_t>{1.0};
            asym_power_sensor_update.p_sigma = r_nan;
            asym_power_sensor_update.q_sigma = r_nan;
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            CHECK(asym_power_sensor.get_terminal_type() == terminal_type);

            asym_sensor_output = asym_power_sensor.get_output<symmetric_t>(s_sym);
            asym_sensor_output_asym_param = asym_power_sensor.get_output<asymmetric_t>(s_asym);

            // Check asymmetric output for symmetric parameters
            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(sym_sensor_param.value()) == doctest::Approx(-9.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(-12.0 * 1e-3));

            CHECK(asym_sensor_output.p_residual == doctest::Approx(9.9 * 1e3));
            CHECK(asym_sensor_output.q_residual == doctest::Approx(12.7 * 1e3));

            // Check asymmetric output for asymmetric parameters
            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(3.6 / 1e1 / 2));
            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(-9.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(-12.0 * 1e-3));

            CHECK(asym_sensor_output_asym_param.p_residual[0] == doctest::Approx(9.9 * 1e3 / 3.0));
            CHECK(asym_sensor_output_asym_param.q_residual[1] == doctest::Approx(12.7 * 1e3 / 3.0));

            // update with partial nan
            asym_power_sensor_update.p_measured = {6.0 * 1e3, nan, 7.0 * 1e3};
            asym_power_sensor_update.q_measured = {8.0 * 1e3, 9.0 * 1e3, nan};
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            CHECK(real(sym_sensor_param.value()) == doctest::Approx(-16.0 * 1e-3));
            CHECK(imag(sym_sensor_param.value()) == doctest::Approx(-21.0 * 1e-3));

            CHECK(real(asym_sensor_param.value()[0]) == doctest::Approx(-18.0 * 1e-3));
            CHECK(real(asym_sensor_param.value()[1]) == doctest::Approx(-9.0 * 1e-3));
            CHECK(real(asym_sensor_param.value()[2]) == doctest::Approx(-21.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[0]) == doctest::Approx(-24.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[1]) == doctest::Approx(-27.0 * 1e-3));
            CHECK(imag(asym_sensor_param.value()[2]) == doctest::Approx(-12.0 * 1e-3));

            // Check update with different p and q sigma variances
            asym_power_sensor_update.p_sigma = {1.0e5, 0.5e5, 2.0e5};
            asym_power_sensor_update.q_sigma = {3.0e5, 2.0e5, 4.0e5};
            asym_power_sensor.update(asym_power_sensor_update);

            sym_sensor_param = asym_power_sensor.calc_param<symmetric_t>();
            asym_sensor_param = asym_power_sensor.calc_param<asymmetric_t>();

            CHECK(sym_sensor_param.real_component.variance == doctest::Approx(15.75 / 1e2));
            CHECK(sym_sensor_param.imag_component.variance == doctest::Approx(87.0 / 1e2));

            CHECK(asym_sensor_param.real_component.variance[0] == doctest::Approx(9.0 / 1e2));
            CHECK(asym_sensor_param.real_component.variance[1] == doctest::Approx(2.25 / 1e2));
            CHECK(asym_sensor_param.real_component.variance[2] == doctest::Approx(36.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[0] == doctest::Approx(81.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[1] == doctest::Approx(36.0 / 1e2));
            CHECK(asym_sensor_param.imag_component.variance[2] == doctest::Approx(144.0 / 1e2));
        }
    }

    SUBCASE("Asymmetric Power Sensor - Partial initialization and full update") {
        PowerSensorInput<asymmetric_t> asym_power_sensor_input{};
        asym_power_sensor_input.p_measured = r_nan;
        asym_power_sensor_input.q_measured = RealValue<asymmetric_t>{1.0};
        asym_power_sensor_input.p_sigma = r_nan;
        asym_power_sensor_input.q_sigma = r_nan;

        PowerSensorUpdate<asymmetric_t> asym_power_sensor_update{};
        asym_power_sensor_update.p_measured = RealValue<asymmetric_t>{1.0};
        asym_power_sensor_update.q_measured = r_nan;
        asym_power_sensor_update.p_sigma = r_nan;
        asym_power_sensor_update.q_sigma = r_nan;

        PowerSensor<asymmetric_t> asym_power_sensor{asym_power_sensor_input};
        asym_power_sensor.update(asym_power_sensor_update);

        auto const result = asym_power_sensor.get_output<asymmetric_t>({});
        CHECK(result.p_residual[0] != r_nan[0]);
        CHECK(result.p_residual[1] != r_nan[1]);
        CHECK(result.p_residual[2] != r_nan[2]);
        CHECK(result.q_residual[0] != r_nan[0]);
        CHECK(result.q_residual[1] != r_nan[1]);
        CHECK(result.q_residual[2] != r_nan[2]);
    }

    SUBCASE("Construction and update") {
        PowerSensorInput<symmetric_t> const sym_power_sensor_input{.id = 7,
                                                                   .measured_object = 3,
                                                                   .measured_terminal_type =
                                                                       MeasuredTerminalType::branch_from,
                                                                   .power_sigma = 269258.24035672517,
                                                                   .p_measured = -2e5,
                                                                   .q_measured = -1e6,
                                                                   .p_sigma = 2.5e5,
                                                                   .q_sigma = 1e5};
        PowerSensorUpdate<symmetric_t> const sym_power_sensor_update{.id = 7,
                                                                     .power_sigma = sym_power_sensor_input.power_sigma,
                                                                     .p_measured = sym_power_sensor_input.p_measured,
                                                                     .q_measured = sym_power_sensor_input.q_measured,
                                                                     .p_sigma = sym_power_sensor_input.p_sigma,
                                                                     .q_sigma = sym_power_sensor_input.q_sigma};

        SymPowerSensor sym_power_sensor{sym_power_sensor_input};
        auto const orig_calc_param = sym_power_sensor.calc_param<symmetric_t>();

        sym_power_sensor.update(sym_power_sensor_update);
        auto const updated_calc_param = sym_power_sensor.calc_param<symmetric_t>();

        CHECK(orig_calc_param.value() == updated_calc_param.value());
        CHECK(orig_calc_param.real_component.variance == updated_calc_param.real_component.variance);
        CHECK(orig_calc_param.imag_component.variance == updated_calc_param.imag_component.variance);
    }

    SUBCASE("Update inverse - sym") {
        constexpr auto power_sigma = 1.0;
        constexpr auto p_measured = 2.0;
        constexpr auto q_measured = 3.0;
        constexpr auto p_sigma = 4.0;
        constexpr auto q_sigma = 5.0;
        PowerSensor<symmetric_t> const power_sensor{{.id = 1,
                                                     .measured_object = 1,
                                                     .measured_terminal_type = MeasuredTerminalType::branch3_1,
                                                     .power_sigma = power_sigma,
                                                     .p_measured = p_measured,
                                                     .q_measured = q_measured,
                                                     .p_sigma = p_sigma,
                                                     .q_sigma = q_sigma}};

        PowerSensorUpdate<symmetric_t> ps_update{
            .id = 1, .power_sigma = nan, .p_measured = nan, .q_measured = nan, .p_sigma = nan, .q_sigma = nan};
        auto expected = ps_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("power_sigma") {
            SUBCASE("same") { ps_update.power_sigma = power_sigma; }
            SUBCASE("different") { ps_update.power_sigma = 0.0; }
            expected.power_sigma = power_sigma;
        }

        SUBCASE("p_measured") {
            SUBCASE("same") { ps_update.p_measured = p_measured; }
            SUBCASE("different") { ps_update.p_measured = 0.0; }
            expected.p_measured = p_measured;
        }

        SUBCASE("q_measured") {
            SUBCASE("same") { ps_update.q_measured = q_measured; }
            SUBCASE("different") { ps_update.q_measured = 0.0; }
            expected.q_measured = q_measured;
        }

        SUBCASE("p_sigma") {
            SUBCASE("same") { ps_update.p_sigma = p_sigma; }
            SUBCASE("different") { ps_update.p_sigma = 0.0; }
            expected.p_sigma = p_sigma;
        }

        SUBCASE("q_sigma") {
            SUBCASE("same") { ps_update.q_sigma = q_sigma; }
            SUBCASE("different") { ps_update.q_sigma = 0.0; }
            expected.q_sigma = q_sigma;
        }

        SUBCASE("multiple") {
            ps_update.power_sigma = 0.0;
            ps_update.p_measured = 0.0;
            ps_update.q_measured = 0.0;
            ps_update.p_sigma = 0.0;
            ps_update.q_sigma = 0.0;
            expected.power_sigma = power_sigma;
            expected.p_measured = p_measured;
            expected.p_sigma = p_sigma;
            expected.q_sigma = q_sigma;
        }

        auto const inv = power_sensor.inverse(ps_update);

        CHECK(inv.id == expected.id);
        check_nan_preserving_equality(inv.power_sigma, expected.power_sigma);
        check_nan_preserving_equality(inv.p_measured, expected.p_measured);
        check_nan_preserving_equality(inv.q_measured, expected.q_measured);
        check_nan_preserving_equality(inv.p_sigma, expected.p_sigma);
        check_nan_preserving_equality(inv.q_sigma, expected.q_sigma);
    }

    SUBCASE("Update inverse - asym") {
        constexpr auto power_sigma = 1.0;
        RealValue<asymmetric_t> const p_measured{2.0, 3.0, 4.0};
        RealValue<asymmetric_t> const q_measured{5.0, 6.0, 7.0};
        RealValue<asymmetric_t> const p_sigma{7.0, 8.0, 9.0};
        RealValue<asymmetric_t> const q_sigma{10.0, 11.0, 12.0};

        PowerSensorUpdate<asymmetric_t> ps_update{
            .id = 1, .power_sigma = nan, .p_measured = r_nan, .q_measured = r_nan, .p_sigma = r_nan, .q_sigma = r_nan};
        auto expected = ps_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("power_sigma") {
            SUBCASE("same") { ps_update.power_sigma = power_sigma; }
            SUBCASE("different") { ps_update.power_sigma = 0.0; }
            expected.power_sigma = power_sigma;
        }

        SUBCASE("p_measured") {
            SUBCASE("same") { ps_update.p_measured = p_measured; }
            SUBCASE("1 different") {
                ps_update.p_measured = {0.0, nan, nan};
                expected.p_measured = {p_measured(0), nan, nan};
            }
            SUBCASE("all different") {
                ps_update.p_measured = {0.0, 0.1, 0.2};
                expected.p_measured = p_measured;
            }
        }

        SUBCASE("p_sigma") {
            SUBCASE("same") { ps_update.p_sigma = p_sigma; }
            SUBCASE("1 different") {
                ps_update.p_sigma = {0.0, nan, nan};
                expected.p_sigma = {p_sigma(0), nan, nan};
            }
            SUBCASE("all different") {
                ps_update.p_sigma = {0.0, 0.4, 0.6};
                expected.p_sigma = p_sigma;
            }
        }

        SUBCASE("q_sigma") {
            SUBCASE("same") { ps_update.q_sigma = q_sigma; }
            SUBCASE("1 different") {
                ps_update.q_sigma = {0.0, nan, nan};
                expected.q_sigma = {q_sigma(0), nan, nan};
            }
            SUBCASE("all different") {
                ps_update.q_sigma = {0.0, 0.4, 0.6};
                expected.q_sigma = q_sigma;
            }
        }

        SUBCASE("multiple") {
            ps_update.power_sigma = 0.0;
            ps_update.p_measured = {0.0, 0.1, 0.2};
            ps_update.p_measured = {0.0, 0.2, 0.4};
            ps_update.p_sigma = {0.0, 0.3, 0.6};
            ps_update.q_sigma = {0.0, 0.4, 0.8};
            expected.power_sigma = power_sigma;
            expected.p_measured = p_measured;
            expected.p_sigma = p_sigma;
            expected.q_sigma = q_sigma;
        }

        for (auto const measured_terminal_type :
             {MeasuredTerminalType::branch_from, MeasuredTerminalType::generator, MeasuredTerminalType::load}) {
            PowerSensor<asymmetric_t> const power_sensor{{.id = 1,
                                                          .measured_object = 1,
                                                          .measured_terminal_type = measured_terminal_type,
                                                          .power_sigma = power_sigma,
                                                          .p_measured = p_measured,
                                                          .q_measured = q_measured,
                                                          .p_sigma = p_sigma,
                                                          .q_sigma = q_sigma}};
            auto const inv = power_sensor.inverse(ps_update);

            CHECK(inv.id == expected.id);
            check_nan_preserving_equality(inv.power_sigma, expected.power_sigma);
            check_nan_preserving_equality(inv.p_measured, expected.p_measured);
            check_nan_preserving_equality(inv.q_measured, expected.q_measured);
            check_nan_preserving_equality(inv.p_sigma, expected.p_sigma);
            check_nan_preserving_equality(inv.q_sigma, expected.q_sigma);
        }
    }
}
} // namespace power_grid_model
