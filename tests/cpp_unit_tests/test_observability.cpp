// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/math_solver/observability.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

namespace {
void check_not_observable(MathModelTopology const& topo, MathModelParam<symmetric_t> param,
                          StateEstimationInput<symmetric_t> const& se_input) {
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
    YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
    math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};

    CHECK_THROWS_AS(math_solver::detail::necessary_observability_check(measured_values, y_bus.shared_topology()),
                    NotObservableError);
}
} // namespace

TEST_CASE("Necessary observability check") {
    /*
                  /-branch_0-\
            bus_2              bus_1 --branch_0-- bus_0 -- source
                  \-branch_1-/
    */
    MathModelTopology topo;
    topo.slack_bus = 0;
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
    topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 1, 1}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0, 0}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 1}};

    MathModelParam<symmetric_t> param;
    param.source_param = {10.0 - 50.0i};
    param.branch_param = {{1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}, {1.0, -1.0, -1.0, 1.0}};
    auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.load_gen_status = {1, 1};
    se_input.measured_voltage = {{1.0 + 1.0i, 1.0}};
    se_input.measured_bus_injection = {{1.0, 1.0, 1.0}};
    se_input.measured_branch_from_power = {{1.0, 1.0, 1.0}};

    SUBCASE("Observable grid") {
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus{topo_ptr, param_ptr};
        math_solver::MeasuredValues<symmetric_t> const measured_values{y_bus.shared_topology(), se_input};
        CHECK_NOTHROW(math_solver::detail::necessary_observability_check(measured_values, y_bus.shared_topology()));
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

        SUBCASE("voltage phasor unavailable condition for unobservable grid") {
            // setting only real part of measurement makes it magnitude sensor
            se_input.measured_voltage = {{{1.0, nan}, 1.0}};
            check_not_observable(topo, param, se_input);
        }

        SUBCASE("voltage phasor available condition for unobservable grid") {
            check_not_observable(topo, param, se_input);
        }

        SUBCASE("Parallel branch get counted as one sensor") {
            // Add sensor on branch 3 to side. Hence 2 parallel sensors
            topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0, 1}};
            se_input.measured_branch_to_power = {{1.0, 1.0, 1.0}};
            check_not_observable(topo, param, se_input);
        }
    }
}

} // namespace power_grid_model
