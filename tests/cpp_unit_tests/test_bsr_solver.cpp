// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "catch2/catch.hpp"
#include "power_grid_model/math_solver/bsr_solver.hpp"
#include "power_grid_model/three_phase_tensor.hpp"

namespace power_grid_model {

template <class T>
void check_result(std::vector<T> const& x, std::vector<T> const& x_solver) {
    CHECK(x.size() == x_solver.size());
    for (size_t i = 0; i < x.size(); i++) {
        CHECK(cabs(x[i] - x_solver[i]) < numerical_tolerance);
    }
}

TEST_CASE("Test BSR solver") {
    // 4 * 4 matrix, with diagonal
    auto indptr = std::make_shared<IdxVector const>(IdxVector{0, 1, 3, 5, 6});
    auto col_indices = std::make_shared<IdxVector const>(IdxVector{0, 1, 2, 1, 2, 3});

    /// <summary>
    /// x 0 0 0
    /// 0 x x 0
    /// 0 x x 0
    /// 0 0 0 x
    /// </summary>
    std::vector<double> data = {
        1, 0, 0, 2,  // 0, 0
        0, 0, 0, 0,  // 1, 1
        2, 0, 0, 3,  // 1, 2
        3, 0, 0, 4,  // 2, 1
        0, 0, 0, 0,  // 2, 2
        4, 0, 0, 5,
    };

    std::vector<double> rhs = {1, 2, 2, 3, 6, 8, 8, 10};

    std::vector<double> x = {1, 1, 2, 2, 1, 1, 2, 2};

    std::vector<double> x_solver(8);
    Idx matrix_size_in_block = 4;
    Idx block_size = 2;

    BSRSolver<double> solver{matrix_size_in_block, block_size, indptr, col_indices};

    // complex
    std::vector<DoubleComplex> data_comp(data.size());
    std::copy(data.begin(), data.end(), data_comp.begin());
    std::vector<DoubleComplex> rhs_comp = {1.0i, 2.0i, 2.0i, 3.0i, 6.0, 8.0, 8, 10};
    std::vector<DoubleComplex> x_comp = {1.0i, 1.0i, 2.0, 2.0, 1.0i, 1.0i, 2.0, 2.0};
    std::vector<DoubleComplex> x_solver_comp(8);
    BSRSolver<DoubleComplex> solver_comp{matrix_size_in_block, block_size, indptr, col_indices};

    SECTION("Test calculation") {
        solver.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
        solver_comp.solve(data_comp.data(), rhs_comp.data(), x_solver_comp.data());
        check_result(x_comp, x_solver_comp);
    }

    SECTION("Test copy") {
        // copy construction
        BSRSolver<double> s1{solver};
        s1.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
        solver.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
        // copy assignment
        s1 = solver;
        s1.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
        solver.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
        // self assignment
        auto& s2 = s1;
        s1 = s2;
        s1.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
    }

    SECTION("Test move") {
        // move construction
        BSRSolver<double> s1{std::move(solver)};
        s1.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
        // move assignment
        solver = std::move(s1);
        solver.solve(data.data(), rhs.data(), x_solver.data());
        check_result(x, x_solver);
    }

    SECTION("Test singular") {
        data[0] = 0.0;
        data[12] = 0.0;
        data[15] = 0.0;
        CHECK_THROWS_AS(solver.solve(data.data(), rhs.data(), x_solver.data()), SparseMatrixError);
    }

    SECTION("Test prefactorize") {
        solver.prefactorize(data.data());
        solver.solve(data.data(), rhs.data(), x_solver.data(), true);
        check_result(x, x_solver);

        // basically data * 2
        std::vector<double> other_data = {
            2.0, 0.0, 0.0, 4.0,  // 0, 0
            0.0, 0.0, 0.0, 0.0,  // 1, 1
            4.0, 0.0, 0.0, 6.0,  // 1, 2
            6.0, 0.0, 0.0, 8.0,  // 2, 1
            0.0, 0.0, 0.0, 0.0,  // 2, 2
            8.0, 0.0, 0.0, 10.0,
        };

        // basically x / 2
        std::vector<double> other_x = {0.5, 0.5, 1.0, 1.0, 0.5, 0.5, 1.0, 1.0};

        // because all data should be prefactorized, changing the data should not
        // change the result when use_prefactorization = true
        solver.solve(other_data.data(), rhs.data(), x_solver.data(), true);
        check_result(x, x_solver);

        // prefactorize other_data, then solve and compare with other_x
        solver.prefactorize(other_data.data());
        solver.solve(other_data.data(), rhs.data(), x_solver.data(), true);
        check_result(other_x, x_solver);

        // solve and compare with other_x without using prefactorization
        solver.solve(other_data.data(), rhs.data(), x_solver.data(), false);
        check_result(other_x, x_solver);
    }
}

}  // namespace power_grid_model
