// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_VOLTAGE_SENSOR_HPP
#define POWER_GRID_MODEL_COMPONENT_VOLTAGE_SENSOR_HPP

#include "sensor.hpp"

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

#include <limits>

namespace power_grid_model {

class GenericVoltageSensor : public Sensor {
   public:
    static constexpr char const* name = "generic_voltage_sensor";

    explicit GenericVoltageSensor(GenericVoltageSensorInput const& generic_voltage_sensor_input)
        : Sensor{generic_voltage_sensor_input} {};

    template <bool sym>
    VoltageSensorOutput<sym> get_output(ComplexValue<sym> const& u) const {
        if constexpr (sym) {
            assert(u != 0.0 + 0.0i);
            return get_sym_output(u);
        }
        else {
            assert(u[0] != 0.0 + 0.0i && "Voltage should not be 0.0 + 0.0i V");
            assert(u[1] != 0.0 + 0.0i && "Voltage should not be 0.0 + 0.0i V");
            assert(u[2] != 0.0 + 0.0i && "Voltage should not be 0.0 + 0.0i V");
            assert(abs(arg(u[0]) - arg(u[1])) > std::numeric_limits<double>::epsilon() &&
                   "Voltage angles should not be equal");
            assert(abs(arg(u[1]) - arg(u[2])) > std::numeric_limits<double>::epsilon() &&
                   "Voltage angles should not be equal");
            assert(abs(arg(u[2]) - arg(u[0])) > std::numeric_limits<double>::epsilon() &&
                   "Voltage angles should not be equal");
            return get_asym_output(u);
        }
    }

    template <bool sym>
    VoltageSensorOutput<sym> get_null_output() const {
        return {{id(), false}, {}, {}};
    }

    SensorShortCircuitOutput get_null_sc_output() const {
        return {{id(), false}};
    }

   private:
    virtual VoltageSensorOutput<true> get_sym_output(ComplexValue<true> const& u) const = 0;
    virtual VoltageSensorOutput<false> get_asym_output(ComplexValue<false> const& u) const = 0;
};

template <bool sym>
class VoltageSensor : public GenericVoltageSensor {
   public:
    static constexpr char const* name = sym ? "sym_voltage_sensor" : "asym_voltage_sensor";
    using InputType = VoltageSensorInput<sym>;
    using UpdateType = VoltageSensorUpdate<sym>;
    template <bool sym_calc>
    using OutputType = VoltageSensorOutput<sym_calc>;

    explicit VoltageSensor(VoltageSensorInput<sym> const& voltage_sensor_input, double u_rated)
        : GenericVoltageSensor{voltage_sensor_input},
          u_rated_{u_rated},
          u_sigma_{voltage_sensor_input.u_sigma / (u_rated_ * u_scale<sym>)},
          u_measured_{voltage_sensor_input.u_measured / (u_rated_ * u_scale<sym>)},
          u_angle_measured_{voltage_sensor_input.u_angle_measured} {};

    UpdateChange update(VoltageSensorUpdate<sym> const& voltage_sensor_update) {
        double const scalar = 1 / (u_rated_ * u_scale<sym>);
        update_real_value<sym>(voltage_sensor_update.u_measured, u_measured_, scalar);
        update_real_value<sym>(voltage_sensor_update.u_angle_measured, u_angle_measured_, 1.0);

        if (!is_nan(voltage_sensor_update.u_sigma)) {
            u_sigma_ = voltage_sensor_update.u_sigma / (u_rated_ * u_scale<sym>);
        }

        return {false, false};
    }

   private:
    double u_rated_;
    double u_sigma_;
    RealValue<sym> u_measured_;
    RealValue<sym> u_angle_measured_;

    bool has_angle() const {
        if constexpr (sym) {
            return !is_nan(u_angle_measured_);
        }
        else {
            return !u_angle_measured_.isNaN().any();
        }
    }

    SensorCalcParam<true> sym_calc_param() const final {
        double const u_variance = u_sigma_ * u_sigma_;
        if (has_angle()) {
            ComplexValue<true> const u = pos_seq(u_measured_ * exp(1i * u_angle_measured_));
            return {u, u_variance};
        }
        else {
            ComplexValue<true> const u{mean_val(u_measured_), nan};
            return {u, u_variance};
        }
    }

    SensorCalcParam<false> asym_calc_param() const final {
        double const u_variance = u_sigma_ * u_sigma_;
        if (has_angle()) {
            ComplexValue<false> const u{u_measured_ * exp(1i * u_angle_measured_)};
            return {u, u_variance};
        }
        else {
            ComplexValue<false> const u = RealValue<false>{u_measured_} + DoubleComplex{0.0, nan};
            return {u, u_variance};
        }
    }

    VoltageSensorOutput<true> get_sym_output(ComplexValue<true> const& u) const final {
        VoltageSensorOutput<true> value;
        value.id = id();
        value.energized = 1;

        DoubleComplex const u1_measured = sym_calc_param().value;
        bool const has_angle = !is_nan(imag(u1_measured));
        if (has_angle) {
            value.u_residual = (cabs(u1_measured) - cabs(u)) * u_rated_;
        }
        else {
            value.u_residual = (real(u1_measured) - cabs(u)) * u_rated_;
        }
        value.u_angle_residual = arg(u1_measured) - arg(u);
        return value;
    }

    VoltageSensorOutput<false> get_asym_output(ComplexValue<false> const& u) const final {
        VoltageSensorOutput<false> value;
        value.id = id();
        value.energized = 1;
        value.u_residual = (u_measured_ - cabs(u)) * u_rated_ / sqrt3;
        value.u_angle_residual = u_angle_measured_ - arg(u);
        return value;
    }
};

using SymVoltageSensor = VoltageSensor<true>;
using AsymVoltageSensor = VoltageSensor<false>;

}  // namespace power_grid_model
#endif
