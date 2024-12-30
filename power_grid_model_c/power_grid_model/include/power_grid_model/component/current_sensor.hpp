// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "sensor.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"

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
    virtual CurrentSensorCalcParam<symmetric_t> sym_calc_param() const = 0;
    virtual CurrentSensorCalcParam<asymmetric_t> asym_calc_param() const = 0;

    virtual CurrentSensorOutput<symmetric_t> get_sym_output(ComplexValue<symmetric_t> const& i) const = 0;
    virtual CurrentSensorOutput<asymmetric_t> get_asym_output(ComplexValue<asymmetric_t> const& i) const = 0;
};

template <symmetry_tag current_sensor_symmetry_> class CurrentSensor : public GenericCurrentSensor {
  public:
    using current_sensor_symmetry = current_sensor_symmetry_;

    static constexpr char const* name =
        is_symmetric_v<current_sensor_symmetry> ? "sym_current_sensor" : "asym_current_sensor";
    using InputType = CurrentSensorInput<current_sensor_symmetry>;
    using UpdateType = CurrentSensorUpdate<current_sensor_symmetry>;
    template <symmetry_tag sym_calc> using OutputType = CurrentSensorOutput<sym_calc>;

    // TODO: add u_rated to calculate base_current = base_power_3p / u_rated / sqrt3
    explicit CurrentSensor(CurrentSensorInput<current_sensor_symmetry> const& current_sensor_input)
        : GenericCurrentSensor{current_sensor_input},
          i_sigma_{current_sensor_input.i_sigma / base_power<current_sensor_symmetry>},
          i_angle_measured_{current_sensor_input.i_angle_measured} {
        set_current(current_sensor_input.i_measured);
    };

    UpdateChange update(CurrentSensorUpdate<current_sensor_symmetry> const& update_data) {
        // TODO
        return {false, false};
    }

    PowerSensorUpdate<current_sensor_symmetry> inverse(PowerSensorUpdate<current_sensor_symmetry> update_data) const {
        // TODO

        return update_data;
    }

  private:
    ComplexValue<current_sensor_symmetry> i_measured_{};
    RealValue<current_sensor_symmetry> i_angle_measured_{};
    double i_sigma_{};

    void set_current(RealValue<current_sensor_symmetry> const& i_measured) {
        // TODO
    }

    CurrentSensorCalcParam<symmetric_t> sym_calc_param() const final {
        CurrentSensorCalcParam<symmetric_t> calc_param{};
        // TODO
        return calc_param;
    }
    CurrentSensorCalcParam<asymmetric_t> asym_calc_param() const final {
        CurrentSensorCalcParam<asymmetric_t> calc_param{};
        // TODO
        return calc_param;
    }
    CurrentSensorOutput<symmetric_t> get_sym_output(ComplexValue<symmetric_t> const& i) const final {
        return get_generic_output<symmetric_t>(i);
    }
    CurrentSensorOutput<asymmetric_t> get_asym_output(ComplexValue<asymmetric_t> const& i) const final {
        return get_generic_output<asymmetric_t>(i);
    }
    template <symmetry_tag sym_calc>
    CurrentSensorOutput<sym_calc> get_generic_output(ComplexValue<sym_calc> const& i) const {
        CurrentSensorOutput<sym_calc> output{};
        return output;
    }
};

using SymCurrentSensor = CurrentSensor<symmetric_t>;
using AsymCurrentSensor = CurrentSensor<asymmetric_t>;

} // namespace power_grid_model