// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_math_solver_common.hpp"
#include "test_math_solver_pf.hpp" // NOLINT(misc-include-cleaner)

#include <doctest/doctest.h>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/grouped_index_vector.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/newton_raphson_pf_solver.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

TYPE_TO_STRING_AS("NewtonRaphsonPFSolver<symmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonPFSolver<power_grid_model::symmetric_t>);
TYPE_TO_STRING_AS("NewtonRaphsonPFSolver<asymmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonPFSolver<power_grid_model::asymmetric_t>);

namespace power_grid_model::math_solver {
namespace {
using newton_raphson_pf::PFJacBlock;
} // namespace

TEST_CASE("Test block") {
    SUBCASE("symmetric") {
        PFJacBlock<symmetric_t> b{};
        b.h() += 1.0;
        b.n() += 2.0;
        b.m() += 3.0;
        b.l() += 4.0;
        CHECK(b.h() == 1.0);
        CHECK(b.n() == 2.0);
        CHECK(b.m() == 3.0);
        CHECK(b.l() == 4.0);
    }

    SUBCASE("Asymmetric") {
        PFJacBlock<asymmetric_t> b{};
        RealTensor<asymmetric_t> const h{1.0};
        RealTensor<asymmetric_t> const n{2.0};
        RealTensor<asymmetric_t> const m{3.0};
        RealTensor<asymmetric_t> const l{4.0};
        b.h() += h;
        b.n() += n;
        b.m() += m;
        b.l() += l;
        check_close<asymmetric_t>(b.h(), h, numerical_tolerance);
        check_close<asymmetric_t>(b.n(), n, numerical_tolerance);
        check_close<asymmetric_t>(b.m(), m, numerical_tolerance);
        check_close<asymmetric_t>(b.l(), l, numerical_tolerance);
    }
}

TEST_CASE_TEMPLATE_INVOKE(test_math_solver_pf_id, NewtonRaphsonPFSolver<symmetric_t>);
TEST_CASE_TEMPLATE_INVOKE(test_math_solver_pf_id, NewtonRaphsonPFSolver<asymmetric_t>);

TEST_CASE("Newton-Raphson PV reactive-power limits switch one-way to PQ") {
    using enum LoadGenType;

    /**
     * Bus 0 (Slack) ---branch--- Bus 1 (PV with voltage regulator)
     *   |                          |
     * Source                    Load/Gen (with 1 voltage regulator)
     * (u=1.0)                   (P=0.5, regulates to u=1.0)
     */
    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 1}};
    topo.load_gen_type = {const_pq};
    topo.voltage_regulators_per_load_gen = {from_sparse, {0, 1}};

    MathModelParam<symmetric_t> param;
    constexpr DoubleComplex y{10.0, -20.0};
    param.branch_param = {{y, -y, -y, y}};
    param.shunt_param = {};
    param.source_param = {{.y1 = 1e6, .y0 = 1e6}};

    auto const input = [](double q_min, double q_max) {
        return PowerFlowInput<symmetric_t>{
            .source = {1.0},
            .s_injection = {0.5},
            .voltage_regulator = {{.status = 1, .u_ref = 1.0, .q_min = q_min, .q_max = q_max, .generator_id = 42}},
            .load_gen_status = {1}};
    };

    const YBus<symmetric_t> y_bus{topo, param};
    common::logging::NoLogger log;
    constexpr bool cache_run = false;

    SUBCASE("No violation remains PV") {
        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        auto const output = solver.run_power_flow(y_bus, input(-1.0, 1.0), 1e-12, 20, cache_run, log);

        CHECK(output.bus[1].bus_type == BusType::pv);
        CHECK(cabs(output.u[1]) == doctest::Approx(1.0));
        CHECK(output.voltage_regulator[0].limit_violated == LimitViolation::none);
        CHECK(imag(output.load_gen[0].s) > -1.0);
        CHECK(imag(output.load_gen[0].s) < 1.0);
    }

    SUBCASE("Qmax violation clamps and remains PQ") {
        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        constexpr double q_max = -0.3;
        auto const output = solver.run_power_flow(y_bus, input(-1.0, q_max), 1e-12, 20, cache_run, log);

        CHECK(output.voltage_regulator[0].limit_violated == LimitViolation::upper);
        CHECK(imag(output.load_gen[0].s) == doctest::Approx(q_max));
        CHECK(output.bus[1].bus_type == BusType::pq);
        CHECK(cabs(output.u[1]) != doctest::Approx(1.0));
    }

    SUBCASE("Qmin violation clamps and remains PQ") {
        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        constexpr double q_min = -0.2;
        auto const output = solver.run_power_flow(y_bus, input(q_min, 1.0), 1e-12, 20, cache_run, log);

        CHECK(output.voltage_regulator[0].limit_violated == LimitViolation::lower);
        CHECK(imag(output.load_gen[0].s) == doctest::Approx(q_min));
        CHECK(output.bus[1].bus_type == BusType::pq);
        CHECK(cabs(output.u[1]) != doctest::Approx(1.0));
    }
}

TEST_CASE("Newton-Raphson PV reactive-power limit switches are deterministic") {
    using enum LoadGenType;

    /**
     * Bus 0 (Slack) ---branch--- Bus 1 (PV) ---branch--- Bus 2 (PV)
     *   |                          |                         |
     * Source                   Load/Gen 0              Load/Gen 1
     * (u=1.0)                (1 regulator)           (1 regulator)
     *                        P=0.5, u=1.0            P=0.5, u=1.0
     */
    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}, {1, 2}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 1, 2}};
    topo.load_gen_type = {const_pq, const_pq};
    topo.voltage_regulators_per_load_gen = {from_sparse, {0, 1, 2}};

    MathModelParam<symmetric_t> param;
    constexpr DoubleComplex y{10.0, -20.0};
    param.branch_param = {{y, -y, -y, y}, {y, -y, -y, y}};
    param.shunt_param = {};
    param.source_param = {{.y1 = 1e6, .y0 = 1e6}};

    PowerFlowInput<symmetric_t> input;
    input.source = {1.0};
    input.s_injection = {0.5, 0.5};
    input.voltage_regulator = {{.status = 1, .u_ref = 1.0, .q_min = -1.0, .q_max = -0.3, .generator_id = 41},
                               {.status = 1, .u_ref = 1.0, .q_min = -1.0, .q_max = -0.3, .generator_id = 42}};
    input.load_gen_status = {1, 1};

    const YBus<symmetric_t> y_bus{topo, param};
    common::logging::NoLogger log;

    NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};

    constexpr bool cache_run = false;
    auto const output = solver.run_power_flow(y_bus, input, 1e-12, 20, cache_run, log);

    // both regulators violate limit and both nodes are switched to PQ
    CHECK(output.voltage_regulator[0].limit_violated == LimitViolation::upper);
    CHECK(output.voltage_regulator[1].limit_violated == LimitViolation::upper);
    CHECK(imag(output.load_gen[0].s) == doctest::Approx(-0.3));
    CHECK(imag(output.load_gen[1].s) == doctest::Approx(-0.3));
    CHECK(cabs(output.u[1]) != doctest::Approx(1.0));
    CHECK(cabs(output.u[2]) != doctest::Approx(1.0));
}

TEST_CASE("Newton-Raphson bus types and Q-limits") {
    using enum LoadGenType;

    /**
     * Network topology:
     * Bus 0 (Slack) -- Bus 1 (no reg) -- Bus 2 (single reg) -- Bus 3 (two regs)
     */
    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0, 0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}, {1, 2}, {2, 3}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0, 0}};

    // 4 load_gens: Bus 1 (no reg), Bus 2 (1 reg), Bus 3 (2 regs)
    topo.load_gens_per_bus = {from_sparse, {0, 0, 1, 2, 4}};
    topo.load_gen_type = {const_pq, const_pq, const_pq, const_pq};

    // 3 voltage regulators (only on buses 2 and 3)
    topo.voltage_regulators_per_load_gen = {from_sparse, {0, 0, 1, 1, 2, 2, 3, 3}};

    MathModelParam<symmetric_t> param;
    constexpr DoubleComplex y{10.0, -20.0};
    param.branch_param = {{y, -y, -y, y}, {y, -y, -y, y}, {y, -y, -y, y}};
    param.shunt_param = {};
    param.source_param = {{.y1 = 1e6, .y0 = 1e6}};

    PowerFlowInput<symmetric_t> input;
    input.source = {1.0};
    input.s_injection = {0.2, 0.3, 0.2, 0.2}; // All generators, balanced
    input.voltage_regulator = {
        {.status = 1, .u_ref = 1.05, .q_min = -1.0, .q_max = 1.0, .generator_id = 0},  // Bus 2
        {.status = 1, .u_ref = 1.05, .q_min = -0.5, .q_max = 0.5, .generator_id = 1},  // Bus 3 gen1
        {.status = 1, .u_ref = 1.05, .q_min = -0.7, .q_max = 0.8, .generator_id = 2}}; // Bus 3 gen2
    input.load_gen_status = {1, 1, 1, 1};

    const YBus<symmetric_t> y_bus{topo, param};
    common::logging::NoLogger log;
    NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
    constexpr bool cache_run = false;

    SUBCASE("Bus type determination") {
        auto output = solver.run_power_flow(y_bus, input, 1e-12, 20, cache_run, log);

        CHECK(output.bus[0].bus_type == BusType::slack);
        CHECK(output.bus[1].bus_type == BusType::pq); // No regulator
        CHECK(output.bus[2].bus_type == BusType::pv); // Single regulator
        CHECK(output.bus[3].bus_type == BusType::pv); // Two regulators
    }

    SUBCASE("Single regulator controls voltage") {
        auto output = solver.run_power_flow(y_bus, input, 1e-12, 20, cache_run, log);

        CHECK(cabs(output.u[2]) == doctest::Approx(1.05).epsilon(0.01));
        CHECK(output.voltage_regulator[0].limit_violated == LimitViolation::none);
    }

    SUBCASE("Two regulators: combined Q limits are aggregated") {
        auto output = solver.run_power_flow(y_bus, input, 1e-12, 20, cache_run, log);

        // Bus 3: combined limits q_min = -0.5-0.7 = -1.2, q_max = 0.5+0.8 = 1.3
        CHECK(output.bus[3].bus_type == BusType::pv);
        CHECK(cabs(output.u[3]) == doctest::Approx(1.05).epsilon(0.01));

        double const total_q = imag(output.load_gen[2].s) + imag(output.load_gen[3].s);
        CHECK(total_q > -1.2);
        CHECK(total_q < 1.3);
        CHECK(output.voltage_regulator[1].limit_violated == LimitViolation::none);
        CHECK(output.voltage_regulator[2].limit_violated == LimitViolation::none);
    }

    SUBCASE("Disabled regulator makes bus PQ") {
        auto input_disabled = input;
        input_disabled.voltage_regulator[0].status = 0;

        auto output = solver.run_power_flow(y_bus, input_disabled, 1e-12, 20, cache_run, log);

        CHECK(output.bus[2].bus_type == BusType::pq);
    }

    SUBCASE("Inactive load_gen ignores its regulator") {
        auto input_inactive = input;
        input_inactive.load_gen_status[1] = 0; // Disable gen on bus 2

        auto output = solver.run_power_flow(y_bus, input_inactive, 1e-12, 20, cache_run, log);

        CHECK(output.bus[2].bus_type == BusType::pq);
    }

    SUBCASE("Partially inactive load_gens: one active keeps bus PV") {
        auto input_partial = input;
        input_partial.load_gen_status[2] = 0; // Disable first gen on bus 3

        auto output = solver.run_power_flow(y_bus, input_partial, 1e-12, 20, cache_run, log);

        // One regulator still active → bus stays PV
        CHECK(output.bus[3].bus_type == BusType::pv);
        CHECK(cabs(output.u[3]) == doctest::Approx(1.05).epsilon(0.01));
    }

    SUBCASE("All load_gens inactive: bus becomes PQ") {
        auto input_all_inactive = input;
        input_all_inactive.load_gen_status[2] = 0; // Disable both gens on bus 3
        input_all_inactive.load_gen_status[3] = 0;

        auto output = solver.run_power_flow(y_bus, input_all_inactive, 1e-12, 20, cache_run, log);

        // No active regulators → bus becomes PQ
        CHECK(output.bus[3].bus_type == BusType::pq);
    }
}

TEST_CASE("Newton-Raphson handling with one-sided Q-limits") {
    using enum LoadGenType;

    /**
     * Network topology:
     * Bus 0 (Slack) -- Bus 1 (two generators with regulators) -- Bus 2 (load)
     */
    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}, {1, 2}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0, 0}};

    // 3 load_gens: 2 generators on bus 1, 1 load on bus 2
    topo.load_gens_per_bus = {from_sparse, {0, 0, 2, 3}};
    topo.load_gen_type = {const_pq, const_pq, const_pq};

    // 2 voltage regulators on the generators
    topo.voltage_regulators_per_load_gen = {from_sparse, {0, 1, 2, 2}};

    MathModelParam<symmetric_t> param;
    constexpr DoubleComplex y{10.0, -20.0};
    param.branch_param = {{y, -y, -y, y}, {y, -y, -y, y}};
    param.shunt_param = {};
    param.source_param = {{.y1 = 1e6, .y0 = 1e6}};

    const YBus<symmetric_t> y_bus{topo, param};
    common::logging::NoLogger log;
    constexpr bool cache_run = false;

    // Test cases with difference combinations of Q-Limits

    SUBCASE("Limited and unlimited regulator") {
        // Step 1: establish base case with finite limits and high load -> pq switch
        PowerFlowInput<symmetric_t> input_finite;
        input_finite.source = {1.0};
        input_finite.s_injection = {0.05, 0.05, DoubleComplex{-0.1, -0.15}}; // 0.15 pu Q demand
        input_finite.voltage_regulator = {
            {.status = 1, .u_ref = 1.05, .q_min = -5.0, .q_max = 0.05, .generator_id = 0},
            {.status = 1, .u_ref = 1.05, .q_min = -5.0, .q_max = 0.05, .generator_id = 1}};
        // Combined: q_max = 0.1 < 0.15 demand at load
        input_finite.load_gen_status = {1, 1, 1};

        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        auto output_finite = solver.run_power_flow(y_bus, input_finite, 1e-12, 20, cache_run, log);

        REQUIRE(output_finite.bus[1].bus_type == BusType::pq);
        REQUIRE(output_finite.voltage_regulator[0].limit_violated != LimitViolation::none);
        REQUIRE(output_finite.voltage_regulator[1].limit_violated != LimitViolation::none);

        const double total_q_finite = imag(output_finite.load_gen[0].s) + imag(output_finite.load_gen[1].s);
        REQUIRE(total_q_finite == 0.1); // Clamped at combined limit

        // Step 2: Make one regulator unlimited, load stays unchanged -> no pq switch or bus violation
        auto input_mixed = input_finite;
        input_mixed.voltage_regulator[0].q_min = nan;
        input_mixed.voltage_regulator[0].q_max = nan;
        // bus limit now NaN

        auto output_mixed = solver.run_power_flow(y_bus, input_mixed, 1e-12, 20, cache_run, log);

        // no pq switching anymore
        CHECK(output_mixed.bus[1].bus_type == BusType::pv); // Bus stays PV
        CHECK(cabs(output_mixed.u[1]) == doctest::Approx(1.05).epsilon(0.01));

        // Regulator 0 (unlimited) should not report violation
        CHECK(output_mixed.voltage_regulator[0].limit_violated == LimitViolation::none);

        // Regulator 1 (limited) still reports violation
        CHECK(output_mixed.voltage_regulator[1].limit_violated == LimitViolation::upper);

        const double total_q_mixed = imag(output_mixed.load_gen[0].s) + imag(output_mixed.load_gen[1].s);
        CHECK(total_q_mixed > 0.1); // Exceeds previous finite combined limit
    }

    SUBCASE("Two one-sided regulators") {
        // Step 1: establish base case with finite limits and high load -> pq switch
        PowerFlowInput<symmetric_t> input_finite;
        input_finite.source = {1.0};
        input_finite.s_injection = {0.05, 0.05, DoubleComplex{-0.1, -0.15}};
        input_finite.voltage_regulator = {
            {.status = 1, .u_ref = 1.05, .q_min = -5.0, .q_max = 0.05, .generator_id = 0},
            {.status = 1, .u_ref = 1.05, .q_min = -5.0, .q_max = 0.05, .generator_id = 1}};
        input_finite.load_gen_status = {1, 1, 1};

        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        auto output_finite = solver.run_power_flow(y_bus, input_finite, 1e-12, 20, cache_run, log);

        REQUIRE(output_finite.bus[1].bus_type == BusType::pq);
        REQUIRE(output_finite.voltage_regulator[0].limit_violated != LimitViolation::none);
        REQUIRE(output_finite.voltage_regulator[1].limit_violated != LimitViolation::none);

        // Step 2: make limits one-sided -> no pq switching
        auto input_onesided = input_finite;
        input_onesided.voltage_regulator[0].q_max = nan; // Remove upper limit
        input_onesided.voltage_regulator[1].q_min = nan; // Remove lower limit
        // Combined bus limit now NaN

        auto output_onesided = solver.run_power_flow(y_bus, input_onesided, 1e-12, 20, cache_run, log);

        // Bus stays PV
        CHECK(output_onesided.bus[1].bus_type == BusType::pv);
        CHECK(cabs(output_onesided.u[1]) == doctest::Approx(1.05).epsilon(0.01));

        // Individual violation for regulator that kept its q_max limit
        CHECK(output_onesided.voltage_regulator[0].limit_violated == LimitViolation::none);
        CHECK(output_onesided.voltage_regulator[1].limit_violated == LimitViolation::upper);

        const double total_q = imag(output_onesided.load_gen[0].s) + imag(output_onesided.load_gen[1].s);
        CHECK(total_q > 0.1); // total Q exceeds previous finite combined limit
    }

    SUBCASE("One-sided regulators extended with finite limits") {
        // Step 1: establish base case with one-sided regulators -> no pq switching
        PowerFlowInput<symmetric_t> input_onesided;
        input_onesided.source = {1.0};
        input_onesided.s_injection = {0.05, 0.05, DoubleComplex{-0.1, -0.15}};
        input_onesided.voltage_regulator = {
            {.status = 1, .u_ref = 1.05, .q_min = -5.0, .q_max = nan, .generator_id = 0},  // One-sided
            {.status = 1, .u_ref = 1.05, .q_min = nan, .q_max = 0.05, .generator_id = 1}}; // One-sided
        input_onesided.load_gen_status = {1, 1, 1};

        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        auto output_onesided = solver.run_power_flow(y_bus, input_onesided, 1e-12, 20, cache_run, log);

        REQUIRE(output_onesided.bus[1].bus_type == BusType::pv);
        REQUIRE(output_onesided.voltage_regulator[0].limit_violated == LimitViolation::none);
        REQUIRE(output_onesided.voltage_regulator[1].limit_violated == LimitViolation::upper);

        // Step 2: add missing limits -> expect pq switching
        auto input_completed = input_onesided;
        input_completed.voltage_regulator[0].q_max = 0.05; // Add missing upper
        input_completed.voltage_regulator[1].q_min = -5.0; // Add missing lower
        // Combined: q_max = 0.1

        auto output_completed = solver.run_power_flow(y_bus, input_completed, 1e-12, 20, cache_run, log);

        CHECK(output_completed.bus[1].bus_type == BusType::pq);
        CHECK(output_completed.voltage_regulator[0].limit_violated == LimitViolation::upper);
        CHECK(output_completed.voltage_regulator[1].limit_violated == LimitViolation::upper);

        const double total_q = imag(output_completed.load_gen[0].s) + imag(output_completed.load_gen[1].s);
        CHECK(cabs(total_q) == doctest::Approx(0.1).epsilon(numerical_tolerance)); // Clamped at combined limit
    }

    SUBCASE("Bus stays PV with unlimited generators and low load") {
        PowerFlowInput<symmetric_t> input;
        input.source = {1.0};
        input.s_injection = {0.05, 0.05, DoubleComplex{-0.1, -0.02}}; // Low Q load
        input.voltage_regulator = {{.status = 1, .u_ref = 1.05, .q_min = nan, .q_max = nan, .generator_id = 0},
                                   {.status = 1, .u_ref = 1.05, .q_min = nan, .q_max = nan, .generator_id = 1}};
        input.load_gen_status = {1, 1, 1};

        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        auto output = solver.run_power_flow(y_bus, input, 1e-12, 20, cache_run, log);

        CHECK(output.bus[1].bus_type == BusType::pv);
        CHECK(cabs(output.u[1]) == doctest::Approx(1.05).epsilon(0.01));
        CHECK(output.voltage_regulator[0].limit_violated == LimitViolation::none);
        CHECK(output.voltage_regulator[1].limit_violated == LimitViolation::none);
    }

    SUBCASE("Bus stays PV with limited generators and low load") {
        PowerFlowInput<symmetric_t> input;
        input.source = {1.0};
        input.s_injection = {0.05, 0.05, DoubleComplex{-0.1, -0.02}}; // Low Q load
        input.voltage_regulator = {{.status = 1, .u_ref = 1.05, .q_min = -5.0, .q_max = 1.0, .generator_id = 0},
                                   {.status = 1, .u_ref = 1.05, .q_min = -5.0, .q_max = 1.0, .generator_id = 1}};
        input.load_gen_status = {1, 1, 1};

        NewtonRaphsonPFSolver<symmetric_t> solver{y_bus, topo};
        auto output = solver.run_power_flow(y_bus, input, 1e-12, 20, cache_run, log);

        CHECK(output.bus[1].bus_type == BusType::pv);
        CHECK(cabs(output.u[1]) == doctest::Approx(1.05).epsilon(0.01));
        CHECK(output.voltage_regulator[0].limit_violated == LimitViolation::none);
        CHECK(output.voltage_regulator[1].limit_violated == LimitViolation::none);

        const double total_q = imag(output.load_gen[0].s) + imag(output.load_gen[1].s);
        CHECK(total_q < 2.0); // Within combined limit
    }
}

} // namespace power_grid_model::math_solver
