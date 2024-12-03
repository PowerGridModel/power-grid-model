// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow solvers are tested

#include "test_math_solver_common.hpp"

#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/math_solver/sparse_lu_solver.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

namespace power_grid_model {
template <typename SolverType>
inline auto run_state_estimation(SolverType& solver, YBus<typename SolverType::sym> const& y_bus,
                                 StateEstimationInput<typename SolverType::sym> const& input, double err_tol,
                                 Idx max_iter, CalculationInfo& calculation_info) {
    static_assert(SolverType::is_iterative); // otherwise, call different version
    return solver.run_state_estimation(y_bus, input, err_tol, max_iter, calculation_info);
};

template <symmetry_tag sym_type> struct SESolverTestGrid : public SteadyStateSolverTestGrid<sym_type> {
    using sym = sym_type;

    // state estimation input
    // symmetric, with u angle
    auto se_input_angle() const {
        auto const output_reference = this->output_ref();

        StateEstimationInput<sym> result;
        if constexpr (is_symmetric_v<sym>) {
            result.shunt_status = {1};
            result.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
            result.source_status = {1};
            result.measured_voltage = {
                {output_reference.u[0], 1.0}, {output_reference.u[2], 1.0}, {output_reference.u[2], 1.0}};
            result.measured_bus_injection = {{output_reference.source[0].s + output_reference.load_gen[0].s +
                                                  output_reference.load_gen[1].s + output_reference.load_gen[2].s,
                                              0.5, 0.5}};
            result.measured_source_power = {{output_reference.source[0].s, 0.5, 0.5},
                                            {output_reference.source[0].s, 0.5, 0.5}};
            result.measured_load_gen_power = {
                {output_reference.load_gen[3].s, 0.5, 0.5},
                {output_reference.load_gen[4].s, 0.5, 0.5},
                {output_reference.load_gen[5].s, 0.5, 0.5},
                {500.0, 0.5, 0.5},
            };
            result.measured_shunt_power = {
                {output_reference.shunt[0].s, 0.5, 0.5},
            };

            result.measured_branch_from_power = {
                {output_reference.branch[0].s_f, 0.5, 0.5},
            };
            result.measured_branch_to_power = {
                {output_reference.branch[0].s_t, 0.5, 0.5},
                {output_reference.branch[0].s_t, 0.5, 0.5},
                {output_reference.branch[1].s_t, 0.5, 0.5},
            };
        } else {
            result.shunt_status = {1};
            result.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
            result.source_status = {1};
            result.measured_voltage = {{ComplexValue<asymmetric_t>{output_reference.u[0]}, 1.0},
                                       {ComplexValue<asymmetric_t>{output_reference.u[2]}, 1.0},
                                       {ComplexValue<asymmetric_t>{output_reference.u[2]}, 1.0}};
            result.measured_bus_injection = {{(output_reference.source[0].s + output_reference.load_gen[0].s +
                                               output_reference.load_gen[1].s + output_reference.load_gen[2].s) *
                                                  RealValue<asymmetric_t>{1.0},
                                              RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}}};
            result.measured_source_power = {{output_reference.source[0].s * RealValue<asymmetric_t>{1.0},
                                             RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}},
                                            {output_reference.source[0].s * RealValue<asymmetric_t>{1.0},
                                             RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}}};
            result.measured_load_gen_power = {
                {output_reference.load_gen[3].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_reference.load_gen[4].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_reference.load_gen[5].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {500.0 * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}},
            };
            result.measured_shunt_power = {
                {output_reference.shunt[0].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
            };

            result.measured_branch_from_power = {
                {output_reference.branch[0].s_f * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
            };
            result.measured_branch_to_power = {
                {output_reference.branch[0].s_t * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_reference.branch[0].s_t * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_reference.branch[1].s_t * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
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
        result.measured_load_gen_power[2].value *= 3.0;
        return result;
    };
};

TEST_CASE_TEMPLATE_DEFINE("Test math solver - SE", SolverType, test_math_solver_se_id) {
    constexpr auto error_tolerance{1e-10};
    constexpr auto num_iter{20};

    using sym = typename SolverType::sym;

    SESolverTestGrid<sym> const grid;

    // topo and param ptr
    auto param_ptr = std::make_shared<MathModelParam<sym> const>(grid.param());
    auto topo_ptr = std::make_shared<MathModelTopology const>(grid.topo());
    YBus<sym> const y_bus{topo_ptr, param_ptr};

    SUBCASE("Test se with angle") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        SolverOutput<sym> output;

        auto const se_input = grid.se_input_angle();
        output = run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, info);
        assert_output(output, grid.output_ref());
    }

    SUBCASE("Test se without angle") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        SolverOutput<sym> output;

        auto const se_input = grid.se_input_no_angle();
        output = run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, info);
        assert_output(output, grid.output_ref(), true);
    }

    SUBCASE("Test se with angle, const z") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        SolverOutput<sym> output;

        auto const se_input = grid.se_input_angle_const_z();
        output = run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, info);
        assert_output(output, grid.output_ref_z());
    }

    SUBCASE("Test se with angle and different power variances") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;

        auto se_input = grid.se_input_angle();
        auto& branch_from_power = se_input.measured_branch_from_power.front();
        branch_from_power.p_variance = RealValue<sym>{0.25};
        branch_from_power.q_variance = RealValue<sym>{0.75};
        SolverOutput<sym> output;

        output = run_state_estimation(solver, y_bus, se_input, error_tolerance, num_iter, info);
        assert_output(output, grid.output_ref());
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
    se_input.measured_voltage = {{1.0, 1.0}};

    SolverType solver{y_bus_sym, topo_ptr};
    CalculationInfo info;
    SolverOutput<symmetric_t> output;

    output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

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

    MathModelParam<symmetric_t> param;
    param.branch_param = {{1.0e3, -1.0e3, -1.0e3, 1.0e3}};

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.load_gen_status = {1};
    se_input.measured_voltage = {{1.0, 0.1}};

    CalculationInfo info;
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

        se_input.measured_source_power = {{1.93, 0.05, 0.05}};
        se_input.measured_branch_from_power = {{1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};

        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

        CHECK(real(output.bus_injection[0]) == doctest::Approx(1.95));
        CHECK(real(output.source[0].s) == doctest::Approx(1.95));
        CHECK(real(output.branch[0].s_f) == doctest::Approx(1.95));
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

        se_input.measured_load_gen_power = {{-1.93, 0.05, 0.05}};
        se_input.measured_branch_to_power = {{-1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

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

        se_input.measured_bus_injection = {{2.2, 0.1, 0.1}};
        se_input.measured_source_power = {{1.93, 0.05, 0.05}};
        se_input.measured_branch_from_power = {{1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

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

        se_input.measured_bus_injection = {{2.2, 0.1, 0.1}};
        se_input.measured_source_power = {{1.93, 0.05, 0.05}};
        se_input.measured_branch_from_power = {{1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

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

        se_input.measured_bus_injection = {{-2.2, 0.1, 0.1}};
        se_input.measured_load_gen_power = {{-1.93, 0.05, 0.05}};
        se_input.measured_branch_to_power = {{-1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

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
        se_input.measured_load_gen_power = {{-3.0, 0.05, 0.05}, {1.0, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

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
        se_input.measured_load_gen_power = {{-1.8, 0.05, 0.05}, {0.9, 0.05, 0.05}};
        se_input.measured_bus_injection = {{-1.1, 0.1, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

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
        se_input.measured_load_gen_power = {{-1.8, 0.05, 0.05}, {0.9, 0.025, 0.075}};
        se_input.measured_bus_injection = {{-1.1, 0.1, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        SolverType solver{y_bus_sym, topo_ptr};

        output = run_state_estimation(solver, y_bus_sym, se_input, error_tolerance, num_iter, info);

        // the different aggregation of the load gen's P and Q measurements cause differences compared to the case with
        // identical variances
        CHECK(real(output.bus_injection[1]) > doctest::Approx(-1.0));
        CHECK(real(output.load_gen[0].s) < doctest::Approx(-1.85));
        CHECK(real(output.load_gen[1].s) > doctest::Approx(0.85));
    }

    const ComplexValue<symmetric_t> load_gen_s =
        std::accumulate(output.load_gen.begin(), output.load_gen.end(), ComplexValue<symmetric_t>{},
                        [](auto const& first, auto const& second) { return first + second.s; });

    CHECK(output.bus_injection[0] == output.branch[0].s_f);
    CHECK(output.bus_injection[0] == output.source[0].s);
    CHECK(output.bus_injection[1] == output.branch[0].s_t);
    CHECK(real(output.bus_injection[1]) == doctest::Approx(real(load_gen_s)));
}

} // namespace power_grid_model