// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "sensor.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"

#include <limits>

namespace power_grid_model {

class GenericVoltageSensor : public Sensor {
  public:
    static constexpr char const* name = "generic_voltage_sensor";

    explicit GenericVoltageSensor(GenericVoltageSensorInput const& generic_voltage_sensor_input)
        : Sensor{generic_voltage_sensor_input} {};

    template <symmetry_tag sym> VoltageSensorOutput<sym> get_output(ComplexValue<sym> const& u) const {
        if constexpr (is_symmetric_v<sym>) {
            assert(u != 0.0 + 0.0i);
            return get_sym_output(u);
        } else {
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

    template <symmetry_tag sym> VoltageSensorOutput<sym> get_null_output() const {
        return {.id = id(), .energized = false, .u_residual = {}, .u_angle_residual = {}};
    }

    SensorShortCircuitOutput get_null_sc_output() const { return {.id = id(), .energized = 0}; }

    // getter for calculation param
    template <symmetry_tag sym> VoltageSensorCalcParam<sym> calc_param() const {
        if constexpr (is_symmetric_v<sym>) {
            return sym_calc_param();
        } else {
            return asym_calc_param();
        }
    }

  private:
    // virtual function getter for sym and asym param
    // override them in real sensors function
    virtual VoltageSensorCalcParam<symmetric_t> sym_calc_param() const = 0;
    virtual VoltageSensorCalcParam<asymmetric_t> asym_calc_param() const = 0;

    virtual VoltageSensorOutput<symmetric_t> get_sym_output(ComplexValue<symmetric_t> const& u) const = 0;
    virtual VoltageSensorOutput<asymmetric_t> get_asym_output(ComplexValue<asymmetric_t> const& u) const = 0;
};

template <symmetry_tag sym> class VoltageSensor : public GenericVoltageSensor {
  public:
    static constexpr char const* name = is_symmetric_v<sym> ? "sym_voltage_sensor" : "asym_voltage_sensor";
    using InputType = VoltageSensorInput<sym>;
    using UpdateType = VoltageSensorUpdate<sym>;
    template <symmetry_tag sym_calc> using OutputType = VoltageSensorOutput<sym_calc>;

    explicit VoltageSensor(VoltageSensorInput<sym> const& voltage_sensor_input, double u_rated)
        : GenericVoltageSensor{voltage_sensor_input},
          u_rated_{u_rated},
          u_sigma_{voltage_sensor_input.u_sigma / (u_rated_ * u_scale<sym>)},
          u_measured_{voltage_sensor_input.u_measured / (u_rated_ * u_scale<sym>)},
          u_angle_measured_{voltage_sensor_input.u_angle_measured} {};

    UpdateChange update(VoltageSensorUpdate<sym> const& update_data) {
        assert(update_data.id == this->id());

        double const scalar = 1 / (u_rated_ * u_scale<sym>);

        update_real_value<sym>(update_data.u_measured, u_measured_, scalar);
        update_real_value<sym>(update_data.u_angle_measured, u_angle_measured_, 1.0);

        if (!is_nan(update_data.u_sigma)) {
            u_sigma_ = update_data.u_sigma * scalar;
        }

        return {false, false};
    }

    VoltageSensorUpdate<sym> inverse(VoltageSensorUpdate<sym> update_data) const {
        assert(update_data.id == this->id());

        double const scalar = u_rated_ * u_scale<sym>;

        set_if_not_nan(update_data.u_measured, u_measured_ * scalar);
        set_if_not_nan(update_data.u_angle_measured, u_angle_measured_);
        set_if_not_nan(update_data.u_sigma, u_sigma_ * scalar);

        return update_data;
    }

  private:
    double u_rated_;
    double u_sigma_;
    RealValue<sym> u_measured_;
    RealValue<sym> u_angle_measured_;

    bool has_angle() const {
        if constexpr (is_symmetric_v<sym>) {
            return !is_nan(u_angle_measured_);
        } else {
            return !u_angle_measured_.isNaN().any();
        }
    }

    VoltageSensorCalcParam<symmetric_t> sym_calc_param() const final {
        double const u_variance = u_sigma_ * u_sigma_;
        if (has_angle()) {
            ComplexValue<symmetric_t> const u = pos_seq(u_measured_ * exp(1i * u_angle_measured_));
            return {u, u_variance};
        }
        ComplexValue<symmetric_t> const u{mean_val(u_measured_), nan};
        return {u, u_variance};
    }

    VoltageSensorCalcParam<asymmetric_t> asym_calc_param() const final {
        double const u_variance = u_sigma_ * u_sigma_;
        if (has_angle()) {
            ComplexValue<asymmetric_t> const u{u_measured_ * exp(1i * u_angle_measured_)};
            return {u, u_variance};
        }
        ComplexValue<asymmetric_t> const u = RealValue<asymmetric_t>{u_measured_} + DoubleComplex{0.0, nan};
        return {u, u_variance};
    }

    VoltageSensorOutput<symmetric_t> get_sym_output(ComplexValue<symmetric_t> const& u) const final {
        VoltageSensorOutput<symmetric_t> value;
        value.id = id();
        value.energized = 1;

        DoubleComplex const u1_measured = sym_calc_param().value;
        bool const has_angle = !is_nan(imag(u1_measured));
        if (has_angle) {
            value.u_residual = (cabs(u1_measured) - cabs(u)) * u_rated_;
        } else {
            value.u_residual = (real(u1_measured) - cabs(u)) * u_rated_;
        }
        value.u_angle_residual = arg(u1_measured) - arg(u);
        return value;
    }

    VoltageSensorOutput<asymmetric_t> get_asym_output(ComplexValue<asymmetric_t> const& u) const final {
        VoltageSensorOutput<asymmetric_t> value;
        value.id = id();
        value.energized = 1;
        value.u_residual = (u_measured_ - cabs(u)) * u_rated_ / sqrt3;
        value.u_angle_residual = u_angle_measured_ - arg(u);
        return value;
    }
};

using SymVoltageSensor = VoltageSensor<symmetric_t>;
using AsymVoltageSensor = VoltageSensor<asymmetric_t>;

} // namespace power_grid_model
