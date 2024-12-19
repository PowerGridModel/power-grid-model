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

// count flow sensors into integer matrix with ybus structure
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

// re-organize the flow sensor for radial grid without phasor measurement
// this mutate the flow sensor vector to try to assign injection sensor to branch sensor
// all the branch should be measured if the system is observable
// this is a sufficient condition check
// if the grid is not radial, the behavior is undefined.
inline void assign_injection_sensor_radial(YBusStructure const& y_bus_structure, std::vector<int8_t>& flow_sensors) {
    Idx const n_bus = std::ssize(y_bus_structure.row_indptr) - 1;
    // loop the row without the last bus
    for (Idx row = 0; row != n_bus - 1; ++row) {
        Idx const current_bus = row;
        Idx const bus_entry_current = y_bus_structure.bus_entry[current_bus];
        Idx const branch_entry_upstream = bus_entry_current + 1;
        // there should be only one upstream branch in the upper diagonal
        // so the next of branch_entry_upstream is already the next row
        // because the grid is radial
        assert(y_bus_structure.row_indptr[current_bus + 1] == branch_entry_upstream + 1);
        Idx const upstream_bus = y_bus_structure.col_indices[branch_entry_upstream];
        Idx const bus_entry_upstream = y_bus_structure.bus_entry[upstream_bus];
        // if the upstream branch is not measured
        if (flow_sensors[branch_entry_upstream] == 0) {
            if (flow_sensors[bus_entry_current] == 1) {
                // try to steal from current bus
                std::swap(flow_sensors[branch_entry_upstream], flow_sensors[bus_entry_current]);
            } else if (flow_sensors[bus_entry_upstream] == 1) {
                // if not possible, steal from upstream bus
                std::swap(flow_sensors[branch_entry_upstream], flow_sensors[bus_entry_upstream]);
            }
        }
        // remove the current bus injection regardless of the original state
        flow_sensors[bus_entry_current] = 0;
    }
    // set last bus injection to zero
    flow_sensors[y_bus_structure.bus_entry[n_bus - 1]] = 0;
}

} // namespace detail

template <symmetry_tag sym>
inline void necessary_observability_check(MeasuredValues<sym> const& measured_values, MathModelTopology const& topo,
                                          YBusStructure const& y_bus_structure) {
    Idx const n_bus{topo.n_bus()};

    auto const [n_voltage_sensor, n_voltage_phasor_sensor] = detail::count_voltage_sensors(n_bus, measured_values);
    if (n_voltage_sensor < 1) {
        throw NotObservableError{"No voltage sensor found!\n"};
    }

    std::vector<int8_t> flow_sensors = detail::count_flow_sensors(measured_values, topo, y_bus_structure);
    // count flow sensors, note we manually specify the intial value type to avoid overflow
    Idx const n_flow_sensor = std::reduce(flow_sensors.cbegin(), flow_sensors.cend(), Idx{}, std::plus<Idx>{});

    // check nessessary condition for observability
    if (n_voltage_phasor_sensor == 0 && n_flow_sensor < n_bus - 1) {
        throw NotObservableError{};
    }
    if (n_voltage_phasor_sensor > 0 && n_flow_sensor + n_voltage_phasor_sensor < n_bus) {
        throw NotObservableError{};
    }

    // for radial grid without phasor measurement, try to assign injection sensor to branch sensor
    // we can then check sufficient condition for observability
    if (topo.is_radial && n_voltage_phasor_sensor == 0) {
        detail::assign_injection_sensor_radial(y_bus_structure, flow_sensors);
        // count flow sensors again
        Idx const n_flow_sensor_new = std::reduce(flow_sensors.cbegin(), flow_sensors.cend(), Idx{}, std::plus<Idx>{});
        if (n_flow_sensor_new < n_bus - 1) {
            throw NotObservableError{"The number of power sensors appears sufficient, but they are not independent "
                                     "enough. The system is still not observable.\n"};
        }
    }
}

} // namespace power_grid_model::math_solver
