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

    Idx const n_bus{topo->n_bus()};
    std::vector<BranchIdx> const& branch_bus_idx{topo->branch_bus_idx};
    Idx n_voltage_magnitude_sensor{};
    Idx n_voltage_phasor_sensor{};
    Idx n_branch_sensor{};
    Idx n_injection_sensor{};

    // count voltage measurements
    for (Idx bus = 0; bus != n_bus; ++bus) {
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

    // count voltage measurements
    for (Idx bus = 0; bus != n_bus; ++bus) {
        if (measured_values.has_bus_injection(bus)) {
            n_injection_sensor++;
        }
    }

    // count branch measurements
    std::vector<bool> measured_nodes(n_bus, false);
    for (Idx branch = 0; branch != static_cast<Idx>(branch_bus_idx.size()); ++branch) {
        auto const& [node_from, node_to] = branch_bus_idx[branch];
        if (node_from == -1 || node_to == -1) {
            continue;
        }
        if ((measured_values.has_branch_from(branch) || measured_values.has_branch_to(branch)) &&
            !(measured_nodes[node_from] && measured_nodes[node_to])) {
            n_branch_sensor++;
            measured_nodes[node_from] = true;
            measured_nodes[node_to] = true;
        }
    }

    Idx const n_power_sensor = n_branch_sensor + n_injection_sensor;
    if (n_voltage_phasor_sensor == 0 && n_power_sensor < n_bus - 1) {
        throw NotObservableError{};
    }
    if (n_voltage_phasor_sensor > 0 && n_power_sensor + n_voltage_phasor_sensor < n_bus) {
        throw NotObservableError{};
    }
}
} // namespace power_grid_model::math_solver
