// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "sensor.hpp"

namespace power_grid_model {

class GenericCurrentSensor : public Sensor {
  public:
    static constexpr char const* name = "generic_current_sensor";

    explicit GenericCurrentSensor(GenericCurrentSensorInput const& generic_current_sensor_input)
        : Sensor{generic_current_sensor_input},
          terminal_type_{generic_current_sensor_input.measured_terminal_type},
          angle_measurement_type_{generic_current_sensor_input.angle_measurement_type} {}

    MeasuredTerminalType get_terminal_type() const { return terminal_type_; }
    AngleMeasurementType get_angle_measurement_type() const { return angle_measurement_type_; }

    template <symmetry_tag sym> CurrentSensorOutput<sym> get_output(ComplexValue<sym> const& i) const {
        if constexpr (is_symmetric_v<sym>) {
            return get_sym_output(i);
        } else {
            return get_asym_output(i);
        }
    }

    template <symmetry_tag sym> CurrentSensorOutput<sym> get_null_output() const {
        return {.id = id(), .energized = false, .i_residual = {}, .i_angle_residual = {}};
    }

    SensorShortCircuitOutput get_null_sc_output() const { return {.id = id(), .energized = 0}; }

    // getter for calculation param
    template <symmetry_tag sym> PowerSensorCalcParam<sym> calc_param() const {
        if constexpr (is_symmetric_v<sym>) {
            return sym_calc_param();
        } else {
            return asym_calc_param();
        }
    }

  protected:
    double convert_direction() const {
        if (terminal_type_ == MeasuredTerminalType::load || terminal_type_ == MeasuredTerminalType::shunt) {
            return -1.0; // For shunt and load the direction in the math model is opposite to the direction in the
                         // physical model
        }
        return 1.0;
    }

  private:
    MeasuredTerminalType terminal_type_;
    AngleMeasurementType angle_measurement_type_;

    // virtual function getter for sym and asym param
    // override them in real sensors function
    virtual PowerSensorCalcParam<symmetric_t> sym_calc_param() const = 0;
    virtual PowerSensorCalcParam<asymmetric_t> asym_calc_param() const = 0;

    virtual PowerSensorOutput<symmetric_t> get_sym_output(ComplexValue<symmetric_t> const& i) const = 0;
    virtual PowerSensorOutput<asymmetric_t> get_asym_output(ComplexValue<asymmetric_t> const& i) const = 0;
};

} // namespace power_grid_model
