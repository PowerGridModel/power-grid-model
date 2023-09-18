// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/sparse_idx_vector.hpp>

namespace power_grid_model::detail {

TEST_CASE("Sparse idx data strucuture for topology") {
    IdxVector const sample_indptr{0, 3, 6, 7};
    SparseIdxVector sparse_idx_vector{sample_indptr};

    SUBCASE("sparse idx vector mapping") {
        CHECK(sparse_idx_vector[Idx{0}] == 0);
        CHECK(sparse_idx_vector[Idx{1}] == 0);
        CHECK(sparse_idx_vector[Idx{2}] == 0);
        CHECK(sparse_idx_vector[Idx{3}] == 1);
        CHECK(sparse_idx_vector[Idx{4}] == 1);
        CHECK(sparse_idx_vector[Idx{5}] == 1);
        CHECK(sparse_idx_vector[Idx{6}] == 2);
        CHECK(sparse_idx_vector.at(Idx{1}) == 0);
        CHECK(sparse_idx_vector.at(Idx{4}) == 1);
    }
}

} // namespace power_grid_model::detail
