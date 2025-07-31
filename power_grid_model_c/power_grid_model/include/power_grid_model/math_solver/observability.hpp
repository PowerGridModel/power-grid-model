// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../common/exception.hpp"

namespace power_grid_model::math_solver {

namespace detail {
struct ObservabilitySensorsResult {
    std::vector<int8_t> flow_sensors;
    std::vector<int8_t> voltage_phasor_sensors;
    bool is_possibly_ill_conditioned{false};
};

// count flow and voltage phasor sensors for the observability check
// use the ybus structure
// the lower triangular part is always zero
// the diagonal part will be one if there is a complete bus injection sensor or a voltage phasor sensor
// the upper triangular part will be one when there exist branch flow sensors and the branch is fully connected
// return a ObservabilitySensorsResult struct with
//      a vector of flow sensor count
//      a vector of voltage phasor sensor count
//      a boolean indicating if the system is possibly ill-conditioned
template <symmetry_tag sym>
ObservabilitySensorsResult count_observability_sensors(MeasuredValues<sym> const& measured_values,
                                                       MathModelTopology const& topo,
                                                       YBusStructure const& y_bus_structure) {
    Idx const n_bus{topo.n_bus()};

    ObservabilitySensorsResult result{.flow_sensors = std::vector<int8_t>(y_bus_structure.row_indptr.back(), 0),
                                      .voltage_phasor_sensors = std::vector<int8_t>(n_bus, 0),
                                      .is_possibly_ill_conditioned = false};

    auto has_flow_sensor = [&measured_values](Idx branch) {
        return measured_values.has_branch_from_power(branch) || measured_values.has_branch_to_power(branch) ||
               measured_values.has_branch_from_current(branch) || measured_values.has_branch_to_current(branch);
    };

    auto is_branch_connected = [&topo](Idx branch) {
        return topo.branch_bus_idx[branch][0] != -1 && topo.branch_bus_idx[branch][1] != -1;
    };

    for (Idx row = 0; row != n_bus; ++row) {
        bool has_at_least_one_sensor{false};
        // lower triangle is ignored and kept as zero
        // diagonal for bus injection measurement
        if (measured_values.has_bus_injection(row)) {
            result.flow_sensors[y_bus_structure.bus_entry[row]] = 1;
            has_at_least_one_sensor = true;
        }
        // upper triangle for branch flow measurement
        for (Idx ybus_index = y_bus_structure.bus_entry[row] + 1; ybus_index != y_bus_structure.row_indptr[row + 1];
             ++ybus_index) {
            for (Idx element_index = y_bus_structure.y_bus_entry_indptr[ybus_index];
                 element_index != y_bus_structure.y_bus_entry_indptr[ybus_index + 1]; ++element_index) {
                auto const& element = y_bus_structure.y_bus_element[element_index];
                // shunt should not be considered
                if (element.element_type == YBusElementType::shunt) {
                    continue;
                }
                // if the branch is fully connected and measured, we consider it as a valid flow sensor
                // we only need one flow sensor, so the loop will break
                Idx const branch = element.idx;
                if (has_flow_sensor(branch) && is_branch_connected(branch)) {
                    result.flow_sensors[ybus_index] = 1;
                    has_at_least_one_sensor = true;
                    break;
                }
            }
        }
        // diagonal for voltage phasor sensors
        if (measured_values.has_voltage(row) && measured_values.has_angle_measurement(row)) {
            has_at_least_one_sensor = true;
            result.voltage_phasor_sensors[row] = 1;
        }
        // the system could be ill-conditioned if there is no flow sensor for one bus, except the last bus
        if (!has_at_least_one_sensor && row != n_bus - 1) {
            result.is_possibly_ill_conditioned = true;
        }
    }
    return result;
}

// re-organize the flow and voltage phasor sensors for a radial grid
// this mutates the flow sensor and voltage phasor sensor vectors by trying to first
// assign an injection sensor to a branch sensor if available, if not then it tries
// to assign a voltage phasor sensor to a branch sensor
// if the system is observable, all the branches should be measured afterwards
// if the grid is not radial, the behavior is undefined

inline void assign_independent_sensors_radial(YBusStructure const& y_bus_structure, std::vector<int8_t>& flow_sensors,
                                              std::vector<int8_t>& voltage_phasor_sensors) {
    Idx const n_bus{std::ssize(y_bus_structure.row_indptr) - 1};
    // loop the row without the last bus
    for (Idx row = 0; row != n_bus - 1; ++row) {
        Idx const current_bus = row;
        // upstream_bus_diagonal, concerns only the voltage phasor sensors as they are only on
        // the buses (diagonal entries)
        Idx const upstream_bus_diagonal = current_bus + 1;
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
            } else if (voltage_phasor_sensors[upstream_bus_diagonal] == 1) {
                // if not possible, try to steal voltage phasor sensor from upstream bus
                std::swap(flow_sensors[branch_entry_upstream], voltage_phasor_sensors[upstream_bus_diagonal]);
            }
        }
        // remove the current bus injection sensors regardless of the original state
        flow_sensors[bus_entry_current] = 0;
    }
    // set last bus injection to zero
    flow_sensors[y_bus_structure.bus_entry[n_bus - 1]] = 0;
}

inline bool necessary_observability_condition(ObservabilitySensorsResult const& observability_sensors, Idx const n_bus,
                                              Idx& n_voltage_phasor_sensors, bool has_global_angle_current) {
    auto const flow_sensors = std::span<const int8_t>{observability_sensors.flow_sensors};
    auto const voltage_phasor_sensors = std::span<const int8_t>{observability_sensors.voltage_phasor_sensors};
    // count total flow sensors and phasor voltage sensors, note we manually specify the intial value type to avoid
    // overflow
    Idx const n_flow_sensors = std::reduce(flow_sensors.begin(), flow_sensors.end(), Idx{}, std::plus<Idx>{});
    n_voltage_phasor_sensors =
        std::reduce(voltage_phasor_sensors.begin(), voltage_phasor_sensors.end(), Idx{}, std::plus<Idx>{});

    if (n_voltage_phasor_sensors == 0 && n_flow_sensors < n_bus - 1) {
        throw NotObservableError{
            "The total number of independent power sensors is not enough to make the grid observable."};
    }
    // If there are any voltage phasor sensor, one will be reserved as reference and not be used:
    //      n_flow_sensors + n_voltage_phasor_sensors - 1 < n_bus - 1
    if (n_voltage_phasor_sensors > 0 && n_flow_sensors + n_voltage_phasor_sensors < n_bus) {
        throw NotObservableError{"The total number of independent power sensors and voltage phasor sensors is not "
                                 "enough to make the grid observable."};
    }
    if (has_global_angle_current && n_voltage_phasor_sensors == 0) {
        throw NotObservableError{
            "Global angle current sensors require at least one voltage angle measurement as a reference point.\n"};
    }

    return true;
}

inline bool sufficient_observability_condition(YBusStructure const& y_bus_structure,
                                               ObservabilitySensorsResult& observability_sensors,
                                               Idx const n_voltage_phasor_sensors) {
    std::vector<int8_t>& flow_sensors = observability_sensors.flow_sensors;
    std::vector<int8_t>& voltage_phasor_sensors = observability_sensors.voltage_phasor_sensors;
    Idx const n_bus{std::ssize(y_bus_structure.row_indptr) - 1};

    // for a radial grid, try to assign injection or phasor voltage sensors to unmeasured branches
    assign_independent_sensors_radial(y_bus_structure, flow_sensors, voltage_phasor_sensors);

    // count independent flow sensors and remaining voltage phasor sensors
    Idx const n_independent_flow_sensors =
        std::reduce(observability_sensors.flow_sensors.cbegin(), observability_sensors.flow_sensors.cend(), Idx{},
                    std::plus<Idx>{});
    Idx const n_remaining_voltage_phasor_sensors =
        std::reduce(observability_sensors.voltage_phasor_sensors.cbegin(),
                    observability_sensors.voltage_phasor_sensors.cend(), Idx{}, std::plus<Idx>{});

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
    bool is_observable{false};
    bool is_possibly_ill_conditioned{false};
    constexpr bool use_perturbation() const { return is_possibly_ill_conditioned && is_observable; }
};

template <symmetry_tag sym>
inline ObservabilityResult observability_check(MeasuredValues<sym> const& measured_values,
                                               MathModelTopology const& topo, YBusStructure const& y_bus_structure) {
    bool is_necessary_condition_met{false};
    bool is_sufficient_condition_met{false};
    Idx const n_bus{topo.n_bus()};
    assert(n_bus == std::ssize(y_bus_structure.row_indptr) - 1);

    if (!measured_values.has_voltage_measurements()) {
        throw NotObservableError{"No voltage sensor found!\n"};
    }

    detail::ObservabilitySensorsResult observability_sensors =
        detail::count_observability_sensors(measured_values, topo, y_bus_structure);
    Idx n_voltage_phasor_sensors{};

    // check necessary condition for observability
    is_necessary_condition_met = detail::necessary_observability_condition(
        observability_sensors, n_bus, n_voltage_phasor_sensors, measured_values.has_global_angle_current());

    // check the sufficient condition for observability
    // the check is currently only implemented for radial grids
    if (topo.is_radial) {
        is_sufficient_condition_met = detail::sufficient_observability_condition(y_bus_structure, observability_sensors,
                                                                                 n_voltage_phasor_sensors);
    }
    //  ToDo(JGuo): meshed network will require a different treatment
    return ObservabilityResult{.is_observable = is_necessary_condition_met && is_sufficient_condition_met,
                               .is_possibly_ill_conditioned = observability_sensors.is_possibly_ill_conditioned};
}

} // namespace power_grid_model::math_solver
