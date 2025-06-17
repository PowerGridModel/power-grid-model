// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../common/exception.hpp"

namespace power_grid_model::math_solver {

namespace detail {
// count flow and voltage phasor sensors for the observability check
// use the ybus structure
// the lower triangular part is always zero
// the diagonal part will be one if there is bus injection or a voltage phasor sensor
// the upper triangle part will be one if there is branch flow sensor and the branch is fully connected
// return a tuple of
//      a vector of flow sensor count
//      a vector of voltage phasor sensor count
//      a boolean indicating if the system is possibly ill-conditioned
template <symmetry_tag sym>
std::tuple<std::vector<int8_t>, std::vector<int8_t>, bool>
count_observability_sensors(MeasuredValues<sym> const& measured_values, MathModelTopology const& topo,
                            YBusStructure const& y_bus_structure) {
    Idx const n_bus{topo.n_bus()};
    auto voltage_phasor_sensors = std::vector<int8_t>(n_bus, 0);
    auto flow_sensors = std::vector<int8_t>(y_bus_structure.row_indptr.back(), 0);
    auto possibly_ill_conditioned = false;
    for (Idx row = 0; row != n_bus; ++row) {
        bool has_at_least_one_sensor{false};
        // lower triangle is ignored and kept as zero
        // diagonal for bus injection measurement
        if (measured_values.has_bus_injection(row)) {
            flow_sensors[y_bus_structure.bus_entry[row]] = 1;
            has_at_least_one_sensor = true;
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
                    if ((measured_values.has_branch_from_power(branch) || measured_values.has_branch_to_power(branch) ||
                         measured_values.has_branch_from_current(branch) ||
                         measured_values.has_branch_to_current(branch)) &&
                        topo.branch_bus_idx[branch][0] != -1 && topo.branch_bus_idx[branch][1] != -1) {
                        flow_sensors[ybus_index] = 1;
                        has_at_least_one_sensor = true;
                        break;
                    }
                }
            }
        }
        // diagonal for voltage phasor sensors
        if (measured_values.has_voltage(row) && measured_values.has_angle_measurement(row)) {
            has_at_least_one_sensor = true;
            voltage_phasor_sensors[row] = 1;
        }
        // the system could be ill-conditioned if there is no flow sensor for one bus, except the last bus
        if (!has_at_least_one_sensor && row != n_bus - 1) {
            possibly_ill_conditioned = true;
        }
    }
    return std::make_tuple(std::move(flow_sensors), std::move(voltage_phasor_sensors), possibly_ill_conditioned);
}

// re-organize the flow sensor for radial grid without phasor measurement
// this mutate the flow sensor vector to try to assign injection sensor to branch sensor
// all the branch should be measured if the system is observable
// this is a sufficient condition check
// if the grid is not radial, the behavior is undefined.
inline void assign_independent_sensors_radial(YBusStructure const& y_bus_structure, std::vector<int8_t>& flow_sensors,
                                              std::vector<int8_t>& voltage_phasor_sensors, Idx const n_bus) {
    // loop the row without the last bus
    for (Idx row = 0; row != n_bus - 1; ++row) {
        Idx const current_bus = row;
        Idx const bus_entry_current = y_bus_structure.bus_entry[current_bus];
        Idx const branch_entry_upstream = bus_entry_current + 1;
        // there should be only one upstream branch in the upper diagonal
        // so the next of branch_entry_upstream is already the next row
        // because the grid is radial
        // parallel branches (same from and to nodes) are considered as one
        // branch for observability purposes
        assert(y_bus_structure.row_indptr[current_bus + 1] == branch_entry_upstream + 1);
        Idx const upstream_bus = y_bus_structure.col_indices[branch_entry_upstream];
        Idx const bus_entry_upstream = y_bus_structure.bus_entry[upstream_bus];
        // if the upstream branch is not measured
        if (flow_sensors[branch_entry_upstream] == 0) {
            if (flow_sensors[bus_entry_current] == 1) {
                // try to steal injection sensor from current bus
                std::swap(flow_sensors[branch_entry_upstream], flow_sensors[bus_entry_current]);
            } else if (flow_sensors[bus_entry_upstream] == 1) {
                // if not possible, try to steal injection sensor from upstream bus
                std::swap(flow_sensors[branch_entry_upstream], flow_sensors[bus_entry_upstream]);
            } else if (voltage_phasor_sensors[current_bus] == 1) {
                // if not possible, try to steal voltage phasor sensor from current bus
                std::swap(flow_sensors[branch_entry_upstream], voltage_phasor_sensors[current_bus]);
            } else if (voltage_phasor_sensors[current_bus + 1] == 1) {
                // if not possible, try to steal voltage phasor sensor from upstream bus
                std::swap(flow_sensors[branch_entry_upstream], voltage_phasor_sensors[current_bus + 1]);
            }
        }
        // remove the current bus injection sensors regardless of the original state
        flow_sensors[bus_entry_current] = 0;
    }
    // set last bus injection to zero
    flow_sensors[y_bus_structure.bus_entry[n_bus - 1]] = 0;
}

inline void necessary_observability_condition(Idx const n_voltage_phasor_sensors, Idx const n_flow_sensors,
                                              Idx const n_bus, bool const has_global_angle_current) {
    if (n_voltage_phasor_sensors == 0 && n_flow_sensors < n_bus - 1) {
        throw NotObservableError{};
    }
    if (n_voltage_phasor_sensors > 0 && n_flow_sensors + n_voltage_phasor_sensors < n_bus) {
        throw NotObservableError{};
    }

    if (has_global_angle_current && n_voltage_phasor_sensors == 0) {
        throw NotObservableError{
            "Global angle current sensors require at least one voltage angle measurement as a reference point.\n"};
    }
}

inline bool sufficient_observability_condition(YBusStructure const& y_bus_structure, std::vector<int8_t>& flow_sensors,
                                               std::vector<int8_t>& voltage_phasor_sensors,
                                               Idx const n_voltage_phasor_sensors, Idx const n_bus) {
    // for a radial grid, try to assign injection or phasor voltage sensors to unmeasured branches
    assign_independent_sensors_radial(y_bus_structure, flow_sensors, voltage_phasor_sensors, n_bus);

    // count independent flow sensors and remaining voltage phasor sensors
    Idx const n_independent_flow_sensors =
        std::reduce(flow_sensors.cbegin(), flow_sensors.cend(), Idx{}, std::plus<Idx>{});
    Idx const n_remaining_voltage_phasor_sensors =
        std::reduce(voltage_phasor_sensors.cbegin(), voltage_phasor_sensors.cend(), Idx{}, std::plus<Idx>{});

    if ((n_independent_flow_sensors < n_bus - 1) ||
        (n_voltage_phasor_sensors > 0 && n_remaining_voltage_phasor_sensors < 1)) {
        throw NotObservableError{"The number of power, current, and voltage phasor sensors appears sufficient, but "
                                 "they are not independent "
                                 "enough. The system is still not observable.\n"};
    }
    return true;
}

} // namespace detail

struct ObservabilityResult {
    bool is_sufficiently_observable{false};
    bool is_possibly_ill_conditioned{false};
    constexpr bool use_perturbation() const { return is_possibly_ill_conditioned && is_sufficiently_observable; }
};

template <symmetry_tag sym>
inline ObservabilityResult observability_check(MeasuredValues<sym> const& measured_values,
                                               MathModelTopology const& topo, YBusStructure const& y_bus_structure) {
    Idx const n_bus{topo.n_bus()};
    ObservabilityResult result{};

    if (!measured_values.has_voltage_measurements()) {
        throw NotObservableError{"No voltage sensor found!\n"};
    }

    auto [flow_sensors, voltage_phasor_sensors, is_possibly_ill_conditioned] =
        detail::count_observability_sensors(measured_values, topo, y_bus_structure);
    result.is_possibly_ill_conditioned = is_possibly_ill_conditioned;

    // count flow and phasor voltage sensors, note we manually specify the intial value type to avoid overflow
    Idx const n_flow_sensors = std::reduce(flow_sensors.cbegin(), flow_sensors.cend(), Idx{}, std::plus<Idx>{});
    Idx const n_voltage_phasor_sensors =
        std::reduce(voltage_phasor_sensors.cbegin(), voltage_phasor_sensors.cend(), Idx{}, std::plus<Idx>{});

    // check necessary condition for observability
    detail::necessary_observability_condition(n_voltage_phasor_sensors, n_flow_sensors, n_bus,
                                              measured_values.has_global_angle_current());

    // check the sufficient condition for observability
    // the check is currently only implemented for radial grids
    if (topo.is_radial) {
        result.is_sufficiently_observable = detail::sufficient_observability_condition(
            y_bus_structure, flow_sensors, voltage_phasor_sensors, n_voltage_phasor_sensors, n_bus);
    }
    return result;
}

} // namespace power_grid_model::math_solver
