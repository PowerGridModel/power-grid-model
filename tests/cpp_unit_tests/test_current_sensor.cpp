// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/current_sensor.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
TEST_CASE("Test current sensor") {
    SUBCASE("Symmetric Current Sensor - generator, branch_from, branch_to, source") {
        for (auto const terminal_type : {MeasuredTerminalType::generator, MeasuredTerminalType::branch_from,
                                         MeasuredTerminalType::branch_to, MeasuredTerminalType::source}) {
            CAPTURE(terminal_type);

            CurrentSensorInput<symmetric_t> sym_current_sensor_input{};
            sym_current_sensor_input.id = 0;
            sym_current_sensor_input.measured_object = 1;
            sym_current_sensor_input.measured_terminal_type = terminal_type;
            sym_current_sensor_input.angle_measurement_type = AngleMeasurementType::local;
            sym_current_sensor_input.i_sigma = 1.0;
            sym_current_sensor_input.i_measured = 1.0 * 1e3;
            sym_current_sensor_input.i_angle_measured = 0.0;
            sym_current_sensor_input.i_angle_sigma = nan;

            double const u_rated = 10.0e3;
            double const base_current = base_power_3p / u_rated / sqrt3;

            ComplexValue<symmetric_t> const i_sym = (1.0 * 1e3 + 1i * 0.0) / base_current;
            ComplexValue<asymmetric_t> const i_asym = i_sym * RealValue<asymmetric_t>{1.0};

            CurrentSensor<symmetric_t> sym_current_sensor{sym_current_sensor_input, u_rated};

            CurrentSensorCalcParam<symmetric_t> sym_sensor_param = sym_current_sensor.calc_param<symmetric_t>();
            CurrentSensorCalcParam<asymmetric_t> asym_sensor_param = sym_current_sensor.calc_param<asymmetric_t>();

            CurrentSensorOutput<symmetric_t> sym_sensor_output = sym_current_sensor.get_output<symmetric_t>(i_sym);
            CurrentSensorOutput<asymmetric_t> sym_sensor_output_asym_param =
                sym_current_sensor.get_output<asymmetric_t>(i_asym);

            // Check symmetric sensor output for symmetric parameters
            CHECK(sym_sensor_param.i_variance == doctest::Approx(0.0));
            CHECK(sym_sensor_param.i_angle_variance == doctest::Approx(0.0));
            CHECK(real(sym_sensor_param.value) == doctest::Approx(0.0));
            CHECK(imag(sym_sensor_param.value) == doctest::Approx(0.0));

            CHECK(sym_sensor_output.id == 0);
            CHECK(sym_sensor_output.energized == 0);
            CHECK(sym_sensor_output.i_residual == doctest::Approx(0.0));
            CHECK(sym_sensor_output.i_angle_residual == doctest::Approx(0.0));

            // Check symmetric sensor output for asymmetric parameters
            CHECK(asym_sensor_param.i_variance[0] == doctest::Approx(0.0));
            CHECK(asym_sensor_param.i_angle_variance[1] == doctest::Approx(0.0));
            CHECK(real(asym_sensor_param.value[0]) == doctest::Approx(0.0));
            CHECK(imag(asym_sensor_param.value[1]) == doctest::Approx(0.0));

            CHECK(sym_sensor_output_asym_param.id == 0);
            CHECK(sym_sensor_output_asym_param.energized == 0);
            CHECK(sym_sensor_output_asym_param.i_residual[0] == doctest::Approx(0.0));
            CHECK(sym_sensor_output_asym_param.i_angle_residual[1] == doctest::Approx(0.0));

            CHECK(sym_current_sensor.get_terminal_type() == terminal_type);

            CHECK(sym_current_sensor.get_angle_measurement_type() == AngleMeasurementType::local);
        }
    }
}

} // namespace power_grid_model
