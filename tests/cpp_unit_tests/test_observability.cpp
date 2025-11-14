// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/math_solver/observability.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <ranges>

#include <doctest/doctest.h>

#include <algorithm>
#include <numeric>

namespace power_grid_model {

namespace {
void check_whether_observable(bool is_observable, MathModelTopology const& topo,
                              MathModelParam<symmetric_t> const& param,
                              StateEstimationInput<symmetric_t> const& se_input) {
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
    YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
    math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

    if (is_observable) {
        CHECK_NOTHROW(math_solver::observability::observability_check(measured_values, y_bus.math_topology(),
                                                                      y_bus.y_bus_structure()));
    } else {
        CHECK_THROWS_AS(math_solver::observability::observability_check(measured_values, y_bus.math_topology(),
                                                                        y_bus.y_bus_structure()),
                        NotObservableError);
    }
}

void check_observable(MathModelTopology const& topo, MathModelParam<symmetric_t> const& param,
                      StateEstimationInput<symmetric_t> const& se_input) {
    check_whether_observable(true, topo, param, se_input);
}

void check_not_observable(MathModelTopology const& topo, MathModelParam<symmetric_t> const& param,
                          StateEstimationInput<symmetric_t> const& se_input) {
    check_whether_observable(false, topo, param, se_input);
}
} // namespace

// Original integration tests
TEST_CASE("Observable voltage sensor - basic integration test") {
    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}, {1, 2}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 0, 0}};
    topo.power_sensors_per_bus = {from_sparse, {0, 0, 0, 0}};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 1, 2}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
    topo.current_sensors_per_branch_from = {from_sparse, {0, 0, 0}};
    topo.current_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 1}};

    MathModelParam<symmetric_t> param;
    param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
    param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.measured_voltage = {{.value = 1.0, .variance = 1.0}};
    se_input.measured_branch_from_power = {
        {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}},
        {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}}};

    check_observable(topo, param, se_input);
}

TEST_CASE("Test Observability - scan_network_sensors") {
    using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;
    using enum power_grid_model::math_solver::detail::ConnectivityStatus;
    using power_grid_model::math_solver::detail::scan_network_sensors;

    SUBCASE("Basic sensor scanning with simple topology") {
        // Create a simple 3-bus radial network: bus0--bus1--bus2
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.phase_shift = {0.0, 0.0, 0.0};
        topo.branch_bus_idx = {{0, 1}, {1, 2}};
        topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 1, 2, 3}};
        topo.load_gen_type = {LoadGenType::const_pq, LoadGenType::const_pq, LoadGenType::const_pq};
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1, 1}}; // Bus injection sensor at bus 2
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1, 1}}; // Branch sensor on branch 0
        topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 0, 0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 2}}; // Voltage sensors at bus 0 and 1

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
        param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};
        se_input.measured_voltage = {
            {.value = 1.0 + 0.5i, .variance = 1.0}, // Bus 0 - voltage phasor sensor
            {.value = {0.9, nan}, .variance = 1.0}  // Bus 1 - voltage magnitude sensor only
        };
        se_input.measured_bus_injection = {
            {.real_component = {.value = 2.0, .variance = 1.0}, .imag_component = {.value = 1.0, .variance = 1.0}}};
        se_input.measured_branch_from_power = {
            {.real_component = {.value = 1.5, .variance = 1.0}, .imag_component = {.value = 0.5, .variance = 1.0}}};
        se_input.load_gen_status = {1, 1, 1};

        // Create YBus and MeasuredValues
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        // Test scan_network_sensors
        std::vector<BusNeighbourhoodInfo> neighbour_results(3);
        auto result = scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // Verify basic structure
        CHECK(result.flow_sensors.size() == y_bus.y_bus_structure().row_indptr.back());
        CHECK(result.voltage_phasor_sensors.size() == 3); // n_bus
        CHECK(result.bus_injections.size() == 4);         // n_bus + 1

        // Verify voltage phasor sensors
        CHECK(result.voltage_phasor_sensors[0] == 1); // Bus 0 has voltage phasor (complex measurement)
        CHECK(result.voltage_phasor_sensors[1] == 0); // Bus 1 has only magnitude (no angle)
        CHECK(result.voltage_phasor_sensors[2] == 0); // Bus 2 has no voltage sensor

        // Verify bus injections - should count the bus injection sensor at bus 2
        CHECK(result.bus_injections[2] == 0); // Bus 2 has no injection sensor
        CHECK(result.total_injections == 1);  //
        CHECK(result.is_possibly_ill_conditioned == true);

        // Verify neighbour results structure
        CHECK(neighbour_results.size() == 3);
        for (size_t i = 0; i < neighbour_results.size(); ++i) {
            CHECK(neighbour_results[i].bus == static_cast<Idx>(i));
        }

        // Bus 2 should have node_measured status due to injection sensor
        CHECK(neighbour_results[0].direct_neighbours[0].status == branch_native_measurement_unused);
        CHECK(neighbour_results[2].status == has_no_measurement);
    }

    SUBCASE("Meshed network") {
        // Create a 6-bus meshed network:
        //                       bus0 (injection sensor)
        //                        [|] (branch sensor)
        //  bus1-[branch-sensor]-bus2 -(voltage)---[branch-sensor]----- bus3
        //                        [|] (branch sensor)                   [|] (branch sensor)
        //                       bus4 (injection sensor) -------------- bus5
        //
        // Branch sensors: bus1-bus2, bus3-bus5
        // Expected neighbour_result: {0: [2], 1: [2], 2: [3,4], 3: [5], 4: [5], 5:[]}

        using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;

        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.phase_shift = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

        // Define branches:
        // branch 0: bus0-bus2, branch 1: bus1-bus2, branch 2: bus2-bus3,
        // branch 3: bus2-bus4, branch 4: bus3-bus5, branch 5: bus4-bus5
        topo.branch_bus_idx = {{0, 2}, {1, 2}, {2, 3}, {2, 4}, {3, 5}, {4, 5}};

        topo.sources_per_bus = {from_sparse, {0, 1, 1, 1, 1, 1, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0, 0, 0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 0, 0, 0, 0, 0}}; // No load_gens for simplicity

        // Power sensors: bus 0, bus 4 have injection sensors (2 total sensors)
        // Format: bus0 has sensors [0:1), bus1 has [1:1), bus2 has [1:1), bus3 has [1:1), bus4 has [1:2), bus5 has
        // [2:2)
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1, 1, 1, 2, 2}};
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}}; // No load_gens
        topo.power_sensors_per_shunt = {from_sparse, {0}};

        // Branch sensors: branch 1 (bus1-bus2), branch 2 (bus2-bus3), branch 3 (bus2-bus4), // NOSONAR
        // branch 4 (bus3-bus5) have power sensors. 6 branches: branch0[0:0), branch1[0:1),  // NOSONAR
        // branch2[1:2), branch3[2:3), branch4[3:4), branch5[4:4) // NOSONAR
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 1, 2, 3, 4, 4}};
        topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0, 0, 0, 0, 0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 0, 0, 0, 0, 0, 0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0, 0, 0, 0, 0, 0, 0}};

        // Voltage sensor: bus 2 has voltage sensor // NOSONAR
        // bus0[0:0), bus1[0:0), bus2[0:1), bus3[1:1), bus4[1:1), bus5[1:1) // NOSONAR
        topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 0, 1, 1, 1, 1}};

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
        param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0},
                              {1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};

        // Initialize all measurement vectors to correct sizes first
        se_input.measured_voltage.resize(topo.voltage_sensors_per_bus.element_size());
        se_input.measured_bus_injection.resize(topo.power_sensors_per_bus.element_size());
        se_input.measured_branch_from_power.resize(topo.power_sensors_per_branch_from.element_size());
        se_input.measured_branch_to_power.resize(topo.power_sensors_per_branch_to.element_size());
        se_input.measured_branch_from_current.resize(topo.current_sensors_per_branch_from.element_size());
        se_input.measured_branch_to_current.resize(topo.current_sensors_per_branch_to.element_size());
        se_input.measured_shunt_power.resize(topo.power_sensors_per_shunt.element_size());
        se_input.measured_load_gen_power.resize(topo.power_sensors_per_load_gen.element_size());
        se_input.measured_source_power.resize(topo.power_sensors_per_source.element_size());

        // Voltage measurement: bus 2 has voltage sensor (magnitude only - no phasor)
        if (se_input.measured_voltage.empty()) {
            se_input.measured_voltage[0] = {.value = {1.0, nan}, .variance = 1.0}; // Bus 2: magnitude only
        }

        // Power injection measurements: bus 0, bus 4 (2 measurements to match 2 sensors)
        if (se_input.measured_bus_injection.size() >= 2) {
            se_input.measured_bus_injection[0] = {.real_component = {.value = 1.0, .variance = 1.0},
                                                  .imag_component = {.value = 1.0, .variance = 1.0}};
            se_input.measured_bus_injection[1] = {.real_component = {.value = 1.0, .variance = 1.0},
                                                  .imag_component = {.value = 1.0, .variance = 1.0}};
        }

        // Branch power measurements: branch 1 (bus1-bus2), branch 2 (bus2-bus3), branch 4 (bus3-bus5) (3 measurements
        // to match 3 sensors)
        if (se_input.measured_branch_from_power.size() >= 3) {
            se_input.measured_branch_from_power[0] = {.real_component = {.value = 1.0, .variance = 1.0},
                                                      .imag_component = {.value = 1.0, .variance = 1.0}};
            se_input.measured_branch_from_power[1] = {.real_component = {.value = 1.0, .variance = 1.0},
                                                      .imag_component = {.value = 1.0, .variance = 1.0}};
            se_input.measured_branch_from_power[2] = {.real_component = {.value = 1.0, .variance = 1.0},
                                                      .imag_component = {.value = 1.0, .variance = 1.0}};
        }

        // No source power measurements needed

        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};

        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        std::vector<BusNeighbourhoodInfo> neighbour_results(6);
        auto result = scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // Check that we have the expected sensor arrays
        CHECK(result.flow_sensors.size() == y_bus.y_bus_structure().row_indptr.back());
        CHECK(result.voltage_phasor_sensors.size() == 6); // n_bus
        CHECK(result.bus_injections.size() == 7);         // n_bus + 1

        // Check voltage sensors: bus 2 has voltage sensor (magnitude only, not phasor)
        CHECK(result.voltage_phasor_sensors[2] == 0); // Bus 2 has magnitude only (no phasor)

        // Check bus injection sensors: bus 0, 4 have injection sensors
        CHECK(result.bus_injections[0] == 1); // Bus 0 has injection sensor
        CHECK(result.bus_injections[1] == 1); // Bus 1 has zero-injection
        CHECK(result.bus_injections[4] == 1); // Bus 4 has injection sensor
        CHECK(result.total_injections == 6);  // Total count should be at least 2

        // Verify each bus has correct index
        for (size_t i = 0; i < neighbour_results.size(); ++i) {
            CHECK(neighbour_results[i].bus == static_cast<Idx>(i));
        }

        // Check connectivity status as per your specification
        // {0: [2], 1: [2], 2: [3,4], 3: [5], 4: [5], 5:[]}
        // Note: Buses without loads/generators get pseudo measurements (zero injection)
        CHECK(neighbour_results[0].status == node_measured);       // bus 0 has injection sensor
        CHECK(neighbour_results[1].status == node_measured);       // bus 1 has pseudo measurement (zero injection)
        CHECK(neighbour_results[2].status == node_measured);       // bus 2 has voltage sensor
        CHECK(neighbour_results[2].direct_neighbours.size() == 2); // bus 2 has 2 neighbours
        CHECK(neighbour_results[2].direct_neighbours[1].status ==
              branch_native_measurement_unused);             // bus 2 and bus 4 is connected by a measured edge
        CHECK(neighbour_results[3].status == node_measured); // bus 3 has pseudo measurement (zero injection)
        CHECK(neighbour_results[4].status == node_measured); // bus 4 has injection sensor
        CHECK(neighbour_results[5].status == node_measured); // bus 5 has pseudo measurement (zero injection)
    }

    SUBCASE("Empty network sensors") {
        // Create minimal topology with no sensors
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.phase_shift = {0.0};
        topo.branch_bus_idx = {};
        topo.sources_per_bus = {from_sparse, {0, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0}};
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0}};
        topo.power_sensors_per_branch_to = {from_sparse, {0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 0}};

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};
        // No measurements

        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        std::vector<BusNeighbourhoodInfo> neighbour_results(1);
        auto result = scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // All sensor vectors should be initialized but empty/zero
        CHECK(result.flow_sensors.size() == y_bus.y_bus_structure().row_indptr.back());
        CHECK(result.voltage_phasor_sensors.size() == 1);
        CHECK(result.bus_injections.size() == 2);

        // All sensors should be zero
        CHECK(std::ranges::all_of(result.flow_sensors, [](int8_t val) { return val == 0; }));
        CHECK(std::ranges::all_of(result.voltage_phasor_sensors, [](int8_t val) { return val == 0; }));
        CHECK(result.bus_injections.back() == 0); // No bus injections

        // Should be marked as possibly ill-conditioned due to no sensors
        CHECK(result.is_possibly_ill_conditioned == false);
    }

    SUBCASE("Mixed sensor types") {
        // Create topology with various sensor types
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.phase_shift = {0.0, 0.0};
        topo.branch_bus_idx = {{0, 1}};
        topo.sources_per_bus = {from_sparse, {0, 1, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 0}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 0}};
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0}}; // No power sensors
        topo.power_sensors_per_branch_to = {from_sparse, {0, 0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 1}}; // Current sensor on branch 0
        topo.current_sensors_per_branch_to = {from_sparse, {0, 0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2}}; // Voltage sensors on both buses

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
        param.branch_param = {{1.0, -1.0, -1.0, 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};
        se_input.measured_voltage = {
            {.value = 1.0 + 0.0i, .variance = 1.0},  // Bus 0 - voltage phasor
            {.value = 0.95 + 0.05i, .variance = 1.0} // Bus 1 - voltage phasor
        };
        se_input.measured_branch_from_current = {{.angle_measurement_type = AngleMeasurementType::local_angle,
                                                  .measurement = {.real_component = {.value = 1.0, .variance = 1.0},
                                                                  .imag_component = {.value = 0.1, .variance = 1.0}}}};

        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        std::vector<BusNeighbourhoodInfo> neighbour_results(2);
        auto result = scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // Both buses should have voltage phasor sensors
        CHECK(result.voltage_phasor_sensors[0] == 1);
        CHECK(result.voltage_phasor_sensors[1] == 1);

        // Should detect branch current sensor as flow sensor
        // Find the branch entry in the Y-bus structure and verify it's detected
        bool found_branch_sensor = false;
        std::ranges::for_each(result.flow_sensors, [&](int8_t val) {
            if (val == 1) {
                found_branch_sensor = true;
            }
        });
        CHECK(found_branch_sensor); // Current sensor should be detected as flow sensor

        // Should not be ill-conditioned with sufficient sensors
        CHECK(result.is_possibly_ill_conditioned == false);
    }
}

TEST_CASE("Test Observability - prepare_starting_nodes") {
    using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;
    using enum power_grid_model::math_solver::detail::ConnectivityStatus;
    using power_grid_model::math_solver::detail::prepare_starting_nodes;

    SUBCASE("Nodes without measurements - preferred starting points") {
        // Create a simple 4-bus network with mixed measurement status
        std::vector<BusNeighbourhoodInfo> neighbour_list(4);

        // Bus 0: has measurement
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 2, .status = branch_native_measurement_unused}};

        // Bus 1: no measurement, no edge measurements on connected branches
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 2: has measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;
        neighbour_list[2].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused}};

        // Bus 3: no measurement, no edge measurements on connected branches
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = has_no_measurement;
        neighbour_list[3].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        std::vector<Idx> starting_candidates;
        prepare_starting_nodes(neighbour_list, 4, starting_candidates);

        // Should find buses 1 and 3 as starting candidates
        // (nodes without measurements and all edges have no edge measurements)
        CHECK(starting_candidates.size() == 2);
        CHECK(std::ranges::find(starting_candidates, 1) != starting_candidates.end());
        CHECK(std::ranges::find(starting_candidates, 3) != starting_candidates.end());
    }

    SUBCASE("Nodes without measurements but with edge measurements") {
        // Network where unmeasured nodes have edge measurements
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // Bus 0: has measurement
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused}};

        // Bus 1: no measurement, but connected edge has measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused},
                                               {.bus = 2, .status = branch_native_measurement_unused}};

        // Bus 2: no measurement, but connected edge has measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = has_no_measurement;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused}};

        std::vector<Idx> starting_candidates;
        prepare_starting_nodes(neighbour_list, 3, starting_candidates);

        // Should fallback to nodes without measurements (buses 1 and 2)
        // since no "ideal" starting points exist
        CHECK(starting_candidates.size() == 2);
        CHECK(std::ranges::find(starting_candidates, 1) != starting_candidates.end());
        CHECK(std::ranges::find(starting_candidates, 2) != starting_candidates.end());
    }

    SUBCASE("All nodes have measurements - fallback to first node") {
        // Network where all nodes have measurements
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // All buses have measurements
        for (Idx i = 0; i < 3; ++i) {
            neighbour_list[i].bus = i;
            neighbour_list[i].status = node_measured;
            if (i < 2) {
                neighbour_list[i].direct_neighbours = {{.bus = i + 1, .status = has_no_measurement}};
            }
        }

        std::vector<Idx> starting_candidates;
        prepare_starting_nodes(neighbour_list, 3, starting_candidates);

        // Should fallback to first node (bus 0)
        CHECK(starting_candidates.size() == 1);
        CHECK(starting_candidates[0] == 0);
    }

    SUBCASE("Single bus network") {
        // Edge case: single bus
        std::vector<BusNeighbourhoodInfo> neighbour_list(1);

        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {}; // No neighbours

        std::vector<Idx> starting_candidates;
        prepare_starting_nodes(neighbour_list, 1, starting_candidates);

        // Should find the single unmeasured bus
        CHECK(starting_candidates.size() == 1);
        CHECK(starting_candidates[0] == 0);
    }

    SUBCASE("Empty network") {
        // Edge case: empty network
        std::vector<BusNeighbourhoodInfo> const neighbour_list;
        std::vector<Idx> starting_candidates;

        prepare_starting_nodes(neighbour_list, 0, starting_candidates);

        // Should fallback to first node (0) even with empty network
        CHECK(starting_candidates.size() == 1);
        CHECK(starting_candidates[0] == 0);
    }

    SUBCASE("Mixed connectivity statuses") {
        // Test with various connectivity statuses
        std::vector<BusNeighbourhoodInfo> neighbour_list(5);

        // Bus 0: node measured
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        // Bus 1: has no measurement, ideal starting point
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement}};

        // Bus 2: downstream measured
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = branch_discovered_with_from_node_sensor;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        // Bus 3: upstream measured
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = branch_discovered_with_to_node_sensor;
        neighbour_list[3].direct_neighbours = {{.bus = 4, .status = has_no_measurement}};

        // Bus 4: branch measured used
        neighbour_list[4].bus = 4;
        neighbour_list[4].status = branch_native_measurement_consumed;
        neighbour_list[4].direct_neighbours = {{.bus = 3, .status = has_no_measurement}};

        std::vector<Idx> starting_candidates;
        prepare_starting_nodes(neighbour_list, 5, starting_candidates);

        // Should find bus 1 as the ideal starting point
        // (has_no_measurement and all connected edges have no edge measurements)
        CHECK(starting_candidates.size() == 1);
        CHECK(starting_candidates[0] == 1);
    }
}

TEST_CASE("Test Observability - complete_bidirectional_neighbourhood_info") {
    using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;
    using power_grid_model::math_solver::detail::complete_bidirectional_neighbourhood_info;
    using enum power_grid_model::math_solver::detail::ConnectivityStatus;

    SUBCASE("Basic expansion test") {
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // Initialize test data
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 2, .status = node_measured}};

        neighbour_list[1].bus = 1;
        neighbour_list[1].status = node_measured;

        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;

        // Test the function
        complete_bidirectional_neighbourhood_info(neighbour_list);

        // Basic verification - structure should be maintained
        CHECK(neighbour_list.size() == 3);
        CHECK(neighbour_list[0].bus == 0);
        CHECK(neighbour_list[1].bus == 1);
        CHECK(neighbour_list[2].bus == 2);

        // Verify bus statuses remain unchanged
        CHECK(neighbour_list[0].status == has_no_measurement);
        CHECK(neighbour_list[1].status == node_measured);
        CHECK(neighbour_list[2].status == node_measured);

        // Verify Bus 0 connections (should remain as originally set)
        CHECK(neighbour_list[0].direct_neighbours.size() == 2);
        CHECK(neighbour_list[0].direct_neighbours[0].bus == 1);
        CHECK(neighbour_list[0].direct_neighbours[0].status == has_no_measurement);
        CHECK(neighbour_list[0].direct_neighbours[1].bus == 2);
        CHECK(neighbour_list[0].direct_neighbours[1].status == node_measured);

        // Verify Bus 1 connections (should have reverse connection added)
        CHECK(neighbour_list[1].direct_neighbours.size() == 1);
        CHECK(neighbour_list[1].direct_neighbours[0].bus == 0);
        CHECK(neighbour_list[1].direct_neighbours[0].status == has_no_measurement);

        // Verify Bus 2 connections (should have reverse connection added)
        CHECK(neighbour_list[2].direct_neighbours.size() == 1);
        CHECK(neighbour_list[2].direct_neighbours[0].bus == 0);
        CHECK(neighbour_list[2].direct_neighbours[0].status == node_measured);
    }

    SUBCASE("Complex network with multiple connection types") {
        std::vector<BusNeighbourhoodInfo> neighbour_list(4);

        // Initialize test data - create a partially connected network
        // Bus 0 connects to buses 1 and 3
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 1 connects to bus 2 (but not back to 0 yet)
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 2, .status = branch_discovered_with_from_node_sensor}};

        // Bus 2 has existing connection to bus 3
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = branch_discovered_with_to_node_sensor;
        neighbour_list[2].direct_neighbours = {{.bus = 3, .status = branch_native_measurement_consumed}};

        // Bus 3 initially has no connections
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = node_measured;
        neighbour_list[3].direct_neighbours = {};

        // Test the function
        complete_bidirectional_neighbourhood_info(neighbour_list);

        // Verify all buses maintain their original status
        CHECK(neighbour_list[0].status == node_measured);
        CHECK(neighbour_list[1].status == has_no_measurement);
        CHECK(neighbour_list[2].status == branch_discovered_with_to_node_sensor);
        CHECK(neighbour_list[3].status == node_measured);

        // Verify Bus 0 connections (original + reverse from 1 and 3)
        CHECK(neighbour_list[0].direct_neighbours.size() == 2);
        // Find connection to bus 1
        auto bus0_to_bus1 =
            std::ranges::find_if(neighbour_list[0].direct_neighbours, [](const auto& n) { return n.bus == 1; });
        REQUIRE(bus0_to_bus1 != neighbour_list[0].direct_neighbours.end());
        CHECK(bus0_to_bus1->status == branch_native_measurement_unused);
        // Find connection to bus 3
        auto bus0_to_bus3 =
            std::ranges::find_if(neighbour_list[0].direct_neighbours, [](const auto& n) { return n.bus == 3; });
        REQUIRE(bus0_to_bus3 != neighbour_list[0].direct_neighbours.end());
        CHECK(bus0_to_bus3->status == has_no_measurement);

        // Verify Bus 1 connections (original + reverse from 0)
        CHECK(neighbour_list[1].direct_neighbours.size() == 2);
        // Find connection to bus 0 (reverse added)
        auto bus1_to_bus0 =
            std::ranges::find_if(neighbour_list[1].direct_neighbours, [](const auto& n) { return n.bus == 0; });
        REQUIRE(bus1_to_bus0 != neighbour_list[1].direct_neighbours.end());
        CHECK(bus1_to_bus0->status == branch_native_measurement_unused);
        // Find connection to bus 2 (original)
        auto bus1_to_bus2 =
            std::ranges::find_if(neighbour_list[1].direct_neighbours, [](const auto& n) { return n.bus == 2; });
        REQUIRE(bus1_to_bus2 != neighbour_list[1].direct_neighbours.end());
        CHECK(bus1_to_bus2->status == branch_discovered_with_from_node_sensor);

        // Verify Bus 2 connections (original + reverse from 1)
        CHECK(neighbour_list[2].direct_neighbours.size() == 2);
        // Find connection to bus 1 (reverse added)
        auto bus2_to_bus1 =
            std::ranges::find_if(neighbour_list[2].direct_neighbours, [](const auto& n) { return n.bus == 1; });
        REQUIRE(bus2_to_bus1 != neighbour_list[2].direct_neighbours.end());
        CHECK(bus2_to_bus1->status == branch_discovered_with_from_node_sensor);
        // Find connection to bus 3 (original)
        auto bus2_to_bus3 =
            std::ranges::find_if(neighbour_list[2].direct_neighbours, [](const auto& n) { return n.bus == 3; });
        REQUIRE(bus2_to_bus3 != neighbour_list[2].direct_neighbours.end());
        CHECK(bus2_to_bus3->status == branch_native_measurement_consumed);

        // Verify Bus 3 connections (reverse from 0 and 2)
        CHECK(neighbour_list[3].direct_neighbours.size() == 2);
        // Find connection to bus 0 (reverse added)
        auto bus3_to_bus0 =
            std::ranges::find_if(neighbour_list[3].direct_neighbours, [](const auto& n) { return n.bus == 0; });
        REQUIRE(bus3_to_bus0 != neighbour_list[3].direct_neighbours.end());
        CHECK(bus3_to_bus0->status == has_no_measurement);
        // Find connection to bus 2 (reverse added)
        auto bus3_to_bus2 =
            std::ranges::find_if(neighbour_list[3].direct_neighbours, [](const auto& n) { return n.bus == 2; });
        REQUIRE(bus3_to_bus2 != neighbour_list[3].direct_neighbours.end());
        CHECK(bus3_to_bus2->status == branch_native_measurement_consumed);
    }

    SUBCASE("Network with existing bidirectional connections") {
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // Initialize with some connections already bidirectional
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 2, .status = node_measured}};

        neighbour_list[1].bus = 1;
        neighbour_list[1].status = node_measured;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = branch_native_measurement_unused}};

        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused}};

        // Test the function
        complete_bidirectional_neighbourhood_info(neighbour_list);

        // Verify Bus 0 connections remain the same (already complete)
        CHECK(neighbour_list[0].direct_neighbours.size() == 2);

        // Verify Bus 1 connections remain the same (already complete)
        CHECK(neighbour_list[1].direct_neighbours.size() == 2);

        // Verify Bus 2 gets the missing reverse connection to bus 0
        CHECK(neighbour_list[2].direct_neighbours.size() == 2);
        auto bus2_to_bus0 =
            std::ranges::find_if(neighbour_list[2].direct_neighbours, [](const auto& n) { return n.bus == 0; });
        REQUIRE(bus2_to_bus0 != neighbour_list[2].direct_neighbours.end());
        CHECK(bus2_to_bus0->status == node_measured);
        auto bus2_to_bus1 =
            std::ranges::find_if(neighbour_list[2].direct_neighbours, [](const auto& n) { return n.bus == 1; });
        REQUIRE(bus2_to_bus1 != neighbour_list[2].direct_neighbours.end());
        CHECK(bus2_to_bus1->status == branch_native_measurement_unused);
    }

    SUBCASE("Empty neighbour list") {
        std::vector<BusNeighbourhoodInfo> empty_list;
        CHECK_NOTHROW(complete_bidirectional_neighbourhood_info(empty_list));
        CHECK(empty_list.empty());
    }
}

// TODO: properly clean up after y-bus access refactoring
TEST_CASE("Test Observability - assign_independent_sensors_radial") {
    using power_grid_model::math_solver::YBusStructure;
    using power_grid_model::math_solver::detail::assign_independent_sensors_radial;

    SUBCASE("Integration test with minimal setup") {
        // Create a simple 2-bus radial network: bus0--bus1
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.phase_shift = {0.0, 0.0};
        topo.branch_bus_idx = {{0, 1}};
        topo.sources_per_bus = {from_sparse, {0, 1, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 0}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 0}};
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0}};
        topo.power_sensors_per_branch_to = {from_sparse, {0, 0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0, 0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 0}};

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
        param.branch_param = {{1.0, -1.0, -1.0, 1.0}};

        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};

        // Test the function with real YBusStructure
        // First, inspect the actual YBus structure to size our vectors correctly
        auto const& y_bus_struct = y_bus.y_bus_structure();
        auto const n_ybus_entries = static_cast<Idx>(y_bus_struct.col_indices.size());
        auto const n_bus = static_cast<Idx>(y_bus_struct.bus_entry.size());

        std::vector<int8_t> flow_sensors(n_ybus_entries, 0);  // Initialize to correct size
        std::vector<int8_t> voltage_phasor_sensors(n_bus, 0); // Initialize to correct size

        // Set up initial sensors if vectors are large enough
        if (n_ybus_entries > 0) {
            flow_sensors[0] = 1; // bus0 injection
        }
        if (n_bus > 1) {
            voltage_phasor_sensors[1] = 1; // voltage phasor at bus1
        }

        assign_independent_sensors_radial(y_bus_struct, flow_sensors, voltage_phasor_sensors);

        // Verify basic behavior - bus injections should be removed
        // The exact reassignment depends on the YBus structure, so we test general properties
        if (n_bus > 1) {
            CHECK(flow_sensors[y_bus_struct.bus_entry[n_bus - 1]] == 0); // last bus injection should be 0
        }

        // Total sensors should be preserved (just reassigned)
        Idx const initial_total = 2; // We started with 1 flow + 1 voltage = 2 total
        Idx const final_flow = std::ranges::fold_left(flow_sensors, 0, std::plus<>{});
        Idx const final_voltage = std::ranges::fold_left(voltage_phasor_sensors, 0, std::plus<>{});
        CHECK(final_flow + final_voltage <= initial_total); // Some sensors might be reassigned or removed
    }

    SUBCASE("Function should not crash with empty sensors") {
        // Test with minimal topology to ensure the function handles edge cases
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.phase_shift = {0.0};
        topo.branch_bus_idx = {}; // No branches
        topo.sources_per_bus = {from_sparse, {0, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0}};
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0}};
        topo.power_sensors_per_branch_to = {from_sparse, {0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 0}};

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};

        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};

        // Size vectors correctly based on actual YBus structure
        auto const& y_bus_struct = y_bus.y_bus_structure();
        auto const n_ybus_entries = static_cast<Idx>(y_bus_struct.col_indices.size());
        auto const n_bus = static_cast<Idx>(y_bus_struct.bus_entry.size());

        std::vector<int8_t> flow_sensors(n_ybus_entries, 0);
        std::vector<int8_t> voltage_phasor_sensors(n_bus, 0);

        // Should handle single bus case gracefully
        CHECK_NOTHROW(assign_independent_sensors_radial(y_bus_struct, flow_sensors, voltage_phasor_sensors));

        // Last bus injection should be removed if there are buses
        if (n_bus > 0) {
            CHECK(flow_sensors[y_bus_struct.bus_entry[n_bus - 1]] == 0);
        }
    }
}

TEST_CASE("Test Observability - find_spanning_tree_from_node") {
    using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;
    using enum power_grid_model::math_solver::detail::ConnectivityStatus;
    using power_grid_model::math_solver::detail::find_spanning_tree_from_node;

    SUBCASE("Simple spanning tree with native edge measurements") {
        // Create a 3-bus network with native edge measurements
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // Bus 0: no measurement, starting point
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused},
                                               {.bus = 2, .status = has_no_measurement}};

        // Bus 1: no measurement, connected via native edge measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused},
                                               {.bus = 2, .status = branch_native_measurement_unused}};

        // Bus 2: no measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = has_no_measurement;
        neighbour_list[2].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 1, .status = branch_native_measurement_unused}};

        Idx const start_bus = 0;
        Idx const n_bus = 3;

        bool const result = find_spanning_tree_from_node(start_bus, n_bus, neighbour_list);

        // Should successfully find spanning tree using native edge measurements
        CHECK(result == true);
    }

    SUBCASE("Simple linear chain with sufficient measurements") {
        // Create a simple 3-bus linear chain with measurements at key points
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // Bus 0: has node measurement, starting point
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        // Bus 1: no measurement, but connected to measured nodes
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement}};

        // Bus 2: has measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        Idx const start_bus = 1;
        Idx const n_bus = 3;

        bool const result = find_spanning_tree_from_node(start_bus, n_bus, neighbour_list);

        CHECK(result == true);
    }

    SUBCASE("Mixed measurement types") {
        // Create a network with various measurement types
        std::vector<BusNeighbourhoodInfo> neighbour_list(4);

        // Bus 0: no measurement, starting point
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused}};

        // Bus 1: has measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = node_measured;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused},
                                               {.bus = 2, .status = has_no_measurement},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 2: no measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = has_no_measurement;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 3: has measurement
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = node_measured;
        neighbour_list[3].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement}};

        Idx const start_bus = 2;
        Idx const n_bus = 4;

        bool const result = find_spanning_tree_from_node(start_bus, n_bus, neighbour_list);

        CHECK(result == true);
    }

    SUBCASE("Insufficient connectivity - should fail") {
        // Create a network where not all nodes can be reached
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // Bus 0: no measurement, starting point
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        // Bus 1: no measurement, no useful connections
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement}};

        // Bus 2: isolated, no measurements, no connections to 0 or 1
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = has_no_measurement;
        neighbour_list[2].direct_neighbours = {}; // Isolated

        Idx const start_bus = 0;
        Idx const n_bus = 3;

        bool const result = find_spanning_tree_from_node(start_bus, n_bus, neighbour_list);

        // Should fail because bus 2 is isolated and cannot be reached
        CHECK(result == false);
    }

    SUBCASE("Test basic function behavior - no expectation of success") {
        // Edge case: single bus - just test that function doesn't crash
        std::vector<BusNeighbourhoodInfo> neighbour_list(1);

        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {}; // No neighbours

        Idx const start_bus = 0;
        Idx const n_bus = 1;

        // Just test that the function executes without crashing
        CHECK_NOTHROW(find_spanning_tree_from_node(start_bus, n_bus, neighbour_list));
    }

    SUBCASE("Restart from another candidate") {
        // Seven node ring that requires a restart from the second candidate
        std::vector<BusNeighbourhoodInfo> neighbour_list(7);

        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 6, .status = has_no_measurement}};
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = node_measured;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement}};
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 3, .status = has_no_measurement},
                                               {.bus = 4, .status = has_no_measurement}};
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = has_no_measurement;
        neighbour_list[3].direct_neighbours = {{.bus = 2, .status = has_no_measurement}};

        neighbour_list[4].bus = 4;
        neighbour_list[4].status = node_measured;
        neighbour_list[4].direct_neighbours = {{.bus = 2, .status = has_no_measurement},
                                               {.bus = 5, .status = has_no_measurement}};
        neighbour_list[5].bus = 5;
        neighbour_list[5].status = node_measured;
        neighbour_list[5].direct_neighbours = {{.bus = 4, .status = has_no_measurement},
                                               {.bus = 6, .status = branch_native_measurement_unused}};
        neighbour_list[6].bus = 6;
        neighbour_list[6].status = node_measured;
        neighbour_list[6].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 5, .status = branch_native_measurement_unused}};

        // fail attempt
        bool const first_attempt = find_spanning_tree_from_node(0, 7, neighbour_list);
        CHECK(first_attempt == false);

        // success attempt
        bool const second_attempt = find_spanning_tree_from_node(3, 7, neighbour_list);
        CHECK(second_attempt == true);
    }

    SUBCASE("Reassignment needed") {
        // Seven node radial network where reassignment happens
        std::vector<BusNeighbourhoodInfo> neighbour_list(7);

        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = node_measured;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = branch_native_measurement_unused}};
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused},
                                               {.bus = 3, .status = has_no_measurement},
                                               {.bus = 5, .status = has_no_measurement}};
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = node_measured;
        neighbour_list[3].direct_neighbours = {{.bus = 2, .status = has_no_measurement},
                                               {.bus = 4, .status = has_no_measurement}};
        neighbour_list[4].bus = 4;
        neighbour_list[4].status = node_measured;
        neighbour_list[4].direct_neighbours = {{.bus = 3, .status = has_no_measurement}};

        neighbour_list[5].bus = 5;
        neighbour_list[5].status = node_measured;
        neighbour_list[5].direct_neighbours = {{.bus = 2, .status = has_no_measurement},
                                               {.bus = 6, .status = has_no_measurement}};
        neighbour_list[6].bus = 6;
        neighbour_list[6].status = has_no_measurement;
        neighbour_list[6].direct_neighbours = {{.bus = 5, .status = has_no_measurement}};

        bool const first_attempt = find_spanning_tree_from_node(0, 7, neighbour_list);

        // Without reassignment, this would fail and only success starting from bus 6
        CHECK(first_attempt == true);

        bool const second_attempt = find_spanning_tree_from_node(6, 7, neighbour_list);
        CHECK(second_attempt == true);
    }

    SUBCASE("All nodes have measurements - should succeed easily") {
        // Network where every node has measurements
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // All buses have measurements
        for (Idx i = 0; i < 3; ++i) {
            neighbour_list[i].bus = i;
            neighbour_list[i].status = node_measured;
            if (i < 2) {
                neighbour_list[i].direct_neighbours = {{.bus = i + 1, .status = has_no_measurement}};
            }
        }

        Idx const start_bus = 0;
        Idx const n_bus = 3;

        bool const result = find_spanning_tree_from_node(start_bus, n_bus, neighbour_list);

        CHECK(result == true);
    }

    SUBCASE("Algorithm execution without crash - general behavior test") {
        // Create a network and test that algorithm executes without issues
        std::vector<BusNeighbourhoodInfo> neighbour_list(4);

        // Bus 0: starting point, no measurement
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 2, .status = branch_native_measurement_unused}};

        // Bus 1: has measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = node_measured;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 2: no measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = has_no_measurement;
        neighbour_list[2].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused}};

        // Bus 3: no measurement
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = has_no_measurement;
        neighbour_list[3].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        Idx const start_bus = 0;
        Idx const n_bus = 4;

        // Test that function executes and returns a boolean result
        bool const result = find_spanning_tree_from_node(start_bus, n_bus, neighbour_list);

        CHECK(result == false);
    }
}

TEST_CASE("Test Observability - necessary_condition") {
    using power_grid_model::math_solver::detail::necessary_condition;
    using power_grid_model::math_solver::detail::ObservabilitySensorsResult;

    SUBCASE("Sufficient measurements") {
        ObservabilitySensorsResult sensors;
        sensors.flow_sensors = {1, 1, 0, 1};
        sensors.voltage_phasor_sensors = {1, 0, 1};
        sensors.bus_injections = {1, 1, 2}; // cumulative count ending at 2
        sensors.is_possibly_ill_conditioned = false;

        Idx const n_bus = 3;
        Idx n_voltage_phasor{};

        CHECK_NOTHROW(necessary_condition(sensors, n_bus, n_voltage_phasor, false));
        CHECK(n_voltage_phasor == 2); // Should count voltage phasor sensors
    }

    SUBCASE("Insufficient measurements") {
        ObservabilitySensorsResult sensors;
        sensors.flow_sensors = {0, 0, 0};
        sensors.voltage_phasor_sensors = {1, 0, 0}; // only one voltage measurement
        sensors.bus_injections = {1, 1, 1};         // only one injection
        sensors.is_possibly_ill_conditioned = false;

        Idx const n_bus = 3;
        Idx n_voltage_phasor = 1;

        CHECK_THROWS_AS(necessary_condition(sensors, n_bus, n_voltage_phasor, false), NotObservableError);
    }

    SUBCASE("Empty sensors") {
        // Edge case: no buses means trivially observable
        // All vectors empty - should not be observable
        ObservabilitySensorsResult const sensors;

        Idx n_voltage_phasor = 0;
        CHECK_NOTHROW(necessary_condition(sensors, 0, n_voltage_phasor, false));
    }
}

TEST_CASE("Test Observability - sufficient_condition_radial_with_voltage_phasor") {
    using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;
    using power_grid_model::math_solver::detail::scan_network_sensors;
    using power_grid_model::math_solver::detail::sufficient_condition_radial_with_voltage_phasor;

    SUBCASE("Observable radial network with voltage phasor sensors") {
        // Create a simple 4-bus radial network: bus0--bus1--bus2--bus3
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.is_radial = true;
        topo.phase_shift = {0.0, 0.0, 0.0, 0.0};
        topo.branch_bus_idx = {{0, 1}, {1, 2}, {2, 3}};
        topo.sources_per_bus = {from_sparse, {0, 1, 1, 1, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 0, 0, 0}};
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1, 2, 2}}; // Injection sensors at bus 0 and 2
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 1, 1}}; // Branch sensor on branch 1
        topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0, 0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 0, 0, 0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0, 0, 0, 0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 2, 2}}; // Voltage phasor sensors at bus 0 and 1

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
        param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};
        se_input.measured_voltage = {
            {.value = 1.0 + 0.1i, .variance = 1.0},  // Bus 0 - voltage phasor sensor
            {.value = 0.95 + 0.05i, .variance = 1.0} // Bus 1 - voltage phasor sensor
        };
        se_input.measured_bus_injection = {
            {.real_component = {.value = 1.5, .variance = 1.0}, .imag_component = {.value = 0.5, .variance = 1.0}},
            {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.2, .variance = 1.0}}};
        se_input.measured_branch_from_power = {
            {.real_component = {.value = 0.8, .variance = 1.0}, .imag_component = {.value = 0.1, .variance = 1.0}}};

        // Create YBus and scan sensors
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        std::vector<BusNeighbourhoodInfo> neighbour_results(4);
        auto observability_sensors =
            scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // Count voltage phasor sensors
        Idx const n_voltage_phasor_sensors =
            std::ranges::fold_left(observability_sensors.voltage_phasor_sensors, 0, std::plus<>{});

        // Verify that it returns true (no exception thrown means observable)
        bool const result = sufficient_condition_radial_with_voltage_phasor(
            y_bus.y_bus_structure(), observability_sensors, n_voltage_phasor_sensors);
        CHECK(result == true);

        // Verify that sensors were reassigned properly
        Idx const n_bus = 4;
        Idx const final_flow_sensors = std::ranges::fold_left(observability_sensors.flow_sensors, 0, std::plus<>{});
        Idx const final_voltage_sensors =
            std::ranges::fold_left(observability_sensors.voltage_phasor_sensors, 0, std::plus<>{});

        // Should have n_bus-1 independent flow sensors for radial network
        CHECK(final_flow_sensors >= n_bus - 1);
        // Should retain at least 1 voltage phasor sensor as reference
        CHECK(final_voltage_sensors >= 1);
    }

    SUBCASE("Test sensor reassignment behavior") {
        // Create a 3-bus radial network to test sensor reassignment
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.is_radial = true;
        topo.phase_shift = {0.0, 0.0, 0.0};
        topo.branch_bus_idx = {{0, 1}, {1, 2}};
        topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 1, 1}}; // load at bus 2
        topo.load_gen_type = {LoadGenType::const_pq};
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 2, 2}}; // Injection sensors at bus 0 and 1
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 0}}; // No branch sensors
        topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 0, 0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 1}}; // Voltage sensor at bus 0

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
        param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};
        se_input.measured_voltage = {
            {.value = 1.0 + 0.1i, .variance = 1.0} // Voltage phasor sensor at bus 0
        };
        se_input.measured_bus_injection = {
            {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}},
            {.real_component = {.value = 0.8, .variance = 1.0}, .imag_component = {.value = 0.1, .variance = 1.0}}};
        se_input.load_gen_status = {1};

        // Create YBus and scan sensors
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        std::vector<BusNeighbourhoodInfo> neighbour_results(3);
        auto observability_sensors =
            scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // Store initial sensor counts
        Idx const initial_voltage_sensors =
            std::ranges::fold_left(observability_sensors.voltage_phasor_sensors, 0, std::plus<>{});

        // Count voltage phasor sensors for the function
        Idx const n_voltage_phasor_sensors = initial_voltage_sensors;

        // Test that the function works and modifies the sensor vectors
        bool const result = sufficient_condition_radial_with_voltage_phasor(
            y_bus.y_bus_structure(), observability_sensors, n_voltage_phasor_sensors);
        CHECK(result == true);

        // Verify that sensors were modified by the internal assign_independent_sensors_radial call
        Idx const final_flow_sensors = std::ranges::fold_left(observability_sensors.flow_sensors, 0, std::plus<>{});
        Idx const final_voltage_sensors =
            std::ranges::fold_left(observability_sensors.voltage_phasor_sensors, 0, std::plus<>{});

        // For a 3-bus radial network, should have 2 independent flow sensors
        CHECK(final_flow_sensors == 2);

        // Should retain at least 1 voltage phasor sensor as reference if we started with any
        if (n_voltage_phasor_sensors > 0) {
            CHECK(final_voltage_sensors >= 1);
        }
    }

    SUBCASE("No voltage phasor sensors but sufficient flow sensors") {
        // Create a 3-bus radial network with sufficient flow sensors but no voltage phasor sensors
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.is_radial = true;
        topo.phase_shift = {0.0, 0.0, 0.0};
        topo.branch_bus_idx = {{0, 1}, {1, 2}};
        topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 0, 0}};
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 2, 2}}; // Injection sensors at bus 0 and 1
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 0}}; // No branch sensors
        topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 0, 0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0, 0, 0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 1}}; // Only magnitude sensors

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};
        param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};
        se_input.measured_voltage = {
            {.value = {1.0, nan}, .variance = 1.0} // Magnitude only (no phasor)
        };
        se_input.measured_bus_injection = {
            {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}},
            {.real_component = {.value = 0.8, .variance = 1.0}, .imag_component = {.value = 0.1, .variance = 1.0}}};

        // Create YBus and scan sensors
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        std::vector<BusNeighbourhoodInfo> neighbour_results(3);
        auto observability_sensors =
            scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // Count voltage phasor sensors (should be 0)
        Idx const n_voltage_phasor_sensors =
            std::ranges::fold_left(observability_sensors.voltage_phasor_sensors, 0, std::plus<>{});
        CHECK(n_voltage_phasor_sensors == 0);

        // Should pass with sufficient flow sensors even without voltage phasor sensors
        bool const result = sufficient_condition_radial_with_voltage_phasor(
            y_bus.y_bus_structure(), observability_sensors, n_voltage_phasor_sensors);
        CHECK(result == true);
    }

    SUBCASE("Single bus network - edge case") {
        // Create a single bus network
        MathModelTopology topo;
        topo.slack_bus = 0;
        topo.is_radial = true;
        topo.phase_shift = {0.0};
        topo.branch_bus_idx = {}; // No branches
        topo.sources_per_bus = {from_sparse, {0, 1}};
        topo.shunts_per_bus = {from_sparse, {0, 0}};
        topo.load_gens_per_bus = {from_sparse, {0, 0}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0}};
        topo.power_sensors_per_source = {from_sparse, {0, 0}};
        topo.power_sensors_per_load_gen = {from_sparse, {0}};
        topo.power_sensors_per_shunt = {from_sparse, {0}};
        topo.power_sensors_per_branch_from = {from_sparse, {0}};
        topo.power_sensors_per_branch_to = {from_sparse, {0}};
        topo.current_sensors_per_branch_from = {from_sparse, {0}};
        topo.current_sensors_per_branch_to = {from_sparse, {0}};
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1}};

        MathModelParam<symmetric_t> param;
        param.source_param = {SourceCalcParam{.y1 = 1.0, .y0 = 1.0}};

        StateEstimationInput<symmetric_t> se_input;
        se_input.source_status = {1};
        se_input.measured_voltage = {
            {.value = 1.0 + 0.0i, .variance = 1.0} // Single voltage phasor sensor
        };

        // Create YBus and scan sensors
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        std::vector<BusNeighbourhoodInfo> neighbour_results(1);
        auto observability_sensors =
            scan_network_sensors(measured_values, topo, y_bus.y_bus_structure(), neighbour_results);

        // Count voltage phasor sensors
        Idx const n_voltage_phasor_sensors =
            std::ranges::fold_left(observability_sensors.voltage_phasor_sensors, 0, std::plus<>{});

        // Single bus with voltage phasor should be observable (n_bus-1 = 0 flow sensors needed)
        bool const result = sufficient_condition_radial_with_voltage_phasor(
            y_bus.y_bus_structure(), observability_sensors, n_voltage_phasor_sensors);
        CHECK(result == true);
    }
}

TEST_CASE("Test Observability - sufficient_condition_meshed_without_voltage_phasor") {
    using power_grid_model::math_solver::detail::BusNeighbourhoodInfo;
    using power_grid_model::math_solver::detail::complete_bidirectional_neighbourhood_info;
    using enum power_grid_model::math_solver::detail::ConnectivityStatus;
    using power_grid_model::math_solver::detail::sufficient_condition_meshed_without_voltage_phasor;

    SUBCASE("Simple meshed network with sufficient measurements") {
        // Create a 4-bus meshed network with loop: bus0--bus1--bus2--bus3--bus0
        std::vector<BusNeighbourhoodInfo> neighbour_list(4);

        // Bus 0: has measurement
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 1: no measurement, connected to measured nodes
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = branch_native_measurement_unused}};

        // Bus 2: has measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 3: no measurement
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = has_no_measurement;
        neighbour_list[3].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement}};

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Should successfully find spanning tree in meshed network with sufficient measurements
        CHECK(result == true);
    }

    SUBCASE("Meshed network with native edge measurements") {
        // Create a triangle network: bus0--bus1--bus2--bus0 with native edge measurements
        std::vector<BusNeighbourhoodInfo> neighbour_list(3);

        // Bus 0: no measurement, but has native edge measurement
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = branch_native_measurement_unused},
                                               {.bus = 2, .status = has_no_measurement}};

        // Bus 1: no measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused},
                                               {.bus = 2, .status = branch_native_measurement_unused}};

        // Bus 2: no measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = has_no_measurement;
        neighbour_list[2].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 1, .status = branch_native_measurement_unused}};

        // Expand bidirectional connections
        complete_bidirectional_neighbourhood_info(neighbour_list);

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Should find spanning tree using native edge measurements
        CHECK(result == true);
    }

    SUBCASE("Complex meshed network with multiple loops") {
        // Create a 5-bus meshed network with multiple measurement types
        std::vector<BusNeighbourhoodInfo> neighbour_list(5);

        // Bus 0: has measurement, central node
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement},
                                               {.bus = 3, .status = branch_native_measurement_unused}};

        // Bus 1: no measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement},
                                               {.bus = 4, .status = has_no_measurement}};

        // Bus 2: has measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = node_measured;
        neighbour_list[2].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 1, .status = has_no_measurement},
                                               {.bus = 4, .status = has_no_measurement}};

        // Bus 3: no measurement
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = has_no_measurement;
        neighbour_list[3].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused},
                                               {.bus = 4, .status = branch_native_measurement_unused}};

        // Bus 4: no measurement
        neighbour_list[4].bus = 4;
        neighbour_list[4].status = has_no_measurement;
        neighbour_list[4].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement},
                                               {.bus = 3, .status = branch_native_measurement_unused}};

        // Expand bidirectional connections
        complete_bidirectional_neighbourhood_info(neighbour_list);

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Should handle complex meshed network with multiple loops
        CHECK(result == true);
    }

    SUBCASE("Insufficient measurements in meshed network") {
        // Create a meshed network where spanning tree cannot be formed
        std::vector<BusNeighbourhoodInfo> neighbour_list(4);

        // Bus 0: no measurement, isolated from sufficient measurements
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = has_no_measurement;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        // Bus 1: no measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement}};

        // Bus 2: no measurement
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = has_no_measurement;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 3, .status = has_no_measurement}};

        // Bus 3: has measurement but disconnected from the chain
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = node_measured;
        neighbour_list[3].direct_neighbours = {{.bus = 2, .status = has_no_measurement}};

        // Expand bidirectional connections
        complete_bidirectional_neighbourhood_info(neighbour_list);

        // Should fail due to insufficient measurements
        CHECK_THROWS_AS(sufficient_condition_meshed_without_voltage_phasor(neighbour_list), NotObservableError);
    }

    SUBCASE("Single bus network - edge case") {
        // Edge case: single bus
        std::vector<BusNeighbourhoodInfo> neighbour_list(1);

        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {}; // No neighbours

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Single bus with measurement should be trivially observable
        CHECK(result == true);
    }

    SUBCASE("Two bus network - simple case") {
        // Simple two bus network
        std::vector<BusNeighbourhoodInfo> neighbour_list(2);

        // Bus 0: has measurement
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement}};

        // Bus 1: no measurement
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = has_no_measurement;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement}};

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Two bus network with one measurement should be observable
        CHECK(result == true);
    }

    SUBCASE("Empty network - edge case") {
        // Edge case: empty network
        std::vector<BusNeighbourhoodInfo> const neighbour_list;

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Empty network should be trivially observable
        CHECK(result == true);
    }

    SUBCASE("Algorithm behavior test with various connectivity statuses") {
        // Test with various connectivity statuses to ensure robust behavior
        std::vector<BusNeighbourhoodInfo> neighbour_list(6);

        // Bus 0: node measured
        neighbour_list[0].bus = 0;
        neighbour_list[0].status = node_measured;
        neighbour_list[0].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 5, .status = branch_native_measurement_unused}};

        // Bus 1: downstream measured
        neighbour_list[1].bus = 1;
        neighbour_list[1].status = branch_discovered_with_from_node_sensor;
        neighbour_list[1].direct_neighbours = {{.bus = 0, .status = has_no_measurement},
                                               {.bus = 2, .status = has_no_measurement}};

        // Bus 2: upstream measured
        neighbour_list[2].bus = 2;
        neighbour_list[2].status = branch_discovered_with_to_node_sensor;
        neighbour_list[2].direct_neighbours = {{.bus = 1, .status = has_no_measurement},
                                               {.bus = 3, .status = branch_native_measurement_unused}};

        // Bus 3: branch measured used
        neighbour_list[3].bus = 3;
        neighbour_list[3].status = branch_native_measurement_consumed;
        neighbour_list[3].direct_neighbours = {{.bus = 2, .status = branch_native_measurement_unused},
                                               {.bus = 4, .status = has_no_measurement}};

        // Bus 4: has no measurement
        neighbour_list[4].bus = 4;
        neighbour_list[4].status = has_no_measurement;
        neighbour_list[4].direct_neighbours = {{.bus = 3, .status = has_no_measurement},
                                               {.bus = 5, .status = has_no_measurement}};

        // Bus 5: has measurement
        neighbour_list[5].bus = 5;
        neighbour_list[5].status = node_measured;
        neighbour_list[5].direct_neighbours = {{.bus = 0, .status = branch_native_measurement_unused},
                                               {.bus = 4, .status = has_no_measurement}};

        // Expand bidirectional connections
        complete_bidirectional_neighbourhood_info(neighbour_list);

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Should handle various connectivity statuses without crashing
        CHECK(result == true);
    }

    SUBCASE("Highly connected meshed network") {
        // Create a fully connected 4-node network (complete graph)
        std::vector<BusNeighbourhoodInfo> neighbour_list(4);

        for (Idx i = 0; i < 4; ++i) {
            neighbour_list[i].bus = i;
            neighbour_list[i].status = (i == 0 || i == 2) ? node_measured : has_no_measurement;
            neighbour_list[i].direct_neighbours.clear();

            // Connect to all other nodes
            for (Idx const j : std::views::iota(0, 4) | std::views::filter([i](Idx x) { return x != i; })) {
                auto const edge_status = (i == 1 && j == 3) ? branch_native_measurement_unused : has_no_measurement;
                neighbour_list[i].direct_neighbours.push_back({.bus = j, .status = edge_status});
            }
        }

        neighbour_list[1].direct_neighbours[1].status = branch_native_measurement_unused; // Add another measurement
        neighbour_list[2].direct_neighbours[1].status = branch_native_measurement_unused; // otherwise not observable

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        // Highly connected network with multiple measurements should be observable
        CHECK(result == true);
    }

    SUBCASE("Performance test with larger network") {
        // Test with a larger meshed network to verify algorithm doesn't hang
        constexpr Idx n_bus = 8;
        std::vector<BusNeighbourhoodInfo> neighbour_list(n_bus);

        // Create a ring topology with additional cross connections
        for (Idx i = 0; i < n_bus; ++i) {
            neighbour_list[i].bus = i;
            neighbour_list[i].status = (i % 3 == 0) ? node_measured : has_no_measurement;

            // Ring connections
            Idx const next_bus = (i + 1) % n_bus;
            Idx const prev_bus = (i + n_bus - 1) % n_bus;

            auto next_status = (i == 2) ? branch_native_measurement_unused : has_no_measurement;
            auto prev_status = has_no_measurement;

            neighbour_list[i].direct_neighbours = {{.bus = next_bus, .status = next_status},
                                                   {.bus = prev_bus, .status = prev_status}};

            // Add some cross connections for mesh
            if (i < n_bus / 2) {
                Idx const cross_bus = i + n_bus / 2;
                auto const cross_status = (i == 1) ? branch_native_measurement_unused : has_no_measurement;
                neighbour_list[i].direct_neighbours.push_back({.bus = cross_bus, .status = cross_status});
            }
        }
        neighbour_list[3].direct_neighbours[1].status = branch_native_measurement_unused; // part of creation

        // Expand bidirectional connections
        complete_bidirectional_neighbourhood_info(neighbour_list);

        // Add two more measurements to ensure observability
        neighbour_list[5].status = node_measured;
        neighbour_list[0].direct_neighbours[2].status = branch_native_measurement_unused;
        neighbour_list[4].direct_neighbours[2].status = branch_native_measurement_unused;

        bool const result = sufficient_condition_meshed_without_voltage_phasor(neighbour_list);

        CHECK(result == true);
    }
}

TEST_CASE("Basic observability structure tests") {
    using power_grid_model::math_solver::detail::ObservabilitySensorsResult;

    SUBCASE("Basic structure initialization") {
        ObservabilitySensorsResult result;
        result.flow_sensors = {1, 0, 1};
        result.voltage_phasor_sensors = {1, 0};
        result.bus_injections = {1, 2};
        result.is_possibly_ill_conditioned = false;

        CHECK(result.flow_sensors.size() == 3);
        CHECK(result.voltage_phasor_sensors.size() == 2);
        CHECK(result.bus_injections.size() == 2);
        CHECK(result.is_possibly_ill_conditioned == false);
    }
}

TEST_CASE("Test Observability - Necessary check end to end test") {
    /*
                  /-branch_1-\
            bus_2              bus_1 --branch_0-- bus_0 -- source
                  \-branch_2-/
    */
    MathModelTopology topo;
    topo.slack_bus = 0;
    // parallel branches are considered radial for observability purposes only
    topo.is_radial = true;
    topo.phase_shift = {0.0, 0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}, {1, 2}, {1, 2}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 1, 2}};
    topo.load_gen_type = {LoadGenType::const_pq, LoadGenType::const_pq};
    topo.power_sensors_per_bus = {from_sparse, {0, 0, 0, 1}};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0, 0, 0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 1, 1, 1}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0, 0}};
    topo.current_sensors_per_branch_from = {from_sparse, {0, 0, 0, 0}};
    topo.current_sensors_per_branch_to = {from_sparse, {0, 0, 0, 0}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 1}};

    MathModelParam<symmetric_t> param;
    param.source_param = {SourceCalcParam{.y1 = 10.0 - 50.0i, .y0 = 10.0 - 50.0i}};
    param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.load_gen_status = {1, 1};
    se_input.measured_voltage = {{.value = 1.0 + 2.0i, .variance = 3.0}};
    se_input.measured_bus_injection = {
        {.real_component = {.value = 1.0, .variance = 2.0}, .imag_component = {.value = 0.0, .variance = 3.0}}};
    se_input.measured_branch_from_power = {
        {.real_component = {.value = 3.0, .variance = 2.0}, .imag_component = {.value = 0.0, .variance = 1.0}}};

    SUBCASE("Observable grid") {
        SUBCASE("Voltage phasor sensor only") { check_observable(topo, param, se_input); }
        SUBCASE("Voltage magnitude sensor only") {
            // setting only real part of measurement makes it magnitude sensor
            se_input.measured_voltage = {{.value = {1.0, nan}, .variance = 2.0}};
            check_observable(topo, param, se_input);
        }
    }

    SUBCASE("No voltage sensor") {
        topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 0, 0}};
        se_input.measured_voltage = {};
        check_not_observable(topo, param, se_input);
    }
    SUBCASE("Count sensors") {
        // reduce 1 injection power sensor in upcoming cases
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 0, 0}};
        se_input.measured_bus_injection = {};

        SUBCASE("Voltage phasor unavailable condition for unobservable grid") {
            se_input.measured_voltage = {{.value = {1.0, nan}, .variance = 5.0}};
            check_not_observable(topo, param, se_input);
        }

        SUBCASE("Voltage phasor available condition for unobservable grid") {
            check_not_observable(topo, param, se_input);
        }

        SUBCASE("Power sensors on parallel branches gets counted as one sensor") {
            // add sensor on branch 2 to-side
            // move sensor on branch 0 to-side to branch 1 to side
            // hence 2 parallel sensors
            topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 1, 1}};
            topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0, 1}};
            se_input.measured_branch_to_power = {{.real_component = {.value = 100.0, .variance = 20.0},
                                                  .imag_component = {.value = 0.0, .variance = 30.0}}};
            check_not_observable(topo, param, se_input);
        }
    }
    SUBCASE("Not independent") {
        // set branch sensor to bus_1 <-branch_1-> bus_2
        // it is not independent with injection sensor of bus_2
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 1, 1}};
        // set non phasor measurement
        se_input.measured_voltage = {{.value = {33.0, nan}, .variance = 66.0}};
        // this will throw NotObservableError
        check_not_observable(topo, param, se_input);
    }
    SUBCASE("Current sensors also measure branch flow") {
        using enum AngleMeasurementType;

        topo.power_sensors_per_branch_from = {from_dense, {}, 3};
        se_input.measured_branch_from_power = {};
        topo.current_sensors_per_branch_from = {from_dense, {0}, 3};

        DecomposedComplexRandVar<symmetric_t> const current_measurement{
            .real_component = {.value = 10.0, .variance = 100.0}, .imag_component = {.value = 0.0, .variance = 200.0}};

        SUBCASE("With voltage phasor measurement") {
            SUBCASE("Local current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = local_angle, .measurement = current_measurement}};
                check_observable(topo, param, se_input);
            }
            SUBCASE("Global angle current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = global_angle, .measurement = current_measurement}};
                check_observable(topo, param, se_input);
            }
        }
        SUBCASE("No voltage phasor measurement and single current sensor") {
            se_input.measured_voltage = {{.value = {500.0, nan}, .variance = 50.0}};

            SUBCASE("Local current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = local_angle, .measurement = current_measurement}};
                check_observable(topo, param, se_input);
            }
            SUBCASE("Global angle current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = global_angle, .measurement = current_measurement}};
                check_not_observable(topo, param, se_input);
            }
        }
        SUBCASE("With voltage phasor measurement and two current sensors") {
            topo.current_sensors_per_branch_from = {from_dense, {0, 2}, 3};

            SUBCASE("Local current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = local_angle, .measurement = current_measurement},
                    {.angle_measurement_type = local_angle, .measurement = current_measurement}};
                check_observable(topo, param, se_input);
            }
            SUBCASE("Global angle current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = global_angle, .measurement = current_measurement},
                    {.angle_measurement_type = global_angle, .measurement = current_measurement}};
                check_observable(topo, param, se_input);
            }
        }
        SUBCASE("No voltage phasor measurement and two current sensors") {
            se_input.measured_voltage = {{.value = {555.0, nan}, .variance = 55.0}};
            topo.current_sensors_per_branch_from = {from_dense, {0, 2}, 3};

            SUBCASE("Local current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = local_angle, .measurement = current_measurement},
                    {.angle_measurement_type = local_angle, .measurement = current_measurement}};
                check_observable(topo, param, se_input);
            }
            SUBCASE("Global angle current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = global_angle, .measurement = current_measurement},
                    {.angle_measurement_type = global_angle, .measurement = current_measurement}};
                check_not_observable(topo, param, se_input);
            }
        }
    }
    SUBCASE("Voltage phasor sensors also measure branch flow") {
        SUBCASE("Only voltage phasor sensors as branch flow sensors") {
            // remove all power sensors
            topo.power_sensors_per_bus = {from_sparse, {0, 0, 0, 0}};
            topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 0, 0}};
            se_input.measured_bus_injection = {};
            se_input.measured_branch_from_power = {};
            SUBCASE("Without a reference voltage phasor sensor") {
                // sensor at the source is a magnitude voltage one
                topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 3}};
                se_input.measured_voltage = {{.value = {999.0, nan}, .variance = 44.0},
                                             {.value = 888.0 + 111.0i, .variance = 55.0},
                                             {.value = 777.0 + 222.0i, .variance = 66.0}};
                check_not_observable(topo, param, se_input);
            }
            SUBCASE("With a reference voltage phasor sensor") {
                // sensor at the source is a phasor voltage one
                topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 3}};
                se_input.measured_voltage = {{.value = 100.0 + 10.0i, .variance = 1.0},
                                             {.value = 200.0 + 20.0i, .variance = 2.0},
                                             {.value = 300.0 + 30.0i, .variance = 3.0}};
                check_observable(topo, param, se_input);
            }
        }
        SUBCASE("Voltage phasor and power sensors as branch flow sensors") {
            // keep branch power sensors only
            topo.power_sensors_per_bus = {from_sparse, {0, 0, 0, 0}};
            se_input.measured_bus_injection = {};
            SUBCASE("Without a reference voltage phasor sensor") {
                // sensor at the source is a magnitude voltage one
                topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 2}};
                se_input.measured_voltage = {{.value = {50.0, nan}, .variance = 1.0},
                                             {.value = 30.0 + 30.0i, .variance = 2.0}};
                check_not_observable(topo, param, se_input);
            }
            SUBCASE("With a reference voltage phasor sensor") {
                // sensor at the source is a phasor voltage one
                topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 2}};
                se_input.measured_voltage = {{.value = 100.0 + 300.0i, .variance = 5.0},
                                             {.value = 200.0 + 400.0i, .variance = 6.0}};
                check_observable(topo, param, se_input);
            }
        }
        SUBCASE("Voltage phasor and current sensors as branch flow sensors") {
            // add current sensors
            using enum AngleMeasurementType;
            topo.current_sensors_per_branch_to = {from_sparse, {0, 1, 1, 1}};
            DecomposedComplexRandVar<symmetric_t> const current_measurement{
                .real_component = {.value = 100.0, .variance = 15.0},
                .imag_component = {.value = 0.0, .variance = 10.0}};

            // remove all power sensors
            topo.power_sensors_per_bus = {from_sparse, {0, 0, 0, 0}};
            topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 0, 0}};
            se_input.measured_bus_injection = {};
            se_input.measured_branch_from_power = {};
            SUBCASE("Without a reference voltage phasor sensor") {
                // sensor at the source is a magnitude voltage one
                topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 2}};
                se_input.measured_voltage = {{.value = {10.0, nan}, .variance = 0.1},
                                             {.value = 100.0 + 200.0i, .variance = 9.0}};
                SUBCASE("Local current sensor") {
                    se_input.measured_branch_to_current = {
                        {.angle_measurement_type = local_angle, .measurement = current_measurement}};
                    check_not_observable(topo, param, se_input);
                }
                SUBCASE("Global current sensor") {
                    se_input.measured_branch_to_current = {
                        {.angle_measurement_type = global_angle, .measurement = current_measurement}};
                    check_not_observable(topo, param, se_input);
                }
            }
            SUBCASE("With a reference voltage phasor sensor") {
                // sensor at the source is a phasor voltage one
                topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2, 2}};
                se_input.measured_voltage = {{.value = 10.0 + 20.0i, .variance = 5.0},
                                             {.value = 30.0 + 40.0i, .variance = 4.0}};
                SUBCASE("Local current sensor") {
                    se_input.measured_branch_to_current = {
                        {.angle_measurement_type = local_angle, .measurement = current_measurement}};
                    check_observable(topo, param, se_input);
                }
                SUBCASE("Global current sensor") {
                    se_input.measured_branch_to_current = {
                        {.angle_measurement_type = global_angle, .measurement = current_measurement}};
                    check_observable(topo, param, se_input);
                }
            }
        }
    }
}

} // namespace power_grid_model
