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
        CHECK_NOTHROW(
            math_solver::observability_check(measured_values, y_bus.math_topology(), y_bus.y_bus_structure()));
    } else {
        CHECK_THROWS_AS(
            math_solver::observability_check(measured_values, y_bus.math_topology(), y_bus.y_bus_structure()),
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
