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
    std::vector<int8_t> flow_sensors;           // power sensor and current sensor
    std::vector<int8_t> voltage_phasor_sensors; // voltage phasor sensors
    std::vector<int8_t> bus_injections;         // bus injections, zero injection and power sensors at buses
    int8_t total_injections{0};                 // total number of injections at buses
    bool is_possibly_ill_conditioned{false};
};
enum class ConnectivityStatus : std::int8_t {
    is_not_connected = -1,  // not connected, redundant
    has_no_measurement = 0, // connected branch or node, but no measurement (may have been used)
    node_measured = 0b010,  // the node has measurement and is not yet used
    branch_discovered_with_from_node_sensor = 0b001, // branch is discovered with node measurement at from side; >>
    branch_discovered_with_to_node_sensor = 0b100,   // branch is discovered with node measurement at to side; <<
    branch_native_measurement_unused = 0b111,        // branch has its own measurement, unused; & 0b101
    branch_native_measurement_consumed = 0b101       // branch discovered with its own measurement, used; | 0b010
};
// direct connected neighbour list
struct BusNeighbourhoodInfo {
    struct neighbour {
        Idx bus;                                                         // the bus index
        ConnectivityStatus status{ConnectivityStatus::is_not_connected}; // the neighbour connectivity status
    };
    Idx bus;                                                           // this bus index
    ConnectivityStatus status{ConnectivityStatus::has_no_measurement}; // this bus connectivity status
    std::vector<neighbour> direct_neighbours;                          // list of direct connected neighbours
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
ObservabilitySensorsResult scan_network_sensors(MeasuredValues<sym> const& measured_values,
                                                MathModelTopology const& topo, YBusStructure const& y_bus_structure,
                                                std::vector<BusNeighbourhoodInfo>& bus_neighbourhood_info) {
    Idx const n_bus{topo.n_bus()};

    ObservabilitySensorsResult result{.flow_sensors = std::vector<int8_t>(y_bus_structure.row_indptr.back(), 0),
                                      .voltage_phasor_sensors = std::vector<int8_t>(n_bus, 0),
                                      .bus_injections = std::vector<int8_t>(n_bus + 1, 0),
                                      .is_possibly_ill_conditioned = false};

    auto has_flow_sensor = [&measured_values](Idx branch) {
        return measured_values.has_branch_from_power(branch) || measured_values.has_branch_to_power(branch) ||
               measured_values.has_branch_from_current(branch) || measured_values.has_branch_to_current(branch);
    };

    auto is_branch_connected = [&topo](Idx branch) {
        return topo.branch_bus_idx[branch][0] != -1 && topo.branch_bus_idx[branch][1] != -1;
    };

    for (Idx bus = 0; bus != n_bus; ++bus) {
        bool has_at_least_one_sensor{false};
        Idx const current_bus_entry = y_bus_structure.bus_entry[bus];
        bus_neighbourhood_info[bus].bus = bus;
        // lower triangle is ignored ~~and kept as zero~~
        // diagonal for bus injection measurement
        if (measured_values.has_bus_injection(bus)) {
            result.bus_injections[bus] = 1;
            result.total_injections += 1;
            result.flow_sensors[current_bus_entry] = 1;
            has_at_least_one_sensor = true;
            bus_neighbourhood_info[bus].status = ConnectivityStatus::node_measured; // only treat power/0 injection
        }
        // upper triangle for branch flow measurement
        for (Idx ybus_index = current_bus_entry + 1; ybus_index != y_bus_structure.row_indptr[bus + 1]; ++ybus_index) {
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
                Idx const neighbour_bus = y_bus_structure.col_indices[ybus_index];
                BusNeighbourhoodInfo::neighbour const neighbour_info{.bus = neighbour_bus,
                                                                     .status = ConnectivityStatus::has_no_measurement};
                bus_neighbourhood_info[bus].direct_neighbours.push_back(neighbour_info);
                if (has_flow_sensor(branch) && is_branch_connected(branch)) {
                    result.flow_sensors[ybus_index] = 1;
                    has_at_least_one_sensor = true;
                    bus_neighbourhood_info[bus].direct_neighbours.back().status =
                        ConnectivityStatus::branch_native_measurement_unused;
                    break;
                }
            }
        }
        // diagonal for voltage phasor sensors
        if (measured_values.has_voltage(bus) && measured_values.has_angle_measurement(bus)) {
            has_at_least_one_sensor = true;
            result.voltage_phasor_sensors[bus] = 1;
        }
        // the system could be ill-conditioned if there is no flow sensor for one bus, except the last bus
        if (!has_at_least_one_sensor && bus != n_bus - 1) {
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
    for (Idx bus = 0; bus != n_bus - 1; ++bus) {
        Idx const current_bus = bus;
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

inline bool necessary_condition(ObservabilitySensorsResult const& observability_sensors, Idx const n_bus,
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

inline bool sufficient_condition_radial_with_voltage_phasor(YBusStructure const& y_bus_structure,
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

inline void complete_bidirectional_neighbourhood_info(std::vector<BusNeighbourhoodInfo>& bus_neighbourhood_info) {
    Idx const n_bus{static_cast<Idx>(bus_neighbourhood_info.size())};
    for (Idx bus = 0; bus != n_bus; ++bus) {
        for (auto const& neighbour : bus_neighbourhood_info[bus].direct_neighbours) {
            auto& reverse_neighbour_list = bus_neighbourhood_info[neighbour.bus].direct_neighbours;
            auto it = std::ranges::find_if(
                reverse_neighbour_list, [&bus](auto const& reverse_neighbour) { return reverse_neighbour.bus == bus; });
            if (it == reverse_neighbour_list.end()) {
                BusNeighbourhoodInfo::neighbour const reverse_neighbour_info{.bus = bus, .status = neighbour.status};
                reverse_neighbour_list.push_back(reverse_neighbour_info);
            }
        }
    }
}

inline void prepare_starting_nodes(std::vector<detail::BusNeighbourhoodInfo> const& neighbour_list, Idx n_bus,
                                   std::vector<Idx>& starting_candidates) {
    // First find a list of starting points. These are nodes without measurements and all edges connecting to it has no
    // edge measurements.
    for (Idx bus = 0; bus < n_bus; ++bus) {
        if (neighbour_list[bus].status == ConnectivityStatus::has_no_measurement) {
            bool all_neighbours_no_edge_measurement = true;
            for (const auto& neighbour : neighbour_list[bus].direct_neighbours) {
                if (neighbour.status == ConnectivityStatus::branch_native_measurement_unused) {
                    all_neighbours_no_edge_measurement = false;
                    break;
                }
            }
            if (all_neighbours_no_edge_measurement) {
                starting_candidates.push_back(bus);
            }
        }
    }

    // If no such starting point, find nodes without measurements
    if (starting_candidates.empty()) {
        for (Idx bus = 0; bus < n_bus; ++bus) {
            if (neighbour_list[bus].status == ConnectivityStatus::has_no_measurement) {
                starting_candidates.push_back(bus);
            }
        }
    }

    // If no nodes without measurements, start from first node
    // (but network should be observable, so this is just a fallback)
    if (starting_candidates.empty()) {
        starting_candidates.push_back(0);
    }
}

inline bool find_spanning_tree_from_node(Idx start_bus, Idx n_bus,
                                         std::vector<detail::BusNeighbourhoodInfo> const& _neighbour_list) {
    // Make a fresh copy for this attempt
    std::vector<detail::BusNeighbourhoodInfo> local_neighbour_list = _neighbour_list;

    enum class BusVisited : std::uint8_t { NotVisited, Visited };
    // Initialize tracking structures
    std::vector<BusVisited> visited(n_bus, BusVisited::NotVisited);
    std::vector<std::pair<Idx, Idx>> discovered_edges; // not poped, only for documenting
    std::vector<std::pair<Idx, Idx>> edge_track;       // for backtracking
    bool downwind = false;                             // downwind flag, 'need' to use measurement at current bus

    Idx current_bus = start_bus;
    Idx const max_iterations = n_bus * n_bus; // prevent infinite loops
    Idx iteration = 0;

    // Define lambda functions for the different priorities and backtracking
    auto try_native_edge_measurements = [&local_neighbour_list, &visited, &discovered_edges, &edge_track, &current_bus,
                                         &downwind](bool& step_success) {
        visited[current_bus] = BusVisited::Visited;
        for (auto& neighbour : local_neighbour_list[current_bus].direct_neighbours) {
            if (neighbour.status == ConnectivityStatus::branch_native_measurement_unused &&
                visited[neighbour.bus] == BusVisited::NotVisited) {
                // Mark edge as discovered and neighbour as visited
                discovered_edges.emplace_back(current_bus, neighbour.bus);
                edge_track.emplace_back(current_bus, neighbour.bus);
                visited[neighbour.bus] = BusVisited::Visited;

                // Update status to branch_native_measurement_consumed
                neighbour.status = ConnectivityStatus::branch_native_measurement_consumed;
                // Update reverse connection
                for (auto& reverse_neighbour : local_neighbour_list[neighbour.bus].direct_neighbours) {
                    if (reverse_neighbour.bus == current_bus) {
                        reverse_neighbour.status = ConnectivityStatus::branch_native_measurement_consumed;
                        break;
                    }
                }

                downwind = true; // set downwind
                current_bus = neighbour.bus;
                step_success = true;
                return true;
            }
        }
        return false;
    };

    auto try_downwind_measurement = [&local_neighbour_list, &visited, &discovered_edges, &edge_track, &current_bus,
                                     &downwind](bool& step_success, bool current_bus_no_measurement) {
        if (!current_bus_no_measurement && downwind) {
            for (auto& neighbour : local_neighbour_list[current_bus].direct_neighbours) {
                if (neighbour.status == ConnectivityStatus::has_no_measurement &&
                    visited[neighbour.bus] == BusVisited::NotVisited) {
                    discovered_edges.emplace_back(current_bus, neighbour.bus);
                    edge_track.emplace_back(current_bus, neighbour.bus);
                    visited[current_bus] = BusVisited::Visited;
                    visited[neighbour.bus] = BusVisited::Visited;

                    // Update status to branch_discovered_with_from_node_sensor
                    neighbour.status = ConnectivityStatus::branch_discovered_with_from_node_sensor;
                    // Update reverse connection
                    for (auto& reverse_neighbour : local_neighbour_list[neighbour.bus].direct_neighbours) {
                        if (reverse_neighbour.bus == current_bus) {
                            reverse_neighbour.status = ConnectivityStatus::branch_discovered_with_to_node_sensor;
                            break;
                        }
                    }

                    // Use current node's measurement
                    local_neighbour_list[current_bus].status = ConnectivityStatus::has_no_measurement;
                    current_bus = neighbour.bus;
                    step_success = true;
                    return true;
                }
            }
        }
        return false;
    };

    auto try_general_connection_rules = [&local_neighbour_list, &visited, &discovered_edges, &edge_track,
                                         &current_bus](bool& step_success, bool current_bus_no_measurement) {
        // Helper lambda to handle common edge processing logic
        auto process_edge = [&local_neighbour_list, &visited, &discovered_edges, &edge_track, &current_bus,
                             &step_success](auto& neighbour, ConnectivityStatus neighbour_status,
                                            ConnectivityStatus reverse_status, bool use_current_node) {
            discovered_edges.emplace_back(current_bus, neighbour.bus);
            edge_track.emplace_back(current_bus, neighbour.bus);
            visited[current_bus] = BusVisited::Visited;
            visited[neighbour.bus] = BusVisited::Visited;

            // Update neighbour status
            neighbour.status = neighbour_status;
            // Update reverse connection
            for (auto& reverse_neighbour : local_neighbour_list[neighbour.bus].direct_neighbours) {
                if (reverse_neighbour.bus == current_bus) {
                    reverse_neighbour.status = reverse_status;
                    break;
                }
            }

            // Use measurement from appropriate node
            if (use_current_node) {
                local_neighbour_list[current_bus].status = ConnectivityStatus::has_no_measurement;
            } else {
                local_neighbour_list[neighbour.bus].status = ConnectivityStatus::has_no_measurement;
            }

            current_bus = neighbour.bus;
            step_success = true;
            return true;
        };

        for (auto& neighbour : local_neighbour_list[current_bus].direct_neighbours) {
            if (visited[neighbour.bus] == BusVisited::Visited) {
                continue;
            }

            bool const neighbour_bus_has_no_measurement =
                local_neighbour_list[neighbour.bus].status == ConnectivityStatus::has_no_measurement;

            if (!current_bus_no_measurement && neighbour_bus_has_no_measurement) {
                // Case: current has measurement, neighbour empty (not in downwind mode)
                return process_edge(neighbour, ConnectivityStatus::branch_discovered_with_from_node_sensor,
                                    ConnectivityStatus::branch_discovered_with_to_node_sensor, true);
            }
            if (!neighbour_bus_has_no_measurement) {
                // Case: neighbour has measurement
                return process_edge(neighbour, ConnectivityStatus::branch_discovered_with_from_node_sensor,
                                    ConnectivityStatus::branch_discovered_with_to_node_sensor, false);
            }
        }
        return false;
    };

    // Helper function to reassign nodal measurement between two connected nodes
    auto reassign_nodal_measurement = [&local_neighbour_list](Idx from_node, Idx to_node) {
        // no reassignment possible if reached via edge measurement
        if (auto const branch_it =
                std::ranges::find_if(local_neighbour_list[from_node].direct_neighbours,
                                     [to_node](auto const& neighbour) { return neighbour.bus == to_node; });
            branch_it != local_neighbour_list[from_node].direct_neighbours.end() &&
            branch_it->status == ConnectivityStatus::branch_native_measurement_consumed) {
            return;
        }

        // Restore measurement at to_node
        local_neighbour_list[to_node].status = ConnectivityStatus::node_measured;

        // Use measurement at from_node
        local_neighbour_list[from_node].status = ConnectivityStatus::has_no_measurement;

        // Update connection statuses between the two nodes
        // Find and update the connection from from_node to to_node
        for (auto& neighbour : local_neighbour_list[from_node].direct_neighbours) {
            if (neighbour.bus == to_node) {
                // Change to upstream connection (from to_node to from_node perspective)
                neighbour.status = ConnectivityStatus::branch_discovered_with_to_node_sensor;
                break;
            }
        }

        // Find and update the reverse connection from to_node to from_node
        for (auto& neighbour : local_neighbour_list[to_node].direct_neighbours) {
            if (neighbour.bus == from_node) {
                // Change from from_node to to_node perspective
                neighbour.status = ConnectivityStatus::branch_discovered_with_from_node_sensor;
                break;
            }
        }
    };

    auto try_backtrack = [&edge_track, &local_neighbour_list, &reassign_nodal_measurement, &current_bus,
                          &downwind](bool& step_success) {
        if (!edge_track.empty()) {
            // Simple backtracking - go back along the last edge
            auto [last_edge_from, last_edge_to] = edge_track.back();
            edge_track.pop_back();

            Idx backtrack_to_bus;
            // Determine which node to backtrack to
            if (last_edge_from == current_bus) {
                backtrack_to_bus = last_edge_to;
            } else {
                // Find connected node
                backtrack_to_bus = last_edge_from;
            }

            // Consider reassignment if needed (downwind and current node still has measurement unused)
            if (downwind && local_neighbour_list[current_bus].status == ConnectivityStatus::node_measured) {
                // Reassign measurement from current node to the node we're backtracking to
                reassign_nodal_measurement(current_bus, backtrack_to_bus);
            }

            current_bus = backtrack_to_bus;
            step_success = true; // We made progress by backtracking
            return true;
        }
        return false;
    };

    while (std::ranges::count(visited, BusVisited::Visited) < n_bus && iteration < max_iterations) {
        ++iteration;
        bool step_success = false;

        if (bool const current_bus_no_measurement =
                local_neighbour_list[current_bus].status == ConnectivityStatus::has_no_measurement;
            // First priority: Check for native edge measurements
            !try_native_edge_measurements(step_success) &&
            // Second priority: If current node has measurement and we're in downwind mode
            !try_downwind_measurement(step_success, current_bus_no_measurement) &&
            // Third priority: General connection rules
            !try_general_connection_rules(step_success, current_bus_no_measurement)) {
            // If no progress, try backtracking
            try_backtrack(step_success);
        }

        if (!step_success) {
            break; // No more progress possible
        }
    }

    // Check if all nodes were visited (spanning tree found)
    return std::ranges::count(visited, BusVisited::Visited) == n_bus;
}

inline bool
sufficient_condition_meshed_without_voltage_phasor(std::vector<detail::BusNeighbourhoodInfo> const& neighbour_list) {
    auto const n_bus = static_cast<Idx>(neighbour_list.size());
    std::vector<Idx> starting_candidates;
    prepare_starting_nodes(neighbour_list, n_bus, starting_candidates);

    // Try each starting candidate
    bool const found_spanning_tree = std::ranges::any_of(starting_candidates, [&](Idx const start_bus) {
        return find_spanning_tree_from_node(start_bus, n_bus, neighbour_list);
    });

    if (!found_spanning_tree) {
        throw NotObservableError{"Meshed observability check fail. Network unobservable.\n"};
    }

    return true;
}

} // namespace detail

namespace observability {
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

    std::vector<detail::BusNeighbourhoodInfo> bus_neighbourhood_info(static_cast<std::size_t>(n_bus));
    detail::ObservabilitySensorsResult observability_sensors =
        detail::scan_network_sensors(measured_values, topo, y_bus_structure, bus_neighbourhood_info);

    // from unidirectional neighbour list to bidirectional
    detail::complete_bidirectional_neighbourhood_info(bus_neighbourhood_info);

    Idx n_voltage_phasor_sensors{};

    // check necessary condition for observability
    is_necessary_condition_met = detail::necessary_condition(observability_sensors, n_bus, n_voltage_phasor_sensors,
                                                             measured_values.has_global_angle_current());
    // Early return if necessary condition is not met
    if (!is_necessary_condition_met) {
        return ObservabilityResult{.is_observable = false,
                                   .is_possibly_ill_conditioned = observability_sensors.is_possibly_ill_conditioned};
    }

    //  Sufficient early out, enough nodal measurement equals observable
    if (observability_sensors.total_injections > n_bus - 2) {
        return ObservabilityResult{.is_observable = true,
                                   .is_possibly_ill_conditioned = observability_sensors.is_possibly_ill_conditioned};
    }

    // check the sufficient condition for observability
    // the check is currently only implemented for radial grids
    if (topo.is_radial) {
        is_sufficient_condition_met = detail::sufficient_condition_radial_with_voltage_phasor(
            y_bus_structure, observability_sensors, n_voltage_phasor_sensors);
    } else {
        is_sufficient_condition_met =
            detail::sufficient_condition_meshed_without_voltage_phasor(bus_neighbourhood_info);
    }

    return ObservabilityResult{.is_observable = is_necessary_condition_met && is_sufficient_condition_met,
                               .is_possibly_ill_conditioned = observability_sensors.is_possibly_ill_conditioned};
}

} // namespace observability

} // namespace power_grid_model::math_solver
