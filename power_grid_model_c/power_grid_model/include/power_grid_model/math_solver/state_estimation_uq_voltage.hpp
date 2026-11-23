// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// Voltage magnitude and angle uncertainty of the state estimation output of one island, and the entry point that
// ILSE and NRSE call after they have converged.

#include "measured_values.hpp"
#include "sparse_lu_solver.hpp"
#include "state_estimation_uq.hpp"
#include "state_estimation_uq_workspace.hpp"
#include "y_bus.hpp"

#include "../common/common.hpp"
#include "../common/counting_iterator.hpp"
#include "../common/three_phase_tensor.hpp"

#include <algorithm>
#include <cassert>
#include <complex>
#include <iterator>
#include <vector>

namespace power_grid_model::math_solver::state_estimation_uq {

// The functions below take a SparseCovarianceWorkspace.
// For ILSE its state is the complex voltage U with proper complex covariance V, from which the angle and magnitude
// variances are derived.
// For NRSE its state is the voltage angle and magnitude, so their variances are read directly.

// Variance Var(theta_i) of one voltage angle, before it is made relative to the slack bus.
template <class Workspace> double absolute_angle_variance(Workspace const& workspace, Idx bus, Idx phase) {
    if constexpr (Workspace::uses_complex_state) {
        // With a complex state, the angle variance is Re(V_ii) / (2 |U_i|^2).
        return workspace.state_variance(bus, phase) / (2.0 * std::norm(workspace.phase_voltage(bus, phase)));
    } else {
        return workspace.state_variance(bus, phase);
    }
}

// Covariance Cov(theta_i, theta_s) of one voltage angle with the angle of the reference phase of the slack bus.
template <class Workspace> double angle_covariance_with_slack(Workspace const& workspace, Idx bus, Idx phase) {
    if constexpr (Workspace::uses_complex_state) {
        // With a complex state, the angle covariance is Re(V_is / (U_i conj(U_s))) / 2.
        DoubleComplex const voltage = workspace.phase_voltage(bus, phase);
        DoubleComplex const slack_voltage = workspace.phase_voltage(workspace.slack_bus(), Workspace::reference_phase);
        return (workspace.slack_covariance(bus, phase) / (voltage * std::conj(slack_voltage))).real() / 2.0;
    } else {
        return workspace.slack_covariance(bus, phase);
    }
}

// Uncertainty of one voltage angle as state estimation reports it.
// Without a voltage angle measurement, state estimation reports every angle relative to the angle theta_s of the
// reference phase of the slack bus. The variance of such a relative angle is
// Var(theta_i - theta_s) = Var(theta_i) + Var(theta_s) - 2 Cov(theta_i, theta_s).
// The reference angle's variance is exactly zero.
template <class Workspace> UQPhaseValue angle_uncertainty(Workspace const& workspace, Idx bus, Idx phase) {
    Idx const slack_bus = workspace.slack_bus();
    RealSum sum;
    if (!workspace.report_relative_angle()) {
        sum.add(absolute_angle_variance(workspace, bus, phase));
    } else if (bus != slack_bus || phase != Workspace::reference_phase) {
        sum.add(absolute_angle_variance(workspace, bus, phase));                            // Var(theta_i)
        sum.add(absolute_angle_variance(workspace, slack_bus, Workspace::reference_phase)); // Var(theta_s)
        sum.add(-2.0 * angle_covariance_with_slack(workspace, bus, phase));                 // -2 Cov(theta_i, theta_s)
    }
    return finalize_variance(sum);
}

// Uncertainty of one voltage magnitude, which does not depend on the angle reference.
template <class Workspace> UQPhaseValue magnitude_uncertainty(Workspace const& workspace, Idx bus, Idx phase) {
    RealSum sum;
    if constexpr (Workspace::uses_complex_state) {
        // With a complex state, the magnitude variance is Re(V_ii) / 2.
        sum.add(workspace.state_variance(bus, phase) / 2.0);
    } else {
        sum.add(workspace.state_variance(bus, phase + Workspace::phase_count));
    }
    return finalize_variance(sum);
}

// The requested voltage uncertainties of one bus.
template <class Workspace>
VoltageUQResult<typename Workspace::sym> voltage_uncertainty(Workspace const& workspace,
                                                             VoltageUQRequest const& request) {
    assert(request.bus >= 0);
    assert(request.bus < workspace.n_bus());

    VoltageUQResult<typename Workspace::sym> result;
    for (Idx const phase : IdxRange{Workspace::phase_count}) {
        if (request.magnitude) {
            result.magnitude.phase[phase] = magnitude_uncertainty(workspace, request.bus, phase);
        }
        if (request.angle) {
            result.angle.phase[phase] = angle_uncertainty(workspace, request.bus, phase);
        }
    }
    return result;
}

// The voltage uncertainties of all requested buses. The result rows follow the order of the request.
template <class Workspace>
StateEstimationUQResult<typename Workspace::sym> calculate_voltage_uq_results(Workspace const& workspace,
                                                                              StateEstimationUQRequest const& request) {
    StateEstimationUQResult<typename Workspace::sym> result;
    result.voltage.reserve(request.voltage.size());
    std::ranges::transform(
        request.voltage, std::back_inserter(result.voltage),
        [&workspace](VoltageUQRequest const& bus_request) { return voltage_uncertainty(workspace, bus_request); });
    return result;
}

// The voltage uncertainties of one bus if they cannot be calculated.
// Every requested quantity is invalid. Quantities that were not requested stay not_requested.
template <symmetry_tag sym> VoltageUQResult<sym> invalid_voltage_uncertainty(VoltageUQRequest const& request) {
    static constexpr UQPhaseValue invalid{
        .raw_variance = nan, .absolute_scale = nan, .sigma = nan, .raw_status = UQVarianceStatus::invalid};

    VoltageUQResult<sym> result;
    if (request.magnitude) {
        result.magnitude.phase.fill(invalid);
    }
    if (request.angle) {
        result.angle.phase.fill(invalid);
    }
    return result;
}

// The results of an island for which the uncertainty cannot be calculated, because the LU factorization that it
// needs is not available. The estimate itself is unaffected.
template <symmetry_tag sym> StateEstimationUQResult<sym> invalid_results(StateEstimationUQRequest const& request) {
    StateEstimationUQResult<sym> result;
    result.voltage.reserve(request.voltage.size());
    std::ranges::transform(request.voltage, std::back_inserter(result.voltage), invalid_voltage_uncertainty<sym>);
    return result;
}

// The requested uncertainties of one island at the converged estimate `u`.
// `solver`, `lu_factors` and `block_perm` are the LU factorization of the augmented matrix at that estimate, as the
// constructor of SparseCovarianceWorkspace requires it. `lu_factors` is overwritten.
template <symmetry_tag sym, class Gain, class Unknown>
StateEstimationUQResult<sym>
calculate_results(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_values, ComplexValueVector<sym> const& u,
                  StateEstimationUQRequest const& request, SparseLUSolver<Gain, Unknown, Unknown>& solver,
                  std::vector<Gain>& lu_factors,
                  typename SparseLUSolver<Gain, Unknown, Unknown>::BlockPermArray const& block_perm) {
    SparseCovarianceWorkspace<Gain, Unknown> const workspace{y_bus, measured_values, u, solver, lu_factors, block_perm};
    return calculate_voltage_uq_results(workspace, request);
}

} // namespace power_grid_model::math_solver::state_estimation_uq
