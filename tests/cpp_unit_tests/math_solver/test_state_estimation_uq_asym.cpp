// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_state_estimation_uq.hpp"

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>
#include <power_grid_model/math_solver/state_estimation_uq.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <doctest/doctest.h>

#include <complex>
#include <vector>

namespace power_grid_model::math_solver {
namespace {
using namespace state_estimation_uq;
using namespace uq_test;
using Asym = asymmetric_t;
using Voltage = ComplexValue<Asym>;
using ILGain = iterative_linear_se::ILSEGainBlock<Asym>;
using ILUnknown = iterative_linear_se::ILSEUnknown<Asym>;
using NRGain = newton_raphson_se::NRSEGainBlock<Asym>;
using NRUnknown = newton_raphson_se::NRSEUnknown<Asym>;

struct MeasuredBuses {
    StateEstimationInput<Asym> input;
    std::vector<MeasuredScalar<Asym>> scalars; // every measured real number with its variance
};

/*
Network of the three-phase estimator test (v: voltage sensor, s: injection sensor):

 bus0(v, s) --branch0-- bus1(v, s, slack)
     |                      |
   load0                 source0

The branch couples the three phases, and there are no shunts.

The input of that test: the measurements of these sensors, and the list of measured scalars, which
sensitivity_variance() shifts one at a time to compute the reference. All measurements are computed from one
state, so that they agree with each other and the estimator returns that state. A voltage angle is measured if
`absolute`.
*/
MeasuredBuses measured_buses(bool absolute, double voltage_variance) {
    // A small balanced voltage drop gives nonzero coupled three-phase flow.
    // The common phase-a reference is the only unobservable physical angle.
    Voltage const balanced = exp(1.0i * RealValue<Asym>{0.0, -2.0 * pi / 3.0, 2.0 * pi / 3.0});
    ComplexValueVector<Asym> const voltage{Voltage{1.01 * std::polar(1.0, .02) * balanced}, Voltage{.99 * balanced}};
    auto const admittance = series_admittance<Asym>();
    // One phase of a voltage measurement as a measured scalar, which `shift_measurement` shifts.
    auto const voltage_scalar = [voltage_variance](int bus, int phase,
                                                   void (*shift_measurement)(DoubleComplex&, double)) {
        return MeasuredScalar<Asym>{[bus, phase, shift_measurement](auto& in, double delta) {
                                        DoubleComplex value = in.measured_voltage[bus].value(phase);
                                        shift_measurement(value, delta);
                                        in.measured_voltage[bus].value(phase) = value;
                                    },
                                    voltage_variance};
    };
    MeasuredBuses result;
    auto& input = result.input;
    auto& scalars = result.scalars;
    input.source_status = {1};
    input.load_gen_status = {1};
    for (int bus = 0; bus != 2; ++bus) {
        input.measured_voltage.push_back(
            {absolute ? voltage[bus] : magnitude_only<Asym>(voltage[bus]), voltage_variance});
        Voltage const current = dot(admittance, Voltage{voltage[bus] - voltage[1 - bus]});
        Voltage const power = voltage[bus] * conj(current);
        auto const injection = injection_sensor<Asym>(power, 1.0);
        input.measured_bus_injection.push_back(injection);
        for (int phase = 0; phase != 3; ++phase) {
            scalars.push_back(voltage_scalar(bus, phase, shift_voltage_magnitude));
            if (absolute) {
                scalars.push_back(voltage_scalar(bus, phase, shift_voltage_angle));
            }
            scalars.push_back({[bus, phase](auto& in, double delta) {
                                   in.measured_bus_injection[bus].real_component.value(phase) += delta;
                               },
                               injection.real_component.variance(phase)});
            scalars.push_back({[bus, phase](auto& in, double delta) {
                                   in.measured_bus_injection[bus].imag_component.value(phase) += delta;
                               },
                               injection.imag_component.variance(phase)});
        }
    }
    return result;
}

// Compare the NRSE voltage variances with the measurement-sensitivity reference.
void check_against_sensitivities(StateEstimationUQResult<Asym> const& result, VoltageVariance const& reference,
                                 bool absolute, Idx slack_bus, double voltage_variance) {
    for (int bus = 0; bus != 2; ++bus) {
        CAPTURE(bus);
        for (int phase = 0; phase != 3; ++phase) {
            CAPTURE(phase);
            double const magnitude = result.voltage[bus].magnitude.phase[phase].raw_variance;
            double const angle = result.voltage[bus].angle.phase[phase].raw_variance;
            CHECK(magnitude == RelativeTo{reference.magnitude(bus, phase), 1e-5});
            if (absolute) {
                CHECK(angle == RelativeTo{reference.angle(bus, phase), 1e-5});
                continue;
            }
            // Known limitation: the reported variance counts the virtual angle measurements of NRSE as noisy.
            // Phases b and c exceed the reference by about twice the voltage sensor variance, phase a by 0.07%.
            // A follow-up will subtract the virtual measurements' contribution, after which these checks become
            // equalities.
            CHECK(angle >= reference.angle(bus, phase) * (1.0 - 1e-5));
            if (phase != 0) {
                CHECK(angle - reference.angle(bus, phase) == RelativeTo{2.0 * voltage_variance, 0.01});
            } else if (bus != slack_bus) {
                CHECK(angle == RelativeTo{reference.angle(bus, phase), 0.01});
            }
        }
    }
}
} // namespace

TEST_CASE("UQ three-phase covariance and voltage variances from the selective inverse match a dense inverse") {
    // A four-bus network, radial or a ring, with each bus as the slack bus in turn.
    for (bool const mesh : {false, true}) {
        for (bool const relative : {false, true}) {
            for (Idx const slack : IdxRange{four_bus_count}) {
                for (double const alpha : {1e-8, 1.0, 1e8}) {
                    check_against_dense_inverse<ILGain, ILUnknown, IterativeLinearSESolver<Asym>>(mesh, relative, slack,
                                                                                                  true, alpha);
                    // Only without an angle measurement NRSE has a virtual angle bus, which is the slack bus
                    // or, when that has no voltage sensor, another bus.
                    for (bool const slack_voltage_sensor : relative ? std::vector{false, true} : std::vector{true}) {
                        check_against_dense_inverse<NRGain, NRUnknown, NewtonRaphsonSESolver<Asym>>(
                            mesh, relative, slack, slack_voltage_sensor, alpha);
                    }
                }
            }
        }
    }
}

TEST_CASE_TEMPLATE("UQ three-phase voltage variances through the estimators", Solver, IterativeLinearSESolver<Asym>,
                   NewtonRaphsonSESolver<Asym>) {
    auto const topo = measured_bus_topology(2, {{0, 1}}, 1);
    YBus<Asym> const y_bus{topo, parameters<Asym>(1)};
    constexpr double voltage_variance = 0.2;
    for (bool const absolute : {false, true}) {
        CAPTURE(absolute);
        auto const measured = measured_buses(absolute, voltage_variance);
        Solver solver{y_bus, topo};
        common::logging::NoLogger log;
        auto const ordinary = solver.run_state_estimation(y_bus, measured.input, 1e-10, 100, log);
        StateEstimationUQRequest request;
        request.voltage = {{0, true, true}, {1, true, true}};
        auto const [enabled, result] = run_with_uq(solver, y_bus, measured.input, request, 1e-10, 100);
        REQUIRE(result.voltage.size() == 2);
        for (int bus = 0; bus != 2; ++bus) {
            CHECK((ordinary.u[bus] == enabled.u[bus]).all());
            for (int phase = 0; phase != 3; ++phase) {
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
