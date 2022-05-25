// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "catch2/catch.hpp"
#include "power_grid_model/math_solver/sparse_lu_solver.hpp"
#include "power_grid_model/three_phase_tensor.hpp"

namespace power_grid_model {

using lu_trait_double = math_model_impl::sparse_lu_entry_trait<double, double, double>;
static_assert(!lu_trait_double::is_block);
static_assert(lu_trait_double::block_size == 1);
static_assert(std::is_same_v<lu_trait_double::Scalar, double>);

using lu_trait_tensor = math_model_impl::sparse_lu_entry_trait<Eigen::Array33cd, Eigen::Array3cd, Eigen::Array3cd>;
static_assert(std::is_base_of_v<Eigen::ArrayBase<Eigen::Array33cd>, Eigen::Array33cd>);
static_assert(lu_trait_tensor::is_block);
static_assert(lu_trait_tensor::block_size == 3);
static_assert(std::is_same_v<lu_trait_tensor::Scalar, DoubleComplex>);

template <class T>
void check_result(std::vector<T> const& x, std::vector<T> const& x_solver) {
    CHECK(x.size() == x_solver.size());
    for (size_t i = 0; i < x.size(); i++) {
        if constexpr (check_scalar_v<T>) {
            CHECK(cabs(x[i] - x_solver[i]) < numerical_tolerance);
        }
        else {
            CHECK((cabs(x[i] - x_solver[i]) < numerical_tolerance).all());
        }
    }
}

// test block calculation with 2*2
using Tensor = Eigen::Array<double, 2, 2, Eigen::RowMajor>;
using Array = Eigen::Array<double, 2, 1, Eigen::ColMajor>;

TEST_CASE("Test Sparse LU solver") {
    // 3 * 3 matrix, with diagonal, two fill-ins
    /// x x x
    /// x x f
    /// x f x

    auto row_indptr = std::make_shared<IdxVector const>(IdxVector{0, 3, 6, 9});
    auto col_indices = std::make_shared<IdxVector const>(IdxVector{0, 1, 2, 0, 1, 2, 0, 1, 2});
    auto diag_lu = std::make_shared<IdxVector const>(IdxVector{0, 4, 8});
    auto data_mapping = std::make_shared<IdxVector const>(IdxVector{0, 1, 2, 3, 4, 6, 8});

    SECTION("Test scalar(double) calculation") {
        // [4 1 5        3          21
        //  3 7 f     * [-1]   =  [ 2 ]
        //  2 f 6]       2          18
        std::vector<double> data = {
            4, 1, 5,  // row 0
            3, 7,     // row 1
            2, 6      // row 2
        };
        std::vector<double> rhs = {21, 2, 18};
        std::vector<double> x_ref = {3, -1, 2};
        std::vector<double> x(3, 0.0);
        SparseLUSolver<double, double, double> solver{row_indptr, col_indices, diag_lu, data_mapping};

        SECTION("Test calculation") {
            solver.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
        }

        SECTION("Test copy and move") {
            SparseLUSolver<double, double, double> s1{solver};
            // copy construction
            s1.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
            // copy assignment
            s1 = solver;
            s1.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
            // self assignment
            auto& s2 = s1;
            s1 = s2;
            s1.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
            // move construction
            SparseLUSolver<double, double, double> s3{std::move(solver)};
            s3.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
        }

        SECTION("Test (pseudo) singular") {
            data[0] = 0.0;
            CHECK_THROWS_AS(solver.solve(data.data(), rhs.data(), x.data()), SparseMatrixError);
        }

        SECTION("Test prefactorize") {
            solver.prefactorize(data.data());
            solver.solve(data.data(), rhs.data(), x.data(), true);
            check_result(x, x_ref);

            // basically data / 2
            std::vector<double> other_data = data;
            for (double& d : other_data) {
                d /= 2.0;
            }
            // basically x * 2
            std::vector<double> other_x_ref = x_ref;
            for (double& p : other_x_ref) {
                p *= 2.0;
            }

            // because all data should be prefactorized, changing the data should not
            // change the result when use_prefactorization = true
            solver.solve(other_data.data(), rhs.data(), x.data(), true);
            check_result(x, x_ref);

            // prefactorize other_data, then solve and compare with other_x_ref
            solver.prefactorize(other_data.data());
            solver.solve(other_data.data(), rhs.data(), x.data(), true);
            check_result(x, other_x_ref);

            // solve and compare with other_x without using prefactorization
            solver.solve(other_data.data(), rhs.data(), x.data(), false);
            check_result(x, other_x_ref);

            // invalidate pre-factorization
            // and re-run with original data with pre-factorization enabled
            // it should still re-do the factorization
            solver.invalidate_prefactorization();
            solver.solve(data.data(), rhs.data(), x.data(), true);
            check_result(x, x_ref);
        }
    }

    SECTION("Test block(double 2*2) calculation") {
        // [  0 1   1   2   3   4           3             40
        //  100 0   7  -1   5   6           4            355
        //    1 2   0 200   f   f       * [ -1 ]   =  [ -189 ]
        //   -3 4   3   1   f   f       *   -2             3
        //    5 6   f   f   1   0           5             44
        //   -7 8   f   f   0 100           6            611

        //  2 f 6]        2          18
        std::vector<Tensor> data = {
            {{0, 1}, {100, 0}},  // 0, 0
            {{1, 2}, {7, -1}},   // 0, 1
            {{3, 4}, {5, 6}},    // 0, 2
            {{1, 2}, {-3, 4}},   // 1, 0
            {{0, 200}, {3, 1}},  // 1, 1
            {{5, 6}, {-7, 8}},   // 2, 0
            {{1, 0}, {0, 100}},  // 2, 2
        };
        std::vector<Array> rhs = {{38, 356}, {-389, 2}, {44, 611}};
        std::vector<Array> x_ref = {{3, 4}, {-1, -2}, {5, 6}};
        std::vector<Array> x(3, Array::Zero());
        SparseLUSolver<Tensor, Array, Array> solver{row_indptr, col_indices, diag_lu, data_mapping};

        SECTION("Test calculation") {
            solver.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
        }

        SECTION("Test copy and move") {
            SparseLUSolver<Tensor, Array, Array> s1{solver};
            // copy construction
            s1.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
            // copy assignment
            s1 = solver;
            s1.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
            // self assignment
            auto& s2 = s1;
            s1 = s2;
            s1.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
            // move construction
            SparseLUSolver<Tensor, Array, Array> s3{std::move(solver)};
            s3.solve(data.data(), rhs.data(), x.data());
            check_result(x, x_ref);
        }

        SECTION("Test (pseudo) singular") {
            data[0](0, 1) = 0.0;
            CHECK_THROWS_AS(solver.solve(data.data(), rhs.data(), x.data()), SparseMatrixError);
        }

        SECTION("Test prefactorize") {
            solver.prefactorize(data.data());
            solver.solve(data.data(), rhs.data(), x.data(), true);
            check_result(x, x_ref);

            // basically data / 2
            std::vector<Tensor> other_data = data;
            for (Tensor& d : other_data) {
                d /= 2.0;
            }
            // basically x * 2
            std::vector<Array> other_x_ref = x_ref;
            for (Array& p : other_x_ref) {
                p *= 2.0;
            }

            // because all data should be prefactorized, changing the data should not
            // change the result when use_prefactorization = true
            solver.solve(other_data.data(), rhs.data(), x.data(), true);
            check_result(x, x_ref);

            // prefactorize other_data, then solve and compare with other_x_ref
            solver.prefactorize(other_data.data());
            solver.solve(other_data.data(), rhs.data(), x.data(), true);
            check_result(x, other_x_ref);

            // solve and compare with other_x without using prefactorization
            solver.solve(other_data.data(), rhs.data(), x.data(), false);
            check_result(x, other_x_ref);

            // invalidate pre-factorization
            // and re-run with original data with pre-factorization enabled
            // it should still re-do the factorization
            solver.invalidate_prefactorization();
            solver.solve(data.data(), rhs.data(), x.data(), true);
            check_result(x, x_ref);
        }
    }
}

}  // namespace power_grid_model
