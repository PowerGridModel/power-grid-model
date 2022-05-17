// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SPARSE_LU_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SPARSE_LU_SOLVER_HPP

#include <memory>

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
        : nnz_{(Idx)data_mapping->size()},
          nnz_lu_{row_indptr->back()},
          row_indptr_{row_indptr},
          col_indices_{col_indices},
          diag_lu_{diag_lu},
          data_mapping_{data_mapping} {
    }

   private:
    Idx nnz_;
    Idx nnz_lu_;
    std::shared_ptr<IdxVector const> row_indptr_;
    std::shared_ptr<IdxVector const> col_indices_;
    std::shared_ptr<IdxVector const> diag_lu_;
    std::shared_ptr<IdxVector const> data_mapping_;
    std::shared_ptr<std::vector<Tensor> const> lu_matrix_;
};

template class SparseLUSolver<DoubleComplex, DoubleComplex, DoubleComplex>;
template class SparseLUSolver<Eigen::Array33cd, Eigen::Array3cd, Eigen::Array3cd>;
template class SparseLUSolver<Eigen::Array22d, Eigen::Array2d, Eigen::Array2d>;
template class SparseLUSolver<Eigen::Array<double, 6, 6>, Eigen::Array<double, 6, 1>, Eigen::Array<double, 6, 1>>;

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif
