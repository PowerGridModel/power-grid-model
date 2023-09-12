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

    SparseIdxVector sparse_idx_vector{sample_indptr};
    SparseVectorData sparse_vector_data{data};

    SUBCASE("sparse idx vector functionalities") {

        auto location_size = sparse_idx_vector.subset(location);
        auto sparse_vector_data_range = sparse_vector_data.subset_data(location_size);
        IdxVector actual_elements_at_1{};
        for (auto i : sparse_vector_data_range) {
            actual_elements_at_1.push_back(i);
        }

        CHECK(actual_elements_at_1 == expected_elements_at_1);
        CHECK(sparse_vector_data_range[1] == 14);
        CHECK(sparse_vector_data_range.size() == 3);
    }

    SUBCASE("Vector functionalities") {
        IdxVector actual_complete_data{};
        for (auto i : sparse_vector_data) {
            actual_complete_data.push_back(i);
        }

        // original vector functionalities
        CHECK(sparse_vector_data[4] == 14);
        CHECK(sparse_vector_data.at(4) == 14);
        CHECK(sparse_idx_vector[1] == 3);
        CHECK(sparse_idx_vector.at(1) == 3);
        CHECK(data == actual_complete_data);
    }
}

} // namespace power_grid_model::detail
