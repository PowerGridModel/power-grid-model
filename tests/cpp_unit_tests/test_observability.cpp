// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/math_solver/observability.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

TEST_CASE("Necessary observability check") {
    /*
    network, v means voltage measured
    variance always 1.0

    bus_1 --branch0-- bus_0(v) --yref-- source
    bus_1 = bus_0 = 1.0
    */
    MathModelTopology topo;
    topo.slack_bus = 1;
    topo.phase_shift = {0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}};
    topo.sources_per_bus = {from_sparse, {0, 0, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 0}};
    topo.power_sensors_per_bus = {from_sparse, {0, 0, 0}};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 0}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0}};

    MathModelParam<symmetric_t> param;
    param.branch_param = {{1.0, -1.0, -1.0, 1.0}};
    auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};

    SUBCASE("Observable grid") {
        topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 1}};
        se_input.measured_voltage = {{1.0, 1.0}};

        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        // Should not throw exception
        math_solver::necessary_observability_check(measured_values, y_bus.shared_topology());
    }

    SUBCASE("No voltage sensors") {
        topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 0}};

        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

        CHECK_THROWS_AS(math_solver::necessary_observability_check(measured_values, y_bus.shared_topology()),
                        NotObservableError);
    }
}

} // namespace power_grid_model
