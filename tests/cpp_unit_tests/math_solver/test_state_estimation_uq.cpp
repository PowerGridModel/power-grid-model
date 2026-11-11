// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_state_estimation_uq.hpp"

#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>

#include <power_grid_model/common/dummy_logging.hpp>

#include <doctest/doctest.h>

#include <cmath>
#include <initializer_list>
#include <limits>
#include <optional>
#include <vector>

namespace power_grid_model::math_solver {
namespace {
using namespace state_estimation_uq;
using namespace uq_test;
using ILGain = iterative_linear_se::ILSEGainBlock<symmetric_t>;
using ILUnknown = iterative_linear_se::ILSEUnknown<symmetric_t>;
using NRGain = newton_raphson_se::NRSEGainBlock<symmetric_t>;
using NRUnknown = newton_raphson_se::NRSEUnknown<symmetric_t>;

/*
Network:

bus0(v, P=Q=0) --branch0-- bus1(slack)
                             |
                          source0

Series admittance is 1 p.u. there are no shunts or appliances at bus0.
At nonzero voltage, S0=0 implies I0=0 and U0=U1 (no branch voltage drop).
Relative NRSE places the virtual angle at measured bus0, away from slack bus1.
*/
MathModelTopology voltage_measurement_topology() {
    auto topo = topology(2, {{0, 1}}, 1);
    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
    return topo;
}

/*
Network (v: voltage sensor, (p): branch power sensor, [i]: branch current sensor at one of the two sides, or
absent):

 source0                                 source1
    |                                       |
 bus0(v) --(p)[i]-- branch0 --[i]-- bus1(v, slack)

Branch0 runs from bus0 to bus1, with series admittance 1 p.u. and no shunts. Source powers are unmeasured.
*/
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

StateEstimationInput<symmetric_t> voltage_measurement_input(bool absolute, double variance) {
    StateEstimationInput<symmetric_t> input;
    input.source_status = {1};
    input.measured_voltage = {
        {.value = absolute ? DoubleComplex{1.0, 0.0} : DoubleComplex{1.0, nan}, .variance = variance}};
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
// agree with each other. With `absolute` the voltage sensors also measure the angle. The estimators have no
// separate angle variance and use the value given for the magnitude, read as rad^2.
MeasuredBranch measured_branch(bool absolute, std::optional<BranchSide> current_side,
                               AngleMeasurementType current_angle_type) {
    constexpr double scale = 1e-4;
    auto const u = measured_branch_bus_voltages();
    MeasuredBranch result;
    auto& input = result.input;
    input.source_status = {1, 1};
    for (Idx bus = 0; bus != 2; ++bus) {
        double const variance = (bus == 0 ? 0.7 : 1.1) * scale;
        input.measured_voltage.push_back({absolute ? u[bus] : DoubleComplex{std::abs(u[bus]), nan}, variance});
        result.scalars.push_back(
            {[bus](auto& in, double delta) { uq_test::shift_voltage_magnitude(in.measured_voltage[bus].value, delta); },
             variance});
        if (absolute) {
            result.scalars.push_back(
                {[bus](auto& in, double delta) { uq_test::shift_voltage_angle(in.measured_voltage[bus].value, delta); },
                 variance});
        }
    }
    DoubleComplex const current_from = u[0] - u[1];
    DoubleComplex const power_from = u[0] * std::conj(current_from);
    input.measured_branch_from_power = {{.real_component = {.value = power_from.real(), .variance = 0.4 * scale},
                                         .imag_component = {.value = power_from.imag(), .variance = 0.9 * scale}}};
    result.scalars.push_back(
        {[](auto& in, double delta) { in.measured_branch_from_power[0].real_component.value += delta; }, 0.4 * scale});
    result.scalars.push_back(
        {[](auto& in, double delta) { in.measured_branch_from_power[0].imag_component.value += delta; }, 0.9 * scale});
    if (current_side) {
        bool const from = current_side == BranchSide::from;
        DoubleComplex const current = from ? current_from : -current_from;
        // A local angle is measured against the voltage angle at the same side: conj(I) * exp(j * arg(U)).
        DoubleComplex const value = current_angle_type == AngleMeasurementType::local_angle
                                        ? std::conj(current) * std::polar(1.0, std::arg(from ? u[0] : u[1]))
                                        : current;
        CurrentSensorCalcParam<symmetric_t> const sensor{
            .angle_measurement_type = current_angle_type,
            .measurement = {.real_component = {.value = value.real(), .variance = 0.3 * scale},
                            .imag_component = {.value = value.imag(), .variance = 0.8 * scale}}};
        (from ? input.measured_branch_from_current : input.measured_branch_to_current) = {sensor};
        auto const measurement = [from](auto& in) -> auto& {
            return (from ? in.measured_branch_from_current : in.measured_branch_to_current)[0].measurement;
        };
        result.scalars.push_back(
            {[measurement](auto& in, double delta) { measurement(in).real_component.value += delta; }, 0.3 * scale});
        result.scalars.push_back(
            {[measurement](auto& in, double delta) { measurement(in).imag_component.value += delta; }, 0.8 * scale});
    }
    return result;
}

} // namespace

TEST_CASE("UQ covariance and voltage variances from the selective inverse match a dense inverse") {
    // A four-bus network, radial or a ring, with each bus as the slack bus in turn.
    for (bool const mesh : {false, true}) {
        for (bool const relative : {false, true}) {
            for (Idx const slack : IdxRange{four_bus_count}) {
                for (double const alpha : {1e-8, 1.0, 1e8}) {
                    check_against_dense_inverse<ILGain, ILUnknown, IterativeLinearSESolver<symmetric_t>>(
                        mesh, relative, slack, true, alpha);
                    // Only without an angle measurement NRSE has a virtual angle bus, which is the slack bus
                    // or, when that has no voltage sensor, another bus.
                    for (bool const slack_voltage_sensor : relative ? std::vector{false, true} : std::vector{true}) {
                        check_against_dense_inverse<NRGain, NRUnknown, NewtonRaphsonSESolver<symmetric_t>>(
                            mesh, relative, slack, slack_voltage_sensor, alpha);
                    }
                }
            }
        }
    }
}

TEST_CASE("UQ variance status thresholds") {
    // The bound is inclusive: a negative variance exactly at the tolerance is roundoff, the next number below
    // it is not.
    double const tolerance = round_rtol;
    CHECK(finalize_variance(-tolerance, 1.0).raw_status == UQVarianceStatus::clipped_roundoff);
    CHECK(finalize_variance(std::nextafter(-tolerance, -1.0), 1.0).raw_status == UQVarianceStatus::invalid);

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
    // Negative beyond roundoff.
    CHECK(variance({1.0, -2.0}).raw_status == UQVarianceStatus::invalid);
    // A single negative term has nothing to cancel against, however small it is.
    CHECK(variance({-1e-20}).raw_status == UQVarianceStatus::invalid);
    // An exact zero is valid.
    auto const zero = variance({0.0});
    CHECK(zero.raw_status == UQVarianceStatus::valid);
    CHECK(zero.sigma == 0.0);
    // Rounding cancellation at a normal scale is a valid zero.
    auto const cancelled = variance({0x1p53, 1.0, -0x1p53});
    CHECK(cancelled.raw_variance == 0.0);
    CHECK(cancelled.raw_status == UQVarianceStatus::valid);
    // Nonfinite variances.
    auto const not_a_number = variance({nan});
    CHECK(not_a_number.raw_status == UQVarianceStatus::invalid);
    CHECK(std::isnan(not_a_number.sigma));
    CHECK(variance({std::numeric_limits<double>::infinity()}).raw_status == UQVarianceStatus::invalid);
}

TEST_CASE_TEMPLATE("UQ on a two-bus network with an exact zero injection", Solver, IterativeLinearSESolver<symmetric_t>,
                   NewtonRaphsonSESolver<symmetric_t>) {
    constexpr double variance = 1e-4;
    auto const topo = voltage_measurement_topology();
    YBus<symmetric_t> const y_bus{topo, parameters<symmetric_t>(1)};
    Solver solver{y_bus, topo};
    common::logging::NoLogger log;
    // The uncertainty for `request`, after checking that the request does not change the estimate.
    auto const run = [&](bool absolute, StateEstimationUQRequest const& request) {
        auto const input = voltage_measurement_input(absolute, variance);
        auto const ordinary = solver.run_state_estimation(y_bus, input, 1e-12, 30, log);
        auto const [enabled, result] = run_with_uq(solver, y_bus, input, request, 1e-12, 30);
        CHECK(enabled.u == ordinary.u);
        return result;
    };

    SUBCASE("The variances follow from the one voltage sensor") {
        // Bus 0 is tied to the slack bus by the exact zero injection, so its only information is the voltage sensor.
        double const expected = Solver::is_NRSE_solver ? variance : variance / 2.0;
        // Relative tolerance. The expected values are exact, so only rounding separates them from the results.
        constexpr double rtol = 1e-12;
        StateEstimationUQRequest request;
        request.voltage = {{0, true, true}, {1, false, true}};
        for (bool const absolute : {false, true}) {
            CAPTURE(absolute);
            auto const result = run(absolute, request);
            REQUIRE(result.voltage.size() == 2);
            auto const& magnitude = result.voltage[0].magnitude.phase[0];
            auto const& angle = result.voltage[0].angle.phase[0];
            auto const& slack_angle = result.voltage[1].angle.phase[0];
            CHECK(magnitude.raw_status == UQVarianceStatus::valid);
            CHECK(magnitude.raw_variance == RelativeTo{expected, rtol});
            CHECK(magnitude.sigma == RelativeTo{std::sqrt(expected), rtol});
            if (absolute) {
                CHECK(angle.raw_variance == RelativeTo{expected, rtol});
                CHECK(slack_angle.raw_variance == RelativeTo{expected, rtol});
            } else {
                // Both buses have the same angle, so the angle of bus 0 relative to the slack bus cancels to zero.
                CHECK(std::abs(angle.raw_variance) < rtol * variance);
                CHECK(angle.raw_status != UQVarianceStatus::invalid);
                CHECK(slack_angle.raw_variance == 0.0);
            }
        }
    }

    SUBCASE("Result rows follow the request order") {
        // Rows out of order, a repeated row, and a row that selects nothing. Without a measured angle, the
        // angle of the slack bus 1 is the reference and identifies its row.
        StateEstimationUQRequest request;
        request.voltage = {{1, false, true}, {0, true, true}, {0, true, true}, {0, false, false}};
        auto const result = run(false, request);
        REQUIRE(result.voltage.size() == 4);
        CHECK(result.voltage[0].magnitude.phase[0].raw_status == UQVarianceStatus::not_requested);
        CHECK(result.voltage[0].angle.phase[0].raw_status == UQVarianceStatus::valid);
        CHECK(result.voltage[0].angle.phase[0].raw_variance == 0.0);
        CHECK(result.voltage[1].magnitude.phase[0].raw_status == UQVarianceStatus::valid);
        CHECK(result.voltage[1].magnitude.phase[0].raw_variance > 0.0);
        CHECK(result.voltage[2].magnitude.phase[0].raw_variance == result.voltage[1].magnitude.phase[0].raw_variance);
        CHECK(result.voltage[2].angle.phase[0].raw_variance == result.voltage[1].angle.phase[0].raw_variance);
        CHECK(result.voltage[3].magnitude.phase[0].raw_status == UQVarianceStatus::not_requested);
        CHECK(result.voltage[3].angle.phase[0].raw_status == UQVarianceStatus::not_requested);
    }

    SUBCASE("An empty request does no work") {
        // The result that the caller supplied stays empty.
        CHECK(run(true, StateEstimationUQRequest{}).voltage.empty());
    }
}

TEST_CASE_TEMPLATE("UQ voltage variances against the estimator's measurement sensitivities", Solver,
                   IterativeLinearSESolver<symmetric_t>, NewtonRaphsonSESolver<symmetric_t>) {
    struct Case {
        bool absolute;
        std::optional<BranchSide> current_side;
        AngleMeasurementType current_angle_type;
    };
    using enum AngleMeasurementType;
    for (Case const& test_case : {Case{true, BranchSide::from, global_angle}, Case{true, BranchSide::to, global_angle},
                                  Case{true, BranchSide::from, local_angle}, Case{true, BranchSide::to, local_angle},
                                  Case{false, BranchSide::from, local_angle}, Case{false, BranchSide::to, local_angle},
                                  Case{false, std::nullopt, local_angle}}) {
        CAPTURE(test_case.absolute);
        INFO("current sensor: " << (!test_case.current_side                      ? "none"
                                    : test_case.current_side == BranchSide::from ? "from side"
                                                                                 : "to side"));
        INFO("local current angle: " << (test_case.current_angle_type == local_angle));
        auto const topo = measured_branch_topology(test_case.current_side);
        YBus<symmetric_t> const y_bus{topo, parameters<symmetric_t>(1)};
        auto const measured = measured_branch(test_case.absolute, test_case.current_side, test_case.current_angle_type);
        Solver solver{y_bus, topo};
        StateEstimationUQRequest request;
        request.voltage = {{0, true, true}, {1, true, true}};
        auto const [estimate, result] = run_with_uq(solver, y_bus, measured.input, request, 1e-12, 500);
        // The measurements agree with one state, which the estimator therefore returns.
        auto const u = measured_branch_bus_voltages();
        DoubleComplex const reference_rotation = test_case.absolute ? 1.0 : std::polar(1.0, -std::arg(u[1]));
        for (Idx bus = 0; bus != 2; ++bus) {
            CHECK(std::abs(estimate.u[bus] - u[bus] * reference_rotation) < 1e-9);
        }
        auto const reference = uq_test::sensitivity_variance(solver, y_bus, measured.input, measured.scalars);
        REQUIRE(result.voltage.size() == 2);
        for (Idx bus = 0; bus != 2; ++bus) {
            CAPTURE(bus);
            auto const& magnitude = result.voltage[bus].magnitude.phase[0];
            auto const& angle = result.voltage[bus].angle.phase[0];
            CHECK(magnitude.raw_status == UQVarianceStatus::valid);
            CHECK(angle.raw_status == UQVarianceStatus::valid);
            bool const reference_angle = !test_case.absolute && bus == 1;
            if (reference_angle) {
                CHECK(angle.raw_variance == 0.0);
                CHECK(reference.angle(bus, 0) < 1e-20);
            }
            if constexpr (Solver::is_NRSE_solver) {
                CHECK(magnitude.raw_variance == RelativeTo{reference.magnitude(bus, 0), 1e-5});
                if (!reference_angle) {
                    CHECK(angle.raw_variance == RelativeTo{reference.angle(bus, 0), 1e-5});
                }
            } else {
                // ILSE treats each sensor variance as the total variance of a proper complex error. For a
                // voltage sensor that variance belongs to the magnitude alone, so ILSE counts it half. With
                // the voltage sensors of this network, ILSE reports 0.4 to 0.6 of the first-order variance.
                // This will be revised later.
                double const magnitude_ratio = magnitude.raw_variance / reference.magnitude(bus, 0);
                CHECK(magnitude_ratio > 0.4);
                CHECK(magnitude_ratio < 0.6);
                if (!reference_angle) {
                    double const angle_ratio = angle.raw_variance / reference.angle(bus, 0);
                    CHECK(angle_ratio > 0.4);
                    CHECK(angle_ratio < 0.6);
                }
            }
        }
    }
}

TEST_CASE("UQ request bounds") {
    constexpr Idx n_bus = 2;
    StateEstimationUQRequest request;
    StateEstimationUQResult<symmetric_t> result;
    StateEstimationUQContext<symmetric_t> context{request, result};
    // No context and an empty request are accepted.
    CHECK_NOTHROW(validate_context<symmetric_t>(nullptr, n_bus));
    CHECK_NOTHROW(validate_context(&context, n_bus));
    // A bus index below zero or at the bus count is rejected, also when another row is valid.
    for (Idx const bus : {Idx{-1}, n_bus}) {
        request.voltage = {{0, true, true}, {bus, true, false}};
        CHECK_THROWS_AS(validate_context(&context, n_bus), InvalidArguments);
    }
    // Bus indices in range are accepted, also in a row that selects nothing.
    request.voltage = {{0, true, true}, {1, false, false}};
    CHECK_NOTHROW(validate_context(&context, n_bus));

    // The solver rejects an invalid request before it estimates, and the supplied result stays untouched.
    auto const topo = voltage_measurement_topology();
    YBus<symmetric_t> const y_bus{topo, parameters<symmetric_t>(1)};
    NewtonRaphsonSESolver<symmetric_t> solver{y_bus, topo};
    common::logging::NoLogger log;
    request.voltage = {{n_bus, true, true}};
    CHECK_THROWS_AS(solver.run_state_estimation(y_bus, voltage_measurement_input(true, 1e-4), 1e-12, 30, log, &context),
                    InvalidArguments);
    CHECK(result.voltage.empty());
}

TEST_CASE_TEMPLATE("UQ factorization failure keeps the estimate and reports requested uncertainty invalid", Solver,
                   IterativeLinearSESolver<symmetric_t>, NewtonRaphsonSESolver<symmetric_t>) {
    // An unmeasured source at bus 0 and voltage sensor + exact zero injection at bus 1 leaves the
    // bus-0 pivot singular without perturbation. Ordinary SE perturbs the pivot and converges; the
    // selective inverse rejects perturbed ILSE factors, and NRSE's unperturbed UQ factorization fails.
    auto topo = voltage_measurement_topology();
    topo.sources_per_bus = {from_sparse, {0, 1, 1}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 1}};
    YBus<symmetric_t> const y_bus{topo, parameters<symmetric_t>(1)};
    Solver solver{y_bus, topo};
    common::logging::NoLogger log;
    // With relative angles the slack column is solved from the perturbed factors before they are rejected.
    for (bool const absolute : {true, false}) {
        CAPTURE(absolute);
        auto const input = voltage_measurement_input(absolute, 1e-4);
        auto const ordinary = solver.run_state_estimation(y_bus, input, 1e-12, 30, log);
        StateEstimationUQRequest request;
        request.voltage = {{0, true, true}, {1, false, false}};
        auto const [enabled, result] = run_with_uq(solver, y_bus, input, request, 1e-12, 30);
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

} // namespace power_grid_model::math_solver
