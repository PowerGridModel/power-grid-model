// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/sparse_lu_solver.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/common/typing.hpp>

#include <Eigen/Core>
#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <vector>

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

template <class MatrixActual, class MatrixExpected>
void check_matrix_result(MatrixActual const& actual, MatrixExpected const& expected) {
    REQUIRE(actual.rows() == expected.rows());
    REQUIRE(actual.cols() == expected.cols());
    CHECK((actual - expected).cwiseAbs().maxCoeff() < numerical_tolerance);
}

template <class Tensor>
Eigen::MatrixXd assemble_dense_matrix(IdxVector const& row_indptr, IdxVector const& col_indices,
                                      std::vector<Tensor> const& data) {
    constexpr Idx block_size = Tensor::RowsAtCompileTime;
    Idx const size = static_cast<Idx>(row_indptr.size()) - 1;
    Eigen::MatrixXd matrix = Eigen::MatrixXd::Zero(size * block_size, size * block_size);
    for (Idx row = 0; row < size; ++row) {
        for (Idx idx = row_indptr[row]; idx < row_indptr[row + 1]; ++idx) {
            Idx const col = col_indices[idx];
            matrix.block(row * block_size, col * block_size, block_size, block_size) = data[idx].matrix();
        }
    }
    return matrix;
}

using Matrix3 = Eigen::Matrix<double, 3, 3, Eigen::ColMajor>;
using RowMajorMatrix3 = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>;

std::vector<double> scalar_lu_test_data() {
    // [4 1 5
    //  3 7 f
    //  2 f 6]
    return {
        4.0, 1.0, 5.0, // row 0
        3.0, 7.0, 0.0, // row 1
        2.0, 0.0, 6.0  // row 2
    };
}

Matrix3 scalar_lu_test_matrix() {
    auto const data = scalar_lu_test_data();
    Eigen::Map<RowMajorMatrix3 const> const matrix{data.data()};
    return matrix;
}

// test block calculation with 2*2
using Tensor = Eigen::Array<double, 2, 2, Eigen::ColMajor>;
using Array = Eigen::Array<double, 2, 1, Eigen::ColMajor>;

struct BlockSparseMatrix {
    IdxVector row_indptr;
    IdxVector col_indices;
    IdxVector diag_lu;
    std::vector<Tensor> data;
};

template <class MatrixExpected>
void check_selective_inverse_result(BlockSparseMatrix const& actual_inverse, MatrixExpected const& expected_inverse) {
    constexpr Idx block_size = Tensor::RowsAtCompileTime;
    Idx const size = static_cast<Idx>(actual_inverse.row_indptr.size()) - 1;
    for (Idx row = 0; row < size; ++row) {
        for (Idx idx = actual_inverse.row_indptr[row]; idx < actual_inverse.row_indptr[row + 1]; ++idx) {
            Idx const col = actual_inverse.col_indices[idx];
            auto const expected_block =
                expected_inverse.block(row * block_size, col * block_size, block_size, block_size);
            check_matrix_result(actual_inverse.data[idx].matrix(), expected_block);
        }
    }
}

void check_selective_inverse_fill_in_nonzero(BlockSparseMatrix const& matrix_data, Idx row, Idx col) {
    auto const first = matrix_data.col_indices.begin() + matrix_data.row_indptr[row];
    auto const last = matrix_data.col_indices.begin() + matrix_data.row_indptr[row + 1];
    auto const found = std::lower_bound(first, last, col);
    REQUIRE_MESSAGE(found != last, "Fill-in block must exist in the sparse pattern");
    REQUIRE_MESSAGE(*found == col, "Fill-in block must exist in the sparse pattern");

    Idx const idx = narrow_cast<Idx>(std::distance(matrix_data.col_indices.begin(), found));
    CAPTURE(row);
    CAPTURE(col);
    CHECK_MESSAGE(cabs(matrix_data.data[idx]).maxCoeff() > numerical_tolerance,
                  "Selective inverse should populate preallocated fill-in block");
}

void calculate_selective_inverse(BlockSparseMatrix& matrix_data) {
    SparseLUSolver<Tensor, Array, Array> matrix_solver{matrix_data.row_indptr, matrix_data.col_indices,
                                                       matrix_data.diag_lu};
    SparseLUSolver<Tensor, Array, Array>::BlockPermArray matrix_block_perm(matrix_data.row_indptr.size() - 1);

    matrix_solver.prefactorize(matrix_data.data, matrix_block_perm);
    matrix_solver.inplace_selective_inverse_with_prefactorized_matrix(matrix_data.data, matrix_block_perm);
}

BlockSparseMatrix one_block_requiring_row_and_column_pivoting_lu_test_matrix() {
    return {
        .row_indptr = IdxVector{0, 1},
        .col_indices = IdxVector{0},
        .diag_lu = IdxVector{0},
        // 100 at (1, 1) forces both a row and column swap, so P and Q are not identity.
        .data = std::vector<Tensor>{{{1, 2}, {3, 100}}},
    };
}

BlockSparseMatrix three_block_rows_with_preallocated_fill_ins_lu_test_matrix() {
    // [  0 1   1   2   3   4           3             38
    //  100 0   7  -1   5   6           4            356
    //    1 2   0 200   f   f       * [ -1 ]   =  [ -389 ]
    //   -3 4   3   1   f   f       *   -2             2
    //    5 6   f   f   1   0           5             44
    //   -7 8   f   f   0 100           6            611]
    // corresponding to a chain network with non-optimal ordering, node 1  --  node 0  --  node 2

    return {
        .row_indptr = IdxVector{0, 3, 6, 9},
        .col_indices =
            IdxVector{
                0, 1, 2, // row 0
                0, 1, 2, // row 1
                0, 1, 2  // row 2
            },
        .diag_lu = IdxVector{0, 4, 8},
        .data =
            std::vector<Tensor>{
                {{0, 1}, {100, 0}}, // 0, 0
                {{1, 2}, {7, -1}},  // 0, 1
                {{3, 4}, {5, 6}},   // 0, 2
                {{1, 2}, {-3, 4}},  // 1, 0
                {{0, 200}, {3, 1}}, // 1, 1
                {{0, 0}, {0, 0}},   // 1, 2
                {{5, 6}, {-7, 8}},  // 2, 0
                {{0, 0}, {0, 0}},   // 2, 1
                {{1, 0}, {0, 100}}, // 2, 2
            },
    };
}

BlockSparseMatrix three_block_rows_without_fill_ins_lu_test_matrix() {
    // Same physical matrix as three_block_rows_with_preallocated_fill_ins_lu_test_matrix(),
    // with block order [1, 0, 2] instead of [0, 1, 2].
    // [  0 200   1  2   0   0          -1          -389
    //    3   1  -3  4   0   0          -2             2
    //    1   2   0  1   3   4       * [ 3 ]   =  [  38 ]
    //    7  -1 100  0   5   6           4          356
    //    0   0   5  6   1   0           5           44
    //    0   0  -7  8   0 100           6          611
    // corresponding to a chain network, node 0  --  node 1  --  node 2
    return {
        .row_indptr = IdxVector{0, 2, 5, 7},
        .col_indices =
            IdxVector{
                0, 1,    // row 0
                0, 1, 2, // row 1
                1, 2     // row 2
            },
        .diag_lu = IdxVector{0, 3, 6},
        .data =
            std::vector<Tensor>{
                {{0, 200}, {3, 1}}, // 0, 0
                {{1, 2}, {-3, 4}},  // 0, 1
                {{1, 2}, {7, -1}},  // 1, 0
                {{0, 1}, {100, 0}}, // 1, 1
                {{3, 4}, {5, 6}},   // 1, 2
                {{5, 6}, {-7, 8}},  // 2, 1
                {{1, 0}, {0, 100}}, // 2, 2
            },
    };
}

BlockSparseMatrix four_node_meshed_with_preallocated_fill_in_lu_test_matrix() {
    // [ 20  1   1  0   0  0  -1  2           1             21
    //    2 21   2 -1   0  0   0  1           2             40
    //    0  1  22 -1   2  1   f  f          -1            -17
    //   -2  1   1 23  -1  0   f  f           3             64
    //    0  0   1 -2  24  2  -2  1       * [ 4 ]   =  [   82 ]
    //    0  0   0  1  -1 25   1  2          -2            -47
    //    1  1   f  f   0 -1  26 -2           2             55
    //   -1  2   f  f   2  1   1 27           1             38]
    // corresponding to a meshed network, node 0  --  node 1
    //                                      |          |
    //                                      node 3  --  node 2
    return {
        .row_indptr = IdxVector{0, 3, 7, 10, 14},
        .col_indices =
            IdxVector{
                0, 1, 3,    // row 0
                0, 1, 2, 3, // row 1
                1, 2, 3,    // row 2
                0, 1, 2, 3  // row 3
            },
        .diag_lu = IdxVector{0, 4, 8, 13},
        .data =
            std::vector<Tensor>{
                {{20, 1}, {2, 21}},  // 0, 0
                {{1, 0}, {2, -1}},   // 0, 1
                {{-1, 2}, {0, 1}},   // 0, 3
                {{0, 1}, {-2, 1}},   // 1, 0
                {{22, -1}, {1, 23}}, // 1, 1
                {{2, 1}, {-1, 0}},   // 1, 2
                {{0, 0}, {0, 0}},    // 1, 3
                {{1, -2}, {0, 1}},   // 2, 1
                {{24, 2}, {-1, 25}}, // 2, 2
                {{-2, 1}, {1, 2}},   // 2, 3
                {{1, 1}, {-1, 2}},   // 3, 0
                {{0, 0}, {0, 0}},    // 3, 1
                {{0, -1}, {2, 1}},   // 3, 2
                {{26, -2}, {1, 27}}, // 3, 3
            },
    };
}
} // namespace

TEST_CASE("Dense LU factor") {
    using LUFactor = DenseLUFactor<Matrix3>;

    Matrix3 const lu_matrix{
        {11.0, 12.0, 13.0},
        {2.0, 22.0, 23.0},
        {3.0, 4.0, 33.0},
    };
    Matrix3 const unit_lower{
        {1.0, 0.0, 0.0},
        {2.0, 1.0, 0.0},
        {3.0, 4.0, 1.0},
    };
    Matrix3 const upper{
        {11.0, 12.0, 13.0},
        {0.0, 22.0, 23.0},
        {0.0, 0.0, 33.0},
    };
    Matrix3 const original_rhs{
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0},
    };

    SUBCASE("Left solve with unit lower factor") {
        Matrix3 rhs = original_rhs;

        LUFactor::triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::lower>(lu_matrix, rhs);

        check_matrix_result(unit_lower * rhs, original_rhs);
    }

    SUBCASE("Left solve with upper factor") {
        Matrix3 rhs = original_rhs;

        LUFactor::triangular_solve_inplace<TriangularSolveSide::left, TriangularFactor::upper>(lu_matrix, rhs);

        check_matrix_result(upper * rhs, original_rhs);
    }

    SUBCASE("Right solve with unit lower factor") {
        Matrix3 rhs = original_rhs;

        LUFactor::triangular_solve_inplace<TriangularSolveSide::right, TriangularFactor::lower>(lu_matrix, rhs);

        check_matrix_result(rhs * unit_lower, original_rhs);
    }

    SUBCASE("Right solve with upper factor") {
        Matrix3 rhs = original_rhs;

        LUFactor::triangular_solve_inplace<TriangularSolveSide::right, TriangularFactor::upper>(lu_matrix, rhs);

        check_matrix_result(rhs * upper, original_rhs);
    }

    SUBCASE("Dense inverse") {
        Matrix3 const matrix = scalar_lu_test_matrix();
        Matrix3 const expected_inverse = (Matrix3{
                                              {42.0, -6.0, -35.0},
                                              {-18.0, 14.0, 15.0},
                                              {-14.0, 2.0, 25.0},
                                          } /
                                          80.0); // cofactor matrix divided by determinant

        Matrix3 factorized_matrix = matrix; // renamed from lu_matrix
        LUFactor::BlockPerm block_perm{};
        bool const use_pivot_perturbation = false;
        bool has_pivot_perturbation = false;

        LUFactor::factorize_block_in_place(factorized_matrix, block_perm, epsilon, use_pivot_perturbation,
                                           has_pivot_perturbation);
        Matrix3 const factorized_lu_matrix = factorized_matrix;
        Matrix3 const inverse = LUFactor::dense_inverse(factorized_matrix, block_perm);

        CHECK(has_pivot_perturbation == false);
        check_matrix_result(factorized_matrix, factorized_lu_matrix);
        check_matrix_result(inverse, expected_inverse);
        check_matrix_result(matrix * inverse, Matrix3::Identity());
    }
}

TEST_CASE("Test Sparse LU solver") {
    // 3 * 3 matrix, with diagonal, two fill-ins
    /// x x x
    /// x x f
    /// x f x

    auto row_indptr = IdxVector{0, 3, 6, 9};
    auto col_indices = IdxVector{0, 1, 2, 0, 1, 2, 0, 1, 2};
    auto diag_lu = IdxVector{0, 4, 8};

    SUBCASE("Scalar(double) calculation") {
        // [4 1 5        3          21
        //  3 7 f     * [-1]   =  [ 2 ]
        //  2 f 6]       2          18
        std::vector<double> data = scalar_lu_test_data();
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
            auto const& data_ref = data;
            solver.solve_with_prefactorized_matrix(data_ref, block_perm, rhs, x);
            check_result(x, x_ref);
        }
        // our use case only need selective inversion for block sparse matrices
        SUBCASE("Selective inversion error with scalar sparse matrix") {
            solver.prefactorize(data, block_perm);
            CHECK_THROWS_AS(solver.inplace_selective_inverse_with_prefactorized_matrix(data, block_perm),
                            SparseMatrixError);
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
        auto const matrix = three_block_rows_with_preallocated_fill_ins_lu_test_matrix();
        std::vector<Tensor> data = matrix.data;
        std::vector<Array> const rhs = {{38, 356}, {-389, 2}, {44, 611}};
        std::vector<Array> const x_ref = {{3, 4}, {-1, -2}, {5, 6}};
        std::vector<Array> x(3, Array::Zero());
        SparseLUSolver<Tensor, Array, Array> solver{matrix.row_indptr, matrix.col_indices, matrix.diag_lu};
        SparseLUSolver<Tensor, Array, Array>::BlockPermArray block_perm(matrix.row_indptr.size() - 1);

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
            auto const& data_ref = data;
            solver.solve_with_prefactorized_matrix(data_ref, block_perm, rhs, x);
            check_result(x, x_ref);
        }

        SUBCASE("Selective inverse with prefactorized matrix") {
            SUBCASE("One block agrees with dense inverse") {
                auto matrix_data = one_block_requiring_row_and_column_pivoting_lu_test_matrix();

                using DenseMatrix = Eigen::Matrix<double, 2, 2, Eigen::ColMajor>;
                using LUFactor = DenseLUFactor<DenseMatrix>;
                DenseMatrix dense_lu = matrix_data.data[0].matrix();
                LUFactor::BlockPerm dense_block_perm{};
                bool has_pivot_perturbation = false;
                LUFactor::factorize_block_in_place(dense_lu, dense_block_perm, epsilon, false, has_pivot_perturbation);
                DenseMatrix const expected_inverse = LUFactor::dense_inverse(dense_lu, dense_block_perm);

                calculate_selective_inverse(matrix_data);

                CHECK_FALSE(has_pivot_perturbation);
                check_matrix_result(matrix_data.data[0].matrix(), expected_inverse);
            }

            SUBCASE("Three block rows with preallocated fill-ins") {
                auto matrix_data = three_block_rows_with_preallocated_fill_ins_lu_test_matrix();
                Eigen::MatrixXd const expected_inverse =
                    assemble_dense_matrix(matrix_data.row_indptr, matrix_data.col_indices, matrix_data.data).inverse();

                calculate_selective_inverse(matrix_data);

                check_selective_inverse_fill_in_nonzero(matrix_data, 1, 2);
                check_selective_inverse_fill_in_nonzero(matrix_data, 2, 1);
                check_selective_inverse_result(matrix_data, expected_inverse);
            }

            SUBCASE("Three block rows without fill-ins") {
                auto matrix_data = three_block_rows_without_fill_ins_lu_test_matrix();
                Eigen::MatrixXd const expected_inverse =
                    assemble_dense_matrix(matrix_data.row_indptr, matrix_data.col_indices, matrix_data.data).inverse();

                calculate_selective_inverse(matrix_data);

                check_selective_inverse_result(matrix_data, expected_inverse);
            }

            SUBCASE("Four-node meshed network with preallocated fill-in") {
                auto matrix_data = four_node_meshed_with_preallocated_fill_in_lu_test_matrix();
                Eigen::MatrixXd const expected_inverse =
                    assemble_dense_matrix(matrix_data.row_indptr, matrix_data.col_indices, matrix_data.data).inverse();

                calculate_selective_inverse(matrix_data);

                check_selective_inverse_fill_in_nonzero(matrix_data, 1, 3);
                check_selective_inverse_fill_in_nonzero(matrix_data, 3, 1);
                check_selective_inverse_result(matrix_data, expected_inverse);
            }
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
        auto row_indptr = IdxVector{0, 4, 8, 12, 16};
        auto col_indices = IdxVector{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
        auto diag_lu = IdxVector{0, 5, 10, 15};
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

        SUBCASE("Selective inversion error with perturbation") {
            solver.prefactorize(data, perm, true);
            CHECK_THROWS_AS(solver.inplace_selective_inverse_with_prefactorized_matrix(data, perm), SparseMatrixError);
        }
    }

    SUBCASE("Block variant") {
        auto row_indptr = IdxVector{0, 2, 4};
        auto col_indices = IdxVector{0, 1, 0, 1};
        auto diag_lu = IdxVector{0, 3};
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

        SUBCASE("Selective inversion error with perturbation") {
            solver.prefactorize(data, block_perm, true);
            CHECK_THROWS_AS(solver.inplace_selective_inverse_with_prefactorized_matrix(data, block_perm),
                            SparseMatrixError);
        }
    }
}

} // namespace power_grid_model::math_solver
