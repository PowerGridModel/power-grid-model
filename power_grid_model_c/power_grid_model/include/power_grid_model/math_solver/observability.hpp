// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"

#include "../common/exception.hpp"

namespace power_grid_model::math_solver {

template <symmetry_tag sym>
inline void necessary_observability_check(MeasuredValues<sym> const& measured_values,
                                          std::shared_ptr<MathModelTopology const> topo) {

    Idx n_bus_{topo->n_bus()};
    // std::vector<BranchIdx> const& branch_bus_idx{topo->branch_bus_idx};
    Idx n_voltage_magnitude_sensor{};
    Idx n_voltage_phasor_sensor{};
    Idx n_branch_sensor{};
    Idx n_injection_sensor{};

    for (Idx bus = 0; bus != n_bus_; ++bus) {
        if (measured_values.has_voltage(bus)) {
            n_voltage_magnitude_sensor++;
            if (measured_values.has_angle_measurement(bus)) {
                n_voltage_phasor_sensor++;
            }
        }
    }

    if (n_voltage_magnitude_sensor + n_voltage_phasor_sensor < 1) {
        throw NotObservableError{};
    }

    for (Idx bus = 0; bus != n_bus_; ++bus) {
        if (measured_values.has_bus_injection(bus)) {
            n_injection_sensor++;
        }
    }
    // for (BranchIdx const branch : branch_bus_idx) {
    //     if (measured_values.has_branch_from(branch_bus_idx[0]) || measured_values.has_branch_to(branch_bus_idx[0]) ||
    //         measured_values.has_branch_from(branch_bus_idx[1]) || measured_values.has_branch_to(branch_bus_idx[1])) {
    //         n_branch_sensor++;
    //     }
    // }
    n_branch_sensor = 1000;

    Idx const n_power_sensor = n_branch_sensor + n_injection_sensor;

    if (n_voltage_phasor_sensor == 0 && n_power_sensor < n_bus_ - 1) {
        throw NotObservableError{};
    } else if (n_voltage_phasor_sensor > 0 && n_power_sensor + n_voltage_phasor_sensor < n_bus_) {
        throw NotObservableError{};
    }
}
} // namespace power_grid_model::math_solver
