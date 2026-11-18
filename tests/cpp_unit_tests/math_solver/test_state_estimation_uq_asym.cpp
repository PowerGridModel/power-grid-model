// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_state_estimation_uq.hpp"

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>
#include <power_grid_model/math_solver/state_estimation_uq.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <doctest/doctest.h>

#include <complex>
#include <iterator>
#include <vector>

TYPE_TO_STRING_AS("IterativeLinearSESolver<asymmetric_t>",
                  power_grid_model::math_solver::IterativeLinearSESolver<power_grid_model::asymmetric_t>);
TYPE_TO_STRING_AS("NewtonRaphsonSESolver<asymmetric_t>",
                  power_grid_model::math_solver::NewtonRaphsonSESolver<power_grid_model::asymmetric_t>);

namespace power_grid_model::math_solver {
namespace {
using namespace state_estimation_uq;
using namespace uq_test;
using Asym = asymmetric_t;
using Voltage = ComplexValue<Asym>;

constexpr Idx n_bus = 2;

struct MeasuredBuses {
    StateEstimationInput<Asym> input;
    std::vector<MeasuredScalar<Asym>> scalars; // every measured real number with its variance
};

// Network of the three-phase estimator test (v: voltage sensor, s: injection sensor):
//
//  bus0(v, s) --branch0-- bus1(v, s, slack)
//      |                      |
//    load0                 source0
//
// The branch couples the three phases, and there are no shunts.
//
// The input of that test: the measurements of these sensors, and the list of measured scalars, which
// sensitivity_variance() shifts one at a time to compute the reference. All measurements are computed from one
// state, so that they agree with each other and the estimator returns that state. A voltage angle is measured if
// `absolute`.
MeasuredBuses measured_buses(bool absolute, double voltage_variance) {
    // A small balanced voltage drop gives nonzero coupled three-phase flow.
    // The common phase-a reference is the only unobservable physical angle.
    Voltage const balanced = exp(1.0i * RealValue<Asym>{0.0, -deg_120, deg_120});
    ComplexValueVector<Asym> const voltage{Voltage{1.01 * std::polar(1.0, 0.02) * balanced}, Voltage{0.99 * balanced}};
    auto const admittance = series_admittance<Asym>();
    // One phase of a voltage measurement as a measured scalar, which `shift_measurement` shifts.
    auto const voltage_scalar = [voltage_variance](Idx bus, Idx phase, auto shift_measurement) {
        return MeasuredScalar<Asym>{[bus, phase, shift_measurement](auto& in, double delta) {
                                        DoubleComplex const measured = in.measured_voltage[bus].value(phase);
                                        in.measured_voltage[bus].value(phase) = shift_measurement(measured, delta);
                                    },
                                    voltage_variance};
    };
    MeasuredBuses result;
    auto& input = result.input;
    auto& scalars = result.scalars;
    input.source_status = {1};
    input.load_gen_status = {1};
    for (Idx const bus : IdxRange{n_bus}) {
        input.measured_voltage.emplace_back(absolute ? voltage[bus] : magnitude_only<Asym>(voltage[bus]),
                                            voltage_variance);
        Voltage const current = dot(admittance, Voltage{voltage[bus] - voltage[1 - bus]});
        Voltage const power = voltage[bus] * conj(current);
        auto const injection = injection_sensor<Asym>(power, 1.0);
        input.measured_bus_injection.push_back(injection);
        for (Idx const phase : IdxRange{n_phase<Asym>}) {
            scalars.push_back(voltage_scalar(bus, phase, shift_voltage_magnitude));
            if (absolute) {
                scalars.push_back(voltage_scalar(bus, phase, shift_voltage_angle));
            }
            scalars.emplace_back(
                [bus, phase](auto& in, double delta) {
                    in.measured_bus_injection[bus].real_component.value(phase) += delta;
                },
                injection.real_component.variance(phase));
            scalars.emplace_back(
                [bus, phase](auto& in, double delta) {
                    in.measured_bus_injection[bus].imag_component.value(phase) += delta;
                },
                injection.imag_component.variance(phase));
        }
    }
    return result;
}

// Compare the NRSE voltage variances with the measurement-sensitivity reference.
void check_against_sensitivities(StateEstimationUQResult<Asym> const& result, VoltageVariance const& reference,
                                 bool absolute, Idx slack_bus, double voltage_variance) {
    for (Idx const bus : IdxRange{n_bus}) {
        CAPTURE(bus);
        for (Idx const phase : IdxRange{n_phase<Asym>}) {
            CAPTURE(phase);
            double const magnitude = result.voltage[bus].magnitude.phase[phase].raw_variance;
            double const angle = result.voltage[bus].angle.phase[phase].raw_variance;
            CHECK(magnitude == relative_to(reference.magnitude(bus, phase), 1e-5));
            if (absolute) {
                CHECK(angle == relative_to(reference.angle(bus, phase), 1e-5));
                continue;
            }
            // Known limitation: the reported variance counts the virtual angle measurements of NRSE as noisy.
            // Phases b and c exceed the reference by about twice the voltage sensor variance, phase a by 0.07%.
            // TODO(Plusero): these checks become equalities once the contribution of the virtual measurements is
            // subtracted.
            CHECK(angle >= reference.angle(bus, phase) * (1.0 - 1e-5));
            if (phase != 0) {
                CHECK(angle - reference.angle(bus, phase) == relative_to(2.0 * voltage_variance, 0.01));
            } else if (bus != slack_bus) {
                CHECK(angle == relative_to(reference.angle(bus, phase), 0.01));
            }
        }
    }
}
} // namespace

TEST_CASE_TEMPLATE_INVOKE(test_uq_dense_inverse_id, asymmetric_t);

TEST_CASE_TEMPLATE("UQ three-phase voltage variances through the estimators", Solver, IterativeLinearSESolver<Asym>,
                   NewtonRaphsonSESolver<Asym>) {
    constexpr double voltage_variance = 0.2;
    constexpr double err_tol = 1e-10;
    constexpr Idx max_iter = 100;
    auto const topo = measured_bus_topology(n_bus, {{0, 1}}, 1);
    YBus<Asym> const y_bus{topo, parameters<Asym>(1)};
    for (bool const absolute : {false, true}) {
        CAPTURE(absolute);
        auto const measured = measured_buses(absolute, voltage_variance);
        Solver solver{y_bus, topo};
        common::logging::NoLogger log;
        auto const ordinary = solver.run_state_estimation(y_bus, measured.input, err_tol, max_iter, log);
        auto const request = all_voltages_request(n_bus);
        auto const [enabled, result] = run_with_uq(solver, y_bus, measured.input, request, err_tol, max_iter);
        REQUIRE(std::ssize(result.voltage) == n_bus);
        for (Idx const bus : IdxRange{n_bus}) {
            CHECK((ordinary.u[bus] == enabled.u[bus]).all());
            for (Idx const phase : IdxRange{n_phase<Asym>}) {
                CHECK(result.voltage[bus].magnitude.phase[phase].raw_status == UQVarianceStatus::valid);
                CHECK(result.voltage[bus].angle.phase[phase].raw_status == UQVarianceStatus::valid);
            }
        }
        if (!absolute) {
            // Only phase a of the slack bus is the reference; its other phases keep an uncertainty.
            CHECK(result.voltage[1].angle.phase[0].sigma == 0.0);
            CHECK(result.voltage[1].angle.phase[1].sigma > 0.0);
            CHECK(result.voltage[1].angle.phase[2].sigma > 0.0);
        }
        if constexpr (Solver::is_NRSE_solver) {
            auto const reference = sensitivity_variance(solver, y_bus, measured.input, measured.scalars);
            check_against_sensitivities(result, reference, absolute, topo.slack_bus, voltage_variance);
        }
    }
}

} // namespace power_grid_model::math_solver
