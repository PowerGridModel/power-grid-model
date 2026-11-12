// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// Shared fixtures and references for the state-estimation uncertainty tests, for both symmetries:
// - small networks with the sensor layout the tests need,
// - the augmented gain matrix written out from its definition, compared through a dense inverse,
// - the first-order variance of the estimate from the estimator's own sensitivity to each measurement.

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/measured_values.hpp>
#include <power_grid_model/math_solver/observability.hpp>
#include <power_grid_model/math_solver/state_estimation_uq.hpp>
#include <power_grid_model/math_solver/state_estimation_uq_voltage.hpp>
#include <power_grid_model/math_solver/state_estimation_uq_workspace.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <Eigen/Core>
#include <Eigen/LU>
#include <doctest/doctest.h>

#include <array>
#include <cmath>
#include <complex>
#include <functional>
#include <numbers>
#include <ostream>
#include <vector>

namespace power_grid_model::math_solver::uq_test {

// Right-hand side of a comparison with a purely relative tolerance.
// doctest::Approx adds an absolute term of the size of its epsilon, which hides any error in a variance far below one.
struct RelativeTo {
    double expected;
    double tolerance;

    friend bool operator==(double value, RelativeTo const& rhs) {
        return std::abs(value - rhs.expected) <= rhs.tolerance * std::abs(rhs.expected);
    }
    friend std::ostream& operator<<(std::ostream& os, RelativeTo const& rhs) {
        return os << rhs.expected << " (relative tolerance " << rhs.tolerance << ")";
    }
};

// ----- Values per phase -----

template <symmetry_tag sym> inline constexpr int n_phase = is_symmetric_v<sym> ? 1 : 3;

inline double phase_value(double x, int /*phase*/) { return x; }
inline double phase_value(RealValue<asymmetric_t> const& x, int phase) { return x(phase); }
inline DoubleComplex phase_value(DoubleComplex const& x, int /*phase*/) { return x; }
inline DoubleComplex phase_value(ComplexValue<asymmetric_t> const& x, int phase) { return x(phase); }
inline DoubleComplex tensor_entry(DoubleComplex const& y, int /*row*/, int /*col*/) { return y; }
inline DoubleComplex tensor_entry(ComplexTensor<asymmetric_t> const& y, int row, int col) { return y(row, col); }

// Turns one number per phase (a, b, c) into a per-phase value.
// Only the first for a symmetric calculation
// All three for an asymmetric one.
template <symmetry_tag sym> RealValue<sym> per_phase(std::array<double, 3> const& values) {
    if constexpr (is_symmetric_v<sym>) {
        return values[0];
    } else {
        return {values[0], values[1], values[2]};
    }
}

// A voltage measurement without angle has a NaN imaginary part.
template <symmetry_tag sym> ComplexValue<sym> magnitude_only(ComplexValue<sym> const& u) {
    if constexpr (is_symmetric_v<sym>) {
        return {std::abs(u), nan};
    } else {
        return {DoubleComplex{std::abs(u(0)), nan}, DoubleComplex{std::abs(u(1)), nan},
                DoubleComplex{std::abs(u(2)), nan}};
    }
}

// ----- Networks -----

// `n` buses with one source at the slack bus `slack`, and no sensors.
inline MathModelTopology topology(Idx n, std::vector<BranchIdx> branches, Idx slack) {
    MathModelTopology topo;
    topo.slack_bus = slack;
    topo.is_radial = std::ssize(branches) == n - 1;
    topo.phase_shift.resize(n, 0.0);
    topo.branch_bus_idx = std::move(branches);
    // YBus expects the fill-ins of the bus elimination from the topology engine, which the tests bypass.
    // They are computed here by eliminating the buses in order.
    std::vector<std::vector<bool>> adjacency(n, std::vector<bool>(n, false));
    for (auto const& edge : topo.branch_bus_idx) {
        adjacency[edge[0]][edge[1]] = adjacency[edge[1]][edge[0]] = true;
    }
    for (Idx pivot = 0; pivot < n; ++pivot) {
        for (Idx first = pivot + 1; first < n; ++first) {
            for (Idx second = first + 1; second < n; ++second) {
                if (adjacency[pivot][first] && adjacency[pivot][second] && !adjacency[first][second]) {
                    adjacency[first][second] = adjacency[second][first] = true;
                    topo.fill_in.push_back({first, second});
                }
            }
        }
    }
    IdxVector const none(n + 1, 0);
    IdxVector source(n + 1, 0);
    for (Idx bus = slack + 1; bus <= n; ++bus) {
        source[bus] = 1;
    }
    topo.sources_per_bus = {from_sparse, source};
    topo.shunts_per_bus = {from_sparse, none};
    topo.load_gens_per_bus = {from_sparse, none};
    topo.voltage_sensors_per_bus = {from_sparse, none};
    topo.power_sensors_per_bus = {from_sparse, none};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    IdxVector const no_branch_sensors(topo.branch_bus_idx.size() + 1, 0);
    topo.power_sensors_per_branch_from = {from_sparse, no_branch_sensors};
    topo.power_sensors_per_branch_to = {from_sparse, no_branch_sensors};
    return topo;
}

// The same network with a voltage sensor and an injection sensor at every bus, and a load at every bus but the slack.
inline MathModelTopology measured_bus_topology(Idx n, std::vector<BranchIdx> branches, Idx slack) {
    auto topo = topology(n, std::move(branches), slack);
    IdxVector one_per_bus(n + 1);
    IdxVector loads(n + 1);
    for (Idx bus = 0; bus <= n; ++bus) {
        one_per_bus[bus] = bus;
        loads[bus] = bus > slack ? bus - 1 : bus;
    }
    topo.load_gens_per_bus = {from_sparse, loads};
    topo.load_gen_type.assign(n - 1, LoadGenType::const_pq);
    topo.power_sensors_per_load_gen = {from_sparse, IdxVector(n, 0)};
    topo.voltage_sensors_per_bus = {from_sparse, one_per_bus};
    topo.power_sensors_per_bus = {from_sparse, one_per_bus};
    return topo;
}

// Series admittance of every branch: 1 p.u., or a tensor with coupling between the phases.
template <symmetry_tag sym> ComplexTensor<sym> series_admittance() {
    if constexpr (is_symmetric_v<sym>) {
        return 1.0;
    } else {
        return ComplexTensor<asymmetric_t>{DoubleComplex{4.0, -12.0}, DoubleComplex{0.3, -0.8}};
    }
}
template <symmetry_tag sym> MathModelParam<sym> parameters(Idx branches) {
    auto const y = series_admittance<sym>();
    MathModelParam<sym> param;
    param.branch_param.resize(branches, {y, ComplexTensor<sym>{-y}, ComplexTensor<sym>{-y}, y});
    return param;
}

// The true voltage at `bus` in the four-bus network, from which its measurements are computed.
template <symmetry_tag sym> ComplexValue<sym> bus_voltage(Idx bus) {
    constexpr double third = 2.0 * std::numbers::pi / 3.0;
    double const offset = 0.01 * static_cast<double>(bus);
    double const angle = offset - 0.03;
    if constexpr (is_symmetric_v<sym>) {
        return std::polar(1.01 + offset, angle);
    } else {
        return {std::polar(1.01 + offset, angle), std::polar(0.99 + offset, angle - third),
                std::polar(1.0 + offset, angle + third)};
    }
}

// An injection sensor that measures `power`, with different variances per component and phase.
template <symmetry_tag sym>
PowerSensorCalcParam<sym> injection_sensor(ComplexValue<sym> const& power, double variance_scale) {
    return {.real_component = {.value = real(power), .variance = per_phase<sym>({0.3, 0.7, 1.4}) * variance_scale},
            .imag_component = {.value = imag(power), .variance = per_phase<sym>({0.6, 1.2, 0.4}) * variance_scale}};
}

// ----- Estimator run -----

// The estimate of `solver` for `input`, together with the uncertainty it reports for `request`.
template <symmetry_tag sym> struct EstimateWithUQ {
    SolverOutput<sym> estimate;
    StateEstimationUQResult<sym> uq;
};
template <class Solver>
EstimateWithUQ<typename Solver::sym> run_with_uq(Solver& solver, YBus<typename Solver::sym> const& y_bus,
                                                 StateEstimationInput<typename Solver::sym> const& input,
                                                 StateEstimationUQRequest const& request, double err_tol,
                                                 Idx max_iter) {
    common::logging::NoLogger log;
    EstimateWithUQ<typename Solver::sym> result;
    StateEstimationUQContext<typename Solver::sym> context{request, result.uq};
    result.estimate = solver.run_state_estimation(y_bus, input, err_tol, max_iter, log, &context);
    return result;
}

// ----- Dense reference -----

// The augmented gain [G, Q^H; Q, R] at state `u` for voltage and injection sensors, written out from its
// definition instead of taken from a solver. G holds the voltage weights (for NRSE also on the angles that are
// measured or virtual), Q the injection equations (Y for ILSE, the derivatives of the injection for NRSE), and
// R the negated injection variances.
template <symmetry_tag sym, class Gain>
std::vector<Gain> augmented_gain(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured,
                                 ComplexValueVector<sym> const& u, ComplexValueVector<sym> const& injection,
                                 bool relative, Idx virtual_bus) {
    using namespace std::complex_literals;
    constexpr bool uses_complex_state = Eigen::NumTraits<typename Gain::Scalar>::IsComplex;
    constexpr int phases = n_phase<sym>;
    constexpr int N = Gain::RowsAtCompileTime / 2; // state coordinates per bus
    std::vector<Gain> blocks(y_bus.nnz_lu());
    for (Idx row = 0; row != y_bus.size(); ++row) {
        double const voltage_weight = measured.has_voltage(row) ? 1.0 / measured.voltage_var(row) : 0.0;
        auto const& p_variance = measured.bus_injection(row).real_component.variance;
        auto const& q_variance = measured.bus_injection(row).imag_component.variance;
        for (Idx k = y_bus.row_indptr_lu()[row]; k != y_bus.row_indptr_lu()[row + 1]; ++k) {
            Idx const col = y_bus.col_indices_lu()[k];
            Idx const admittance_index = y_bus.map_lu_y_bus()[k];
            if (admittance_index < 0) {
                continue;
            }
            auto& block = blocks[k];
            auto const& y = y_bus.admittance()[admittance_index];
            for (int phase = 0; phase != phases; ++phase) {
                if (row == col) {
                    if constexpr (uses_complex_state) {
                        block(phase, phase) = voltage_weight;
                        block(N + phase, N + phase) =
                            -(phase_value(p_variance, phase) + phase_value(q_variance, phase));
                    } else {
                        block(phase, phase) = (!relative || row == virtual_bus) ? voltage_weight : 0.0;
                        block(phases + phase, phases + phase) = voltage_weight;
                        block(N + phase, N + phase) = -phase_value(p_variance, phase);
                        block(N + phases + phase, N + phases + phase) = -phase_value(q_variance, phase);
                    }
                }
                for (int other = 0; other != phases; ++other) {
                    DoubleComplex const y_entry = tensor_entry(y, phase, other);
                    if constexpr (uses_complex_state) {
                        block(N + phase, other) = y_entry;
                    } else {
                        // Derivatives of the complex injection S_i = U_i conj(sum_k Y_ik U_k) to the angle and
                        // the magnitude of U_k, with delta_ik = 1 for i == k and 0 otherwise. The real parts
                        // are the rows of P and the imaginary parts the rows of Q.
                        //   dS_i / dtheta_k = 1i * (delta_ik S_i - U_i conj(Y_ik U_k))
                        //   dS_i / d|U_k|   = (delta_ik S_i + U_i conj(Y_ik U_k)) / |U_k|
                        DoubleComplex const u_col = phase_value(u[col], other);
                        DoubleComplex const product = phase_value(u[row], phase) * std::conj(y_entry * u_col);
                        DoubleComplex const diagonal =
                            row == col && phase == other ? phase_value(injection[row], phase) : 0.0;
                        DoubleComplex const d_theta = 1.0i * (diagonal - product);
                        DoubleComplex const d_magnitude = (diagonal + product) / std::abs(u_col);
                        block(N + phase, other) = d_theta.real();
                        block(N + phase, phases + other) = d_magnitude.real();
                        block(N + phases + phase, other) = d_theta.imag();
                        block(N + phases + phase, phases + other) = d_magnitude.imag();
                    }
                }
            }
        }
    }
    for (Idx k = 0; k != y_bus.nnz_lu(); ++k) {
        blocks[k].matrix().template block<N, N>(0, N) =
            blocks[y_bus.lu_transpose_entry()[k]].matrix().template block<N, N>(N, 0).adjoint();
    }
    return blocks;
}

/*
The four-bus network of check_against_dense_inverse(), radial or as a ring:

 bus0 --+                      bus0 -- bus1
        +-- bus2 -- bus3        |        |
 bus1 --+                      bus3 -- bus2

Every bus has a voltage sensor and an injection sensor. The slack bus, which is each bus in turn, has the
source, and every other bus has a load. Eliminating bus0 of the ring joins bus1 and bus3, which is its fill-in.
*/
inline constexpr Idx four_bus_count = 4;
inline std::vector<BranchIdx> four_bus_branches(bool mesh) {
    return mesh ? std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 3}, {3, 0}}
                : std::vector<BranchIdx>{{0, 2}, {1, 2}, {2, 3}};
}

// The four-bus topology, where the slack bus has a voltage sensor only if `slack_voltage_sensor`.
inline MathModelTopology four_bus_topology(bool mesh, Idx slack, bool slack_voltage_sensor) {
    auto topo = measured_bus_topology(four_bus_count, four_bus_branches(mesh), slack);
    if (!slack_voltage_sensor) {
        IdxVector sensors(four_bus_count + 1);
        for (Idx bus = 0; bus <= four_bus_count; ++bus) {
            sensors[bus] = bus > slack ? bus - 1 : bus;
        }
        topo.voltage_sensors_per_bus = {from_sparse, sensors};
    }
    return topo;
}

// Measurements of that network that agree with the state `u` and its injections. With `relative` no voltage
// angle is measured. Every injection variance exceeds the voltage variance `alpha`.
template <symmetry_tag sym>
StateEstimationInput<sym> four_bus_input(ComplexValueVector<sym> const& u, ComplexValueVector<sym> const& injection,
                                         bool relative, Idx slack, bool slack_voltage_sensor, double alpha) {
    StateEstimationInput<sym> input;
    input.source_status = {1};
    input.load_gen_status = {1, 1, 1};
    for (Idx bus = 0; bus != four_bus_count; ++bus) {
        if (bus != slack || slack_voltage_sensor) {
            input.measured_voltage.push_back({relative ? magnitude_only<sym>(u[bus]) : u[bus], alpha});
        }
        input.measured_bus_injection.push_back(injection_sensor<sym>(injection[bus], 5.0 * alpha));
    }
    return input;
}

// The dense state covariance C_x = alpha * (state rows and columns of the inverse of the gain `blocks`).
template <symmetry_tag sym, class Gain>
Eigen::Matrix<typename Gain::Scalar, Eigen::Dynamic, Eigen::Dynamic>
dense_state_covariance(YBus<sym> const& y_bus, std::vector<Gain> const& blocks, double alpha) {
    using Dense = Eigen::Matrix<typename Gain::Scalar, Eigen::Dynamic, Eigen::Dynamic>;
    constexpr int size = Gain::RowsAtCompileTime;
    constexpr int N = size / 2;
    Idx const n_bus = y_bus.size();
    Dense dense = Dense::Zero(n_bus * size, n_bus * size);
    for (Idx row = 0; row != n_bus; ++row) {
        for (Idx k = y_bus.row_indptr_lu()[row]; k != y_bus.row_indptr_lu()[row + 1]; ++k) {
            dense.block(row * size, y_bus.col_indices_lu()[k] * size, size, size) = blocks[k].matrix();
        }
    }
    Dense const inverse = dense.fullPivLu().inverse();
    CHECK((dense * inverse - Dense::Identity(dense.rows(), dense.cols())).norm() < 1e-11);
    Dense covariance(n_bus * N, n_bus * N);
    for (Idx row = 0; row != n_bus; ++row) {
        for (Idx col = 0; col != n_bus; ++col) {
            covariance.block(row * N, col * N, N, N) = alpha * inverse.block(row * size, col * size, N, N);
        }
    }
    return covariance;
}

struct ExpectedVoltageVariance {
    double angle;
    double magnitude;
};

// The expected variances of one phase voltage: d C d^T, with C the dense covariance and d the derivative of the
// output with respect to the state. For `relative` the derivative of the slack phase-a angle is subtracted.
template <symmetry_tag sym, class Scalar>
ExpectedVoltageVariance
expected_voltage_variance(Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> const& covariance,
                          ComplexValueVector<sym> const& u, Idx bus, int phase, bool relative, Idx slack) {
    constexpr int phases = n_phase<sym>;
    if constexpr (Eigen::NumTraits<Scalar>::IsComplex) {
        // ILSE: d(theta) = Im(dU / U) and d|U| = Re(conj(U) dU) / |U| of a proper complex dU, each of which has
        // half of the variance of the complex number it is the part of.
        constexpr int N = phases;
        Eigen::RowVectorXcd derivative = Eigen::RowVectorXcd::Zero(covariance.rows());
        derivative(bus * N + phase) += 1.0 / phase_value(u[bus], phase);
        if (relative) {
            derivative(slack * N) -= 1.0 / phase_value(u[slack], 0);
        }
        return {.angle = 0.5 * (derivative * covariance * derivative.adjoint())(0, 0).real(),
                .magnitude = 0.5 * covariance(bus * N + phase, bus * N + phase).real()};
    } else {
        // NRSE: the state is [theta, |U|] per phase.
        constexpr int N = 2 * phases;
        Eigen::RowVectorXd derivative = Eigen::RowVectorXd::Zero(covariance.rows());
        derivative(bus * N + phase) += 1.0;
        if (relative) {
            derivative(slack * N) -= 1.0;
        }
        Idx const magnitude_index = bus * N + phases + phase;
        return {.angle = (derivative * covariance * derivative.transpose())(0, 0),
                .magnitude = covariance(magnitude_index, magnitude_index)};
    }
}

// Compare the voltage variances in `outputs` with those expected from the dense covariance.
template <symmetry_tag sym, class Scalar>
void check_voltage_outputs(StateEstimationUQResult<sym> const& outputs,
                           Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> const& covariance,
                           ComplexValueVector<sym> const& u, bool relative, Idx slack, double tolerance) {
    constexpr int phases = n_phase<sym>;
    Idx const n_bus = std::ssize(u);
    REQUIRE(std::ssize(outputs.voltage) == n_bus);
    for (Idx bus = 0; bus != n_bus; ++bus) {
        CAPTURE(bus);
        for (int phase = 0; phase != phases; ++phase) {
            CAPTURE(phase);
            auto const expected = expected_voltage_variance<sym>(covariance, u, bus, phase, relative, slack);
            auto const& angle = outputs.voltage[bus].angle.phase[phase];
            auto const& magnitude = outputs.voltage[bus].magnitude.phase[phase];
            CHECK(angle.raw_status == UQVarianceStatus::valid);
            CHECK(magnitude.raw_status == UQVarianceStatus::valid);
            CHECK(std::abs(angle.raw_variance - expected.angle) < tolerance);
            CHECK(std::abs(magnitude.raw_variance - expected.magnitude) < tolerance);
        }
    }
    if (relative) {
        // Only phase a of the slack bus is the reference.
        CHECK(outputs.voltage[slack].angle.phase[0].raw_variance == 0.0);
        for (int phase = 1; phase != phases; ++phase) {
            CHECK(outputs.voltage[slack].angle.phase[phase].raw_variance > 0.0);
        }
    }
}

// On the four-bus network above, compare the covariance entries and voltage variances of the workspace, and the
// voltage variances that `Solver` reports, with a dense inverse of the augmented gain. With `relative` no angle
// is measured; `slack_voltage_sensor` then decides whether the virtual angle bus of NRSE is the slack bus.
template <class Gain, class Unknown, class Solver>
void check_against_dense_inverse(bool mesh, bool relative, Idx slack, bool slack_voltage_sensor, double alpha) {
    CAPTURE(Solver::is_NRSE_solver);
    CAPTURE(mesh);
    CAPTURE(relative);
    CAPTURE(slack);
    CAPTURE(slack_voltage_sensor);
    CAPTURE(alpha);
    using sym = typename Solver::sym;
    constexpr int N = Gain::RowsAtCompileTime / 2;
    constexpr Idx n_bus = four_bus_count;

    // The network, its state `u`, and measurements that agree with that state.
    auto const topo = four_bus_topology(mesh, slack, slack_voltage_sensor);
    YBus<sym> const y_bus{topo, parameters<sym>(topo.n_branch())};
    ComplexValueVector<sym> u(n_bus);
    for (Idx bus = 0; bus != n_bus; ++bus) {
        u[bus] = bus_voltage<sym>(bus);
    }
    auto const injection = y_bus.calculate_injection(u);
    auto const input = four_bus_input<sym>(u, injection, relative, slack, slack_voltage_sensor, alpha);
    MeasuredValues<sym> const measured{topo, input};
    CHECK_NOTHROW(observability::observability_check(measured, topo, y_bus.y_bus_structure()));
    CHECK(measured.variance_normalization() == RelativeTo{alpha, 1e-12});

    // The dense reference. NRSE puts its virtual angle measurement at the slack bus, or at the first bus with a
    // voltage sensor when the slack bus has none.
    Idx const first_measured_bus = slack_voltage_sensor || slack != 0 ? 0 : 1;
    Idx const virtual_bus = slack_voltage_sensor ? slack : first_measured_bus;
    auto const blocks = augmented_gain<sym, Gain>(y_bus, measured, u, injection, relative, virtual_bus);
    auto const covariance = dense_state_covariance(y_bus, blocks, alpha);
    double const tolerance = 3e-12 * alpha; // not 1e-12, to loosen a bit for round-off error

    // 1. The covariance entries of the workspace.
    state_estimation_uq::SparseCovarianceWorkspace<Gain, Unknown> const workspace{y_bus, blocks, alpha, u, !relative};
    for (Idx bus = 0; bus != n_bus; ++bus) {
        CAPTURE(bus);
        for (int coordinate = 0; coordinate != N; ++coordinate) {
            CAPTURE(coordinate);
            Idx const index = bus * N + coordinate;
            CHECK(std::abs(workspace.state_variance(bus, coordinate) - covariance(index, index)) < tolerance);
            if (relative) {
                CHECK(std::abs(workspace.slack_covariance(bus, coordinate) - covariance(index, slack * N)) < tolerance);
            }
        }
    }

    // 2. The voltage variances of the workspace.
    StateEstimationUQRequest request;
    request.voltage = {{0, true, true}, {1, true, true}, {2, true, true}, {3, true, true}};
    check_voltage_outputs(state_estimation_uq::calculate_voltage_uq_results(workspace, request), covariance, u,
                          relative, slack, tolerance);

    // 3. The voltage variances of the estimator, which assembles and factorizes its own gain. The measurements
    // were computed from `u`, so `u` fits them with zero residual and the estimator converges to `u`. Without a
    // measured voltage angle, the estimator converges to `u` rotated so that the phase-a angle of the slack bus
    // is zero.
    // The tolerances below are loose because of one case: three-phase ILSE without a measured angle converges
    // slowly in the angle between the phases and stops about 3e-6 from `u`.
    // NRSE agrees with the dense inverse within 1e-13.
    constexpr double estimator_err_tol = 1e-10;
    constexpr double estimator_state_tolerance = 1e-5;
    constexpr double estimator_tolerance = 1e-6;
    Solver solver{y_bus, topo};
    auto const [estimate, estimator_outputs] = run_with_uq(solver, y_bus, input, request, estimator_err_tol, 100);
    DoubleComplex const rotation = relative ? std::polar(1.0, -std::arg(phase_value(u[slack], 0))) : 1.0;
    for (Idx bus = 0; bus != n_bus; ++bus) {
        for (int phase = 0; phase != n_phase<sym>; ++phase) {
            CHECK(std::abs(phase_value(estimate.u[bus], phase) - phase_value(u[bus], phase) * rotation) <
                  estimator_state_tolerance);
        }
    }
    check_voltage_outputs(estimator_outputs, covariance, u, relative, slack, estimator_tolerance * alpha);
}

// ----- Measurement-sensitivity reference -----

// One measured real number: how to shift it in the solver input, and the variance of its error.
template <symmetry_tag sym> struct MeasuredScalar {
    std::function<void(StateEstimationInput<sym>&, double)> shift;
    double variance;
};

inline void shift_voltage_magnitude(DoubleComplex& measured, double delta) {
    if (std::isnan(measured.imag())) {
        measured = {measured.real() + delta, measured.imag()};
    } else {
        measured *= 1.0 + delta / std::abs(measured);
    }
}
inline void shift_voltage_angle(DoubleComplex& measured, double delta) { measured *= std::polar(1.0, delta); }

// Variances per bus (row) and phase (column).
struct VoltageVariance {
    Eigen::ArrayXXd magnitude;
    Eigen::ArrayXXd angle;
};

// First-order variance of the estimate by brute force, as a reference that uses none of the uncertainty code.
// Mechanism: each measured scalar is shifted up and down and the estimator is rerun, and the central difference
// gives d output / d scalar. With independent measurement errors, the variance of an output is the sum of
// (d output / d scalar)^2 * variance over the measured scalars.
// Pros: it shares no formula with the code under test, it takes the angles as the estimator reports them, and
// it equals the covariance from the inverse gain matrix when the measurements agree with one network state.
// Cons: it needs two tightly converged estimator runs per measured scalar, so it only suits small networks.
template <symmetry_tag sym, class Solver>
VoltageVariance sensitivity_variance(Solver& solver, YBus<sym> const& y_bus, StateEstimationInput<sym> const& input,
                                     std::vector<MeasuredScalar<sym>> const& scalars) {
    constexpr double step = 1e-5;
    constexpr double err_tol = 1e-12;
    constexpr Idx max_iter = 500;
    constexpr int phases = n_phase<sym>;
    common::logging::NoLogger log;
    auto const estimate = [&](double delta, MeasuredScalar<sym> const& scalar) {
        auto shifted = input;
        scalar.shift(shifted, delta);
        return solver.run_state_estimation(y_bus, shifted, err_tol, max_iter, log).u;
    };
    Idx const n_bus = y_bus.size();
    VoltageVariance result{.magnitude = Eigen::ArrayXXd::Zero(n_bus, phases),
                           .angle = Eigen::ArrayXXd::Zero(n_bus, phases)};
    for (auto const& scalar : scalars) {
        auto const plus = estimate(step, scalar);
        auto const minus = estimate(-step, scalar);
        for (Idx bus = 0; bus != n_bus; ++bus) {
            for (int phase = 0; phase != phases; ++phase) {
                DoubleComplex const u_plus = phase_value(plus[bus], phase);
                DoubleComplex const u_minus = phase_value(minus[bus], phase);
                double const d_magnitude = (std::abs(u_plus) - std::abs(u_minus)) / (2.0 * step);
                double const d_angle = std::arg(u_plus / u_minus) / (2.0 * step);
                result.magnitude(bus, phase) += d_magnitude * d_magnitude * scalar.variance;
                result.angle(bus, phase) += d_angle * d_angle * scalar.variance;
            }
        }
    }
    return result;
}

} // namespace power_grid_model::math_solver::uq_test
