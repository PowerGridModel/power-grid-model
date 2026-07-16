// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"
#include "../common/typing.hpp"

#include <Eigen/Core>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace power_grid_model::math_solver {

constexpr double epsilon = std::numeric_limits<double>::epsilon();
constexpr double epsilon_perturbation = 1e-13;      // perturbation threshold
constexpr double cap_back_error_denominator = 1e-4; // denominator for cap back error
constexpr double epsilon_sqrt = 1.49011745e-8;      // sqrt(epsilon), sqrt does not have constexpr

// perturb pivot if needed
// pass the value and abs_value by reference
// it will get modified if perturbation happens
// also has_pivot_perturbation will be updated if perturbation happens
template <scalar_value Scalar>
inline void perturb_pivot_if_needed(double perturb_threshold, Scalar& value, double& abs_value,
                                    bool& has_pivot_perturbation) {
    if (abs_value < perturb_threshold) {
        Scalar const scale = (abs_value == 0.0) ? Scalar{1.0} : (value / abs_value);
        value = scale * perturb_threshold;
        has_pivot_perturbation = true;
        abs_value = perturb_threshold;
    }
}

// Triangular solve variants:
// | Side  | Factor | Traversal | Updated RHS view | Operation  | Divide by diagonal?      |
// |-------|--------|-----------|------------------|------------|--------------------------|
// | Left  | L      | Forward   | Row              | L^-1 * rhs | No; L has unit diagonal  |
// | Left  | U      | Backward  | Row              | U^-1 * rhs | Yes                      |
// | Right | L      | Backward  | Column           | rhs * L^-1 | No; L has unit diagonal  |
// | Right | U      | Forward   | Column           | rhs * U^-1 | Yes                      |
// Left-lower and left-upper solves are commonly called forward and backward substitution, respectively.
enum class TriangularSolveSide : bool { left, right };
enum class TriangularFactor : bool { lower, upper }; // L or U

// Dense LU factorization class
// The implementation of the Dense LU factorization was derived from the Eigen library
// https://gitlab.com/libeigen/eigen/-/blob/3.4/Eigen/src/LU/FullPivLU.h
// Copyright (C) 2006-2009 Benoit Jacob <jacob.benoit.1@gmail.com>
// The original license remains: MPL-2.0
// We modified the implementation to add the pivot perturbation
template <rk2_tensor Matrix> class DenseLUFactor {
  public:
    using Scalar = typename Matrix::Scalar;
    static constexpr Idx n_rows = Matrix::RowsAtCompileTime;
    static constexpr Idx n_cols = Matrix::ColsAtCompileTime;
    static_assert(std::in_range<int8_t>(n_rows));
    static_assert(n_rows == n_cols);
    static constexpr int8_t size = n_rows;
    using PermutationType = Eigen::PermutationMatrix<size, size, int8_t>;
    using TranspositionVector = Eigen::Matrix<int8_t, size, 1>;
    struct BlockPerm {
        PermutationType p;
        PermutationType q;
    };

    // factorize in place
    // put permutation in block_perm in place
    // put pivot perturbation in has_pivot_perturbation in place
    template <class Derived>
    static void factorize_block_in_place(Eigen::MatrixBase<Derived>& matrix, BlockPerm& block_perm,
                                         double perturb_threshold, bool use_pivot_perturbation,
                                         bool& has_pivot_perturbation)
        requires(std::same_as<typename Derived::Scalar, Scalar> && rk2_tensor<Derived> &&
                 (Derived::RowsAtCompileTime == size) && (Derived::ColsAtCompileTime == size))
    {
        TranspositionVector row_transpositions{};
        TranspositionVector col_transpositions{};
        double max_pivot{};

        // main loop
        for (int8_t pivot = 0; pivot != size; ++pivot) {
            int row_biggest_eigen{};
            int col_biggest_eigen{};
            // find biggest score in the bottom right corner
            double const biggest_score = matrix.bottomRightCorner(size - pivot, size - pivot)
                                             .cwiseAbs2()
                                             .maxCoeff(&row_biggest_eigen, &col_biggest_eigen);
            // offset with pivot
            auto const row_biggest = static_cast<int8_t>(row_biggest_eigen + pivot);
            auto const col_biggest = static_cast<int8_t>(col_biggest_eigen + pivot);
            assert(row_biggest_eigen + pivot < size);
            assert(col_biggest_eigen + pivot < size);

            // check absolute singular matrix
            if (biggest_score == 0.0 && !use_pivot_perturbation) {
                // pivot perturbation not possible, cannot proceed
                // set identity permutation and break the loop
                for (int8_t remaining_rows_cols = pivot; remaining_rows_cols != size; ++remaining_rows_cols) {
                    row_transpositions[remaining_rows_cols] = remaining_rows_cols;
                    col_transpositions[remaining_rows_cols] = remaining_rows_cols;
                }
                break;
            }

            // perturb pivot if needed
            double abs_pivot = sqrt(biggest_score);
            perturb_pivot_if_needed(perturb_threshold, matrix(row_biggest, col_biggest), abs_pivot,
                                    has_pivot_perturbation);
            max_pivot = std::max(max_pivot, abs_pivot);

            // swap rows and columns
            row_transpositions[pivot] = row_biggest;
            col_transpositions[pivot] = col_biggest;
            if (pivot != row_biggest) {
                matrix.row(pivot).swap(matrix.row(row_biggest));
            }
            if (pivot != col_biggest) {
                matrix.col(pivot).swap(matrix.col(col_biggest));
            }

            // use Gaussian elimination to calculate the bottom right corner
            if (pivot < size - 1) {
                // calculate the pivot column
                matrix.col(pivot).tail(size - pivot - 1) /= matrix(pivot, pivot);
                // calculate the bottom right corner
                matrix.bottomRightCorner(size - pivot - 1, size - pivot - 1).noalias() -=
                    matrix.col(pivot).tail(size - pivot - 1) * matrix.row(pivot).tail(size - pivot - 1);
            }
        }

        // accumulate the permutation
        block_perm.p.setIdentity();
        for (int8_t pivot = size - 1; pivot != -1; --pivot) {
            block_perm.p.applyTranspositionOnTheRight(pivot, row_transpositions[pivot]);
        }
        block_perm.q.setIdentity();
        for (int8_t pivot = 0; pivot != size; ++pivot) {
            block_perm.q.applyTranspositionOnTheRight(pivot, col_transpositions[pivot]);
        }

        // throw SparseMatrixError if the matrix is ill-conditioned
        // only check condition number if pivot perturbation is not used
        double const pivot_threshold = has_pivot_perturbation ? 0.0 : epsilon * max_pivot;
        for (int8_t pivot = 0; pivot != size; ++pivot) {
            if (cabs(matrix(pivot, pivot)) < pivot_threshold || !is_normal(matrix(pivot, pivot))) {
                throw SparseMatrixError{}; // can not specify error code
            }
        }
    }

    // Solve with one of the triangular factors stored in lu_matrix.
    // The lower factor has an implicit unit diagonal; the upper factor has an explicit diagonal.
    // Forward substituion is left-lower solve. Backward substitution is left-upper solve.
    template <TriangularSolveSide side, TriangularFactor factor, class LUDerived, class RHSDerived>
    static void triangular_solve_inplace(Eigen::MatrixBase<LUDerived> const& lu_matrix, RHSDerived& rhs)
        requires(std::same_as<typename LUDerived::Scalar, Scalar> &&
                 std::same_as<typename RHSDerived::Scalar, Scalar> && rk2_tensor<LUDerived> &&
                 (LUDerived::RowsAtCompileTime == size) && (LUDerived::ColsAtCompileTime == size) &&
                 ((side == TriangularSolveSide::left && RHSDerived::RowsAtCompileTime == size) ||
                  (side == TriangularSolveSide::right && RHSDerived::ColsAtCompileTime == size)))
    {
        static constexpr bool forward_traversal =
            (side == TriangularSolveSide::left) == (factor == TriangularFactor::lower);

        for (int8_t step = 0; step < size; ++step) {
            auto const index = static_cast<int8_t>(forward_traversal ? step : size - 1 - step);
            for (int8_t previous_step = 0; previous_step < step; ++previous_step) {
                auto const previous_index =
                    static_cast<int8_t>(forward_traversal ? previous_step : size - 1 - previous_step);
                if constexpr (side == TriangularSolveSide::left) {
                    rhs.row(index) -= lu_matrix(index, previous_index) * rhs.row(previous_index);
                } else {
                    rhs.col(index) -= lu_matrix(previous_index, index) * rhs.col(previous_index);
                }
            }
            if constexpr (factor == TriangularFactor::upper) {
                if constexpr (side == TriangularSolveSide::left) {
                    rhs.row(index) /= lu_matrix(index, index);
                } else {
                    rhs.col(index) /= lu_matrix(index, index);
                }
            }
        }
    }

    // given the factorized block satisfies L * U = P * A * Q
    // compute the inverse of the factorized L * U * X = I
    // returns X = (L * U)^-1 = (P * A * Q)^-1
    template <class Derived>
    static Matrix inverse_factorized_block(Eigen::MatrixBase<Derived> const& lu_matrix)
        requires(std::same_as<typename Derived::Scalar, Scalar> && rk2_tensor<Derived> &&
                 (Derived::RowsAtCompileTime == size) && (Derived::ColsAtCompileTime == size))
    {
        Matrix inverse = Matrix::Identity();
        triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::lower>(lu_matrix, inverse);
        triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::upper>(lu_matrix, inverse);
        return inverse;
    }

    template <class Derived>
    static Matrix dense_inverse(Eigen::MatrixBase<Derived> const& lu_matrix, BlockPerm const& block_perm)
        requires(std::same_as<typename Derived::Scalar, Scalar> && rk2_tensor<Derived> &&
                 (Derived::RowsAtCompileTime == size) && (Derived::ColsAtCompileTime == size))
    {
        // given the factorized block satisfies L * U = P * A * Q
        // return A^-1 = Q * (L * U)^-1 * P
        // lu_matrix is read-only: the packed L/U factor is preserved.
        // inverse_factorized_block() performs in-place substitutions only on
        // its local Identity() RHS, not on lu_matrix.
        return block_perm.q * inverse_factorized_block(lu_matrix) * block_perm.p;
    }
};

template <class Tensor, class RHSVector, class XVector> struct sparse_lu_entry_trait;

template <class Tensor, class RHSVector, class XVector>
concept scalar_value_lu = scalar_value<Tensor> && std::same_as<Tensor, RHSVector> && std::same_as<Tensor, XVector>;

// TODO(mgovers) improve this concept
template <class Derived> int check_array_base(Eigen::ArrayBase<Derived> const& /* array_base */) { return 0; }
template <class ArrayLike>
concept eigen_array = std::same_as<decltype(check_array_base(ArrayLike{})), int>; // should be an eigen array

template <class LHSArrayLike, class RHSArrayLike>
concept matrix_multiplicable =
    eigen_array<LHSArrayLike> && eigen_array<RHSArrayLike> &&
    (static_cast<Idx>(LHSArrayLike::ColsAtCompileTime) == static_cast<Idx>(RHSArrayLike::RowsAtCompileTime));

template <class Tensor, class RHSVector, class XVector>
concept tensor_lu =
    rk2_tensor<Tensor> && column_vector<RHSVector> && column_vector<XVector> &&
    matrix_multiplicable<Tensor, RHSVector> && matrix_multiplicable<Tensor, XVector> &&
    std::same_as<typename Tensor::Scalar, typename RHSVector::Scalar> && // all entries should have same scalar type
    std::same_as<typename Tensor::Scalar, typename XVector::Scalar> &&   // all entries should have same scalar type
    scalar_value<typename Tensor::Scalar>;                               // scalar can only be double or complex double

template <class Tensor, class RHSVector, class XVector>
    requires scalar_value_lu<Tensor, RHSVector, XVector>
struct sparse_lu_entry_trait<Tensor, RHSVector, XVector> {
    static constexpr bool is_block = false;
    static constexpr Idx block_size = 1;
    using Scalar = Tensor;
    using Matrix = Tensor;
    using LUFactor = void;
    struct BlockPerm {};
    using BlockPermArray = Idx;
};

template <class Tensor, class RHSVector, class XVector>
    requires tensor_lu<Tensor, RHSVector, XVector>
struct sparse_lu_entry_trait<Tensor, RHSVector, XVector> {
    static constexpr bool is_block = true;
    static constexpr Idx block_size = Tensor::RowsAtCompileTime;
    using Scalar = typename Tensor::Scalar;
    using Matrix = Eigen::Matrix<Scalar, block_size, block_size, Tensor::Options>;
    using LUFactor = DenseLUFactor<Matrix>;         // LU decomposition with full pivoting in place
    using BlockPerm = typename LUFactor::BlockPerm; // Extract permutation matrices p and q from LUFactor
    using BlockPermArray = std::vector<BlockPerm>;
};

template <class Tensor, class RHSVector, class XVector> class SparseLUSolver {
  public:
    using entry_trait = sparse_lu_entry_trait<Tensor, RHSVector, XVector>;
    static constexpr bool is_block = entry_trait::is_block;
    static constexpr Idx block_size = entry_trait::block_size;
    using Scalar = typename entry_trait::Scalar;
    using LUFactor = typename entry_trait::LUFactor;
    using BlockPerm = typename entry_trait::BlockPerm;
    using BlockPermArray = typename entry_trait::BlockPermArray;
    static constexpr Idx max_iterative_refinement = 5;

    SparseLUSolver(std::span<Idx const> row_indptr,  // indptr including fill-ins
                   std::span<Idx const> col_indices, // indices including fill-ins
                   std::span<Idx const> diag_lu)
        : size_{static_cast<Idx>(row_indptr.size()) - 1},
          nnz_{row_indptr.back()},
          row_indptr_{row_indptr},
          col_indices_{col_indices},
          diag_lu_{diag_lu} {}

    // solve with new matrix data, need to factorize first
    void
    prefactorize_and_solve(std::vector<Tensor>& data,        // matrix data, factorize in-place
                           BlockPermArray& block_perm_array, // pre-allocated permutation array, will be overwritten
                           std::vector<RHSVector> const& rhs, std::vector<XVector>& x,
                           bool use_pivot_perturbation = false) {
        prefactorize(data, block_perm_array, use_pivot_perturbation);
        // call solve with const method
        solve_with_prefactorized_matrix((std::vector<Tensor> const&)data, block_perm_array, rhs, x);
    }

    // solve with existing pre-factorization
    void
    solve_with_prefactorized_matrix(std::vector<Tensor> const& data,        // pre-factorized data, const ref
                                    BlockPermArray const& block_perm_array, // pre-calculated permutation, const ref
                                    std::vector<RHSVector> const& rhs, std::vector<XVector>& x) {
        if (has_pivot_perturbation_) {
            solve_with_refinement(data, block_perm_array, rhs, x);
        } else {
            solve_once(data, block_perm_array, rhs, x);
        }
    }

    // Compute Takahashi dependency blocks over the solver's stored sparse pattern
    // row_indptr_/col_indices_. This pattern includes fill-ins and can be larger
    // than the downstream target pattern, e.g. the original y_bus pattern.
    void inplace_selective_inverse_with_prefactorized_matrix(
        std::vector<Tensor>& data, // pre-factorized data, will be in-place modified to store selective inverse
        BlockPermArray const& block_perm_array // pre-calculated permutation, const ref
    ) const {
        if constexpr (!is_block) {
            // our use case only needs selective inversion for block sparse matrices
            capturing::into_the_void(data, block_perm_array);
            throw SparseMatrixError{};
        } else {
            // we first handle the case without pivot perturbation
            if (has_pivot_perturbation_) {
                throw SparseMatrixError{};
            }
            inplace_selective_inverse_block_matrix(data, block_perm_array);
        }
    }

    // prefactorize in-place
    // the LU matrix has the form A = L * U
    // diagonals of L are one
    // diagonals of U have values
    // fill-ins should be pre-allocated with zero
    // block permutation array should be pre-allocated
    void prefactorize(std::vector<Tensor>& data, BlockPermArray& block_perm_array,
                      bool use_pivot_perturbation = false) {
        reset_matrix_cache();
        if (use_pivot_perturbation) {
            initialize_pivot_perturbation(data);
        }
        double const perturb_threshold = epsilon_perturbation * matrix_norm_;

        // local reference
        auto const& diag_lu = diag_lu_;
        // lu matrix inplace
        std::vector<Tensor>& lu_matrix = data;

        // column position idx per row for LU matrix
        IdxVector col_position_idx(row_indptr_.begin(), row_indptr_.end() - 1);

        // start pivoting, it is always the diagonal
        for (Idx pivot_row_col = 0; pivot_row_col != size_; ++pivot_row_col) {
            Idx const pivot_idx = diag_lu[pivot_row_col];

            // Dense LU factorize pivot for block matrix in-place
            // A_pivot,pivot, becomes P_pivot^-1 * L_pivot * U_pivot * Q_pivot^-1
            // return reference to pivot permutation
            BlockPerm const& block_perm = [&]() -> std::conditional_t<is_block, BlockPerm const&, BlockPerm> {
                if constexpr (is_block) {
                    // use machine precision by default
                    // record block permutation
                    auto pivot_matrix = lu_matrix[pivot_idx].matrix();
                    LUFactor::factorize_block_in_place(pivot_matrix, block_perm_array[pivot_row_col], perturb_threshold,
                                                       use_pivot_perturbation, has_pivot_perturbation_);
                    return block_perm_array[pivot_row_col];
                } else {
                    if (use_pivot_perturbation) {
                        // use machine precision by default
                        // record pivot perturbation
                        double abs_pivot = cabs(lu_matrix[pivot_idx]);
                        perturb_pivot_if_needed(perturb_threshold, lu_matrix[pivot_idx], abs_pivot,
                                                has_pivot_perturbation_);
                    }
                    if (!is_normal(lu_matrix[pivot_idx])) {
                        throw SparseMatrixError{};
                    }
                    return {};
                }
            }();
            // reference to pivot
            Tensor const& pivot = lu_matrix[pivot_idx];

            // for block matrix
            // permute rows of L's in the left of the pivot
            // L_k,pivot = P_pivot * L_k,pivot    k < pivot
            // permute columns of U's above the pivot
            // U_pivot,k = U_pivot,k * Q_pivot    k < pivot
            if constexpr (is_block) {
                // loop rows and columns at the same time
                // since the matrix is symmetric
                for (Idx l_idx = row_indptr_[pivot_row_col]; l_idx < pivot_idx; ++l_idx) {
                    // permute rows of L_k,pivot
                    lu_matrix[l_idx] = (block_perm.p * lu_matrix[l_idx].matrix()).array();
                    // get row and idx of u
                    Idx const u_row = col_indices_[l_idx];
                    Idx const u_idx = col_position_idx[u_row];
                    // we should exactly find the current column
                    assert(col_indices_[u_idx] == pivot_row_col);
                    // permute columns of U_pivot,k
                    lu_matrix[u_idx] = (lu_matrix[u_idx].matrix() * block_perm.q).array();
                    // increment column position
                    ++col_position_idx[u_row];
                }
            }

            // for block matrix
            // calculate U blocks in the right of the pivot, in-place
            // L_pivot * U_pivot,k = P_pivot * A_pivot,k       k > pivot
            if constexpr (is_block) {
                for (Idx u_idx = pivot_idx + 1; u_idx < row_indptr_[pivot_row_col + 1]; ++u_idx) {
                    Tensor& u = lu_matrix[u_idx];
                    // permutation
                    u = (block_perm.p * u.matrix()).array();
                    // forward substitution (left,lower solve), per row in u
                    LUFactor::template triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::lower>(
                        pivot.matrix(), u);
                }
            }

            // Calculate L blocks below the pivot and apply the sparse Schur-complement update:
            // A_k,j = A_k,j - L_k,pivot * U_pivot,j    k, j > pivot
            // The outer and inner loops visit only structurally non-zero blocks of L_k,pivot and U_pivot,j.
            // Because the matrix is symmetric,
            //    looking for col_indices at pivot_row_col, starting from the diagonal (pivot_row_col, pivot_row_col)
            //    we get also the non-zero row indices under the pivot
            for (Idx l_ref_idx = pivot_idx + 1; l_ref_idx < row_indptr_[pivot_row_col + 1]; ++l_ref_idx) {
                // find index of l in corresponding row
                Idx const l_row = col_indices_[l_ref_idx];
                Idx const l_idx = col_position_idx[l_row];
                // we should exactly find the current column
                assert(col_indices_[l_idx] == pivot_row_col);
                // calculating l at (l_row, pivot_row_col)
                if constexpr (is_block) {
                    // for block matrix
                    // calculate L blocks below the pivot, in-place
                    // L_k,pivot * U_pivot = A_k_pivot * Q_pivot    k > pivot
                    Tensor& l = lu_matrix[l_idx];
                    // permutation
                    l = (l.matrix() * block_perm.q).array();
                    // forward substitution, per column in l
                    // l0 = [l00, l10]^T
                    // l1 = [l01, l11]^T
                    // l = [l0, l1]
                    // a = [a0, a1]
                    // u = [[u00, u01]
                    //      [0  , u11]]
                    // l * u = a
                    // l0 * u00 = a0
                    // l0 * u01 + l1 * u11 = a1
                    LUFactor::template triangular_solve_inplace<TriangularSolveSide::right, TriangularFactor::upper>(
                        pivot.matrix(), l);
                } else {
                    // for scalar matrix, just divide
                    // L_k,pivot = A_k,pivot / U_pivot    k > pivot
                    lu_matrix[l_idx] = lu_matrix[l_idx] / pivot;
                }
                Tensor const& l = lu_matrix[l_idx];

                // Perform Schur-complement update for row l_row of the trailing unfactorized block. For each
                // structurally non-zero U entry to the right of the pivot, find the corresponding entry in l_row and
                // apply A(l_row, u_col) -= l * U(pivot_row_col, u_col). it can create fill-ins, but the fill-ins are
                // pre-allocated it is guaranteed to have an entry at (l_row, u_col), if (pivot_row_col, u_col) is
                // non-zero
                Idx a_idx = l_idx;
                // loop all columns in the right of (pivot_row_col, pivot_row_col), at pivot_row
                for (Idx u_idx = pivot_idx + 1; u_idx < row_indptr_[pivot_row_col + 1]; ++u_idx) {
                    Idx const u_col = col_indices_[u_idx];
                    assert(u_col > pivot_row_col);
                    // u_col is strictly increasing, so continue after the previous match.
                    a_idx = find_entry(l_row, u_col, a_idx + 1, row_indptr_[l_row + 1]);
                    // subtract
                    lu_matrix[a_idx] -= dot(l, lu_matrix[u_idx]);
                }
                // iterate column position
                ++col_position_idx[l_row];
            }
            // iterate column position for the pivot
            ++col_position_idx[pivot_row_col];
        }
        // if no pivot perturbation happened, reset cache
        if (!has_pivot_perturbation_) {
            reset_matrix_cache();
        }
    }

  private:
    static constexpr Idx linear_search_threshold = 16;

    Idx size_;
    Idx nnz_; // number of non zeroes (in block)
    std::span<Idx const> row_indptr_;
    std::span<Idx const> col_indices_;
    std::span<Idx const> diag_lu_;
    // cache value for pivot perturbation for the factorize step
    bool has_pivot_perturbation_{false};
    double matrix_norm_{};
    std::optional<std::vector<Tensor>> original_matrix_;
    // cache value for iterative refinement for the solve step
    std::optional<std::vector<XVector>> dx_;
    std::optional<std::vector<RHSVector>> residual_;
    std::optional<std::vector<RHSVector>> rhs_;

    void solve_with_refinement(std::vector<Tensor> const& data,        // pre-factorized data, const ref
                               BlockPermArray const& block_perm_array, // pre-calculated permutation, const ref
                               std::vector<RHSVector> const& rhs, std::vector<XVector>& x) {
        // initialize refinement
        initialize_refinement(rhs, x);
        double backward_error{std::numeric_limits<double>::max()};
        Idx num_iter{};
        // iterate until convergence
        // convergence criteria is the same as the perturbation threshold
        static constexpr double epsilon_converge = epsilon_perturbation;
        while (backward_error > epsilon_converge) {
            // check maximum iteration, including one initial run
            if (num_iter++ == max_iterative_refinement + 1) {
                throw SparseMatrixError{};
            }
            // solve with residual (first time it is the b vector)
            solve_once(data, block_perm_array, residual_.value(), dx_.value());
            // calculate backward error and then iterate x
            backward_error = iterate_and_backward_error(x);
            // calculate residual
            calculate_residual(x);
        }
        // reset refinement cache
        reset_refinement_cache();
    }

    void reset_refinement_cache() {
        dx_.reset();
        residual_.reset();
        rhs_.reset();
    }

    void initialize_refinement(std::vector<RHSVector> const& rhs, std::vector<XVector>& x) {
        // save a copy of rhs
        rhs_ = rhs;
        // initialize x to zero
        for (Idx row = 0; row != size_; ++row) {
            if constexpr (is_block) {
                x[row] = XVector::Zero();
            } else {
                x[row] = 0.0;
            }
        }
        // initialize residual to rhs
        // because r = b - A * x = b - 0 = b
        residual_ = rhs_;
        // initialize dx to zero
        // pre-allocate memory
        dx_ = x;
    }

    void calculate_residual(std::vector<XVector> const& x) {
        auto const& original_matrix = original_matrix_.value();
        auto const& rhs = rhs_.value();
        auto& residual = residual_.value();
        // calculate residual
        for (Idx row = 0; row != size_; ++row) {
            residual[row] = rhs[row];
            // loop all columns
            for (Idx idx = row_indptr_[row]; idx != row_indptr_[row + 1]; ++idx) {
                // subtract
                residual[row] -= dot(original_matrix[idx], x[col_indices_[idx]]);
            }
        }
    }

    double iterate_and_backward_error(std::vector<XVector>& x) {
        auto const& original_matrix = original_matrix_.value();
        auto const& rhs = rhs_.value();
        auto const& residual = residual_.value();
        auto const& dx = dx_.value();
        using RealValueType = std::conditional_t<is_block, Eigen::Array<double, block_size, 1>, double>;
        std::vector<RealValueType> all_denominators(size_);
        double max_denominator{};

        // calculate denominator and get the max value
        for (Idx row = 0; row != size_; ++row) {
            // error denominator by |rhs|
            RealValueType denominator = cabs(rhs[row]);
            // then append |A| * |x|
            for (Idx idx = row_indptr_[row]; idx != row_indptr_[row + 1]; ++idx) {
                denominator += dot(cabs(original_matrix[idx]), cabs(x[col_indices_[idx]]));
            }
            all_denominators[row] = denominator;
            max_denominator = std::max(max_denominator, max_val(denominator));
        }
        // cap min denominator
        double const min_denominator = cap_back_error_denominator * max_denominator;

        // calculate backward error and then iterate x
        double max_berr{};
        for (Idx row = 0; row != size_; ++row) {
            RealValueType const numerator = cabs(residual[row]);
            // cap denominator to min_denominator
            if constexpr (is_block) {
                for (Idx br = 0; br != block_size; ++br) {
                    all_denominators[row](br) = std::max(all_denominators[row](br), min_denominator);
                }
            } else {
                all_denominators[row] = std::max(all_denominators[row], min_denominator);
            }
            // piecewise backward error and get max
            RealValueType const berr = numerator / all_denominators[row];
            max_berr = std::max(max_berr, max_val(berr));
            // iterate x
            x[row] += dx[row];
        }
        return max_berr;
    }

    void initialize_pivot_perturbation(std::vector<Tensor> const& data) {
        // save a copy of original matrix
        original_matrix_ = data;
        // calculate the block-wise non-diagonal infinite norm of the matrix
        // that is:
        // 1. calculate the infinite norm of each individual block
        // 2. sum all norms of the blocks per row, except the diagonal block
        // 3. take the maximum of all the sums
        matrix_norm_ = 0.0;
        for (Idx row = 0; row != size_; ++row) {
            // calculate the sum of the norms of the blocks in the row
            double row_norm = 0.0;
            for (Idx idx = row_indptr_[row]; idx != row_indptr_[row + 1]; ++idx) {
                // skip diagonal
                if (col_indices_[idx] == row) {
                    continue;
                }
                if constexpr (is_block) {
                    row_norm += cabs(data[idx]).rowwise().sum().maxCoeff();
                } else {
                    row_norm += cabs(data[idx]);
                }
            }
            matrix_norm_ = std::max(matrix_norm_, row_norm);
        }
    }

    void reset_matrix_cache() {
        has_pivot_perturbation_ = false;
        matrix_norm_ = 0.0;
        original_matrix_.reset();
    }

    void inplace_selective_inverse_block_matrix(std::vector<Tensor>& data, BlockPermArray const& block_perm_array) const
        requires is_block
    {
        // First compute Z = (P * A * Q)^-1 = U^-1 * L^-1.
        for (Idx pivot_row_col = size_ - 1; pivot_row_col > -1; --pivot_row_col) {
            update_selective_inverse_pivot_row_and_column(data, pivot_row_col);
        }

        // Restore A^-1_ij per sparse entry: Z_ij = Q_i * Z_ij * P_j.
        for (Idx row = 0; row < size_; ++row) {
            for (Idx idx = row_indptr_[row]; idx < row_indptr_[row + 1]; ++idx) {
                data[idx] =
                    (block_perm_array[row].q * data[idx].matrix() * block_perm_array[col_indices_[idx]].p).array();
            }
        }
    }

    // Update selected inverse blocks for pivot p: column below p, row right of p, and diagonal.
    // Trailing Z_ij blocks with i,j > p are already available from the reverse pivot sweep.
    void update_selective_inverse_pivot_row_and_column(std::vector<Tensor>& data, Idx pivot_row_col) const
        requires is_block
    {
        Idx const pivot_idx = diag_lu_[pivot_row_col];
        Idx const u_start = pivot_idx + 1;
        Idx const u_end = row_indptr_[pivot_row_col + 1];
        Idx const n_off_diagonal = u_end - u_start;

        // Buffer LU values from row/column p before overwriting them with Z blocks.
        Tensor const pivot = data[pivot_idx];
        std::vector<Tensor> const u_row(data.begin() + u_start, data.begin() + u_end); // u_row is contagious.
        auto const [l_col, l_indices] = [&]() {
            std::vector<Tensor> values(n_off_diagonal); // l column is scattered we need to extract them.
            IdxVector indices(n_off_diagonal);
            for (Idx offset = 0; offset < n_off_diagonal; ++offset) {
                Idx const l_row = col_indices_[u_start + offset];
                indices[offset] = find_entry(l_row, pivot_row_col, row_indptr_[l_row], diag_lu_[l_row]);
                values[offset] = data[indices[offset]];
            }
            return std::pair{std::move(values), std::move(indices)};
        }();

        // Column below pivot: replace L_kp with Z_kp = -(sum_m Z_km * L_mp) * L_p^-1.
        for (Idx k_offset = 0; k_offset < n_off_diagonal; ++k_offset) {
            Idx const z_row = col_indices_[u_start + k_offset];
            Tensor sum = Tensor::Zero();
            Idx z_idx = l_indices[k_offset];
            for (Idx m_offset = 0; m_offset < n_off_diagonal; ++m_offset) {
                Idx const z_col = col_indices_[u_start + m_offset];
                z_idx = find_entry(z_row, z_col, z_idx + 1, row_indptr_[z_row + 1]);
                sum += dot(data[z_idx], l_col[m_offset]);
            }
            data[l_indices[k_offset]] = -multiply_inverse_unit_lower_right(pivot, sum);
        }

        // Row right of pivot: replace U_pj with Z_pj = -U_p^-1 * sum_m U_pm * Z_mj.
        for (Idx j_offset = 0; j_offset < n_off_diagonal; ++j_offset) {
            Idx const z_col = col_indices_[u_start + j_offset];
            Tensor sum = Tensor::Zero();
            for (Idx m_offset = 0; m_offset < n_off_diagonal; ++m_offset) {
                Idx const z_row = col_indices_[u_start + m_offset];
                Idx const z_idx = find_entry(z_row, z_col, l_indices[m_offset] + 1, row_indptr_[z_row + 1]);
                sum += dot(u_row[m_offset], data[z_idx]);
            }
            data[u_start + j_offset] = -multiply_inverse_upper_left(pivot, sum);
        }

        // Diagonal last: Z_pp = (L_p * U_p)^-1 - U_p^-1 * sum_m U_pm * Z_mp.
        Tensor sum = Tensor::Zero();
        for (Idx m_offset = 0; m_offset < n_off_diagonal; ++m_offset) {
            sum += dot(u_row[m_offset], data[l_indices[m_offset]]);
        }
        data[pivot_idx] =
            LUFactor::inverse_factorized_block(pivot.matrix()).array() - multiply_inverse_upper_left(pivot, sum);
    }

    // Find the data index of entry (row, col), which must exist in the filled LU pattern. Optional bounds restrict
    // the search to a caller-known subrange and default to the whole row.
    Idx find_entry(Idx row, Idx col, std::optional<Idx> search_begin = std::nullopt,
                   std::optional<Idx> search_end = std::nullopt) const {
        Idx const begin_idx = search_begin.value_or(row_indptr_[row]);
        Idx const end_idx = search_end.value_or(row_indptr_[row + 1]);
        assert(row_indptr_[row] <= begin_idx);
        assert(begin_idx < end_idx);
        assert(end_idx <= row_indptr_[row + 1]);
        auto const first = col_indices_.begin() + begin_idx;
        auto const last = col_indices_.begin() + end_idx;
        auto const found = end_idx - begin_idx < linear_search_threshold ? std::find(first, last, col)
                                                                         : std::lower_bound(first, last, col);
        assert(found != last);
        assert(*found == col);
        return narrow_cast<Idx>(std::distance(col_indices_.begin(), found));
    }

    // Compute U_pivot^-1 * block, with U stored in the upper triangle of the packed pivot block.
    static Tensor multiply_inverse_upper_left(Tensor const& pivot, Tensor block)
        requires is_block
    {
        LUFactor::template triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::upper>(pivot.matrix(),
                                                                                                        block);
        return block;
    }

    // Compute block * L_pivot^-1, with L stored in the lower triangle and implicit unit diagonal.

    static Tensor multiply_inverse_unit_lower_right(Tensor const& pivot, Tensor block)
        requires is_block
    {
        LUFactor::template triangular_solve_inplace<TriangularSolveSide::right, TriangularFactor::lower>(pivot.matrix(),
                                                                                                         block);
        return block;
    }

    void solve_once(std::vector<Tensor> const& data,        // pre-factorized data, const ref
                    BlockPermArray const& block_perm_array, // pre-calculated permutation, const ref
                    std::vector<RHSVector> const& rhs, std::vector<XVector>& x) const {
        // local reference
        auto const& diag_lu = diag_lu_;
        auto const& lu_matrix = data;

        // forward substitution with L
        for (Idx row = 0; row != size_; ++row) {
            // permutation if needed
            if constexpr (is_block) {
                x[row] = (block_perm_array[row].p * rhs[row].matrix()).array();
            } else {
                x[row] = rhs[row];
            }

            // loop all columns until diagonal
            for (Idx l_idx = row_indptr_[row]; l_idx < diag_lu[row]; ++l_idx) {
                Idx const col = col_indices_[l_idx];
                // never overshoot
                assert(col < row);
                // forward subtract
                x[row] -= dot(lu_matrix[l_idx], x[col]);
            }
            // forward substitution inside block, for block matrix
            if constexpr (is_block) {
                Tensor const& pivot = lu_matrix[diag_lu[row]];
                LUFactor::template triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::lower>(
                    pivot.matrix(), x[row]);
            }
        }

        // backward substitution with U
        for (Idx row = size_ - 1; row != -1; --row) {
            // loop all columns from diagonal
            for (Idx u_idx = row_indptr_[row + 1] - 1; u_idx > diag_lu[row]; --u_idx) {
                Idx const col = col_indices_[u_idx];
                // always in upper diagonal
                assert(col > row);
                // backward subtract
                x[row] -= dot(lu_matrix[u_idx], x[col]);
            }
            // solve the diagonal pivot
            if constexpr (is_block) {
                // backward substitution inside block
                Tensor const& pivot = lu_matrix[diag_lu[row]];
                LUFactor::template triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::upper>(
                    pivot.matrix(), x[row]);
            } else {
                x[row] = x[row] / lu_matrix[diag_lu[row]];
            }
        }
        // restore permutation for block matrix
        if constexpr (is_block) {
            for (Idx row = 0; row != size_; ++row) {
                x[row] = (block_perm_array[row].q * x[row].matrix()).array();
            }
        }
    }
};

} // namespace power_grid_model::math_solver
