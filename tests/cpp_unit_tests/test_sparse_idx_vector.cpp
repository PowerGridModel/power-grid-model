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

    SUBCASE("Group iterator values") {
        // Check each group
        std::vector<IdxCount> actual_idx_counts{};
        for (size_t i = 0; i < 6; i++) {
            size_t range_size = sample_indptr[i + 1] - sample_indptr[i];
            actual_idx_counts.clear();
            actual_idx_counts.resize(range_size);
            auto group_i = sparse_idx_vector.group_view_iter(i);
            std::copy(group_i.begin(), group_i.end(), actual_idx_counts.begin());
            if (range_size != 0) {
                CHECK(actual_idx_counts.front() == IdxCount{sample_indptr[i]});
                CHECK(actual_idx_counts.back() == IdxCount{sample_indptr[i + 1] - 1});
            }
        }

        // Check all groups
        std::vector<IdxCount> actual_idx_counts_groups{};
        for (auto element_range : sparse_idx_vector.groups()) {
            for (auto& element : element_range) {
                actual_idx_counts_groups.push_back(element);
            }
        }
    }
}

} // namespace power_grid_model::detail
