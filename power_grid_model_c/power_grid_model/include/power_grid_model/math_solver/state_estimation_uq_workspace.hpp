// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// State covariance of one island math model from selected blocks of the inverse augmented gain matrix.

#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../common/common.hpp"
#include "../common/three_phase_tensor.hpp"

#include <complex>
#include <type_traits>
#include <vector>

namespace power_grid_model::math_solver::state_estimation_uq {

// Entries of the state covariance C_x from the inverse of the augmented state-estimation matrix.
// Abar_ps is the matrix that state estimation assembles, with perfect shuffling (one block per bus) and variance
// normalization by alpha. The inverse Zbar = Abar_ps^-1, C_x = alpha * (state rows and columns of Zbar).
// Each bus block starts with the StateDim state coordinates of the bus, which `coordinate` indexes below:
// - ILSE: U (symmetric) or U_a, U_b, U_c (asymmetric), the complex phase voltages
// - NRSE: theta, |U| (symmetric) or theta_a, theta_b, theta_c, |U_a|, |U_b|, |U_c| (asymmetric), in rad and p.u.
template <class Gain, class Unknown, int StateDim = Gain::RowsAtCompileTime / 2> class SparseCovarianceWorkspace {
  public:
    using Scalar = typename Gain::Scalar;
    // True for the complex voltage state of ILSE; false for the real polar state of NRSE.
    static constexpr bool uses_complex_state = std::is_same_v<Scalar, DoubleComplex>;
    static constexpr int phase_count = uses_complex_state ? StateDim : StateDim / 2;
    static_assert(phase_count == 1 || phase_count == 3);
    using Symmetry = std::conditional_t<phase_count == 1, symmetric_t, asymmetric_t>;
    using Solver = SparseLUSolver<Gain, Unknown, Unknown>;

    // LU factors of ILSE, whose augmented matrix does not depend on the state and needs no second factorization.
    struct Factorization {
        Solver& solver;
        typename Solver::BlockPermArray const& permutations;
    };

    // `gain` is the augmented matrix Abar_ps at the estimate `u`. If factorization has been done, `gain` holds its
    // LU factors and `factorization` gives the solver and block permutations that produced them.
    // `absolute_angle` is MeasuredValues::has_angle(), which decides the angle reference of the SE output.
    // Throws SparseMatrixError if the matrix cannot be factorized without pivot perturbation.
    // Only the uncertainty fails then, not the state estimation: calculate_results() catches the error and reports
    // the requested uncertainties as invalid, and the SE results are returned as usual.
    SparseCovarianceWorkspace(YBus<Symmetry> const& y_bus, std::vector<Gain> gain, double variance_normalization,
                              ComplexValueVector<Symmetry> const& u, bool absolute_angle,
                              Factorization const* factorization = nullptr)
        : inverse_{std::move(gain)},
          variance_normalization_{variance_normalization},
          slack_bus_{y_bus.math_topology().slack_bus},
          report_relative_angle_{!absolute_angle},
          lu_diag_{y_bus.lu_diag()},
          u_{u} {
        Solver owned_solver{y_bus.row_indptr_lu(), y_bus.col_indices_lu(), y_bus.lu_diag()};
        Solver& solver = factorization ? factorization->solver : owned_solver;
        typename Solver::BlockPermArray owned_perm;
        if (!factorization) {
            owned_perm.resize(u_.size());
            solver.prefactorize(inverse_, owned_perm, false);
        }
        auto const& perm = factorization ? factorization->permutations : owned_perm;

        // Relative angles need Cov(theta_i, theta_s), from the slack column of Zbar, which is in general not on the
        // LU pattern. Solve for it now, because the selective inverse below overwrites the LU factors.
        if (report_relative_angle_) {
            solve_slack_column(solver, perm);
        }
        // Replace the factors in place by the blocks of Zbar on the LU sparsity pattern.
        solver.inplace_selective_inverse_with_prefactorized_matrix(inverse_, perm);
    }

    Idx slack_bus() const { return slack_bus_; }
    // True when SE reports angles relative to phase a of the slack bus.
    bool report_relative_angle() const { return report_relative_angle_; }

    DoubleComplex phase_voltage(Idx bus, int phase) const {
        if constexpr (phase_count == 1) {
            return u_[bus];
        } else {
            return u_[bus](phase);
        }
    }

    // Variance C_x,ii(coordinate, coordinate) of the given state coordinate of `bus`.
    double state_variance(Idx bus, int coordinate) const {
        return variance_normalization_ * std::real(inverse_[lu_diag_[bus]](coordinate, coordinate));
    }

    // Covariance C_x,is(coordinate, 0) between the given state coordinate of `bus` and the first state coordinate of
    // the slack bus, which is its phase-a voltage for ILSE and its phase-a angle for NRSE.
    // Only available for relative angles.
    Scalar slack_covariance(Idx bus, int coordinate) const {
        return variance_normalization_ * slack_column_[bus](coordinate);
    }

  private:
    using StateColumn = Eigen::Matrix<Scalar, StateDim, 1>;

    // Solve Abar_ps * t = e for the first state coordinate of the slack bus and keep the state rows of t.
    void solve_slack_column(Solver& solver, typename Solver::BlockPermArray const& perm) {
        std::vector<Unknown> rhs(u_.size());
        std::vector<Unknown> solution(u_.size());
        rhs[slack_bus_](0) = 1.0;
        solver.solve_with_prefactorized_matrix(inverse_, perm, rhs, solution);
        slack_column_.resize(u_.size());
        for (Idx bus = 0; bus != static_cast<Idx>(u_.size()); ++bus) {
            slack_column_[bus] = solution[bus].matrix().template topRows<StateDim>();
        }
    }

    std::vector<Gain> inverse_; // LU factors during construction, afterwards blocks of Zbar.
    double variance_normalization_;
    Idx slack_bus_;
    bool report_relative_angle_;
    IdxVector lu_diag_;
    ComplexValueVector<Symmetry> u_;
    std::vector<StateColumn> slack_column_; // state rows of the slack column of Zbar. Empty for absolute angles.
};

} // namespace power_grid_model::math_solver::state_estimation_uq
