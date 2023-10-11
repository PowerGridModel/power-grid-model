// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/grouped_index_vector.hpp>

namespace power_grid_model {

namespace {
using detail::sparse_encode;

struct from_natural_t {};

template <grouped_idx_vector_type IdxVectorType, std::same_as<from_dense_t> ConstructFromTag>
auto construct_from(IdxVector const& element_groups, Idx num_groups) {
    return IdxVectorType{from_dense, element_groups, num_groups};
}

template <grouped_idx_vector_type IdxVectorType, std::same_as<from_sparse_t> ConstructFromTag>
auto construct_from(IdxVector const& element_groups, Idx num_groups) {
    return IdxVectorType{from_sparse, sparse_encode(element_groups, num_groups)};
}

template <std::same_as<DenseGroupedIdxVector> IdxVectorType, std::same_as<from_natural_t> ConstructFromTag>
auto construct_from(IdxVector const& element_groups, Idx num_groups) {
    return IdxVectorType{element_groups, num_groups};
}

template <std::same_as<SparseGroupedIdxVector> IdxVectorType, std::same_as<from_natural_t> ConstructFromTag>
auto construct_from(IdxVector const& element_groups, Idx num_groups) {
    return IdxVectorType{sparse_encode(element_groups, num_groups)};
}

template <typename first, typename second> struct TypePair {
    using A = first;
    using B = second;
};

} // namespace

TEST_CASE_TEMPLATE("Grouped idx data strucuture for topology", IdxVectorConstructor,
                   TypePair<SparseGroupedIdxVector, from_sparse_t>, TypePair<SparseGroupedIdxVector, from_dense_t>,
                   TypePair<SparseGroupedIdxVector, from_natural_t>, TypePair<DenseGroupedIdxVector, from_sparse_t>,
                   TypePair<DenseGroupedIdxVector, from_dense_t>, TypePair<DenseGroupedIdxVector, from_natural_t>) {
    using IdxVectorType = typename IdxVectorConstructor::A;
    using ConstructFromTag = typename IdxVectorConstructor::B;

    IdxVector const groups{1, 1, 1, 3, 3, 3, 4};
    Idx const num_groups{6};
    std::vector<boost::iterator_range<IdxCount>> expected_ranges{{0, 0}, {0, 3}, {3, 3}, {3, 6}, {6, 7}, {7, 7}};
    std::vector<IdxCount> const expected_elements{0, 1, 2, 3, 4, 5, 6};

    auto const idx_vector = construct_from<IdxVectorType, ConstructFromTag>(groups, num_groups);

    SUBCASE("Grouped Idx vector functionalities") {
        // Test get_element_range
        std::vector<IdxCount> actual_idx_counts{};
        for (size_t group_number = 0; group_number < num_groups; group_number++) {
            CHECK(idx_vector.get_element_range(group_number) == expected_ranges[group_number]);
        }

        // Test get_group
        for (size_t element = 0; element < groups.size(); element++) {
            CHECK(idx_vector.get_group(element) == groups[element]);
        }

        // Test sizes
        CHECK(idx_vector.size() == num_groups);
        CHECK(idx_vector.element_size() == expected_elements.size());

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

    SUBCASE("Zipped 1 grouped idx vector") {
        // Test single zipped iteration
        std::vector<boost::iterator_range<IdxCount>> actual_ranges{};
        for (auto [element_range] : zip_sequence(idx_vector)) {
            actual_ranges.push_back(element_range);
        }
        CHECK(actual_ranges == expected_ranges);
    }

    SUBCASE("Zipped 3 same grouped idx vectors") {
        // Create additional 2 grouped idx vectors
        auto const idx_vector_2 = construct_from<IdxVectorType, from_natural_t>(groups, num_groups);
        auto const idx_vector_3 = construct_from<IdxVectorType, from_natural_t>(groups, num_groups);

        // Test 3 zipped iterations
        std::vector<boost::iterator_range<IdxCount>> actual_ranges_1{};
        std::vector<boost::iterator_range<IdxCount>> actual_ranges_2{};
        std::vector<boost::iterator_range<IdxCount>> actual_ranges_3{};
        for (auto [element_range_1, element_range_2, element_range_3] :
             zip_sequence(idx_vector, idx_vector_2, idx_vector_3)) {
            actual_ranges_1.push_back(element_range_1);
            actual_ranges_2.push_back(element_range_2);
            actual_ranges_3.push_back(element_range_3);
        }
        CHECK(actual_ranges_1 == expected_ranges);
        CHECK(actual_ranges_2 == expected_ranges);
        CHECK(actual_ranges_3 == expected_ranges);
    }
}

TEST_CASE_TEMPLATE("2 different grouped structures tests with zip iterator", IdxVectorTypes,
                   TypePair<SparseGroupedIdxVector, SparseGroupedIdxVector>,
                   TypePair<SparseGroupedIdxVector, DenseGroupedIdxVector>,
                   TypePair<DenseGroupedIdxVector, SparseGroupedIdxVector>,
                   TypePair<DenseGroupedIdxVector, DenseGroupedIdxVector>) {
    // Number of groups need to be equal
    Idx const num_groups{6};

    // First grouped idx vector and its expeceted elements and groups
    IdxVector const first_groups{1, 1, 1, 3, 3, 3, 4};
    std::vector<boost::iterator_range<IdxCount>> first_expected_ranges{{0, 0}, {0, 3}, {3, 3}, {3, 6}, {6, 7}, {7, 7}};
    std::vector<IdxCount> const first_expected_elements{0, 1, 2, 3, 4, 5, 6};

    // Second grouped idx vector and its expeceted elements and groups
    IdxVector const second_groups{0, 1, 1, 3, 3, 4, 5, 5};
    std::vector<boost::iterator_range<IdxCount>> second_expected_ranges{{0, 1}, {1, 3}, {3, 3}, {3, 5}, {5, 6}, {6, 8}};
    std::vector<IdxCount> const second_expected_elements{0, 1, 2, 3, 4, 5, 6, 7};

    // Construct both grouped idx vectors
    using T1 = typename IdxVectorTypes::A;
    using T2 = typename IdxVectorTypes::B;
    auto const first_idx_vector = construct_from<T1, from_natural_t>(first_groups, num_groups);
    auto const second_idx_vector = construct_from<T2, from_natural_t>(second_groups, num_groups);

    // Check iteration for all groups for zipped grouped idx vectors
    std::vector<IdxCount> first_actual_idx_counts{};
    std::vector<IdxCount> second_actual_idx_counts{};
    std::vector<boost::iterator_range<IdxCount>> first_actual_ranges{};
    std::vector<boost::iterator_range<IdxCount>> second_actual_ranges{};

    for (auto const [first_group, second_group] : zip_sequence(first_idx_vector, second_idx_vector)) {
        for (auto& element : first_group) {
            first_actual_idx_counts.push_back(element);
        }
        for (auto& element : second_group) {
            second_actual_idx_counts.push_back(element);
        }
        first_actual_ranges.push_back(first_group);
        second_actual_ranges.push_back(second_group);
    }

    CHECK(first_actual_idx_counts == first_expected_elements);
    CHECK(second_actual_idx_counts == second_expected_elements);
    CHECK(first_actual_ranges == first_expected_ranges);
    CHECK(second_actual_ranges == second_expected_ranges);
}

} // namespace power_grid_model
