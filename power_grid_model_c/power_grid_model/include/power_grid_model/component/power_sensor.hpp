// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_POWER_SENSOR_HPP
#define POWER_GRID_MODEL_COMPONENT_POWER_SENSOR_HPP

#include "sensor.hpp"

#include "../calculation_parameters.hpp"
#include "../enum.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"

namespace power_grid_model {

class GenericPowerSensor : public Sensor {
  public:
    static constexpr char const* name = "generic_power_sensor";

    explicit GenericPowerSensor(GenericPowerSensorInput const& generic_power_sensor_input)
        : Sensor{generic_power_sensor_input}, terminal_type_{generic_power_sensor_input.measured_terminal_type} {}

    MeasuredTerminalType get_terminal_type() const { return terminal_type_; }

    template <bool sym> PowerSensorOutput<sym> get_output(ComplexValue<sym> const& s) const {
        if constexpr (sym) {
            return get_sym_output(s);
        } else {
            return get_asym_output(s);
        }
    }

    template <bool sym> PowerSensorOutput<sym> get_null_output() const {
        return {.id = id(), .energized = false, .p_residual = {}, .q_residual = {}};
    }

    SensorShortCircuitOutput get_null_sc_output() const { return {.id = id(), .energized = 0}; }

    // getter for calculation param
    template <bool sym> PowerSensorCalcParam<sym> calc_param() const {
        if constexpr (sym) {
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
    virtual PowerSensorCalcParam<true> sym_calc_param() const = 0;
    virtual PowerSensorCalcParam<false> asym_calc_param() const = 0;

    virtual PowerSensorOutput<true> get_sym_output(ComplexValue<true> const& s) const = 0;
    virtual PowerSensorOutput<false> get_asym_output(ComplexValue<false> const& s) const = 0;
};

template <bool sym> class PowerSensor : public GenericPowerSensor {
  public:
    static constexpr char const* name = sym ? "sym_power_sensor" : "asym_power_sensor";
    using InputType = PowerSensorInput<sym>;
    using UpdateType = PowerSensorUpdate<sym>;
    template <bool sym_calc> using OutputType = PowerSensorOutput<sym_calc>;

    explicit PowerSensor(PowerSensorInput<sym> const& power_sensor_input)
        : GenericPowerSensor{power_sensor_input},
          apparent_power_sigma_{power_sensor_input.power_sigma / base_power<sym>},
          p_sigma_{power_sensor_input.p_sigma / base_power<sym>},
          q_sigma_{power_sensor_input.q_sigma / base_power<sym>} {
        set_power(power_sensor_input.p_measured, power_sensor_input.q_measured);
    };

    UpdateChange update(PowerSensorUpdate<sym> const& update_data) {
        constexpr double scalar = 1.0 / base_power<sym>;

        set_power(update_data.p_measured, update_data.q_measured);

        if (!is_nan(update_data.power_sigma)) {
            apparent_power_sigma_ = update_data.power_sigma * scalar;
        }
        update_real_value<sym>(update_data.p_sigma, p_sigma_, scalar);
        update_real_value<sym>(update_data.q_sigma, q_sigma_, scalar);

        return {false, false};
    }

    PowerSensorUpdate<sym> inverse(PowerSensorUpdate<sym> update_data) const {
        assert(update_data.id == this->id());

        auto const scalar = convert_direction() * base_power<sym>;

        set_if_not_nan(update_data.p_measured, real(s_measured_) * scalar);
        set_if_not_nan(update_data.q_measured, imag(s_measured_) * scalar);
        set_if_not_nan(update_data.power_sigma, apparent_power_sigma_ * base_power<sym>);
        set_if_not_nan(update_data.p_sigma, p_sigma_ * base_power<sym>);
        set_if_not_nan(update_data.q_sigma, q_sigma_ * base_power<sym>);

        return update_data;
    }

  private:
    ComplexValue<sym> s_measured_{};

    double apparent_power_sigma_{};
    RealValue<sym> p_sigma_{};
    RealValue<sym> q_sigma_{};

    void set_power(RealValue<sym> const& p_measured, RealValue<sym> const& q_measured) {
        double const scalar = convert_direction() / base_power<sym>;
        RealValue<sym> ps = real(s_measured_);
        RealValue<sym> qs = imag(s_measured_);
        update_real_value<sym>(p_measured, ps, scalar);
        update_real_value<sym>(q_measured, qs, scalar);
        s_measured_ = ps + 1.0i * qs;
    }

    PowerSensorCalcParam<true> sym_calc_param() const final {
        PowerSensorCalcParam<true> calc_param{};
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
    PowerSensorCalcParam<false> asym_calc_param() const final {
        PowerSensorCalcParam<false> calc_param{};
        if (is_normal(p_sigma_) && is_normal(q_sigma_)) {
            calc_param.p_variance = RealValue<false>{p_sigma_ * p_sigma_};
            calc_param.q_variance = RealValue<false>{q_sigma_ * q_sigma_};
        } else {
            auto const variance = RealValue<false>{is_nan(p_sigma_) ? apparent_power_sigma_ * apparent_power_sigma_ / 2
                                                                    : std::numeric_limits<double>::infinity()};
            calc_param.p_variance = variance;
            calc_param.q_variance = variance;
        }
        assert(is_nan(calc_param.p_variance) == is_nan(calc_param.q_variance));

        calc_param.value = piecewise_complex_value(s_measured_);
        return calc_param;
    }
    PowerSensorOutput<true> get_sym_output(ComplexValue<true> const& s) const final {
        return get_generic_output<true>(s);
    }
    PowerSensorOutput<false> get_asym_output(ComplexValue<false> const& s) const final {
        return get_generic_output<false>(s);
    }
    template <bool sym_calc> PowerSensorOutput<sym_calc> get_generic_output(ComplexValue<sym_calc> const& s) const {
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

using SymPowerSensor = PowerSensor<true>;
using AsymPowerSensor = PowerSensor<false>;

} // namespace power_grid_model

#endif
