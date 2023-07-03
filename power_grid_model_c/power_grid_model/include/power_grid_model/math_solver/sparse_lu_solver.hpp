// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SPARSE_LU_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SPARSE_LU_SOLVER_HPP

#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

#include <memory>

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

template <class Tensor, class RHSVector, class XVector, class = void>
struct sparse_lu_entry_trait;

template <class Tensor, class RHSVector, class XVector>
concept scalar_value_lu = scalar_value<Tensor> && std::same_as<Tensor, RHSVector> && std::same_as<Tensor, XVector>;

// TODO(mgovers) improve this concept
template <class Derived>
int check_array_base(Eigen::ArrayBase<Derived> const&) {
    return 0;
}
template <class ArrayLike>
concept eigen_array = std::same_as<decltype(check_array_base(ArrayLike{})), int>;  // should be an eigen array

template <class LHSArrayLike, class RHSArrayLike>
concept matrix_multiplicable = eigen_array<LHSArrayLike> && eigen_array<RHSArrayLike> &&
    (static_cast<Idx>(LHSArrayLike::ColsAtCompileTime) == static_cast<Idx>(RHSArrayLike::RowsAtCompileTime));

template <class Tensor, class RHSVector, class XVector>
concept tensor_lu = rk2_tensor<Tensor> && column_vector<RHSVector> && column_vector<XVector> &&
    matrix_multiplicable<Tensor, RHSVector> && matrix_multiplicable<Tensor, XVector> &&
    std::same_as<typename Tensor::Scalar, typename RHSVector::Scalar> &&  // all entries should have same scalar type
    std::same_as<typename Tensor::Scalar, typename XVector::Scalar> &&    // all entries should have same scalar type
    scalar_value<typename Tensor::Scalar>;                                // scalar can only be double or complex double

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
    using LUFactor = Eigen::FullPivLU<Eigen::Ref<Matrix>>;  // LU decomposition with full pivoting in place
    struct BlockPerm {
        typename LUFactor::PermutationPType p;
        typename LUFactor::PermutationQType q;
    };  // Extract permutation matrices p and q from LUFactor
    using BlockPermArray = std::vector<BlockPerm>;
};

template <class Tensor, class RHSVector, class XVector>
class SparseLUSolver {
   public:
    using entry_trait = sparse_lu_entry_trait<Tensor, RHSVector, XVector>;
    static constexpr bool is_block = entry_trait::is_block;
    static constexpr Idx block_size = entry_trait::block_size;
    using Scalar = typename entry_trait::Scalar;
    using LUFactor = typename entry_trait::LUFactor;
    using BlockPerm = typename entry_trait::BlockPerm;
    using BlockPermArray = typename entry_trait::BlockPermArray;

    SparseLUSolver(std::shared_ptr<IdxVector const> const& row_indptr,   // indptr including fill-ins
                   std::shared_ptr<IdxVector const> const& col_indices,  // indices including fill-ins
                   std::shared_ptr<IdxVector const> const& diag_lu)
        : size_{(Idx)row_indptr->size() - 1},
          nnz_{row_indptr->back()},
          row_indptr_{row_indptr},
          col_indices_{col_indices},
          diag_lu_{diag_lu} {
    }

    // solve with new matrix data, need to factorize first
    void prefactorize_and_solve(
        std::vector<Tensor>& data,         // matrix data, factorize in-place
        BlockPermArray& block_perm_array,  // pre-allocated permutation array, will be overwritten
        std::vector<RHSVector> const& rhs, std::vector<XVector>& x) {
        prefactorize(data, block_perm_array);
        // call solve with const method
        solve_with_prefactorized_matrix((std::vector<Tensor> const&)data, block_perm_array, rhs, x);
    }

    // solve with existing pre-factorization
    void solve_with_prefactorized_matrix(
        std::vector<Tensor> const& data,         // pre-factoirzed data, const ref
        BlockPermArray const& block_perm_array,  // pre-calculated permutation, const ref
        std::vector<RHSVector> const& rhs, std::vector<XVector>& x) {
        // local reference
        auto const& row_indptr = *row_indptr_;
        auto const& col_indices = *col_indices_;
        auto const& diag_lu = *diag_lu_;
        auto const& lu_matrix = data;

        // forward substitution with L
        for (Idx row = 0; row != size_; ++row) {
            // permutation if needed
            if constexpr (is_block) {
                x[row] = (block_perm_array[row].p * rhs[row].matrix()).array();
            }
            else {
                x[row] = rhs[row];
            }

            // loop all columns until diagonal
            for (Idx l_idx = row_indptr[row]; l_idx < diag_lu[row]; ++l_idx) {
                Idx const col = col_indices[l_idx];
                // never overshoot
                assert(col < row);
                // forward subtract
                x[row] -= dot(lu_matrix[l_idx], x[col]);
            }
            // forward substitution inside block, for block matrix
            if constexpr (is_block) {
                XVector& xb = x[row];
                Tensor const& pivot = lu_matrix[diag_lu[row]];
                for (Idx br = 0; br < block_size; ++br) {
                    for (Idx bc = 0; bc < br; ++bc) {
                        xb(br) -= pivot(br, bc) * xb(bc);
                    }
                }
            }
        }

        // backward substitution with U
        for (Idx row = size_ - 1; row != -1; --row) {
            // loop all columns from diagonal
            for (Idx u_idx = row_indptr[row + 1] - 1; u_idx > diag_lu[row]; --u_idx) {
                Idx const col = col_indices[u_idx];
                // always in upper diagonal
                assert(col > row);
                // backward subtract
                x[row] -= dot(lu_matrix[u_idx], x[col]);
            }
            // solve the diagonal pivot
            if constexpr (is_block) {
                // backward substitution inside block
                XVector& xb = x[row];
                Tensor const& pivot = lu_matrix[diag_lu[row]];
                for (Idx br = block_size - 1; br != -1; --br) {
                    for (Idx bc = block_size - 1; bc > br; --bc) {
                        xb(br) -= pivot(br, bc) * xb(bc);
                    }
                    xb(br) = xb(br) / pivot(br, br);
                }
            }
            else {
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

    // prefactorize in-place
    // the LU matrix has the form A = L * U
    // diagonals of L are one
    // diagonals of U have values
    // fill-ins should be pre-allocated with zero
    // block permutation array should be pre-allocated
    void prefactorize(std::vector<Tensor>& data, BlockPermArray& block_perm_array) {
        // local reference
        auto const& row_indptr = *row_indptr_;
        auto const& col_indices = *col_indices_;
        auto const& diag_lu = *diag_lu_;
        // lu matrix inplace
        std::vector<Tensor>& lu_matrix = data;

        // column position idx per row for LU matrix
        IdxVector col_position_idx(row_indptr.cbegin(), row_indptr.cend() - 1);

        // start pivoting, it is always the diagonal
        for (Idx pivot_row_col = 0; pivot_row_col != size_; ++pivot_row_col) {
            Idx const pivot_idx = diag_lu[pivot_row_col];

            // Dense LU factorize pivot for block matrix in-place
            // A_pivot,pivot, becomes P_pivot^-1 * L_pivot * U_pivot * Q_pivot^-1
            // return reference to pivot permutation
            BlockPerm const& block_perm = [&]() -> std::conditional_t<is_block, BlockPerm const&, BlockPerm> {
                if constexpr (is_block) {
                    LUFactor lu_factor(lu_matrix[pivot_idx]);
                    // set a low threshold, because state estimation can have large differences in eigen values
                    lu_factor.setThreshold(1e-100);
                    if (lu_factor.rank() < block_size) {
                        throw SparseMatrixError{};
                    }
                    // record block permutation
                    block_perm_array[pivot_row_col] = {lu_factor.permutationP(), lu_factor.permutationQ()};
                    return block_perm_array[pivot_row_col];
                }
                else {
                    if (lu_matrix[pivot_idx] == 0.0) {
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
                for (Idx l_idx = row_indptr[pivot_row_col]; l_idx < pivot_idx; ++l_idx) {
                    // permute rows of L_k,pivot
                    lu_matrix[l_idx] = (block_perm.p * lu_matrix[l_idx].matrix()).array();
                    // get row and idx of u
                    Idx const u_row = col_indices[l_idx];
                    Idx const u_idx = col_position_idx[u_row];
                    // we should exactly find the current column
                    assert(col_indices[u_idx] == pivot_row_col);
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
                for (Idx u_idx = pivot_idx + 1; u_idx < row_indptr[pivot_row_col + 1]; ++u_idx) {
                    Tensor& u = lu_matrix[u_idx];
                    // permutation
                    u = (block_perm.p * u.matrix()).array();
                    // forward substitution, per row in u
                    for (Idx block_row = 0; block_row < block_size; ++block_row) {
                        for (Idx block_col = 0; block_col < block_row; ++block_col) {
                            // forward substract
                            u.row(block_row) -= pivot(block_row, block_col) * u.row(block_col);
                        }
                    }
                }
            }

            // start to calculate L below the pivot and U at the right of the pivot column
            // because the matrix is symmetric,
            //    looking for col_indices at pivot_row_col, starting from the diagonal (pivot_row_col, pivot_row_col)
            //    we get also the non-zero row indices under the pivot
            for (Idx l_ref_idx = pivot_idx + 1; l_ref_idx < row_indptr[pivot_row_col + 1]; ++l_ref_idx) {
                // find index of l in corresponding row
                Idx const l_row = col_indices[l_ref_idx];
                Idx const l_idx = col_position_idx[l_row];
                // we should exactly find the current column
                assert(col_indices[l_idx] == pivot_row_col);
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
                    for (Idx block_col = 0; block_col < block_size; ++block_col) {
                        for (Idx block_row = 0; block_row < block_col; ++block_row) {
                            l.col(block_col) -= pivot(block_row, block_col) * l.col(block_row);
                        }
                        // divide diagonal
                        l.col(block_col) = l.col(block_col) / pivot(block_col, block_col);
                    }
                }
                else {
                    // for scalar matrix, just divide
                    // L_k,pivot = A_k,pivot / U_pivot    k > pivot
                    lu_matrix[l_idx] = lu_matrix[l_idx] / pivot;
                }
                Tensor const& l = lu_matrix[l_idx];

                // for all entries in the right of (l_row, u_col)
                //       A(l_row, u_col) = A(l_row, u_col) - l * U(pivot_row_col, u_col),
                //          for u_col > pivot_row_col
                // it can create fill-ins, but the fill-ins are pre-allocated
                // it is garanteed to have an entry at (l_row, u_col), if (pivot_row_col, u_col) is non-zero
                // starting A index from (l_row, pivot_row_col)
                Idx a_idx = l_idx;
                // loop all columns in the right of (pivot_row_col, pivot_row_col), at pivot_row
                for (Idx u_idx = pivot_idx + 1; u_idx < row_indptr[pivot_row_col + 1]; ++u_idx) {
                    Idx const u_col = col_indices[u_idx];
                    assert(u_col > pivot_row_col);
                    // search the a_idx to the u_col,
                    auto const found = std::lower_bound(col_indices.cbegin() + a_idx,
                                                        col_indices.cbegin() + row_indptr[l_row + 1], u_col);
                    // should always found
                    assert(found != col_indices.cbegin() + row_indptr[l_row + 1]);
                    assert(*found == u_col);
                    a_idx = (Idx)std::distance(col_indices.cbegin(), found);
                    // subtract
                    lu_matrix[a_idx] -= dot(l, lu_matrix[u_idx]);
                }
                // iterate column position
                ++col_position_idx[l_row];
            }
            // iterate column position for the pivot
            ++col_position_idx[pivot_row_col];
        }
    }

   private:
    Idx size_;
    Idx nnz_;  // number of non zeroes (in block)
    std::shared_ptr<IdxVector const> row_indptr_;
    std::shared_ptr<IdxVector const> col_indices_;
    std::shared_ptr<IdxVector const> diag_lu_;
};

}  // namespace math_model_impl

template <class Tensor, class RHSVector, class XVector>
using SparseLUSolver = math_model_impl::SparseLUSolver<Tensor, RHSVector, XVector>;

}  // namespace power_grid_model

#endif
