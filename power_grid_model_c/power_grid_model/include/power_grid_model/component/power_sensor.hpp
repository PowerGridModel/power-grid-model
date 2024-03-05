// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "sensor.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"

namespace power_grid_model {

class GenericPowerSensor : public Sensor {
  public:
    static constexpr char const* name = "generic_power_sensor";

    explicit GenericPowerSensor(GenericPowerSensorInput const& generic_power_sensor_input)
        : Sensor{generic_power_sensor_input}, terminal_type_{generic_power_sensor_input.measured_terminal_type} {}

    MeasuredTerminalType get_terminal_type() const { return terminal_type_; }

    template <symmetry_tag sym> PowerSensorOutput<sym> get_output(ComplexValue<sym> const& s) const {
        if constexpr (is_symmetric_v<sym>) {
            return get_sym_output(s);
        } else {
            return get_asym_output(s);
        }
    }

    template <symmetry_tag sym> PowerSensorOutput<sym> get_null_output() const {
        return {.id = id(), .energized = false, .p_residual = {}, .q_residual = {}};
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

    // virtual function getter for sym and asym param
    // override them in real sensors function
    virtual PowerSensorCalcParam<symmetric_t> sym_calc_param() const = 0;
    virtual PowerSensorCalcParam<asymmetric_t> asym_calc_param() const = 0;

    virtual PowerSensorOutput<symmetric_t> get_sym_output(ComplexValue<symmetric_t> const& s) const = 0;
    virtual PowerSensorOutput<asymmetric_t> get_asym_output(ComplexValue<asymmetric_t> const& s) const = 0;
};

template <symmetry_tag power_sensor_symmetry_> class PowerSensor : public GenericPowerSensor {
  public:
    using power_sensor_symmetry = power_sensor_symmetry_;

    static constexpr char const* name =
        is_symmetric_v<power_sensor_symmetry> ? "sym_power_sensor" : "asym_power_sensor";
    using InputType = PowerSensorInput<power_sensor_symmetry>;
    using UpdateType = PowerSensorUpdate<power_sensor_symmetry>;
    template <symmetry_tag sym_calc> using OutputType = PowerSensorOutput<sym_calc>;

    explicit PowerSensor(PowerSensorInput<power_sensor_symmetry> const& power_sensor_input)
        : GenericPowerSensor{power_sensor_input},
          apparent_power_sigma_{power_sensor_input.power_sigma / base_power<power_sensor_symmetry>},
          p_sigma_{power_sensor_input.p_sigma / base_power<power_sensor_symmetry>},
          q_sigma_{power_sensor_input.q_sigma / base_power<power_sensor_symmetry>} {
        set_power(power_sensor_input.p_measured, power_sensor_input.q_measured);
    };

    UpdateChange update(PowerSensorUpdate<power_sensor_symmetry> const& update_data) {
        constexpr double scalar = 1.0 / base_power<power_sensor_symmetry>;

        set_power(update_data.p_measured, update_data.q_measured);

        if (!is_nan(update_data.power_sigma)) {
            apparent_power_sigma_ = update_data.power_sigma * scalar;
        }
        update_real_value<power_sensor_symmetry>(update_data.p_sigma, p_sigma_, scalar);
        update_real_value<power_sensor_symmetry>(update_data.q_sigma, q_sigma_, scalar);

        return {false, false};
    }

    PowerSensorUpdate<power_sensor_symmetry> inverse(PowerSensorUpdate<power_sensor_symmetry> update_data) const {
        assert(update_data.id == this->id());

        auto const scalar = convert_direction() * base_power<power_sensor_symmetry>;

        set_if_not_nan(update_data.p_measured, real(s_measured_) * scalar);
        set_if_not_nan(update_data.q_measured, imag(s_measured_) * scalar);
        set_if_not_nan(update_data.power_sigma, apparent_power_sigma_ * base_power<power_sensor_symmetry>);
        set_if_not_nan(update_data.p_sigma, p_sigma_ * base_power<power_sensor_symmetry>);
        set_if_not_nan(update_data.q_sigma, q_sigma_ * base_power<power_sensor_symmetry>);

        return update_data;
    }

  private:
    ComplexValue<power_sensor_symmetry> s_measured_{};

    double apparent_power_sigma_{};
    RealValue<power_sensor_symmetry> p_sigma_{};
    RealValue<power_sensor_symmetry> q_sigma_{};

    void set_power(RealValue<power_sensor_symmetry> const& p_measured,
                   RealValue<power_sensor_symmetry> const& q_measured) {
        double const scalar = convert_direction() / base_power<power_sensor_symmetry>;
        RealValue<power_sensor_symmetry> ps = real(s_measured_);
        RealValue<power_sensor_symmetry> qs = imag(s_measured_);
        update_real_value<power_sensor_symmetry>(p_measured, ps, scalar);
        update_real_value<power_sensor_symmetry>(q_measured, qs, scalar);
        s_measured_ = ps + 1.0i * qs;
    }

    PowerSensorCalcParam<symmetric_t> sym_calc_param() const final {
        PowerSensorCalcParam<symmetric_t> calc_param{};
        if (is_normal(p_sigma_) && is_normal(q_sigma_)) {
            calc_param.p_variance = mean_val(p_sigma_ * p_sigma_);
            calc_param.q_variance = mean_val(q_sigma_ * q_sigma_);
        } else {
            auto const variance = is_nan(p_sigma_) ? apparent_power_sigma_ * apparent_power_sigma_ / 2
                                                   : std::numeric_limits<double>::infinity();
            calc_param.p_variance = variance;
            calc_param.q_variance = variance;
        }
        assert(is_nan(calc_param.p_variance) == is_nan(calc_param.q_variance));

        calc_param.value = mean_val(s_measured_);
        return calc_param;
    }
    PowerSensorCalcParam<asymmetric_t> asym_calc_param() const final {
        PowerSensorCalcParam<asymmetric_t> calc_param{};
        if (is_normal(p_sigma_) && is_normal(q_sigma_)) {
            calc_param.p_variance = RealValue<asymmetric_t>{p_sigma_ * p_sigma_};
            calc_param.q_variance = RealValue<asymmetric_t>{q_sigma_ * q_sigma_};
        } else {
            auto const variance =
                RealValue<asymmetric_t>{is_nan(p_sigma_) ? apparent_power_sigma_ * apparent_power_sigma_ / 2
                                                         : std::numeric_limits<double>::infinity()};
            calc_param.p_variance = variance;
            calc_param.q_variance = variance;
        }
        assert(is_nan(calc_param.p_variance) == is_nan(calc_param.q_variance));

        calc_param.value = piecewise_complex_value(s_measured_);
        return calc_param;
    }
    PowerSensorOutput<symmetric_t> get_sym_output(ComplexValue<symmetric_t> const& s) const final {
        return get_generic_output<symmetric_t>(s);
    }
    PowerSensorOutput<asymmetric_t> get_asym_output(ComplexValue<asymmetric_t> const& s) const final {
        return get_generic_output<asymmetric_t>(s);
    }
    template <symmetry_tag sym_calc>
    PowerSensorOutput<sym_calc> get_generic_output(ComplexValue<sym_calc> const& s) const {
        PowerSensorOutput<sym_calc> output{};
        ComplexValue<sym_calc> const s_residual{process_mean_val<sym_calc>(s_measured_ - s) * convert_direction() *
                                                base_power<sym_calc>};
        output.id = id();
        output.energized = 1; // power sensor is always energized
        output.p_residual = real(s_residual);
        output.q_residual = imag(s_residual);
        return output;
    }
};

using SymPowerSensor = PowerSensor<symmetric_t>;
using AsymPowerSensor = PowerSensor<asymmetric_t>;

} // namespace power_grid_model
