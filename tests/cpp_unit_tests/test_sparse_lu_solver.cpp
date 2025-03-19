// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/sparse_lu_solver.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::math_solver {
namespace {
using lu_trait_double = math_solver::sparse_lu_entry_trait<double, double, double>;
static_assert(!lu_trait_double::is_block);
static_assert(lu_trait_double::block_size == 1);
static_assert(std::is_same_v<lu_trait_double::Scalar, double>);

using lu_trait_tensor = math_solver::sparse_lu_entry_trait<Eigen::Array33cd, Eigen::Array3cd, Eigen::Array3cd>;
static_assert(std::is_base_of_v<Eigen::ArrayBase<Eigen::Array33cd>, Eigen::Array33cd>);
static_assert(lu_trait_tensor::is_block);
static_assert(lu_trait_tensor::block_size == 3);
static_assert(std::is_same_v<lu_trait_tensor::Scalar, DoubleComplex>);

template <class T> void check_result(std::vector<T> const& x, std::vector<T> const& x_solver) {
    CHECK(x.size() == x_solver.size());
    for (size_t i = 0; i < x.size(); i++) {
        if constexpr (scalar_value<T>) {
            CHECK(cabs(x[i] - x_solver[i]) < numerical_tolerance);
        } else {
            CHECK((cabs(x[i] - x_solver[i]) < numerical_tolerance).all());
        }
    }
}

// test block calculation with 2*2
using Tensor = Eigen::Array<double, 2, 2, Eigen::ColMajor>;
using Array = Eigen::Array<double, 2, 1, Eigen::ColMajor>;
} // namespace

TEST_CASE("Test Sparse LU solver") {
    // 3 * 3 matrix, with diagonal, two fill-ins
    /// x x x
    /// x x f
    /// x f x

    auto row_indptr = std::make_shared<IdxVector const>(IdxVector{0, 3, 6, 9});
    auto col_indices = std::make_shared<IdxVector const>(IdxVector{0, 1, 2, 0, 1, 2, 0, 1, 2});
    auto diag_lu = std::make_shared<IdxVector const>(IdxVector{0, 4, 8});

    SUBCASE("Scalar(double) calculation") {
        // [4 1 5        3          21
        //  3 7 f     * [-1]   =  [ 2 ]
        //  2 f 6]       2          18
        std::vector<double> data = {
            4, 1, 5, // row 0
            3, 7, 0, // row 1
            2, 0, 6  // row 2
        };
        std::vector<double> const rhs = {21, 2, 18};
        std::vector<double> const x_ref = {3, -1, 2};
        std::vector<double> x(3, 0.0);
        SparseLUSolver<double, double, double> solver{row_indptr, col_indices, diag_lu};
        SparseLUSolver<double, double, double>::BlockPermArray block_perm{};

        SUBCASE("Test calculation") {
            solver.prefactorize_and_solve(data, block_perm, rhs, x);
            check_result(x, x_ref);
        }

        SUBCASE("Test (pseudo) singular") {
            data[0] = 0.0;
            CHECK_THROWS_AS(solver.prefactorize_and_solve(data, block_perm, rhs, x), SparseMatrixError);
        }

        SUBCASE("Test prefactorize") {
            solver.prefactorize(data, block_perm);
            solver.solve_with_prefactorized_matrix((std::vector<double> const&)data, block_perm, rhs, x);
            check_result(x, x_ref);
        }

        SUBCASE("Data is prefactorized by solve") {
            auto prefactorized_data = data;
            auto prefactorized_block_perm = block_perm;
            solver.prefactorize(prefactorized_data, prefactorized_block_perm);
            solver.prefactorize_and_solve(data, block_perm, rhs, x);
            CHECK(prefactorized_data == data);
        }
    }

    SUBCASE("Block(double 2*2) calculation") {
        // [  0 1   1   2   3   4           3             38
        //  100 0   7  -1   5   6           4            356
        //    1 2   0 200   f   f       * [ -1 ]   =  [ -389 ]
        //   -3 4   3   1   f   f       *   -2             2
        //    5 6   f   f   1   0           5             44
        //   -7 8   f   f   0 100           6            611

        //  2 f 6]        2          18
        std::vector<Tensor> data = {
            {{0, 1}, {100, 0}}, // 0, 0
            {{1, 2}, {7, -1}},  // 0, 1
            {{3, 4}, {5, 6}},   // 0, 2
            {{1, 2}, {-3, 4}},  // 1, 0
            {{0, 200}, {3, 1}}, // 1, 1
            {{0, 0}, {0, 0}},   // 1, 2
            {{5, 6}, {-7, 8}},  // 2, 0
            {{0, 0}, {0, 0}},   // 2, 1
            {{1, 0}, {0, 100}}, // 2, 2
        };
        std::vector<Array> const rhs = {{38, 356}, {-389, 2}, {44, 611}};
        std::vector<Array> const x_ref = {{3, 4}, {-1, -2}, {5, 6}};
        std::vector<Array> x(3, Array::Zero());
        SparseLUSolver<Tensor, Array, Array> solver{row_indptr, col_indices, diag_lu};
        SparseLUSolver<Tensor, Array, Array>::BlockPermArray block_perm(3);

        SUBCASE("Test calculation") {
            solver.prefactorize_and_solve(data, block_perm, rhs, x);
            check_result(x, x_ref);
        }
        SUBCASE("Test (pseudo) singular") {
            data[0](0, 1) = 0.0;
            CHECK_THROWS_AS(solver.prefactorize_and_solve(data, block_perm, rhs, x), SparseMatrixError);
        }

        SUBCASE("Test prefactorize") {
            solver.prefactorize(data, block_perm);
            solver.solve_with_prefactorized_matrix((std::vector<Tensor> const&)data, block_perm, rhs, x);
            check_result(x, x_ref);
        }
    }
}

TEST_CASE("LU solver with ill-conditioned system") {
    // test with ill-conditioned matrix if we do not do numerical pivoting
    // 4*4 matrix, or 2*2 with 2*2 blocks
    // [                  [         [
    //   0  0  0  -1       8          0
    //   0 -1  0   0       0          0
    //   0  0  5   1    * 10    =    50
    //  -1  0  1  -9       0          2
    //               ]    ]             ]
    //

    SUBCASE("Scalar variant") {
        auto row_indptr = std::make_shared<IdxVector const>(IdxVector{0, 4, 8, 12, 16});
        auto col_indices = std::make_shared<IdxVector const>(IdxVector{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3});
        auto diag_lu = std::make_shared<IdxVector const>(IdxVector{0, 5, 10, 15});
        auto data = std::vector<double>{
            0,  0,  0, -1, // row 0
            0,  -1, 0, 0,  // row 1
            0,  0,  5, 1,  // row 2
            -1, 0,  1, -9  // row 3
        };
        auto const rhs = std::vector<double>{0, 0, 50, 2};
        auto const x_ref = std::vector<double>{8, 0, 10, 0};
        auto x = std::vector<double>(4, 0.0);

        SparseLUSolver<double, double, double> solver{row_indptr, col_indices, diag_lu};
        Idx perm{};

        SUBCASE("Error without perturbation") {
            CHECK_THROWS_AS(solver.prefactorize(data, perm, false), SparseMatrixError);
        }

        SUBCASE("Success with perturbation") {
            CHECK_NOTHROW(solver.prefactorize(data, perm, true));
            solver.solve_with_prefactorized_matrix(data, perm, rhs, x);
            check_result(x, x_ref);
        }
    }

    SUBCASE("Block variant") {
        auto row_indptr = std::make_shared<IdxVector const>(IdxVector{0, 2, 4});
        auto col_indices = std::make_shared<IdxVector const>(IdxVector{0, 1, 0, 1});
        auto diag_lu = std::make_shared<IdxVector const>(IdxVector{0, 3});
        auto data = std::vector<Tensor>{
            {{0, 0}, {0, -1}}, // 0, 0
            {{0, -1}, {0, 0}}, // 0, 1
            {{0, 0}, {-1, 0}}, // 1, 0
            {{5, 1}, {1, -9}}, // 1, 1
        };
        auto const rhs = std::vector<Array>{{0, 0}, {50, 2}};
        auto const x_ref = std::vector<Array>{{8, 0}, {10, 0}};
        auto x = std::vector<Array>(2, Array::Zero());
        auto block_perm = std::vector<SparseLUSolver<Tensor, Array, Array>::BlockPerm>(2);

        SparseLUSolver<Tensor, Array, Array> solver{row_indptr, col_indices, diag_lu};

        SUBCASE("Error without perturbation") {
            CHECK_THROWS_AS(solver.prefactorize(data, block_perm, false), SparseMatrixError);
        }

        SUBCASE("Success with perturbation") {
            CHECK_NOTHROW(solver.prefactorize(data, block_perm, true));
            solver.solve_with_prefactorized_matrix(data, block_perm, rhs, x);
            check_result(x, x_ref);
        }
    }
}

} // namespace power_grid_model::math_solver
