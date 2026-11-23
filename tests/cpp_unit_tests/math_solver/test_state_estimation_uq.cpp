// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_state_estimation_uq.hpp"

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/grouped_index_vector.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>
#include <power_grid_model/math_solver/state_estimation_uq.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <doctest/doctest.h>

#include <cmath>
#include <complex>
#include <initializer_list>
#include <memory>
#include <optional>
#include <vector>

TYPE_TO_STRING_AS("IterativeLinearSESolver<symmetric_t>",
                  power_grid_model::math_solver::IterativeLinearSESolver<power_grid_model::symmetric_t>);
TYPE_TO_STRING_AS("NewtonRaphsonSESolver<symmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonSESolver<power_grid_model::symmetric_t>);

namespace power_grid_model::math_solver {
namespace {
using namespace state_estimation_uq;
using namespace uq_test;

constexpr double tight_err_tol = 1e-12;

// Network:
//
// bus0(v, P=Q=0) --branch0-- bus1(slack)
//                              |
//                           source0
//
// The series admittance is 1 p.u., and there are no shunts or appliances at bus0.
// At nonzero voltage, S0 = 0 implies I0 = 0 and U0 = U1 (no branch voltage drop).
// Without a measured angle, NRSE places its virtual angle measurement at the measured bus0, away from the slack bus1.
MathModelTopology voltage_measurement_topology() {
    auto topo = topology(2, {{0, 1}}, 1);
    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
    return topo;
}

// Network (v: voltage sensor, (p): branch power sensor, [i]: branch current sensor at one of the two sides, or
// absent):
//
//  source0                                 source1
//     |                                       |
//  bus0(v) --(p)[i]-- branch0 --[i]-- bus1(v, slack)
//
// Branch0 runs from bus0 to bus1, with series admittance 1 p.u. and no shunts. The source powers are unmeasured.
MathModelTopology measured_branch_topology(std::optional<BranchSide> current_side) {
    auto topo = topology(2, {{0, 1}}, 1);
    topo.sources_per_bus = {from_sparse, {0, 1, 2}};
    topo.power_sensors_per_source = {from_sparse, {0, 0, 0}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 2}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};
    topo.current_sensors_per_branch_from = {from_sparse, {0, current_side == BranchSide::from ? 1 : 0}};
    topo.current_sensors_per_branch_to = {from_sparse, {0, current_side == BranchSide::to ? 1 : 0}};
    return topo;
}

StateEstimationInput<symmetric_t> voltage_measurement_input(AngleReference angle_reference, double variance) {
    StateEstimationInput<symmetric_t> input;
    input.source_status = {1};
    input.measured_voltage = {
        {.value = angle_reference == AngleReference::absolute ? DoubleComplex{1.0, 0.0} : DoubleComplex{1.0, nan},
         .variance = variance}};
    return input;
}

ComplexValueVector<symmetric_t> measured_branch_bus_voltages() {
    return {std::polar(1.03, 0.04), std::polar(0.98, -0.03)};
}

struct MeasuredBranch {
    StateEstimationInput<symmetric_t> input;
    std::vector<MeasuredScalar<symmetric_t>> scalars; // every measured real number with its variance
};

// Measurements for the network above, all computed from the state measured_branch_bus_voltages(), so that they
// agree with each other. With an absolute angle reference, the voltage sensors also measure the angle. The estimators
// have no separate angle variance and use the value given for the magnitude, read as rad^2.
MeasuredBranch measured_branch(AngleReference angle_reference, std::optional<BranchSide> current_side,
                               AngleMeasurementType current_angle_type) {
    constexpr double scale = 1e-4;
    bool const absolute = angle_reference == AngleReference::absolute;
    auto const u = measured_branch_bus_voltages();
    MeasuredBranch result;
    auto& input = result.input;
    input.source_status = {1, 1};
    for (Idx const bus : IdxRange{2}) {
        double const variance = (bus == 0 ? 0.7 : 1.1) * scale;
        input.measured_voltage.emplace_back(absolute ? u[bus] : DoubleComplex{std::abs(u[bus]), nan}, variance);
        result.scalars.emplace_back(
            [bus](auto& in, double delta) {
                auto& measured = in.measured_voltage[bus].value;
                measured = shift_voltage_magnitude(measured, delta);
            },
            variance);
        if (absolute) {
            result.scalars.emplace_back(
                [bus](auto& in, double delta) {
                    auto& measured = in.measured_voltage[bus].value;
                    measured = shift_voltage_angle(measured, delta);
                },
                variance);
        }
    }
    DoubleComplex const current_from = u[0] - u[1];
    DoubleComplex const power_from = u[0] * std::conj(current_from);
    input.measured_branch_from_power = {{.real_component = {.value = power_from.real(), .variance = 0.4 * scale},
                                         .imag_component = {.value = power_from.imag(), .variance = 0.9 * scale}}};
    result.scalars.emplace_back(
        [](auto& in, double delta) { in.measured_branch_from_power[0].real_component.value += delta; }, 0.4 * scale);
    result.scalars.emplace_back(
        [](auto& in, double delta) { in.measured_branch_from_power[0].imag_component.value += delta; }, 0.9 * scale);
    if (current_side) {
        bool const from = current_side == BranchSide::from;
        DoubleComplex const current = from ? current_from : -current_from;
        DoubleComplex const voltage = from ? u[0] : u[1];
        DoubleComplex const value = current_angle_type == AngleMeasurementType::local_angle
                                        ? std::conj(current) * std::polar(1.0, std::arg(voltage))
                                        : current;
        CurrentSensorCalcParam<symmetric_t> const sensor{
            .angle_measurement_type = current_angle_type,
            .measurement = {.real_component = {.value = value.real(), .variance = 0.3 * scale},
                            .imag_component = {.value = value.imag(), .variance = 0.8 * scale}}};
        (from ? input.measured_branch_from_current : input.measured_branch_to_current) = {sensor};
        auto const measurement = [from](auto& in) -> auto& {
            return (from ? in.measured_branch_from_current : in.measured_branch_to_current)[0].measurement;
        };
        result.scalars.emplace_back(
            [measurement](auto& in, double delta) { measurement(in).real_component.value += delta; }, 0.3 * scale);
        result.scalars.emplace_back(
            [measurement](auto& in, double delta) { measurement(in).imag_component.value += delta; }, 0.8 * scale);
    }
    return result;
}
} // namespace

TEST_CASE_TEMPLATE("UQ voltage variances from the selective inverse match a dense inverse", sym, symmetric_t,
                   asymmetric_t) {
    using enum FourBusNetwork;
    using enum AngleReference;
    for (auto const network : {radial, ring}) {
        for (auto const angle_reference : {absolute, relative}) {
            check_network_against_dense_inverse<sym>(network, angle_reference);
        }
    }
}

TEST_CASE("UQ variance status thresholds") {
    // The bound is inclusive: a negative variance exactly at the tolerance is roundoff, the next number below
    // it is not.
    CHECK(finalize_variance(-roundoff_tolerance, 1.0).raw_status == UQVarianceStatus::clipped_roundoff);
    CHECK(finalize_variance(std::nextafter(-roundoff_tolerance, -1.0), 1.0).raw_status == UQVarianceStatus::invalid);

    // The classified sum of `terms`, accumulated as the production code does.
    auto const variance = [](std::initializer_list<double> terms) {
        RealSum sum;
        for (double const term : terms) {
            sum.add(term);
        }
        return finalize_variance(sum);
    };
    // A small negative remainder of a cancellation is clipped to zero, and its signed value is kept.
    auto const clipped = variance({1.0, -1.0 - 1e-14});
    CHECK(clipped.raw_status == UQVarianceStatus::clipped_roundoff);
    CHECK(clipped.sigma == 0.0);
    CHECK(clipped.raw_variance < 0.0);
    // A single negative term has nothing to cancel against, however small it is.
    CHECK(variance({-1e-20}).raw_status == UQVarianceStatus::invalid);
    // An exact zero is valid.
    auto const zero = variance({0.0});
    CHECK(zero.raw_status == UQVarianceStatus::valid);
    CHECK(zero.sigma == 0.0);
    // A nonfinite variance.
    auto const not_a_number = variance({nan});
    CHECK(not_a_number.raw_status == UQVarianceStatus::invalid);
    CHECK(std::isnan(not_a_number.sigma));
}

TEST_CASE_TEMPLATE("UQ on a two-bus network with an exact zero injection", Solver, IterativeLinearSESolver<symmetric_t>,
                   NewtonRaphsonSESolver<symmetric_t>) {
    constexpr double variance = 1e-4;
    constexpr Idx max_iter = 30;
    auto const topo = voltage_measurement_topology();
    YBus<symmetric_t> const y_bus{topo, parameters<symmetric_t>(1)};
    Solver solver{y_bus, topo};
    common::logging::NoLogger log;
    // The uncertainty for `request`, after checking that the request does not change the estimate.
    auto const run = [&solver, &y_bus, &log](AngleReference angle_reference, StateEstimationUQRequest const& request) {
        auto const input = voltage_measurement_input(angle_reference, variance);
        auto const ordinary = solver.run_state_estimation(y_bus, input, tight_err_tol, max_iter, log);
        auto const [enabled, result] = run_with_uq(solver, y_bus, input, request, tight_err_tol, max_iter);
        CHECK(enabled.u == ordinary.u);
        // The uncertainty calculation overwrites the LU factors of the solver, which a later run does not notice.
        CHECK(solver.run_state_estimation(y_bus, input, tight_err_tol, max_iter, log).u == ordinary.u);
        return result;
    };

    SUBCASE("The variances follow from the one voltage sensor") {
        // Bus 0 is tied to the slack bus by the exact zero injection, so its only information is the voltage sensor.
        double const expected = Solver::is_NRSE_solver ? variance : variance / 2.0;
        // Relative tolerance. The expected values are exact, so only rounding separates them from the results.
        constexpr double rtol = 1e-12;
        StateEstimationUQRequest request;
        request.voltage = {{.bus = 0, .magnitude = true, .angle = true}, {.bus = 1, .magnitude = false, .angle = true}};
        for (auto const angle_reference : {AngleReference::relative, AngleReference::absolute}) {
            CAPTURE(angle_reference);
            auto const result = run(angle_reference, request);
            REQUIRE(result.voltage.size() == 2);
            auto const& magnitude = result.voltage[0].magnitude.phase[0];
            auto const& angle = result.voltage[0].angle.phase[0];
            auto const& slack_angle = result.voltage[1].angle.phase[0];
            CHECK(result.voltage[1].magnitude.phase[0].raw_status == UQVarianceStatus::not_requested);
            CHECK(magnitude.raw_status == UQVarianceStatus::valid);
            CHECK(magnitude.raw_variance == relative_to(expected, rtol));
            CHECK(magnitude.sigma == relative_to(std::sqrt(expected), rtol));
            if (angle_reference == AngleReference::absolute) {
                CHECK(angle.raw_variance == relative_to(expected, rtol));
                CHECK(slack_angle.raw_variance == relative_to(expected, rtol));
            } else {
                // Both buses have the same angle, so the angle of bus 0 relative to the slack bus cancels to zero.
                CHECK(std::abs(angle.raw_variance) < rtol * variance);
                CHECK(angle.raw_status != UQVarianceStatus::invalid);
                CHECK(slack_angle.raw_variance == 0.0);
            }
        }
    }

    SUBCASE("An empty request gives an empty result") {
        // Also if the result of the caller still holds the rows of an earlier request.
        auto const input = voltage_measurement_input(AngleReference::absolute, variance);
        auto const ordinary = solver.run_state_estimation(y_bus, input, tight_err_tol, max_iter, log);
        StateEstimationUQRequest const request;
        StateEstimationUQResult<symmetric_t> result;
        result.voltage.resize(1);
        StateEstimationUQContext<symmetric_t> context{request, result};
        auto const enabled = solver.run_state_estimation(y_bus, input, tight_err_tol, max_iter, log, &context);
        CHECK(enabled.u == ordinary.u);
        CHECK(result.voltage.empty());
    }
}

// Only NRSE is compared. ILSE models a voltage measurement as a phasor with one total variance, so it reports about
// half of these variances (see docs/algorithms/state-estimation-uncertainty.md).
TEST_CASE("UQ voltage variances of NRSE against its measurement sensitivities") {
    constexpr double rtol = 1e-5; // relative tolerance: the accuracy of the finite-difference reference
    struct Case {
        AngleReference angle_reference{AngleReference::absolute};
        std::optional<BranchSide> current_side;
        AngleMeasurementType current_angle_type{};
    };
    using enum AngleReference;
    using enum AngleMeasurementType;
    constexpr Idx max_iter = 500;
    for (Case const& test_case :
         {Case{.angle_reference = absolute, .current_side = BranchSide::from, .current_angle_type = global_angle},
          Case{.angle_reference = absolute, .current_side = BranchSide::to, .current_angle_type = global_angle},
          Case{.angle_reference = absolute, .current_side = BranchSide::from, .current_angle_type = local_angle},
          Case{.angle_reference = absolute, .current_side = BranchSide::to, .current_angle_type = local_angle},
          Case{.angle_reference = relative, .current_side = BranchSide::from, .current_angle_type = local_angle},
          Case{.angle_reference = relative, .current_side = BranchSide::to, .current_angle_type = local_angle},
          Case{.angle_reference = relative, .current_side = std::nullopt, .current_angle_type = local_angle}}) {
        CAPTURE(test_case.angle_reference);
        INFO("current sensor: " << (!test_case.current_side                      ? "none"
                                    : test_case.current_side == BranchSide::from ? "from side"
                                                                                 : "to side"));
        INFO("local current angle: " << (test_case.current_angle_type == local_angle));
        auto const topo = measured_branch_topology(test_case.current_side);
        YBus<symmetric_t> const y_bus{topo, parameters<symmetric_t>(1)};
        auto const measured =
            measured_branch(test_case.angle_reference, test_case.current_side, test_case.current_angle_type);
        NewtonRaphsonSESolver<symmetric_t> solver{y_bus, topo};
        auto const request = all_voltages_request(2);
        auto const [estimate, result] = run_with_uq(solver, y_bus, measured.input, request, tight_err_tol, max_iter);
        // The measurements agree with one state, which the estimator therefore returns.
        auto const u = measured_branch_bus_voltages();
        DoubleComplex const reference_rotation =
            test_case.angle_reference == absolute ? 1.0 : std::polar(1.0, -std::arg(u[1]));
        for (Idx const bus : IdxRange{2}) {
            CHECK(std::abs(estimate.u[bus] - u[bus] * reference_rotation) < 1e-9);
        }
        auto const reference = sensitivity_variance(solver, y_bus, measured.input, measured.scalars);
        REQUIRE(result.voltage.size() == 2);
        for (Idx const bus : IdxRange{2}) {
            CAPTURE(bus);
            auto const& magnitude = result.voltage[bus].magnitude.phase[0];
            auto const& angle = result.voltage[bus].angle.phase[0];
            CHECK(magnitude.raw_status == UQVarianceStatus::valid);
            CHECK(angle.raw_status == UQVarianceStatus::valid);
            CHECK(magnitude.raw_variance == relative_to(reference.magnitude(bus, 0), rtol));
            bool const reference_angle = test_case.angle_reference == relative && bus == 1;
            if (reference_angle) {
                CHECK(angle.raw_variance == 0.0);
                CHECK(reference.angle(bus, 0) < 1e-20);
            } else {
                CHECK(angle.raw_variance == relative_to(reference.angle(bus, 0), rtol));
            }
        }
    }
}

TEST_CASE_TEMPLATE("UQ factorization failure keeps the estimate and reports requested uncertainty invalid", Solver,
                   IterativeLinearSESolver<symmetric_t>, NewtonRaphsonSESolver<symmetric_t>) {
    // An unmeasured source at bus 0 and voltage sensor + exact zero injection at bus 1 leaves the
    // bus-0 pivot singular without perturbation. Ordinary SE perturbs the pivot and converges. The uncertainty
    // needs LU factors without perturbation: ILSE finds its factors perturbed, and the factorization without
    // perturbation that NRSE does for the uncertainty fails.
    constexpr Idx max_iter = 30;
    auto topo = voltage_measurement_topology();
    topo.sources_per_bus = {from_sparse, {0, 1, 1}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 1}};
    YBus<symmetric_t> const y_bus{topo, parameters<symmetric_t>(1)};
    Solver solver{y_bus, topo};
    common::logging::NoLogger log;
    for (auto const angle_reference : {AngleReference::absolute, AngleReference::relative}) {
        CAPTURE(angle_reference);
        auto const input = voltage_measurement_input(angle_reference, 1e-4);
        auto const ordinary = solver.run_state_estimation(y_bus, input, tight_err_tol, max_iter, log);
        StateEstimationUQRequest request;
        request.voltage = {{.bus = 0, .magnitude = true, .angle = true},
                           {.bus = 1, .magnitude = false, .angle = false}};
        auto const [enabled, result] = run_with_uq(solver, y_bus, input, request, tight_err_tol, max_iter);
        CHECK(enabled.u == ordinary.u);
        REQUIRE(result.voltage.size() == 2);
        for (auto const* value : {&result.voltage[0].magnitude.phase[0], &result.voltage[0].angle.phase[0]}) {
            CHECK(value->raw_status == UQVarianceStatus::invalid);
            CHECK(std::isnan(value->sigma));
            CHECK(std::isnan(value->raw_variance));
        }
        CHECK(result.voltage[1].magnitude.phase[0].raw_status == UQVarianceStatus::not_requested);
        CHECK(result.voltage[1].angle.phase[0].raw_status == UQVarianceStatus::not_requested);
    }
}

TEST_CASE("UQ request is forwarded by the math solver to the estimator of the calculation method") {
    // Bus 0 only has its voltage sensor, of which NRSE reports the variance and ILSE half of it.
    constexpr double variance = 1e-4;
    constexpr Idx max_iter = 30;
    using enum CalculationMethod;
    auto const topo = std::make_shared<MathModelTopology const>(voltage_measurement_topology());
    YBus<symmetric_t> const y_bus{*topo, parameters<symmetric_t>(1)};
    auto const input = voltage_measurement_input(AngleReference::absolute, variance);
    StateEstimationUQRequest const request{.voltage = {{.bus = 0, .magnitude = true, .angle = false}}};
    common::logging::NoLogger log;
    for (auto const method : {default_method, iterative_linear, newton_raphson}) {
        CAPTURE(static_cast<int>(method));
        MathSolver<symmetric_t> math_solver{topo};
        StateEstimationUQResult<symmetric_t> result;
        StateEstimationUQContext<symmetric_t> context{request, result};
        math_solver.run_state_estimation(input, tight_err_tol, max_iter, log, method, y_bus, &context);
        REQUIRE(result.voltage.size() == 1);
        double const expected = method == newton_raphson ? variance : variance / 2.0;
        CHECK(result.voltage[0].magnitude.phase[0].raw_variance == relative_to(expected, 1e-12));
        CHECK(result.voltage[0].angle.phase[0].raw_status == UQVarianceStatus::not_requested);
    }
}

} // namespace power_grid_model::math_solver
