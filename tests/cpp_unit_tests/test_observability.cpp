// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/math_solver/observability.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <doctest/doctest.h>

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
        CHECK_NOTHROW(math_solver::necessary_observability_check(measured_values, y_bus.math_topology(),
                                                                 y_bus.y_bus_structure()));
    } else {
        CHECK_THROWS_AS(
            math_solver::necessary_observability_check(measured_values, y_bus.math_topology(), y_bus.y_bus_structure()),
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

TEST_CASE("Necessary observability check") {
    /*
                  /-branch_1-\
            bus_2              bus_1 --branch_0-- bus_0 -- source
                  \-branch_2-/
    */
    MathModelTopology topo;
    topo.slack_bus = 0;
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
    se_input.measured_voltage = {{.value = 1.0 + 1.0i, .variance = 1.0}};
    se_input.measured_bus_injection = {
        {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}}};
    se_input.measured_branch_from_power = {
        {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}}};

    SUBCASE("Observable grid") { check_observable(topo, param, se_input); }

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
            se_input.measured_voltage = {{.value = {1.0, nan}, .variance = 1.0}};
            check_not_observable(topo, param, se_input);
        }

        SUBCASE("voltage phasor available condition for unobservable grid") {
            check_not_observable(topo, param, se_input);
        }

        SUBCASE("Parallel branch get counted as one sensor") {
            // Add sensor on branch 3 to side. Hence 2 parallel sensors
            topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 1, 1}};
            topo.power_sensors_per_branch_to = {from_sparse, {0, 0, 0, 1}};
            se_input.measured_branch_to_power = {
                {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}}};
            check_not_observable(topo, param, se_input);
        }
    }
    SUBCASE("Not independent") {
        // set branch sensor to bus_1 <-branch_1-> bus_2
        // it is not independent with injection sensor of bus_2
        topo.power_sensors_per_branch_from = {from_sparse, {0, 0, 1, 1}};
        // set non phasor measurement
        se_input.measured_voltage = {{.value = {1.0, nan}, .variance = 1.0}};
        // this will throw NotObservableError
        check_not_observable(topo, param, se_input);
    }
    SUBCASE("Current sensors also measure branch flow") {
        using enum AngleMeasurementType;

        topo.power_sensors_per_branch_from = {from_dense, {}, 3};
        se_input.measured_branch_from_power = {};
        topo.current_sensors_per_branch_from = {from_dense, {2}, 3};

        DecomposedComplexRandVar<symmetric_t> const current_measurement{
            .real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 0.0, .variance = 1.0}};

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
            se_input.measured_voltage = {{.value = {1.0, nan}, .variance = 1.0}};

            SUBCASE("Local current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = local_angle, .measurement = current_measurement}};
                check_not_observable(topo, param, se_input);
            }
            SUBCASE("Global angle current sensor") {
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = global_angle, .measurement = current_measurement}};
                check_not_observable(topo, param, se_input);
            }
        }
        SUBCASE("No voltage phasor measurement and two current sensors") {
            se_input.measured_voltage = {{.value = {1.0, nan}, .variance = 1.0}};
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
}
} // namespace power_grid_model
