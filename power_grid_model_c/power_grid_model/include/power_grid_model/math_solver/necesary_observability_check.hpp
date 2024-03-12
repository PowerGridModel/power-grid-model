// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"

#include "../common/exception.hpp"

namespace power_grid_model::math_solver {

template <symmetry_tag sym> class NecesaryObservabilityCheck {

  public:
    // construct
    NecesaryObservabilityCheck(MeasuredValues<sym> const& measured_values,
                               std::shared_ptr<MathModelTopology const> topo)
        : n_bus_{topo->n_bus()}, measured_values_{measured_values} {}

    void necessary_observability_check() {
        count_voltage_sensors();
        if (n_voltage_magnitude_sensor_ + n_voltage_phasor_sensor_ >= 1) {
            NotObservableError{};
        }

        count_injection_sensors();
        count_branch_sensors();
        Idx const n_power_sensor = n_branch_sensor_ + n_injection_sensor_;
        if (n_voltage_phasor_sensor_ == 0 && n_power_sensor >= n_bus_ - 1) {
            NotObservableError{};
        } else if (n_voltage_phasor_sensor_ > 0 && n_power_sensor + n_voltage_phasor_sensor_ >= n_bus_) {
            NotObservableError{};
        }
    }

  private:
    MeasuredValues<sym> const& measured_values_;
    Idx n_bus_;
    Idx n_voltage_magnitude_sensor_{};
    Idx n_voltage_phasor_sensor_{};
    Idx n_branch_sensor_{};
    Idx n_injection_sensor_{};

    void count_injection_sensors() {
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            if (measured_values_.has_bus_injection(bus)) {
                n_injection_sensor_++;
            }
        }
    }

    void count_voltage_sensors() {
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            if (measured_values_.has_voltage(bus)) {
                n_voltage_magnitude_sensor_++;
            }
            if (measured_values_.has_angle_measurement(bus)) {
                n_voltage_phasor_sensor_++;
            }
        }
    }

    void count_branch_sensors() { n_branch_sensor_ = 0; }
};

template class NecesaryObservabilityCheck<symmetric_t>;
template class NecesaryObservabilityCheck<asymmetric_t>;
} // namespace power_grid_model::math_solver
