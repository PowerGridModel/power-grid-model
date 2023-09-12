// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/sparse_idx_vector.hpp>

namespace power_grid_model::detail {

TEST_CASE("Sparse idx data strucuture for topology") {
    IdxVector const sample_indptr{0, 3, 6, 7};
    IdxVector const data{10, 11, 12, 13, 14, 15, 16};
    Idx const location{1};
    IdxVector const expected_elements_at_1{13, 14, 15};

    SparseIdxVector sparse_idx_vector{sample_indptr, data};

    SUBCASE("sparse idx vector functionalities") {

        auto sparse_vector_at_1 = sparse_idx_vector.subset_data(location);
        IdxVector actual_elements_at_1{};
        for (auto i : sparse_vector_at_1) {
            actual_elements_at_1.push_back(i);
        }

        CHECK(actual_elements_at_1 == expected_elements_at_1);
        CHECK(sparse_vector_at_1[1] == 14);
        CHECK(sparse_vector_at_1.size() == 3);
        CHECK(sparse_idx_vector.size() == 7);
    }

    SUBCASE("Vector functionalities") {
        // original vector functionalities
        IdxVector actual_complete_data{};
        for (auto i : sparse_idx_vector) {
            actual_complete_data.push_back(i);
        }

        CHECK(sparse_idx_vector[1] == 3);
        CHECK(sparse_idx_vector.at(1) == 3);
        CHECK(data == actual_complete_data);
    }
}

} // namespace power_grid_model::detail
