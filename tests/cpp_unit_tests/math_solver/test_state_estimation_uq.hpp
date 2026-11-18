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
#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>
#include <power_grid_model/math_solver/measured_values.hpp>
#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>
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
#include <iterator>
#include <utility>
#include <vector>

namespace power_grid_model::math_solver::uq_test {

// Right-hand side of a comparison with a purely relative tolerance.
// By default doctest::Approx adds an absolute term of the size of its epsilon, which hides any error in a variance
// far below one. A zero scale removes that term.
inline doctest::Approx relative_to(double expected, double tolerance) {
    return doctest::Approx(expected).epsilon(tolerance).scale(0.0);
}

// ----- Values per phase -----

template <symmetry_tag sym> inline constexpr Idx n_phase = is_symmetric_v<sym> ? 1 : 3;

inline double phase_value(double x, Idx /*phase*/) { return x; }
inline double phase_value(RealValue<asymmetric_t> const& x, Idx phase) { return x(phase); }
inline DoubleComplex phase_value(DoubleComplex const& x, Idx /*phase*/) { return x; }
inline DoubleComplex phase_value(ComplexValue<asymmetric_t> const& x, Idx phase) { return x(phase); }
inline DoubleComplex tensor_entry(DoubleComplex const& y, Idx /*row*/, Idx /*col*/) { return y; }
inline DoubleComplex tensor_entry(ComplexTensor<asymmetric_t> const& y, Idx row, Idx col) { return y(row, col); }

// Turns one number per phase (a, b, c) into a per-phase value:
// only the first for a symmetric calculation, all three for an asymmetric one.
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

// A request for the magnitude and the angle uncertainty of the buses 0 to n_bus - 1.
inline StateEstimationUQRequest all_voltages_request(Idx n_bus) {
    StateEstimationUQRequest request;
    request.voltage.resize(n_bus);
    for (Idx const bus : IdxRange{n_bus}) {
        request.voltage[bus] = {.bus = bus, .magnitude = true, .angle = true};
    }
    return request;
}

// ----- Networks -----

// `n` buses with one source at the slack bus `slack`, and no sensors.
// YBus expects the fill-ins of the bus elimination from the topology engine, which the tests bypass. A network
// whose elimination in bus order creates fill-ins has to add them to `fill_in` itself.
inline MathModelTopology topology(Idx n, std::vector<BranchIdx> branches, Idx slack) {
    MathModelTopology topo;
    topo.slack_bus = slack;
    topo.is_radial = std::ssize(branches) == n - 1;
    topo.phase_shift.resize(n, 0.0);
    topo.branch_bus_idx = std::move(branches);
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
template <symmetry_tag sym> MathModelParam<sym> parameters(Idx n_branch) {
    auto const y = series_admittance<sym>();
    MathModelParam<sym> param;
    param.branch_param.resize(n_branch, {y, ComplexTensor<sym>{-y}, ComplexTensor<sym>{-y}, y});
    return param;
}

// The true voltage at `bus` in the four-bus network, from which its measurements are computed.
template <symmetry_tag sym> ComplexValue<sym> bus_voltage(Idx bus) {
    double const offset = 0.01 * static_cast<double>(bus);
    double const angle = offset - 0.03;
    if constexpr (is_symmetric_v<sym>) {
        return std::polar(1.01 + offset, angle);
    } else {
        return {std::polar(1.01 + offset, angle), std::polar(0.99 + offset, angle - deg_120),
                std::polar(1.0 + offset, angle + deg_120)};
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

// The diagonal of the gain block of a bus with itself: its voltage weights in G and its negated injection
// variances in R. For NRSE the angle and the magnitude have separate weights.
template <symmetry_tag sym, class Gain>
void set_gain_diagonal(Gain& block, double angle_weight, double voltage_weight, RealValue<sym> const& p_variance,
                       RealValue<sym> const& q_variance) {
    constexpr Idx phases = n_phase<sym>;
    constexpr Idx n_state = Gain::RowsAtCompileTime / 2; // state coordinates per bus
    for (Idx const phase : IdxRange{phases}) {
        if constexpr (Eigen::NumTraits<typename Gain::Scalar>::IsComplex) {
            block(phase, phase) = voltage_weight;
            block(n_state + phase, n_state + phase) =
                -(phase_value(p_variance, phase) + phase_value(q_variance, phase));
        } else {
            block(phase, phase) = angle_weight;
            block(phases + phase, phases + phase) = voltage_weight;
            block(n_state + phase, n_state + phase) = -phase_value(p_variance, phase);
            block(n_state + phases + phase, n_state + phases + phase) = -phase_value(q_variance, phase);
        }
    }
}

// The injection equations of one bus with respect to the state of the bus at the other end of the admittance `y`,
// in the Q part of their gain block. `u_row` and `injection` belong to the bus of the equations and `u_col` to
// the other bus, which is the same bus if `same_bus`.
template <symmetry_tag sym, class Gain>
void set_injection_equations(Gain& block, ComplexTensor<sym> const& y, ComplexValue<sym> const& u_row,
                             ComplexValue<sym> const& u_col, ComplexValue<sym> const& injection, bool same_bus) {
    using namespace std::complex_literals;
    constexpr Idx phases = n_phase<sym>;
    constexpr Idx n_state = Gain::RowsAtCompileTime / 2; // state coordinates per bus
    for (Idx const phase : IdxRange{phases}) {
        for (Idx const other : IdxRange{phases}) {
            DoubleComplex const y_entry = tensor_entry(y, phase, other);
            if constexpr (Eigen::NumTraits<typename Gain::Scalar>::IsComplex) {
                block(n_state + phase, other) = y_entry;
            } else {
                // Derivatives of the complex injection S_i = U_i conj(sum_k Y_ik U_k) to the angle and
                // the magnitude of U_k, with delta_ik = 1 for i == k and 0 otherwise. The real parts
                // are the rows of P and the imaginary parts the rows of Q.
                //   dS_i / dtheta_k = 1i * (delta_ik S_i - U_i conj(Y_ik U_k))
                //   dS_i / d|U_k|   = (delta_ik S_i + U_i conj(Y_ik U_k)) / |U_k|
                DoubleComplex const u_other = phase_value(u_col, other);
                DoubleComplex const product = phase_value(u_row, phase) * std::conj(y_entry * u_other);
                DoubleComplex const diagonal = same_bus && phase == other ? phase_value(injection, phase) : 0.0;
                DoubleComplex const d_theta = 1.0i * (diagonal - product);
                DoubleComplex const d_magnitude = (diagonal + product) / std::abs(u_other);
                block(n_state + phase, other) = d_theta.real();
                block(n_state + phase, phases + other) = d_magnitude.real();
                block(n_state + phases + phase, other) = d_theta.imag();
                block(n_state + phases + phase, phases + other) = d_magnitude.imag();
            }
        }
    }
}

// The augmented gain [G, Q^H; Q, R] at state `u` for voltage and injection sensors, written out from its
// definition instead of taken from a solver. G holds the voltage weights (for NRSE also on the angles that are
// measured or virtual), Q the injection equations (Y for ILSE, the derivatives of the injection for NRSE), and
// R the negated injection variances.
template <symmetry_tag sym, class Gain>
std::vector<Gain> augmented_gain(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured,
                                 ComplexValueVector<sym> const& u, ComplexValueVector<sym> const& injection,
                                 bool relative, Idx virtual_bus) {
    constexpr int n_state = Gain::RowsAtCompileTime / 2; // state coordinates per bus
    std::vector<Gain> blocks(y_bus.nnz_lu());
    for (Idx row = 0; row != y_bus.size(); ++row) {
        double const voltage_weight = measured.has_voltage(row) ? 1.0 / measured.voltage_var(row) : 0.0;
        double const angle_weight = (!relative || row == virtual_bus) ? voltage_weight : 0.0;
        auto const& sensor = measured.bus_injection(row);
        for (Idx k = y_bus.row_indptr_lu()[row]; k != y_bus.row_indptr_lu()[row + 1]; ++k) {
            Idx const col = y_bus.col_indices_lu()[k];
            Idx const admittance_index = y_bus.map_lu_y_bus()[k];
            if (admittance_index < 0) {
                continue;
            }
            if (row == col) {
                set_gain_diagonal<sym>(blocks[k], angle_weight, voltage_weight, sensor.real_component.variance,
                                       sensor.imag_component.variance);
            }
            set_injection_equations<sym>(blocks[k], y_bus.admittance()[admittance_index], u[row], u[col],
                                         injection[row], row == col);
        }
    }
    for (Idx k = 0; k != y_bus.nnz_lu(); ++k) {
        blocks[k].matrix().template block<n_state, n_state>(0, n_state) =
            blocks[y_bus.lu_transpose_entry()[k]].matrix().template block<n_state, n_state>(n_state, 0).adjoint();
    }
    return blocks;
}

// The four-bus network of the comparison with a dense inverse, radial or as a ring:
//
//  bus0 --+                      bus0 -- bus1
//         +-- bus2 -- bus3        |        |
//  bus1 --+                      bus3 -- bus2
//
// Every bus has a voltage sensor and an injection sensor. The slack bus has the source, and every other bus has a
// load. Eliminating bus0 of the ring joins bus1 and bus3, which is its only fill-in.
inline constexpr Idx four_bus_count = 4;

// One case of the comparison with a dense inverse.
struct FourBusCase {
    bool mesh{};                 // the ring instead of the radial network
    bool relative{};             // no voltage angle is measured
    Idx slack{};                 // the slack bus
    bool slack_voltage_sensor{}; // the slack bus has a voltage sensor
    double alpha{};              // the variance of the voltage sensors
};

inline std::vector<BranchIdx> four_bus_branches(bool mesh) {
    return mesh ? std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 3}, {3, 0}}
                : std::vector<BranchIdx>{{0, 2}, {1, 2}, {2, 3}};
}

inline MathModelTopology four_bus_topology(FourBusCase const& test_case) {
    auto topo = measured_bus_topology(four_bus_count, four_bus_branches(test_case.mesh), test_case.slack);
    if (test_case.mesh) {
        topo.fill_in = {{1, 3}};
    }
    if (!test_case.slack_voltage_sensor) {
        IdxVector sensors(four_bus_count + 1);
        for (Idx bus = 0; bus <= four_bus_count; ++bus) {
            sensors[bus] = bus > test_case.slack ? bus - 1 : bus;
        }
        topo.voltage_sensors_per_bus = {from_sparse, sensors};
    }
    return topo;
}

// Measurements of that network that agree with the state `u` and its injections.
// Every injection variance exceeds the voltage variance.
template <symmetry_tag sym>
StateEstimationInput<sym> four_bus_input(FourBusCase const& test_case, ComplexValueVector<sym> const& u,
                                         ComplexValueVector<sym> const& injection) {
    constexpr double injection_variance_scale = 5.0;
    StateEstimationInput<sym> input;
    input.source_status = {1};
    input.load_gen_status = {1, 1, 1};
    for (Idx bus = 0; bus != four_bus_count; ++bus) {
        if (bus != test_case.slack || test_case.slack_voltage_sensor) {
            input.measured_voltage.emplace_back(test_case.relative ? magnitude_only<sym>(u[bus]) : u[bus],
                                                test_case.alpha);
        }
        input.measured_bus_injection.push_back(
            injection_sensor<sym>(injection[bus], injection_variance_scale * test_case.alpha));
    }
    return input;
}

// The dense state covariance C_x = alpha * (state rows and columns of the inverse of the gain `blocks`).
template <symmetry_tag sym, class Gain>
Eigen::Matrix<typename Gain::Scalar, Eigen::Dynamic, Eigen::Dynamic>
dense_state_covariance(YBus<sym> const& y_bus, std::vector<Gain> const& blocks, double alpha) {
    using Dense = Eigen::Matrix<typename Gain::Scalar, Eigen::Dynamic, Eigen::Dynamic>;
    constexpr Idx size = Gain::RowsAtCompileTime;
    constexpr Idx n_state = size / 2;
    Idx const n_bus = y_bus.size();
    Dense dense = Dense::Zero(n_bus * size, n_bus * size);
    for (Idx row = 0; row != n_bus; ++row) {
        for (Idx k = y_bus.row_indptr_lu()[row]; k != y_bus.row_indptr_lu()[row + 1]; ++k) {
            dense.block(row * size, y_bus.col_indices_lu()[k] * size, size, size) = blocks[k].matrix();
        }
    }
    Dense const inverse = dense.fullPivLu().inverse();
    CHECK((dense * inverse - Dense::Identity(dense.rows(), dense.cols())).norm() < 1e-11);
    Dense covariance(n_bus * n_state, n_bus * n_state);
    for (Idx row = 0; row != n_bus; ++row) {
        for (Idx col = 0; col != n_bus; ++col) {
            covariance.block(row * n_state, col * n_state, n_state, n_state) =
                alpha * inverse.block(row * size, col * size, n_state, n_state);
        }
    }
    return covariance;
}

struct ExpectedVoltageVariance {
    double angle{};
    double magnitude{};
};

// The expected variances of one phase voltage: d C d^T, with C the dense covariance and d the derivative of the
// output with respect to the state. For `relative` the derivative of the slack phase-a angle is subtracted.
template <symmetry_tag sym, class Scalar>
ExpectedVoltageVariance
expected_voltage_variance(Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> const& covariance,
                          ComplexValueVector<sym> const& u, Idx bus, Idx phase, bool relative, Idx slack) {
    constexpr Idx phases = n_phase<sym>;
    if constexpr (Eigen::NumTraits<Scalar>::IsComplex) {
        // ILSE: d(theta) = Im(dU / U) and d|U| = Re(conj(U) dU) / |U| of a proper complex dU, each of which has
        // half of the variance of the complex number it is the part of.
        constexpr Idx n_state = phases;
        Eigen::RowVectorXcd derivative = Eigen::RowVectorXcd::Zero(covariance.rows());
        derivative(bus * n_state + phase) += 1.0 / phase_value(u[bus], phase);
        if (relative) {
            derivative(slack * n_state) -= 1.0 / phase_value(u[slack], 0);
        }
        return {.angle = 0.5 * (derivative * covariance * derivative.adjoint())(0, 0).real(),
                .magnitude = 0.5 * covariance(bus * n_state + phase, bus * n_state + phase).real()};
    } else {
        // NRSE: the state is [theta, |U|] per phase.
        constexpr Idx n_state = 2 * phases;
        Eigen::RowVectorXd derivative = Eigen::RowVectorXd::Zero(covariance.rows());
        derivative(bus * n_state + phase) += 1.0;
        if (relative) {
            derivative(slack * n_state) -= 1.0;
        }
        Idx const magnitude_index = bus * n_state + phases + phase;
        return {.angle = (derivative * covariance * derivative.transpose())(0, 0),
                .magnitude = covariance(magnitude_index, magnitude_index)};
    }
}

// Compare the voltage variances in `outputs` with those expected from the dense covariance.
template <symmetry_tag sym, class Scalar>
void check_voltage_outputs(StateEstimationUQResult<sym> const& outputs,
                           Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> const& covariance,
                           ComplexValueVector<sym> const& u, FourBusCase const& test_case, double tolerance) {
    constexpr Idx phases = n_phase<sym>;
    Idx const n_bus = std::ssize(u);
    Idx const slack = test_case.slack;
    REQUIRE(std::ssize(outputs.voltage) == n_bus);
    for (Idx bus = 0; bus != n_bus; ++bus) {
        CAPTURE(bus);
        for (Idx const phase : IdxRange{phases}) {
            CAPTURE(phase);
            auto const expected = expected_voltage_variance<sym>(covariance, u, bus, phase, test_case.relative, slack);
            auto const& angle = outputs.voltage[bus].angle.phase[phase];
            auto const& magnitude = outputs.voltage[bus].magnitude.phase[phase];
            CHECK(angle.raw_status == UQVarianceStatus::valid);
            CHECK(magnitude.raw_status == UQVarianceStatus::valid);
            CHECK(std::abs(angle.raw_variance - expected.angle) < tolerance);
            CHECK(std::abs(magnitude.raw_variance - expected.magnitude) < tolerance);
        }
    }
    if (test_case.relative) {
        // Only phase a of the slack bus is the reference.
        CHECK(outputs.voltage[slack].angle.phase[0].raw_variance == 0.0);
        for (Idx phase = 1; phase != phases; ++phase) {
            CHECK(outputs.voltage[slack].angle.phase[phase].raw_variance > 0.0);
        }
    }
}

// On the four-bus network above, compare the covariance entries and voltage variances of the workspace, and the
// voltage variances that `Solver` reports, with a dense inverse of the augmented gain.
template <class Gain, class Unknown, class Solver> void check_against_dense_inverse(FourBusCase const& test_case) {
    CAPTURE(Solver::is_NRSE_solver);
    CAPTURE(test_case.mesh);
    CAPTURE(test_case.relative);
    CAPTURE(test_case.slack);
    CAPTURE(test_case.slack_voltage_sensor);
    CAPTURE(test_case.alpha);
    using sym = typename Solver::sym;
    using Workspace = state_estimation_uq::SparseCovarianceWorkspace<Gain, Unknown>;
    constexpr Idx n_state = Gain::RowsAtCompileTime / 2;
    constexpr Idx n_bus = four_bus_count;
    bool const relative = test_case.relative;
    Idx const slack = test_case.slack;
    double const alpha = test_case.alpha;

    // The network, its state `u`, and measurements that agree with that state.
    auto const topo = four_bus_topology(test_case);
    YBus<sym> const y_bus{topo, parameters<sym>(topo.n_branch())};
    ComplexValueVector<sym> u(n_bus);
    for (Idx bus = 0; bus != n_bus; ++bus) {
        u[bus] = bus_voltage<sym>(bus);
    }
    auto const injection = y_bus.calculate_injection(u);
    auto const input = four_bus_input<sym>(test_case, u, injection);
    MeasuredValues<sym> const measured{topo, input};
    CHECK_NOTHROW(observability::observability_check(measured, topo, y_bus.y_bus_structure()));
    CHECK(measured.variance_normalization() == relative_to(alpha, 1e-12));

    // The dense reference. NRSE puts its virtual angle measurement at the slack bus, or at the first bus with a
    // voltage sensor if the slack bus has none.
    Idx const first_measured_bus = test_case.slack_voltage_sensor || slack != 0 ? 0 : 1;
    Idx const virtual_bus = test_case.slack_voltage_sensor ? slack : first_measured_bus;
    auto const blocks = augmented_gain<sym, Gain>(y_bus, measured, u, injection, relative, virtual_bus);
    auto const covariance = dense_state_covariance(y_bus, blocks, alpha);
    double const tolerance = 3e-12 * alpha; // not 1e-12, to loosen a bit for round-off error

    // 1. The covariance entries of the workspace, from the LU factors of the same gain.
    typename Workspace::Solver lu_solver{y_bus.row_indptr_lu(), y_bus.col_indices_lu(), y_bus.lu_diag()};
    typename Workspace::BlockPermArray block_perm(n_bus);
    auto lu_factors = blocks;
    lu_solver.prefactorize(lu_factors, block_perm);
    Workspace const workspace{y_bus, measured, u, lu_solver, lu_factors, block_perm};
    for (Idx bus = 0; bus != n_bus; ++bus) {
        CAPTURE(bus);
        for (Idx const coordinate : IdxRange{n_state}) {
            CAPTURE(coordinate);
            Idx const index = bus * n_state + coordinate;
            CHECK(std::abs(workspace.state_variance(bus, coordinate) - covariance(index, index)) < tolerance);
            if (relative) {
                CHECK(std::abs(workspace.slack_covariance(bus, coordinate) - covariance(index, slack * n_state)) <
                      tolerance);
            }
        }
    }

    // 2. The voltage variances of the workspace.
    auto const request = all_voltages_request(n_bus);
    check_voltage_outputs(state_estimation_uq::calculate_voltage_uq_results(workspace, request), covariance, u,
                          test_case, tolerance);

    // 3. The voltage variances of the estimator, which assembles and factorizes its own gain. The measurements
    // were computed from `u`, so `u` fits them with zero residual and the estimator converges to `u`. Without a
    // measured voltage angle, the estimator converges to `u` rotated so that the phase-a angle of the slack bus
    // is zero.
    // ILSE without a measured angle converges slowly: in the three-phase case it stops about 3e-6 from `u`, and its
    // variances are then about 1e-8 * alpha from the dense inverse. NRSE, and ILSE with a measured angle, agree
    // with `u` and with the dense inverse within 1e-13.
    constexpr double estimator_err_tol = 1e-10;
    constexpr Idx estimator_max_iter = 100;
    bool const slow_convergence = !Solver::is_NRSE_solver && relative;
    double const estimator_state_tolerance = slow_convergence ? 1e-5 : 1e-9;
    double const estimator_tolerance = slow_convergence ? 1e-6 : 1e-8;
    Solver solver{y_bus, topo};
    auto const [estimate, estimator_outputs] =
        run_with_uq(solver, y_bus, input, request, estimator_err_tol, estimator_max_iter);
    DoubleComplex const rotation = relative ? std::polar(1.0, -std::arg(phase_value(u[slack], 0))) : 1.0;
    for (Idx bus = 0; bus != n_bus; ++bus) {
        for (Idx const phase : IdxRange{n_phase<sym>}) {
            CHECK(std::abs(phase_value(estimate.u[bus], phase) - phase_value(u[bus], phase) * rotation) <
                  estimator_state_tolerance);
        }
    }
    check_voltage_outputs(estimator_outputs, covariance, u, test_case, estimator_tolerance * alpha);
}

// The comparison for ILSE and NRSE on a four-bus network, radial or a ring, with or without measured angles, with
// each bus as the slack bus in turn, and with voltage variances far below, at and far above one.
template <symmetry_tag sym> void check_network_against_dense_inverse(bool mesh, bool relative) {
    using ILSEGain = iterative_linear_se::ILSEGainBlock<sym>;
    using ILSEUnknown = iterative_linear_se::ILSEUnknown<sym>;
    using NRSEGain = newton_raphson_se::NRSEGainBlock<sym>;
    using NRSEUnknown = newton_raphson_se::NRSEUnknown<sym>;

    for (Idx const slack : IdxRange{four_bus_count}) {
        for (double const alpha : {1e-8, 1.0, 1e8}) {
            FourBusCase const measured_slack{
                .mesh = mesh, .relative = relative, .slack = slack, .slack_voltage_sensor = true, .alpha = alpha};
            check_against_dense_inverse<ILSEGain, ILSEUnknown, IterativeLinearSESolver<sym>>(measured_slack);
            check_against_dense_inverse<NRSEGain, NRSEUnknown, NewtonRaphsonSESolver<sym>>(measured_slack);
            if (relative) {
                // Only without a measured angle NRSE has a virtual angle measurement. It moves away from the
                // slack bus if that has no voltage sensor.
                FourBusCase const unmeasured_slack{
                    .mesh = mesh, .relative = relative, .slack = slack, .slack_voltage_sensor = false, .alpha = alpha};
                check_against_dense_inverse<NRSEGain, NRSEUnknown, NewtonRaphsonSESolver<sym>>(unmeasured_slack);
            }
        }
    }
}

// ----- Measurement-sensitivity reference -----

// One measured real number: how to shift it in the solver input, and the variance of its error.
template <symmetry_tag sym> struct MeasuredScalar {
    std::function<void(StateEstimationInput<sym>&, double)> shift;
    double variance{};
};

// A measured voltage with its magnitude or its angle shifted by `delta`.
// A measurement without angle has a NaN imaginary part, and its real part is the magnitude.
inline DoubleComplex shift_voltage_magnitude(DoubleComplex const& measured, double delta) {
    if (std::isnan(measured.imag())) {
        return {measured.real() + delta, measured.imag()};
    }
    return measured * (1.0 + delta / std::abs(measured));
}
inline DoubleComplex shift_voltage_angle(DoubleComplex const& measured, double delta) {
    return measured * std::polar(1.0, delta);
}

// Variances per bus (row) and phase (column).
struct VoltageVariance {
    Eigen::ArrayXXd magnitude;
    Eigen::ArrayXXd angle;
};

// First-order variance of the estimate by brute force, as a reference that uses none of the uncertainty code.
// Each measured scalar is shifted up and down and the estimator is rerun, and the central difference gives
// d output / d scalar. With independent measurement errors, the variance of an output is the sum of
// (d output / d scalar)^2 * variance over the measured scalars.
// The reference shares no formula with the code under test, and it takes the angles as the estimator reports them.
// It equals the covariance from the inverse gain matrix when the measurements agree with one network state.
// It needs two tightly converged estimator runs per measured scalar, so it only suits small networks.
template <symmetry_tag sym, class Solver>
VoltageVariance sensitivity_variance(Solver& solver, YBus<sym> const& y_bus, StateEstimationInput<sym> const& input,
                                     std::vector<MeasuredScalar<sym>> const& scalars) {
    constexpr double step = 1e-5;
    constexpr double err_tol = 1e-12;
    constexpr Idx max_iter = 500;
    constexpr Idx phases = n_phase<sym>;
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
            for (Idx const phase : IdxRange{phases}) {
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

namespace power_grid_model::math_solver {

TEST_CASE_TEMPLATE_DEFINE("UQ covariance and voltage variances from the selective inverse match a dense inverse", sym,
                          test_uq_dense_inverse_id) {
    for (bool const mesh : {false, true}) {
        for (bool const relative : {false, true}) {
            uq_test::check_network_against_dense_inverse<sym>(mesh, relative);
        }
    }
}

} // namespace power_grid_model::math_solver
