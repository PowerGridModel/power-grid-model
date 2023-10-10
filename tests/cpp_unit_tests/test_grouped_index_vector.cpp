// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/grouped_index_vector.hpp>

namespace power_grid_model::detail {

auto sparse_encode(IdxVector const& element_groups, Idx num_groups) {
    IdxVector result(num_groups + 1);
    Idx count{0};
    for (size_t i=0; i<result.size(); i++)   {
        while (element_groups[count] == i) {
            result[i + 1] = count++;
        }
        result[i+1] = count;
    }
    return result;
}

template <std::same_as<DenseIdxVector> IdxVectorType>
auto construct_from(IdxVector element_groups, Idx num_groups) {
    return DenseIdxVector{std::move(element_groups), num_groups};
}

template <std::same_as<SparseIdxVector> IdxVectorType>
auto construct_from(IdxVector const& element_groups, Idx num_groups) {
    auto indptr = sparse_encode(element_groups, num_groups);
    return SparseIdxVector{indptr};
}

TEST_CASE_TEMPLATE("Sparse idx data strucuture for topology", IdxVectorType, SparseIdxVector, DenseIdxVector) {
    IdxVector const groups{1, 1, 1, 3, 3, 3, 4};
    Idx const num_groups{6};
    std::vector<boost::iterator_range<IdxCount>> expected_ranges{{0, 0}, {0, 3}, {3, 3}, {3, 6}, {6, 7}, {7, 7}};
    std::vector<IdxCount> const expected_elements{0, 1, 2, 3, 4, 5, 6};
    
    auto const idx_vector = construct_from<IdxVectorType>(groups, num_groups);

    // Test get_element_range
    std::vector<IdxCount> actual_idx_counts{};
    for (size_t group_number = 0; group_number < num_groups; group_number++) {
        CHECK(idx_vector.get_element_range(group_number) == expected_ranges[group_number]);
    }

    // Test get_group
    for (size_t element = 0; element < groups.size(); element++) {
        CHECK(idx_vector.get_group(element) == groups[element]);
    }

    // Test Iteration
    std::vector<IdxCount> actual_elements{};
    std::vector<boost::iterator_range<IdxCount>> actual_ranges{};
    for (auto element_range : idx_vector) {
        actual_ranges.push_back(element_range);
        for (auto& element : element_range) {
            actual_elements.push_back(element);
        }
    }
    CHECK(actual_elements == expected_elements);
    CHECK(actual_ranges == expected_ranges);

}

TEST_CASE("Zip iterator") {
    IdxVector const groups{1, 1, 1, 3, 3, 3, 4};
    Idx const num_groups{6};

    // Sparse vector to test
    IdxVector const sample_indptr{0, 0, 3, 3, 6, 7, 7};
    std::vector<IdxCount> expected_idx_counts_groups{0, 1, 2, 3, 4, 5, 6};
    SparseIdxVector sparse_idx_vector{sample_indptr};

    // 2nd sparse vector
    IdxVector const sample_indptr_2{0, 0, 1, 3, 6, 6, 6};
    std::vector<IdxCount> expected_idx_counts_groups_2{0, 1, 2, 3, 4, 5};
    SparseIdxVector sparse_idx_vector_2{sample_indptr_2};

    // Dense Vector (Same configuration as sparse)
    DenseIdxVector dense_idx_vector{groups, num_groups};

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

} // namespace power_grid_model::detail
