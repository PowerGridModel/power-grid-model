// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/sparse_idx_vector.hpp>

namespace power_grid_model::detail {

TEST_CASE("Sparse idx data strucuture for topology") {
    IdxVector const sample_indptr{0, 0, 3, 3, 6, 7, 7};
    SparseIdxVector sparse_idx_vector{sample_indptr};
    IdxVector expected_groups{1, 1, 1, 3, 3, 3, 4};
    IdxVector expected_keys{0, 1, 2, 3, 4, 5, 6};

    IdxVector sample_dense_vector{1, 1, 1, 3, 3, 3, 4};
    DenseIdxVector dense_idx_vector{sample_dense_vector};

    SUBCASE("Sparse Idx vector") {
        // Check each group
        std::vector<IdxCount> actual_idx_counts{};
        for (size_t group_number = 0; group_number < 6; group_number++) {
            size_t range_size = sample_indptr[group_number + 1] - sample_indptr[group_number];
            actual_idx_counts.clear();
            actual_idx_counts.resize(range_size);
            auto group_i = sparse_idx_vector.get_element_range(group_number);
            std::copy(group_i.begin(), group_i.end(), actual_idx_counts.begin());
            if (range_size == 0)    {
                CHECK(actual_idx_counts.empty());
            } else {
                CHECK(actual_idx_counts.front() == IdxCount{sample_indptr[group_number]});
                CHECK(actual_idx_counts.back() == IdxCount{sample_indptr[group_number + 1] - 1});
            }
        }
    }

    SUBCASE("Dense Idx vector") {
        // Check each group
        std::vector<IdxCount> actual_idx_counts{};
        for (size_t group_number = 0; group_number < 6; group_number++) {
            size_t range_size = sample_indptr[group_number + 1] - sample_indptr[group_number];
            actual_idx_counts.clear();
            actual_idx_counts.resize(range_size);
            auto group_i = dense_idx_vector.get_element_range(group_number);
            std::copy(group_i.begin(), group_i.end(), actual_idx_counts.begin());
            if (range_size == 0)    {
                CHECK(actual_idx_counts.empty());
            } else {
                CHECK(actual_idx_counts.front() == IdxCount{sample_indptr[group_number]});
                CHECK(actual_idx_counts.back() == IdxCount{sample_indptr[group_number + 1] - 1});
            }
        }
    }


}

} // namespace power_grid_model::detail
