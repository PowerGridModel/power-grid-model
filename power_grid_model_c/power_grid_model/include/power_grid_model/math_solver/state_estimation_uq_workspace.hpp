// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// State covariance from the selective inverse of the augmented matrix of state estimation.

#include "measured_values.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../common/common.hpp"
#include "../common/three_phase_tensor.hpp"

#include <cassert>
#include <complex>
#include <concepts>
#include <iterator>
#include <span>
#include <type_traits>
#include <vector>

namespace power_grid_model::math_solver::state_estimation_uq {

// Entries of the state covariance C_x.
// Abar_ps is the augmented matrix that state estimation assembles, with perfect shuffling and variance normalization.
// Zbar = Abar_ps^-1, C_x = alpha * (state rows and columns of Zbar).
// The state coordinates are:
//  - ILSE: U (symmetric) or U_a, U_b, U_c (asymmetric), the complex phase voltages
//  - NRSE: theta, |U| (symmetric) or theta_a, theta_b, theta_c, |U_a|, |U_b|, |U_c| (asymmetric), in rad and p.u.
template <class Gain, class Unknown> class SparseCovarianceWorkspace {
  public:
    using Scalar = Gain::Scalar;
    using Solver = SparseLUSolver<Gain, Unknown, Unknown>;
    using BlockPermArray = Solver::BlockPermArray;

    // true for the complex voltage state of ILSE, false for the real polar state of NRSE
    static constexpr bool uses_complex_state = std::same_as<Scalar, DoubleComplex>;
    static constexpr Idx phase_count = Gain::block_traits::sub_block_size;
    // Phase a of the slack bus is the angle reference if no voltage angle is measured.
    // It is also the first state coordinate of a bus, for both states.
    static constexpr Idx reference_phase = 0;
    using sym = std::conditional_t<phase_count == 1, symmetric_t, asymmetric_t>;

    // `lu_factors` holds the LU factors of Abar_ps at the estimate `u`.
    // The factors are replaced in place by the blocks of Zbar on the LU sparsity pattern.
    SparseCovarianceWorkspace(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_values,
                              ComplexValueVector<sym> const& u, Solver& solver, std::vector<Gain>& lu_factors,
                              BlockPermArray const& block_perm)
        : variance_normalization_{measured_values.variance_normalization()},
          slack_bus_{y_bus.math_topology().slack_bus},
          report_relative_angle_{!measured_values.has_angle()},
          lu_diag_{y_bus.lu_diag()},
          u_{u},
          inverse_{lu_factors} {
        assert(!solver.has_pivot_perturbation());

        // Angle variance relative to the slack reference needs covariance with the slack reference.
        // Solve Abar_ps * z = e for the slack column of Zbar, which in general is not covered by LU sparsity pattern,
        // Solve before selective inverse overwrites the LU factors.
        if (report_relative_angle_) {
            auto& rhs = slack_column_; // The solve overwrites the right-hand side in place.
            rhs.resize(u.size());
            rhs[slack_bus_](reference_phase) = 1.0;
            solver.solve_with_prefactorized_matrix(lu_factors, block_perm, rhs, slack_column_);
        }
        solver.inplace_selective_inverse_with_prefactorized_matrix(lu_factors, block_perm);
    }

    constexpr Idx n_bus() const { return std::ssize(u_); }
    constexpr Idx slack_bus() const { return slack_bus_; }
    // true if no voltage angle is measured
    constexpr bool report_relative_angle() const { return report_relative_angle_; }

    DoubleComplex phase_voltage(Idx bus, Idx phase) const {
        if constexpr (is_symmetric_v<sym>) {
            return u_[bus];
        } else {
            return u_[bus](phase);
        }
    }

    // Variance C_x,ii(coordinate, coordinate) of the given state coordinate of `bus`.
    double state_variance(Idx bus, Idx coordinate) const {
        return variance_normalization_ * std::real(inverse_[lu_diag_[bus]](coordinate, coordinate));
    }

    // Covariance C_x,is(coordinate, reference_phase) between the given state coordinate of `bus` and the state
    // coordinate of the reference phase of the slack bus, which is its voltage for ILSE and its angle for NRSE.
    // Only available for relative angles.
    Scalar slack_covariance(Idx bus, Idx coordinate) const {
        assert(report_relative_angle_);
        return variance_normalization_ * slack_column_[bus](coordinate);
    }

  private:
    double variance_normalization_;
    Idx slack_bus_;
    bool report_relative_angle_;
    std::span<Idx const> lu_diag_;
    std::span<ComplexValue<sym> const> u_;
    std::span<Gain const> inverse_;     // blocks of Zbar on the LU sparsity pattern
    std::vector<Unknown> slack_column_; // slack column of Zbar; empty for absolute angles
};

} // namespace power_grid_model::math_solver::state_estimation_uq
