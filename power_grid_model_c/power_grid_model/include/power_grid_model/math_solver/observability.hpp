// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../common/exception.hpp"

namespace power_grid_model::math_solver {

namespace detail {

template <symmetry_tag sym>
std::tuple<Idx, Idx> count_voltage_sensors(const Idx n_bus, const MeasuredValues<sym>& measured_values) {
    Idx n_voltage_sensor{};
    Idx n_voltage_phasor_sensor{};
    for (Idx bus = 0; bus != n_bus; ++bus) {
        if (measured_values.has_voltage(bus)) {
            n_voltage_sensor++;
            if (measured_values.has_angle_measurement(bus)) {
                n_voltage_phasor_sensor++;
            }
        }
    }
    return std::make_tuple(n_voltage_sensor, n_voltage_phasor_sensor);
}

// count flow sensors into ybus structure like
// lower triangle part is always zero
// for diagonal part, it will be one if there is bus injection
// for upper triangle part, it will be one if there is branch flow sensor and the branch is fully connected
template <symmetry_tag sym>
std::vector<int8_t> count_flow_sensors(MeasuredValues<sym> const& measured_values, MathModelTopology const& topo,
                                       YBusStructure const& y_bus_structure) {
    Idx const n_bus{topo.n_bus()};
    std::vector<int8_t> flow_sensors(y_bus_structure.row_indptr.back(), 0); // initialize all to zero
    for (Idx row = 0; row != n_bus; ++row) {
        // lower triangle is ignored and kept as zero
        // diagonal for bus injection measurement
        if (measured_values.has_bus_injection(row)) {
            flow_sensors[y_bus_structure.bus_entry[row]] = 1;
        }
        // upper triangle for branch flow measurement
        for (Idx ybus_index = y_bus_structure.bus_entry[row] + 1; ybus_index != y_bus_structure.row_indptr[row + 1];
             ++ybus_index) {
            for (Idx element_index = y_bus_structure.y_bus_entry_indptr[ybus_index];
                 element_index != y_bus_structure.y_bus_entry_indptr[ybus_index + 1]; ++element_index) {
                YBusElement const& element = y_bus_structure.y_bus_element[element_index];
                // shunt should not be considered
                // if the branch is fully connected and measured, we consider it as a valid flow sensor
                // we only need one flow sensor, so the loop will break
                if (element.element_type != YBusElementType::shunt) {
                    Idx const branch = element.idx;
                    if ((measured_values.has_branch_from(branch) || measured_values.has_branch_to(branch)) &&
                        topo.branch_bus_idx[branch][0] != -1 && topo.branch_bus_idx[branch][1] != -1) {
                        flow_sensors[ybus_index] = 1;
                        break;
                    }
                }
            }
        }
    }
    return flow_sensors;
}

} // namespace detail
template <symmetry_tag sym>
inline void necessary_observability_check(MeasuredValues<sym> const& measured_values, MathModelTopology const& topo,
                                          YBusStructure const& y_bus_structure) {
    Idx const n_bus{topo.n_bus()};

    auto const [n_voltage_sensor, n_voltage_phasor_sensor] = detail::count_voltage_sensors(n_bus, measured_values);
    if (n_voltage_sensor < 1) {
        throw NotObservableError{"no voltage sensor found"};
    }

    auto const flow_sensors = detail::count_flow_sensors(measured_values, topo, y_bus_structure);
    // count flow sensors, note we manually specify the intial value type to avoid overflow
    Idx const n_flow_sensor = std::reduce(flow_sensors.cbegin(), flow_sensors.cend(), Idx{}, std::plus<Idx>{});

    if (n_voltage_phasor_sensor == 0 && n_flow_sensor < n_bus - 1) {
        throw NotObservableError{};
    }
    if (n_voltage_phasor_sensor > 0 && n_flow_sensor + n_voltage_phasor_sensor < n_bus) {
        throw NotObservableError{};
    }
}

} // namespace power_grid_model::math_solver
