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
};

template <class Tensor, class RHSVector, class XVector>
struct sparse_lu_entry_trait<Tensor, RHSVector, XVector, enable_tensor_lu_t<Tensor, RHSVector, XVector>> {
    static constexpr bool is_block = true;
    static constexpr Idx block_size = Tensor::RowsAtCompileTime;
    using Scalar = typename Tensor::Scalar;
};

template <class Tensor, class RHSVector, class XVector>
class SparseLUSolver {
   public:
    using entry_trait = sparse_lu_entry_trait<Tensor, RHSVector, XVector>;
    static constexpr bool is_block = entry_trait::is_block;
    static constexpr Idx block_size = entry_trait::block_size;
    using Scalar = typename entry_trait::Scalar;

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

    void prefactorize(Tensor const* data) {
        // local reference
        auto const& row_indptr = *row_indptr_;
        auto const& col_indices = *col_indices_;
        auto const& diag_lu = *diag_lu_;

        // reset old lu matrix and create new
        prefactorized_ = false;
        lu_matrix_.reset();
        std::vector<Tensor> lu_matrix;
        if constexpr (is_block) {
            lu_matrix.resize(nnz_lu_, Tensor::Zero());
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

        // start pivoting
        for (Idx pivot_row = 0; pivot_row != size_; ++pivot_row) {
            Idx const pivot_idx = diag_lu[pivot_row];
            // inverse pivot
            if constexpr (is_block) {
                Tensor inverse{};
                // direct inverse
                if constexpr (block_size < 5) {
                    bool invertible{};
                    lu_matrix[pivot_idx].matrix().computeInverseWithCheck(inverse, invertible);
                    if (!invertible) {
                        throw SparseMatrixError{};
                    }
                }
                // use full pivot lu
                else {
                    auto lu_fact = lu_matrix[pivot_idx].matrix().fullPivLu();
                    if (lu_fact.rank() < block_size) {
                        throw SparseMatrixError{};
                    }
                    inverse = lu_fact.inverse();
                }
                lu_matrix[pivot_idx] = inverse;
            }
            else {
                if (lu_matrix[pivot_idx] == 0.0) {
                    throw SparseMatrixError{};
                }
                lu_matrix[pivot_idx] = 1.0 / lu_matrix[pivot_idx];
            }
        }

        // move to shared ptr
        lu_matrix_ = std::make_shared<std::vector<Tensor> const>(std::move(lu_matrix));
        prefactorized_ = true;
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
    // diagonals of U are inversed, (cached for later calculation)
    std::shared_ptr<std::vector<Tensor> const> lu_matrix_;
};

template class SparseLUSolver<DoubleComplex, DoubleComplex, DoubleComplex>;
template class SparseLUSolver<Eigen::Array33cd, Eigen::Array3cd, Eigen::Array3cd>;
template class SparseLUSolver<Eigen::Array22d, Eigen::Array2d, Eigen::Array2d>;
template class SparseLUSolver<Eigen::Array<double, 6, 6>, Eigen::Array<double, 6, 1>, Eigen::Array<double, 6, 1>>;

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif
