// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_EIGEN_SUPERLU_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_EIGEN_SUPERLU_SOLVER_HPP
// BSR solver using Eigen SuperLU
// https://eigen.tuxfamily.org/dox/group__Sparse__chapter.html

// only enable if POWER_GRID_MODEL_USE_MKL_AT_RUNTIME is defined
//  or POWER_GRID_MODEL_USE_MKL is not defined
#if defined(POWER_GRID_MODEL_USE_MKL_AT_RUNTIME) || !defined(POWER_GRID_MODEL_USE_MKL)

#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <sstream>

#include "../exception.hpp"
#include "../power_grid_model.hpp"

namespace power_grid_model {

template <class T>
class EigenSuperLUSolver final {
   private:
    static_assert(std::is_same_v<T, double> || std::is_same_v<T, DoubleComplex>);
    using SparseMatrix = Eigen::SparseMatrix<T, Eigen::ColMajor, Idx>;
    using SparseIndexMatrix = Eigen::SparseMatrix<Idx, Eigen::ColMajor, Idx>;
    using SparseSolver = Eigen::SparseLU<SparseMatrix, Eigen::NaturalOrdering<Idx>>;
    using BufferVector = Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, 1>, Eigen::Aligned8>;
    using SparseIdxEntry = Eigen::Triplet<Idx, Idx>;

   public:
    EigenSuperLUSolver(Idx matrix_size_in_block,                    // size of matrix in block form
                       Idx block_size,                              // size of block
                       std::shared_ptr<IdxVector const> const& ia,  // indptr
                       std::shared_ptr<IdxVector const> const& ja   // column indices
                       )
        : matrix_size_in_block_{matrix_size_in_block},
          block_size_{block_size},
          nnz_block_{ia->back()},
          block_capacity_{block_size_ * block_size_},
          nnz_{nnz_block_ * block_capacity_},
          matrix_size_{matrix_size_in_block_ * block_size_} {
        initialize_solver(*ia, *ja);
    }

    void solve(void const* data, void* b, void* x, bool use_prefactorization = false) {
        (void)use_prefactorization;  // suppress unused variable warning
        // copy data
        copy_matrix_data(data);
        BufferVector bm{std::launder(reinterpret_cast<T*>(b)), matrix_size_};
        BufferVector xm{std::launder(reinterpret_cast<T*>(x)), matrix_size_};
        SparseSolver sparse_solver;
        sparse_solver.analyzePattern(sparse_matrix_);
        sparse_solver.factorize(sparse_matrix_);
        if (sparse_solver.info() != Eigen::Success) {
            std::stringstream ss;
            ss << sparse_matrix_;
            std::string msg = sparse_solver.lastErrorMessage();
            msg += "Matrix content: \n" + ss.str() + "\n";
            throw SparseMatrixError{sparse_solver.info(), msg};
        }
        xm = sparse_solver.solve(bm);
        if (sparse_solver.info() != Eigen::Success) {
            throw SparseMatrixError{sparse_solver.info(), sparse_solver.lastErrorMessage()};
        }
    }

    // empty prefactorize function
    void prefactorize(void const* data) {
        (void)data;
    }

    void invalidate_prefactorization() {
    }

   private:
    Idx matrix_size_in_block_;
    Idx block_size_;
    Idx nnz_block_;
    Idx block_capacity_;
    Idx nnz_;
    Idx matrix_size_;
    SparseMatrix sparse_matrix_;
    // SparseSolver sparse_solver_;  // not copyable or movable
    std::shared_ptr<IdxVector const> data_mapping_;

    void initialize_solver(IdxVector const& ia,  // indptr
                           IdxVector const& ja   // column indices
    ) {
        // loop to add triplet
        std::vector<SparseIdxEntry> idx_list;
        idx_list.reserve(nnz_);
        // two loops for blocks in csr
        for (Idx bi = 0; bi != matrix_size_in_block_; ++bi) {
            for (Idx block_ind = ia[bi]; block_ind != ia[bi + 1]; ++block_ind) {
                Idx const bj = ja[block_ind];
                // two loops for rows/columns inside block
                for (Idx ci = 0; ci != block_size_; ++ci) {
                    for (Idx cj = 0; cj != block_size_; ++cj) {
                        Idx const i = bi * block_size_ + ci;
                        Idx const j = bj * block_size_ + cj;
                        Idx const data_ind = block_ind * block_capacity_ + ci * block_size_ + cj;
                        idx_list.emplace_back(i, j, data_ind);
                    }
                }
            }
        }
        // idx matrix
        SparseIndexMatrix idx_matrix{matrix_size_, matrix_size_};
        idx_matrix.setFromTriplets(idx_list.begin(), idx_list.end());
        assert(idx_matrix.isCompressed());
        // copy idx data
        data_mapping_ = std::make_shared<IdxVector const>(idx_matrix.valuePtr(), idx_matrix.valuePtr() + nnz_);
        // initialize sparse matrix
        sparse_matrix_ = idx_matrix.cast<T>();
        assert(sparse_matrix_.isCompressed());
    }

    void copy_matrix_data(void const* data) {
        T const* const input_ptr = std::launder(reinterpret_cast<T const*>(data));
        T* const data_ptr = sparse_matrix_.valuePtr();
        std::transform(data_mapping_->begin(), data_mapping_->end(), data_ptr, [&input_ptr](Idx j) {
            return input_ptr[j];
        });
    }
};

}  // namespace power_grid_model

#endif  // defined(POWER_GRID_MODEL_USE_MKL_AT_RUNTIME) || !defined(POWER_GRID_MODEL_USE_MKL)

#endif
