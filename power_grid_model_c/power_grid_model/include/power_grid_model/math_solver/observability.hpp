// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"

#include "../common/exception.hpp"

namespace power_grid_model::math_solver {

namespace detail {

template <symmetry_tag sym>
Idx count_branch_sensors(const std::vector<BranchIdx>& branch_bus_idx, const Idx n_bus,
                         const MeasuredValues<sym>& measured_values) {
    Idx n_branch_sensor{};
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
    return n_branch_sensor;
}

template <symmetry_tag sym>
Idx count_bus_injection_sensors(const Idx n_bus, const MeasuredValues<sym>& measured_values) {
    Idx n_injection_sensor{};
    for (Idx bus = 0; bus != n_bus; ++bus) {
        if (measured_values.has_bus_injection(bus)) {
            n_injection_sensor++;
        }
    }
    return n_injection_sensor;
}

template <symmetry_tag sym>
std::tuple<Idx, Idx> count_voltage_sensors(const Idx n_bus, const MeasuredValues<sym>& measured_values) {
    Idx n_voltage_magnitude{};
    Idx n_voltage_phasor{};
    for (Idx bus = 0; bus != n_bus; ++bus) {
        if (measured_values.has_voltage(bus)) {
            n_voltage_magnitude++;
            if (measured_values.has_angle_measurement(bus)) {
                n_voltage_phasor++;
            }
        }
    }
    return std::make_tuple(n_voltage_magnitude, n_voltage_phasor);
}

} // namespace detail
template <symmetry_tag sym>
inline void necessary_observability_check(MeasuredValues<sym> const& measured_values,
                                          std::shared_ptr<MathModelTopology const> const& topo) {

    Idx const n_bus{topo->n_bus()};
    std::vector<BranchIdx> const& branch_bus_idx{topo->branch_bus_idx};

    auto const [n_voltage_magnitude, n_voltage_phasor] = detail::count_voltage_sensors(n_bus, measured_values);
    if (n_voltage_magnitude + n_voltage_phasor < 1) {
        throw NotObservableError{};
    }
    Idx const n_injection_sensor = detail::count_bus_injection_sensors(n_bus, measured_values);
    Idx const n_branch_sensor = detail::count_branch_sensors(branch_bus_idx, n_bus, measured_values);

    if (n_voltage_phasor == 0 && n_branch_sensor + n_injection_sensor < n_bus - 1) {
        throw NotObservableError{};
    }
    if (n_voltage_phasor > 0 && n_branch_sensor + n_injection_sensor + n_voltage_phasor < n_bus) {
        throw NotObservableError{};
    }
}

} // namespace power_grid_model::math_solver
