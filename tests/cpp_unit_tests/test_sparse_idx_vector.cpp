// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/sparse_idx_vector.hpp>

namespace power_grid_model::detail {

TEST_CASE("Sparse idx data strucuture for topology") {
    // Sparse vector to test
    IdxVector const sample_indptr{0, 0, 3, 3, 6, 7, 7};
    IdxVector const expected_groups{1, 1, 1, 3, 3, 3, 4};
    std::vector<IdxCount> expected_idx_counts_groups{0, 1, 2, 3, 4, 5, 6};
    SparseIdxVector sparse_idx_vector{sample_indptr};

    // 2nd sparse vector
    IdxVector const sample_indptr_2{0, 0, 1, 3, 6, 6, 6};
    std::vector<IdxCount> expected_idx_counts_groups_2{0, 1, 2, 3, 4, 5};
    SparseIdxVector sparse_idx_vector_2{sample_indptr_2};

    // Dense Vector (Same configuration as sparse)
    DenseIdxVector dense_idx_vector{expected_groups, 7}; // 6 groups

    SUBCASE("Sparse Idx vector") {
        // Test get_element_range
        std::vector<IdxCount> actual_idx_counts{};
        for (size_t group_number = 0; group_number < 6; group_number++) {
            // Prepare values for single group
            size_t range_size = sample_indptr[group_number + 1] - sample_indptr[group_number];
            actual_idx_counts.clear();
            actual_idx_counts.resize(range_size);

            auto group_i = sparse_idx_vector.get_element_range(group_number);
            std::copy(group_i.begin(), group_i.end(), actual_idx_counts.begin());

            // Test values for a single group
            if (range_size == 0) {
                CHECK(actual_idx_counts.empty());
            } else {
                CHECK(actual_idx_counts.front() == IdxCount{sample_indptr[group_number]});
                CHECK(actual_idx_counts.back() == IdxCount{sample_indptr[group_number + 1] - 1});
            }
        }

        // Test get_group
        for (size_t element = 0; element < 7; element++) {
            CHECK(expected_groups[element] == sparse_idx_vector.get_group(element));
        }

        // Test Iteration
        std::vector<IdxCount> actual_idx_counts_groups{};
        for (auto element_range : sparse_idx_vector) {
            for (auto& element : element_range) {
                actual_idx_counts_groups.push_back(element);
            }
        }
        CHECK(actual_idx_counts_groups == expected_idx_counts_groups);
    }

    SUBCASE("Dense Idx vector") {
        // Check each group individually
        std::vector<IdxCount> actual_idx_counts{};
        for (size_t group_number = 0; group_number < 6; group_number++) {
            size_t range_size = sample_indptr[group_number + 1] - sample_indptr[group_number];
            actual_idx_counts.clear();
            actual_idx_counts.resize(range_size);
            auto group_i = dense_idx_vector.get_element_range(group_number);
            std::copy(group_i.begin(), group_i.end(), actual_idx_counts.begin());
            if (range_size == 0) {
                CHECK(actual_idx_counts.empty());
            } else {
                CHECK(actual_idx_counts.front() == IdxCount{sample_indptr[group_number]});
                CHECK(actual_idx_counts.back() == IdxCount{sample_indptr[group_number + 1] - 1});
            }
        }

        // Test get_group
        for (size_t element = 0; element < 7; element++) {
            CHECK(expected_groups[element] == sparse_idx_vector.get_group(element));
        }

        // Test Iteration
        std::vector<IdxCount> actual_idx_counts_groups{};
        for (auto element_range : dense_idx_vector) {
            for (auto& element : element_range) {
                actual_idx_counts_groups.push_back(element);
            }
        }
        CHECK(actual_idx_counts_groups == expected_idx_counts_groups);

        // Test Reverse Iteration
        // std::vector<IdxCount> actual_idx_counts_groups_reverse{};
        // for (auto i = dense_idx_vector.end(); i != dense_idx_vector.begin(); --i) {
        //     for (auto& element : *i) {
        //         actual_idx_counts_groups_reverse.push_back(element);
        //     }
        // }
        // std::vector<IdxCount> expected_idx_counts_groups_reverse{6, 5, 4, 3, 2, 1, 0}; 
        // CHECK(actual_idx_counts_groups_reverse == expected_idx_counts_groups_reverse);


    }

    SUBCASE("Zip iterator") {
        // Check iteration for all groups for zipped 2 sparse vectors
        std::vector<IdxCount> actual_idx_counts_groups{};
        std::vector<IdxCount> actual_idx_counts_groups_2{};
        for (auto const [group, group_2] : zip_sequence(sparse_idx_vector, sparse_idx_vector_2)) {
            for (auto& element : group) {
                actual_idx_counts_groups.push_back(element);
            }
            for (auto& element : group_2) {
                actual_idx_counts_groups_2.push_back(element);
            }
        }
        CHECK(actual_idx_counts_groups == expected_idx_counts_groups);
        CHECK(actual_idx_counts_groups_2 == expected_idx_counts_groups_2);
    }
}

} // namespace power_grid_model::detail
