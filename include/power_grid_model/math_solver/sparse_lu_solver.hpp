// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SPARSE_LU_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SPARSE_LU_SOLVER_HPP

#include <memory>

#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

template <class Tensor, class RHSVector, class XVector, class = void>
struct sparse_lu_entry_trait;

template <class Tensor, class RHSVector, class XVector>
using enable_scalar_lu_t =
    std::enable_if_t<std::is_same_v<Tensor, RHSVector> && std::is_same_v<Tensor, XVector> && check_scalar_v<Tensor>>;

template <class Tensor, class RHSVector, class XVector>
using enable_tensor_lu_t = std::enable_if_t<
    std::is_base_of_v<Eigen::ArrayBase<Tensor>, Tensor> &&                  // tensor should be an eigen array
    std::is_base_of_v<Eigen::ArrayBase<RHSVector>, RHSVector> &&            // rhs vector should be an eigen array
    std::is_base_of_v<Eigen::ArrayBase<XVector>, XVector> &&                // x vector should be an eigen array
    Tensor::RowsAtCompileTime == Tensor::ColsAtCompileTime &&               // tensor should be square
    RHSVector::ColsAtCompileTime == 1 &&                                    // rhs vector should be column vector
    RHSVector::RowsAtCompileTime == Tensor::RowsAtCompileTime &&            // rhs vector should be column vector
    XVector::ColsAtCompileTime == 1 &&                                      // x vector should be column vector
    XVector::RowsAtCompileTime == Tensor::RowsAtCompileTime &&              // x vector should be column vector
    std::is_same_v<typename Tensor::Scalar, typename RHSVector::Scalar> &&  // all entries should have same scalar type
    std::is_same_v<typename Tensor::Scalar, typename XVector::Scalar> &&    // all entries should have same scalar type
    check_scalar_v<typename Tensor::Scalar>>;  // scalar can only be double or complex double

template <class Tensor, class RHSVector, class XVector>
struct sparse_lu_entry_trait<Tensor, RHSVector, XVector, enable_scalar_lu_t<Tensor, RHSVector, XVector>> {
    static constexpr bool is_block = false;
    static constexpr Idx block_size = 1;
    using Scalar = Tensor;
    using LUFactor = int;
    using LUFactorArray = int;
};

template <class Tensor, class RHSVector, class XVector>
struct sparse_lu_entry_trait<Tensor, RHSVector, XVector, enable_tensor_lu_t<Tensor, RHSVector, XVector>> {
    static constexpr bool is_block = true;
    static constexpr Idx block_size = Tensor::RowsAtCompileTime;
    using Scalar = typename Tensor::Scalar;
    using LUFactor = Eigen::FullPivLU<Eigen::Ref<Eigen::Matrix<Scalar, block_size, block_size, Tensor::Options>>>;
    using LUFactorArray = std::vector<LUFactor>;
};

template <class Tensor, class RHSVector, class XVector>
class SparseLUSolver {
   public:
    using entry_trait = sparse_lu_entry_trait<Tensor, RHSVector, XVector>;
    static constexpr bool is_block = entry_trait::is_block;
    static constexpr Idx block_size = entry_trait::block_size;
    using Scalar = typename entry_trait::Scalar;
    using LUFactor = typename entry_trait::LUFactor;
    using LUFactorArray = typename entry_trait::LUFactorArray;

    SparseLUSolver(std::shared_ptr<IdxVector const> const& row_indptr,
                   std::shared_ptr<IdxVector const> const& col_indices, std::shared_ptr<IdxVector const> const& diag_lu,
                   std::shared_ptr<IdxVector const> const& data_mapping)
        : size_{(Idx)row_indptr->size() - 1},
          nnz_{(Idx)data_mapping->size()},
          nnz_lu_{row_indptr->back()},
          row_indptr_{row_indptr},
          col_indices_{col_indices},
          diag_lu_{diag_lu},
          data_mapping_{data_mapping} {
    }

    void solve(Tensor const* data, RHSVector const* rhs, XVector* x, bool use_prefactorization = false) {
        // reset possible pre-factorization if we are not using prefactorization
        prefactorized_ = prefactorized_ && use_prefactorization;
        // run factorization
        if (!prefactorized_) {
            prefactorize(data);
        }

        // local reference
        auto const& row_indptr = *row_indptr_;
        auto const& col_indices = *col_indices_;
        auto const& diag_lu = *diag_lu_;
        auto const& lu_matrix = *lu_matrix_;
        auto const& diag_lu_factor = *diag_lu_factor_;

        // forward substitution with L
        for (Idx row = 0; row != size_; ++row) {
            x[row] = rhs[row];
            // loop all columns until diagonal
            for (Idx col_idx = row_indptr[row]; col_idx < diag_lu[row]; ++col_idx) {
                Idx const col = col_indices[col_idx];
                // never overshoot
                assert(col < row);
                // forward subtract
                x[row] -= dot(lu_matrix[col_idx], x[col]);
            }
        }

        // backward substitution with U
        for (Idx row = size_ - 1; row != -1; --row) {
            // loop all columns from diagonal
            for (Idx col_idx = diag_lu[row] + 1; col_idx < row_indptr[row + 1]; ++col_idx) {
                Idx const col = col_indices[col_idx];
                // always in upper diagonal
                assert(col > row);
                // backward subtract
                x[row] -= dot(lu_matrix[col_idx], x[col]);
            }
            // solve the diagonal pivot
            if constexpr (is_block) {
                x[row] = diag_lu_factor[row].solve(x[row].matrix());
            }
            else {
                x[row] = x[row] / lu_matrix[diag_lu[row]];
            }
        }
    }

    void prefactorize(Tensor const* data) {
        // local reference
        auto const& row_indptr = *row_indptr_;
        auto const& col_indices = *col_indices_;
        auto const& diag_lu = *diag_lu_;

        // reset old lu matrix and create new
        prefactorized_ = false;
        lu_matrix_.reset();
        diag_lu_factor_.reset();
        std::vector<Tensor> lu_matrix;
        LUFactorArray diag_lu_factor{};
        if constexpr (is_block) {
            lu_matrix.resize(nnz_lu_, Tensor::Zero());
            diag_lu_factor.reserve(size_);
        }
        else {
            lu_matrix.resize(nnz_lu_, Scalar{});
        }
        // copy data
        for (Idx i = 0; i != nnz_; ++i) {
            lu_matrix[(*data_mapping_)[i]] = data[i];
        }

        // column position idx per row for L matrix
        IdxVector col_position_idx(row_indptr.cbegin(), row_indptr.cend() - 1);

        // start pivoting, it is always the diagonal
        for (Idx pivot_row_col = 0; pivot_row_col != size_; ++pivot_row_col) {
            Idx const pivot_idx = diag_lu[pivot_row_col];

            // Dense LU factorize pivot for block matrix
            if constexpr (is_block) {
                assert((Idx)diag_lu_factor.size() == pivot_row_col);
                diag_lu_factor.emplace_back(lu_matrix[pivot_idx]);
                if (diag_lu_factor[pivot_row_col].rank() < block_size) {
                    throw SparseMatrixError{};
                }
            }

            // start to calculate L below the pivot and U at the right of the pivot column
            // because the matrix is symmetric,
            //    looking for col_indices at pivot_row_col, starting from the diagonal (pivot_row_col, pivot_row_col)
            //    we get also the non-zero row indices under the pivot
            for (Idx l_row_idx = pivot_idx + 1; l_row_idx < row_indptr[pivot_row_col + 1]; ++l_row_idx) {
                Idx const l_row = col_indices[l_row_idx];
                // we should exactly find the current column
                Idx l_col_idx = col_position_idx[l_row];
                assert(col_indices[l_col_idx] == pivot_row_col);
                // calculating l at (l_row, pivot_row_col)
                if constexpr (is_block) {
                    lu_matrix[l_col_idx] = diag_lu_factor[pivot_row_col].solve(lu_matrix[l_col_idx].matrix());
                }
                else {
                    lu_matrix[l_col_idx] = lu_matrix[l_col_idx] / lu_matrix[pivot_idx];
                }
                Tensor const l = lu_matrix[l_col_idx];
                // for all entries in the right of (l_row, pivot_row_col)
                //       (l_row, pivot_col) = (l_row, pivot_col) - l * (pivot_row_col, pivot_col), for pivot_col >
                //       pivot_row_col
                // it can create fill-ins, but the fill-ins are pre-allocated
                // it is garanteed to have an entry at (l_row, pivot_col), if (pivot_row_col, pivot_col) is non-zero
                // loop all columns in the right of (pivot_row_col, pivot_row_col), at pivot_row
                for (Idx pivot_col_idx = pivot_idx + 1; pivot_col_idx < row_indptr[pivot_row_col + 1];
                     ++pivot_col_idx) {
                    Idx const pivot_col = col_indices[pivot_col_idx];
                    // search the l_col_idx to the pivot_col,
                    while (col_indices[l_col_idx] != pivot_col) {
                        ++l_col_idx;
                        // it should always exist, so no overshooting of the end of the row
                        assert(l_col_idx < row_indptr[l_row + 1]);
                    }
                    // subtract
                    lu_matrix[l_col_idx] -= dot(l, lu_matrix[pivot_col_idx]);
                }
                // iterate column position
                ++col_position_idx[l_row];
            }
        }

        // move to shared ptr
        lu_matrix_ = std::make_shared<std::vector<Tensor> const>(std::move(lu_matrix));
        diag_lu_factor_ = std::make_shared<LUFactorArray const>(std::move(diag_lu_factor));
        prefactorized_ = true;
    }

    void invalidate_prefactorization() {
        prefactorized_ = false;
    }

   private:
    Idx size_;
    Idx nnz_;
    Idx nnz_lu_;
    bool prefactorized_{false};
    std::shared_ptr<IdxVector const> row_indptr_;
    std::shared_ptr<IdxVector const> col_indices_;
    std::shared_ptr<IdxVector const> diag_lu_;
    std::shared_ptr<IdxVector const> data_mapping_;
    // the LU matrix has the form A = L * U
    // diagonals of L are one
    // diagonals of U have values
    std::shared_ptr<std::vector<Tensor> const> lu_matrix_;
    // dense LU factor of diagonals of LU matrix
    // only applicable for block matrix
    std::shared_ptr<LUFactorArray const> diag_lu_factor_;
};

}  // namespace math_model_impl

template <class Tensor, class RHSVector, class XVector>
using SparseLUSolver = math_model_impl::SparseLUSolver<Tensor, RHSVector, XVector>;

}  // namespace power_grid_model

#endif
