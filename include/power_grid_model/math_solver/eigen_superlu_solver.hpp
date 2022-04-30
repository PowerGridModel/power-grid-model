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

    struct BSRHandle {
        Idx matrix_size_in_block;
        Idx block_size;
        Idx nnz_block;
        Idx block_capacity;
        Idx nnz;
        Idx matrix_size;
        std::shared_ptr<IdxVector const> data_mapping;
    };

   public:
    EigenSuperLUSolver(Idx matrix_size_in_block,                    // size of matrix in block form
                       Idx block_size,                              // size of block
                       std::shared_ptr<IdxVector const> const& ia,  // indptr
                       std::shared_ptr<IdxVector const> const& ja   // column indices
    ) {
        bsr_handle_.matrix_size_in_block = matrix_size_in_block;
        bsr_handle_.block_size = block_size;
        bsr_handle_.nnz_block = ia->back();
        bsr_handle_.block_capacity = block_size * block_size;
        bsr_handle_.nnz = bsr_handle_.nnz_block * bsr_handle_.block_capacity;
        bsr_handle_.matrix_size = matrix_size_in_block * block_size;
        initialize_matrix(*ia, *ja);
        initialize_solver();
    }

    // copy construction
    EigenSuperLUSolver(EigenSuperLUSolver const& other)
        : bsr_handle_{other.bsr_handle_}, sparse_matrix_{other.sparse_matrix_} {
        initialize_solver();
    }

    // copy assignment
    EigenSuperLUSolver& operator=(EigenSuperLUSolver const& other) {
        // copy new handle
        bsr_handle_ = other.bsr_handle_;
        // copy new matrix
        sparse_matrix_ = other.sparse_matrix_;
        // re initialize
        initialize_solver();
        prefactorized_ = false;
        return *this;
    }

    void solve(void const* data, void* b, void* x, bool use_prefactorization = false) {
        // reset possible pre-factorization if we are not using prefactorization
        prefactorized_ = prefactorized_ && use_prefactorization;
        // run factorization
        if (!prefactorized_) {
            prefactorize(data);
        }
        // run solve
        BufferVector bm{std::launder(reinterpret_cast<T*>(b)), bsr_handle_.matrix_size};
        BufferVector xm{std::launder(reinterpret_cast<T*>(x)), bsr_handle_.matrix_size};
        xm = sparse_solver_.solve(bm);
        if (sparse_solver_.info() != Eigen::Success) {
            throw SparseMatrixError{sparse_solver_.info(), sparse_solver_.lastErrorMessage()};
        }
    }

    void prefactorize(void const* data) {
        // copy data
        copy_matrix_data(data);
        // factorize
        sparse_solver_.factorize(sparse_matrix_);
        if (sparse_solver_.info() != Eigen::Success) {
            throw SparseMatrixError{sparse_solver_.info(), sparse_solver_.lastErrorMessage()};
        }
        prefactorized_ = true;
    }

    void invalidate_prefactorization() {
        prefactorized_ = false;
    }

   private:
    BSRHandle bsr_handle_;
    SparseMatrix sparse_matrix_;
    SparseSolver sparse_solver_;  // not copyable or movable
    bool prefactorized_{false};

    void initialize_matrix(IdxVector const& ia,  // indptr
                           IdxVector const& ja   // column indices
    ) {
        // loop to add triplet
        std::vector<SparseIdxEntry> idx_list;
        idx_list.reserve(bsr_handle_.nnz);
        // two loops for blocks in csr
        for (Idx bi = 0; bi != bsr_handle_.matrix_size_in_block; ++bi) {
            for (Idx block_ind = ia[bi]; block_ind != ia[bi + 1]; ++block_ind) {
                Idx const bj = ja[block_ind];
                // two loops for rows/columns inside block
                for (Idx ci = 0; ci != bsr_handle_.block_size; ++ci) {
                    for (Idx cj = 0; cj != bsr_handle_.block_size; ++cj) {
                        Idx const i = bi * bsr_handle_.block_size + ci;
                        Idx const j = bj * bsr_handle_.block_size + cj;
                        Idx const data_ind = block_ind * bsr_handle_.block_capacity + ci * bsr_handle_.block_size + cj;
                        idx_list.emplace_back(i, j, data_ind);
                    }
                }
            }
        }
        // idx matrix
        SparseIndexMatrix idx_matrix{bsr_handle_.matrix_size, bsr_handle_.matrix_size};
        idx_matrix.setFromTriplets(idx_list.begin(), idx_list.end());
        assert(idx_matrix.isCompressed());
        // copy idx data
        bsr_handle_.data_mapping =
            std::make_shared<IdxVector const>(idx_matrix.valuePtr(), idx_matrix.valuePtr() + bsr_handle_.nnz);
        // initialize sparse matrix
        sparse_matrix_ = idx_matrix.cast<T>();
        assert(sparse_matrix_.isCompressed());
    }

    void initialize_solver() {
        sparse_solver_.analyzePattern(sparse_matrix_);
    }

    void copy_matrix_data(void const* data) {
        T const* const input_ptr = std::launder(reinterpret_cast<T const*>(data));
        T* const data_ptr = sparse_matrix_.valuePtr();
        std::transform(bsr_handle_.data_mapping->begin(), bsr_handle_.data_mapping->end(), data_ptr,
                       [&input_ptr](Idx j) {
                           return input_ptr[j];
                       });
    }
};

}  // namespace power_grid_model

#endif  // defined(POWER_GRID_MODEL_USE_MKL_AT_RUNTIME) || !defined(POWER_GRID_MODEL_USE_MKL)

#endif
