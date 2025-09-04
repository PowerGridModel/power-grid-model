// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow solvers are tested

#pragma once

#include "test_math_solver_common.hpp"

#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/math_solver/sparse_lu_solver.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

namespace power_grid_model {
template <typename SolverType>
inline auto run_state_estimation(SolverType& solver, YBus<typename SolverType::sym> const& y_bus,
                                 StateEstimationInput<typename SolverType::sym> const& input, double err_tol,
                                 Idx max_iter, Logger& log) {
    static_assert(SolverType::is_iterative); // otherwise, call different version
    return solver.run_state_estimation(y_bus, input, err_tol, max_iter, log);
};

inline auto get_logger() { return common::logging::NoLogger{}; }

template <symmetry_tag sym_type> struct SESolverTestGrid : public SteadyStateSolverTestGrid<sym_type> {
    using sym = sym_type;

    // state estimation input
    // symmetric, with u angle
    auto se_input_angle_without_flow_sensors() const {
        auto const output_reference = this->output_ref();

        StateEstimationInput<sym> result;
        if constexpr (is_symmetric_v<sym>) {
            result.shunt_status = {1};
            result.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
            result.source_status = {1};
            result.measured_voltage = {
                {output_reference.u[0], 1.0}, {output_reference.u[2], 1.0}, {output_reference.u[2], 1.0}};
            auto const sum_s = output_reference.source[0].s + output_reference.load_gen[0].s +
                               output_reference.load_gen[1].s + output_reference.load_gen[2].s;
            result.measured_bus_injection = {{.real_component = {.value = real(sum_s), .variance = 0.5},
                                              .imag_component = {.value = imag(sum_s), .variance = 0.5}}};
            result.measured_source_power = {
                {.real_component = {.value = real(output_reference.source[0].s), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.source[0].s), .variance = 0.5}},
                {.real_component = {.value = real(output_reference.source[0].s), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.source[0].s), .variance = 0.5}}};

            result.measured_load_gen_power = {
                {.real_component = {.value = real(output_reference.load_gen[3].s), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.load_gen[3].s), .variance = 0.5}},
                {.real_component = {.value = real(output_reference.load_gen[4].s), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.load_gen[4].s), .variance = 0.5}},
                {.real_component = {.value = real(output_reference.load_gen[5].s), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.load_gen[5].s), .variance = 0.5}},
                {.real_component = {.value = 500.0, .variance = 0.5},
                 .imag_component = {.value = 0.0, .variance = 0.5}},
            };
            result.measured_shunt_power = {
                {.real_component = {.value = real(output_reference.shunt[0].s), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.shunt[0].s), .variance = 0.5}},
            };

        } else {
            result.shunt_status = {1};
            result.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
            result.source_status = {1};
            result.measured_voltage = {{ComplexValue<asymmetric_t>{output_reference.u[0]}, 1.0},
                                       {ComplexValue<asymmetric_t>{output_reference.u[2]}, 1.0},
                                       {ComplexValue<asymmetric_t>{output_reference.u[2]}, 1.0}};
            ComplexValue<asymmetric_t> const sum_s{output_reference.source[0].s + output_reference.load_gen[0].s +
                                                   output_reference.load_gen[1].s + output_reference.load_gen[2].s};
            result.measured_bus_injection = {
                {.real_component = {.value = real(sum_s), .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(sum_s), .variance = RealValue<asymmetric_t>{0.5}}}};
            result.measured_source_power = {
                {.real_component = {.value = real(output_reference.source[0].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.source[0].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
                {.real_component = {.value = real(output_reference.source[0].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.source[0].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}}};
            result.measured_load_gen_power = {
                {.real_component = {.value = real(output_reference.load_gen[3].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.load_gen[3].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
                {.real_component = {.value = real(output_reference.load_gen[4].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.load_gen[4].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
                {.real_component = {.value = real(output_reference.load_gen[5].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.load_gen[5].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
                {.real_component = {.value = real(500.0 * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(500.0 * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
            };
            result.measured_shunt_power = {
                {.real_component = {.value = real(output_reference.shunt[0].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.shunt[0].s * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
            };
        }
        return result;
    };

    auto se_input_angle() const {
        auto const output_reference = this->output_ref();

        StateEstimationInput<sym> result = se_input_angle_without_flow_sensors();

        if constexpr (is_symmetric_v<sym>) {
            result.measured_branch_from_power = {
                {.real_component = {.value = real(output_reference.branch[0].s_f), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.branch[0].s_f), .variance = 0.5}},
            };
            result.measured_branch_to_power = {
                {.real_component = {.value = real(output_reference.branch[0].s_t), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.branch[0].s_t), .variance = 0.5}},
                {.real_component = {.value = real(output_reference.branch[0].s_t), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.branch[0].s_t), .variance = 0.5}},
                {.real_component = {.value = real(output_reference.branch[1].s_t), .variance = 0.5},
                 .imag_component = {.value = imag(output_reference.branch[1].s_t), .variance = 0.5}},
            };
        } else {
            result.measured_branch_from_power = {
                {.real_component = {.value = real(output_reference.branch[0].s_f * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.branch[0].s_f * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
            };
            result.measured_branch_to_power = {
                {.real_component = {.value = real(output_reference.branch[0].s_t * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.branch[0].s_t * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
                {.real_component = {.value = real(output_reference.branch[0].s_t * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.branch[0].s_t * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
                {.real_component = {.value = real(output_reference.branch[1].s_t * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}},
                 .imag_component = {.value = imag(output_reference.branch[1].s_t * RealValue<asymmetric_t>{1.0}),
                                    .variance = RealValue<asymmetric_t>{0.5}}},
            };
        }
        return result;
    };

    // symmetric, without angle
    // no angle, keep the angle of 2nd measurement of bus2, which will be ignored
    auto se_input_no_angle() const {
        StateEstimationInput<sym> result = se_input_angle();
        result.measured_voltage[0].value = cabs(result.measured_voltage[0].value) + DoubleComplex{0.0, nan};
        result.measured_voltage[1].value = cabs(result.measured_voltage[1].value) + DoubleComplex{0.0, nan};
        return result;
    };

    // with angle, const z
    // set open for load 01, 34, scale load 5 (sensor 2)
    auto se_input_angle_const_z() const {
        StateEstimationInput<sym> result = se_input_angle();
        result.load_gen_status[0] = 0;
        result.load_gen_status[1] = 0;
        result.load_gen_status[3] = 0;
        result.load_gen_status[4] = 0;
        result.measured_load_gen_power[2].real_component.value *= 3.0;
        result.measured_load_gen_power[2].imag_component.value *= 3.0;
        return result;
    };

    auto se_input_angle_current_sensors(AngleMeasurementType angle_measurement_type) const {
        auto const output_reference = this->output_ref();

        StateEstimationInput<sym> result = se_input_angle_without_flow_sensors();

        auto const branch_from_current =
            output_reference.branch | std::ranges::views::transform([angle_measurement_type](auto const& b) {
                if (angle_measurement_type == AngleMeasurementType::local_angle) {
                    return static_cast<ComplexValue<sym>>(cabs(b.i_f) * exp(1.0i * arg(b.s_f)));
                }
                return b.i_f;
            });

        auto const branch_to_current =
            output_reference.branch | std::ranges::views::transform([angle_measurement_type](auto const& b) {
                if (angle_measurement_type == AngleMeasurementType::local_angle) {
                    return static_cast<ComplexValue<sym>>(cabs(b.i_t) * exp(1.0i * arg(b.s_t)));
                }
                return b.i_t;
            });

        if constexpr (is_symmetric_v<sym>) {
            result.measured_branch_from_current = {
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value = real(branch_from_current[0]), .variance = 0.5},
                                 .imag_component = {.value = imag(branch_from_current[0]), .variance = 0.5}}},
            };
            result.measured_branch_to_current = {
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value = real(branch_to_current[0]), .variance = 0.5},
                                 .imag_component = {.value = imag(branch_to_current[0]), .variance = 0.5}}},
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value = real(branch_to_current[0]), .variance = 0.5},
                                 .imag_component = {.value = imag(branch_to_current[0]), .variance = 0.5}}},
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value = real(branch_to_current[1]), .variance = 0.5},
                                 .imag_component = {.value = imag(branch_to_current[1]), .variance = 0.5}}}};
        } else {
            result.measured_branch_from_current = {
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value =
                                                        real(branch_from_current[0] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}},
                                 .imag_component = {.value =
                                                        imag(branch_from_current[0] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}}}},
            };
            result.measured_branch_to_current = {
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value = real(branch_to_current[0] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}},
                                 .imag_component = {.value = imag(branch_to_current[0] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}}}},
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value = real(branch_to_current[0] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}},
                                 .imag_component = {.value = imag(branch_to_current[0] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}}}},
                {.angle_measurement_type = angle_measurement_type,
                 .measurement = {.real_component = {.value = real(branch_to_current[1] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}},
                                 .imag_component = {.value = imag(branch_to_current[1] * RealValue<asymmetric_t>{1.0}),
                                                    .variance = RealValue<asymmetric_t>{0.5}}}},
            };
        }
        return result;
    }

    auto se_topo_power_sensors() const {
        /*
        All components except sensors are same as the common test grid.

        v means voltage measured, p means power measured, pp means double measured

        variance always 1.0
                                                               shunt0 (ys) (p)
        (pp)                     (y0, ys0)               (y1)         |
        source --yref-- bus0(vp) -p-branch0-pp- bus1 --branch1-p-  bus2(vv)
                        |                         |                   |
                    load012                   load345 (p)          load6 (not connected) (p, rubbish value)
                                               for const z,
                                           rubbish value for load3/4
        */
        MathModelTopology topo = this->topo();
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 3}};
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1, 1}};
        topo.power_sensors_per_source = {from_sparse, {0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 0, 0, 0, 1, 2, 3, 4}};
        topo.power_sensors_per_shunt = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1, 1}};
        topo.power_sensors_per_branch_to = {from_sparse, {0, 2, 3}};
        return topo;
    }

    auto se_topo_current_sensors() const {
        /*
        All components except sensors are same as the common test grid.

        v means voltage measured, p means power measured, pp means double measured,
        c means power measured, cc means double measured

        variance always 1.0
                                                               shunt0 (ys) (p)
        (pp)                     (y0, ys0)               (y1)         |
        source --yref-- bus0(vp) -c-branch0-cc- bus1 --branch1-c-  bus2(vv)
                        |                         |                   |
                    load012                   load345 (p)          load6 (not connected) (p, rubbish value)
                                               for const z,
                                           rubbish value for load3/4
        */
        MathModelTopology topo = this->topo();
        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 3}};
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1, 1}};
        topo.power_sensors_per_source = {from_sparse, {0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 0, 0, 0, 1, 2, 3, 4}};
        topo.power_sensors_per_shunt = {from_sparse, {0, 1}};
        topo.current_sensors_per_branch_from = {from_sparse, {0, 1, 1}};
        topo.current_sensors_per_branch_to = {from_sparse, {0, 2, 3}};
        return topo;
    }
};

TEST_CASE_TEMPLATE_DEFINE("Test math solver - SE", SolverType, test_math_solver_se_id) {
    constexpr auto error_tolerance{1e-10};
    constexpr auto num_iter{20};

    using sym = typename SolverType::sym;

    SESolverTestGrid<sym> const grid;

    // topo and param ptr
    auto param_ptr = std::make_shared<MathModelParam<sym> const>(grid.param());

    SUBCASE("Test se with power sensors") {
        auto topo_ptr = std::make_shared<MathModelTopology const>(grid.se_topo_power_sensors());
        YBus<sym> const y_bus{topo_ptr, param_ptr};

        SUBCASE("Test se with angle") {
            SolverType solver{y_bus, topo_ptr};
            auto log = get_logger();

            auto const se_input = grid.se_input_angle();
            SolverOutput<sym> const output =
                run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, log);
            assert_output(output, grid.output_ref());
        }

        SUBCASE("Test se without angle") {
            SolverType solver{y_bus, topo_ptr};
            auto log = get_logger();

            auto const se_input = grid.se_input_no_angle();
            SolverOutput<sym> const output =
                run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, log);
            assert_output(output, grid.output_ref(), true);
        }

        SUBCASE("Test se with angle, const z") {
            SolverType solver{y_bus, topo_ptr};
            auto log = get_logger();

            auto const se_input = grid.se_input_angle_const_z();
            SolverOutput<sym> const output =
                run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, log);
            assert_output(output, grid.output_ref_z());
        }

        SUBCASE("Test se with angle and different power variances") {
            SolverType solver{y_bus, topo_ptr};
            auto log = get_logger();

            auto se_input = grid.se_input_angle();
            auto& branch_from_power = se_input.measured_branch_from_power.front();
            branch_from_power.real_component.variance = RealValue<sym>{0.25};
            branch_from_power.imag_component.variance = RealValue<sym>{0.75};

            SolverOutput<sym> const output =
                run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, log);
            assert_output(output, grid.output_ref());
        }
    }

    SUBCASE("se input angle with current sensors") {
        auto topo_ptr = std::make_shared<MathModelTopology const>(grid.se_topo_current_sensors());
        YBus<sym> const y_bus{topo_ptr, param_ptr};
        SUBCASE("Test se with local angle current sensors") {
            SolverType solver{y_bus, topo_ptr};
            auto log = get_logger();

            auto const se_input = grid.se_input_angle_current_sensors(AngleMeasurementType::local_angle);
            SolverOutput<sym> const output =
                run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, log);
            assert_output(output, grid.output_ref());
        }

        SUBCASE("Test se with global angle current sensors") {
            SolverType solver{y_bus, topo_ptr};
            auto log = get_logger();

            auto const se_input = grid.se_input_angle_current_sensors(AngleMeasurementType::global_angle);
            SolverOutput<sym> const output =
                run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, log);
            assert_output(output, grid.output_ref());
        }
    }
}

TEST_CASE_TEMPLATE_DEFINE("Test math solver - SE, zero variance test", SolverType,
                          test_math_solver_se_zero_variance_id) {
    /*
    network, v means voltage measured
    variance always 1.0

    bus_1 --branch0-- bus_0(v) --yref-- source
    bus_1 = bus_0 = 1.0
    */
    static_assert(is_symmetric_v<typename SolverType::sym>); // asymmetric is not yet implemented

    constexpr auto error_tolerance{1e-10};
    constexpr auto num_iter{20};

    MathModelTopology topo;
    topo.slack_bus = 1;
    topo.phase_shift = {0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}};
    topo.sources_per_bus = {from_sparse, {0, 0, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 0}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 1}};
    topo.power_sensors_per_bus = {from_sparse, {0, 0, 0}};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 0}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0}};
    MathModelParam<symmetric_t> param;
    param.branch_param = {{1.0, -1.0, -1.0, 1.0}};
    auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.measured_voltage = {{.value = 1.0, .variance = 1.0}};

    SolverType solver{y_bus_sym, topo_ptr};
    auto log = get_logger();
    SolverOutput<symmetric_t> output;

    output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

    // check both voltage
    check_close(output.u[0], 1.0);
    check_close(output.u[1], 1.0);
}

TEST_CASE_TEMPLATE_DEFINE("Test math solver - SE, measurements", SolverType, test_math_solver_se_measurements_id) {
    /*
    network

     bus_0 --branch_0-- bus_1
        |                    |
    source_0               load_0

    */
    static_assert(is_symmetric_v<typename SolverType::sym>); // asymmetric is not yet implemented

    constexpr auto error_tolerance{1e-10};
    constexpr auto num_iter{20};

    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 1}};

    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
    topo.power_sensors_per_bus = {from_sparse, {0, 0, 0}};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0, 0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 0}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0}};
    topo.current_sensors_per_branch_from = {from_sparse, {0, 0}};
    topo.current_sensors_per_branch_to = {from_sparse, {0, 0}};

    MathModelParam<symmetric_t> param;
    param.branch_param = {{1.0e3, -1.0e3, -1.0e3, 1.0e3}};

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.load_gen_status = {1};
    se_input.measured_voltage = {{.value = 1.0, .variance = 0.1}};

    auto log = get_logger();
    SolverOutput<symmetric_t> output;

    SUBCASE("Source and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) -(p)-branch_0-- bus_1
            |                       |
        source_0(p)               load_0

        */
        topo.power_sensors_per_source = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};

        se_input.measured_source_power = {
            {.real_component = {.value = 1.93, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};
        se_input.measured_branch_from_power = {
            {.real_component = {.value = 1.97, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};

        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        CHECK(real(output.bus_injection[0]) == doctest::Approx(1.95));
        CHECK(real(output.source[0].s) == doctest::Approx(1.95));
        CHECK(real(output.branch[0].s_f) == doctest::Approx(1.95));
    }

    if (!SolverType::is_NRSE_solver) {
        SUBCASE("ILSE Source and branch current") {
            /*
            We can't compare ILSE with NRSE one to one.
            This test verifies the expected approximate results using the approximation of ILSE.
            The current and power sensors gets similar weightage in ILSE but not in NRSE.

            network, v means voltage measured, c means current measured

            bus_0(v) -(c)-branch_0-- bus_1
                |                       |
            source_0(p)               load_0

            */
            // to make the distinction between global and local angle current measurement
            auto const global_shift = phase_shift(1.0 + 1.0i);

            topo.power_sensors_per_source = {from_sparse, {0, 1}};
            topo.current_sensors_per_branch_from = {from_sparse, {0, 1}};
            se_input.measured_voltage = {{.value = 1.0 * global_shift, .variance = 0.1}};

            auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
            auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
            YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};

            SolverType solver{y_bus_sym, topo_ptr};

            SUBCASE("Local angle current sensor") {
                SUBCASE("No phase shift") {
                    se_input.measured_source_power = {{.real_component = {.value = 1.93, .variance = 0.05},
                                                       .imag_component = {.value = 0.0, .variance = 0.05}}};
                    se_input.measured_branch_from_current = {
                        {.angle_measurement_type = AngleMeasurementType::local_angle,
                         .measurement = {.real_component = {.value = 1.97, .variance = 0.05},
                                         .imag_component = {.value = 0.0, .variance = 0.05}}}};
                    output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

                    CHECK(real(output.bus_injection[0]) == doctest::Approx(1.95));
                    CHECK(real(output.source[0].s) == doctest::Approx(1.95));
                    CHECK(real(output.branch[0].s_f) == doctest::Approx(1.95));
                    CHECK(real(output.branch[0].i_f) == doctest::Approx(real(1.95 * global_shift)));
                    CHECK(imag(output.branch[0].i_f) == doctest::Approx(imag(1.95 * global_shift)));
                }
                SUBCASE("With phase shift") {
                    se_input.measured_source_power = {{.real_component = {.value = 0.0, .variance = 0.05},
                                                       .imag_component = {.value = 1.93, .variance = 0.05}}};
                    se_input.measured_branch_from_current = {
                        {.angle_measurement_type = AngleMeasurementType::local_angle,
                         .measurement = {.real_component = {.value = 0.0, .variance = 0.05},
                                         .imag_component = {.value = 1.97, .variance = 0.05}}}};

                    output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

                    CHECK(imag(output.bus_injection[0]) == doctest::Approx(1.95));
                    CHECK(imag(output.source[0].s) == doctest::Approx(1.95));
                    CHECK(imag(output.branch[0].s_f) == doctest::Approx(1.95));
                    CHECK(real(output.branch[0].i_f) == doctest::Approx(real(1.95 * global_shift)));
                    CHECK(imag(output.branch[0].i_f) == doctest::Approx(-imag(1.95 * global_shift)));
                }
            }
            SUBCASE("Global angle current sensor") {
                se_input.measured_source_power = {{.real_component = {.value = 1.93, .variance = 0.05},
                                                   .imag_component = {.value = 0.0, .variance = 0.05}}};
                se_input.measured_branch_from_current = {
                    {.angle_measurement_type = AngleMeasurementType::global_angle,
                     .measurement = {.real_component = {.value = real(1.97 * global_shift), .variance = 0.05},
                                     .imag_component = {.value = imag(1.97 * global_shift), .variance = 0.05}}}};

                output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

                CHECK(real(output.bus_injection[0]) == doctest::Approx(1.95));
                CHECK(real(output.source[0].s) == doctest::Approx(1.95));
                CHECK(real(output.branch[0].s_f) == doctest::Approx(1.95));
                CHECK(real(output.branch[0].i_f) == doctest::Approx(real(1.95 * global_shift)));
                CHECK(imag(output.branch[0].i_f) == doctest::Approx(imag(1.95 * global_shift)));
            }
        }
    }

    SUBCASE("Load and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-(p)- bus_1
           |                        |
        source_0                 load_0(p)

        */
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_to = {from_sparse, {0, 1}};

        se_input.measured_load_gen_power = {
            {.real_component = {.value = -1.93, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};
        se_input.measured_branch_to_power = {
            {.real_component = {.value = -1.97, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-1.95));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-1.95));
        CHECK(real(output.branch[0].s_t) == doctest::Approx(-1.95));
    }

    SUBCASE("Node injection and source") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(vp) -(p)-branch_0-- bus_1
            |                        |
        source_0(p)                load_0

        */
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.power_sensors_per_source = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};

        se_input.measured_bus_injection = {
            {.real_component = {.value = 2.2, .variance = 0.1}, .imag_component = {.value = 0.0, .variance = 0.1}}};
        se_input.measured_source_power = {
            {.real_component = {.value = 1.93, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};
        se_input.measured_branch_from_power = {
            {.real_component = {.value = 1.97, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        CHECK(real(output.bus_injection[0]) == doctest::Approx(2.0));
        CHECK(real(output.source[0].s) == doctest::Approx(2.0));
        CHECK(real(output.branch[0].s_f) == doctest::Approx(2.0));
    }

    SUBCASE("Node injection, source and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(vp) -(p)-branch_0-- bus_1
            |                        |
        source_0(p)                load_0

        */
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.power_sensors_per_source = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};

        se_input.measured_bus_injection = {
            {.real_component = {.value = 2.2, .variance = 0.1}, .imag_component = {.value = 0.0, .variance = 0.1}}};
        se_input.measured_source_power = {
            {.real_component = {.value = 1.93, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};
        se_input.measured_branch_from_power = {
            {.real_component = {.value = 1.97, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        CHECK(real(output.bus_injection[0]) == doctest::Approx(2.0));
        CHECK(real(output.source[0].s) == doctest::Approx(2.0));
        CHECK(real(output.branch[0].s_f) == doctest::Approx(2.0));
    }

    SUBCASE("Node injection, load and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-(p)- bus_1(p)
           |                        |
        source_0                 load_0(p)

        */
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 1}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_to = {from_sparse, {0, 1}};

        se_input.measured_bus_injection = {
            {.real_component = {.value = -2.2, .variance = 0.1}, .imag_component = {.value = 0.0, .variance = 0.1}}};
        se_input.measured_load_gen_power = {
            {.real_component = {.value = -1.93, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};
        se_input.measured_branch_to_power = {
            {.real_component = {.value = -1.97, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-2.0));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-2.0));
        CHECK(real(output.branch[0].s_t) == doctest::Approx(-2.0));
    }

    SUBCASE("Load and gen") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-- bus_1
           |                    /   \
        source_0          load_0(p)  gen_1(p)

        */

        topo.load_gens_per_bus = {from_sparse, {0, 0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1, 2}};

        se_input.load_gen_status = {1, 1};
        se_input.measured_load_gen_power = {
            {.real_component = {.value = -3.0, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}},
            {.real_component = {.value = 1.0, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-2.0));
        CHECK(real(output.branch[0].s_t) == doctest::Approx(-2.0));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-3.0));
        CHECK(real(output.load_gen[1].s) == doctest::Approx(1.0));
    }

    SUBCASE("Node injection, load and gen") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-- bus_1(p)
           |                    /   \
        source_0          load_0(p)  gen_1(p)
        */

        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1, 2}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 1}};

        se_input.load_gen_status = {1, 1};
        se_input.measured_load_gen_power = {
            {.real_component = {.value = -1.8, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}},
            {.real_component = {.value = 0.9, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}}};
        se_input.measured_bus_injection = {
            {.real_component = {.value = -1.1, .variance = 0.1}, .imag_component = {.value = 0.0, .variance = 0.1}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-1.0));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-1.85));
        CHECK(real(output.load_gen[1].s) == doctest::Approx(0.85));
    }

    SUBCASE("Node injection, load and gen with different variances") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-- bus_1(p)
           |                    /   \
        source_0          load_0(p)  gen_1(p)
        */

        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1, 2}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 1}};

        se_input.load_gen_status = {1, 1};
        se_input.measured_load_gen_power = {
            {.real_component = {.value = -1.8, .variance = 0.05}, .imag_component = {.value = 0.0, .variance = 0.05}},
            {.real_component = {.value = 0.9, .variance = 0.025}, .imag_component = {.value = 0.0, .variance = 0.075}}};
        se_input.measured_bus_injection = {
            {.real_component = {.value = -1.1, .variance = 0.1}, .imag_component = {.value = 0.0, .variance = 0.1}}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, log);

        // the different aggregation of the load gen's P and Q measurements cause differences compared to the case with
        // identical variances
        CHECK(real(output.bus_injection[1]) > doctest::Approx(-1.0));
        CHECK(real(output.load_gen[0].s) < doctest::Approx(-1.85));
        CHECK(real(output.load_gen[1].s) > doctest::Approx(0.85));
    }

    const ComplexValue<symmetric_t> load_gen_s =
        std::accumulate(output.load_gen.begin(), output.load_gen.end(), ComplexValue<symmetric_t>{},
                        [](auto const& first, auto const& second) { return first + second.s; });

    if (!SolverType::has_global_current_sensor_implemented) {
        // TODO(figueroa1395): remove this check when global angle current sensor is implemented for
        // NRSE solver
        CHECK(true);
    } else {
        CHECK(output.bus_injection[0] == output.branch[0].s_f);
        CHECK(output.bus_injection[0] == output.source[0].s);
        CHECK(output.bus_injection[1] == output.branch[0].s_t);
        CHECK(real(output.bus_injection[1]) == doctest::Approx(real(load_gen_s)));
    }
}

} // namespace power_grid_model
