// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/grouped_index_vector.hpp>

namespace power_grid_model {

namespace {
using CountingRange = boost::iterator_range<IdxCount>;
using CountingRanges = std::vector<CountingRange>;

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

TEST_CASE_TEMPLATE("Grouped idx data structure", IdxVectorConstructor, TypePair<SparseGroupedIdxVector, from_sparse_t>,
                   TypePair<SparseGroupedIdxVector, from_dense_t>, TypePair<SparseGroupedIdxVector, from_natural_t>,
                   TypePair<DenseGroupedIdxVector, from_sparse_t>, TypePair<DenseGroupedIdxVector, from_dense_t>,
                   TypePair<DenseGroupedIdxVector, from_natural_t>) {
    using IdxVectorType = typename IdxVectorConstructor::A;
    using ConstructFromTag = typename IdxVectorConstructor::B;

    IdxVector const groups{1, 1, 1, 3, 3, 3, 4};
    Idx const num_groups{6};
    CountingRanges expected_ranges{{0, 0}, {0, 3}, {3, 3}, {3, 6}, {6, 7}, {7, 7}};
    std::vector<IdxCount> const expected_elements{0, 1, 2, 3, 4, 5, 6};

    auto const idx_vector = construct_from<IdxVectorType, ConstructFromTag>(groups, num_groups);

    // Test get_element_range
    std::vector<IdxCount> const actual_idx_counts{};
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
    CountingRanges actual_ranges{};
    for (auto const& element_range : idx_vector) {
        actual_ranges.push_back(element_range);
        for (auto& element : element_range) {
            actual_elements.push_back(element);
        }
    }
    CHECK(actual_elements == expected_elements);
    CHECK(actual_ranges == expected_ranges);
}

TEST_CASE_TEMPLATE("Enumerated zip iterator for grouped index data structures", IdxVectorTypes,
                   TypePair<SparseGroupedIdxVector, SparseGroupedIdxVector>,
                   TypePair<SparseGroupedIdxVector, DenseGroupedIdxVector>,
                   TypePair<DenseGroupedIdxVector, SparseGroupedIdxVector>,
                   TypePair<DenseGroupedIdxVector, DenseGroupedIdxVector>) {
    using A = typename IdxVectorTypes::A;
    using B = typename IdxVectorTypes::B;
    using C = typename IdxVectorTypes::A; // reusing for brevity

    // Number of groups need to be equal
    Idx const num_groups{6};

    // First grouped idx vector and its expeceted elements and groups
    IdxVector const groups_a{1, 1, 1, 3, 3, 3, 4};
    CountingRanges expected_ranges_a{{0, 0}, {0, 3}, {3, 3}, {3, 6}, {6, 7}, {7, 7}};
    std::vector<IdxCount> const expected_elements_a{0, 1, 2, 3, 4, 5, 6};

    // Second grouped idx vector and its expeceted elements and groups
    IdxVector const groups_b{0, 1, 1, 3, 3, 4, 5, 5};
    CountingRanges expected_ranges_b{{0, 1}, {1, 3}, {3, 3}, {3, 5}, {5, 6}, {6, 8}};
    std::vector<IdxCount> const expected_elements_b{0, 1, 2, 3, 4, 5, 6, 7};

    // reuse for brevity
    auto const& groups_c = groups_a;
    auto const& expected_ranges_c = expected_ranges_a;

    // Construct both grouped idx vectors
    auto const idx_vector_a = construct_from<A, from_natural_t>(groups_a, num_groups);
    auto const idx_vector_b = construct_from<B, from_natural_t>(groups_b, num_groups);
    auto const idx_vector_c = construct_from<C, from_natural_t>(groups_c, num_groups);

    SUBCASE("1 input") {
        // Test single zipped iteration
        CountingRanges actual_ranges_a{};
        Idx current_index{};
        for (auto [index, element_range] : enumerated_zip_sequence(idx_vector_a)) {
            actual_ranges_a.push_back(element_range);

            CHECK(index == current_index++);
        }
        CHECK(actual_ranges_a == expected_ranges_a);
    }

    SUBCASE("2 inputs") {
        std::vector<IdxCount> actual_idx_counts_a{};
        std::vector<IdxCount> actual_idx_counts_b{};
        CountingRanges actual_ranges_a{};
        CountingRanges actual_ranges_b{};
        Idx current_index{};
        for (auto const [index, first_group, second_group] : enumerated_zip_sequence(idx_vector_a, idx_vector_b)) {
            for (auto& element : first_group) {
                actual_idx_counts_a.push_back(element);
            }
            for (auto& element : second_group) {
                actual_idx_counts_b.push_back(element);
            }
            actual_ranges_a.push_back(first_group);
            actual_ranges_b.push_back(second_group);

            CHECK(index == current_index++);
        }

        CHECK(actual_idx_counts_a == expected_elements_a);
        CHECK(actual_idx_counts_b == expected_elements_b);
        CHECK(actual_ranges_a == expected_ranges_a);
        CHECK(actual_ranges_b == expected_ranges_b);
    }

    SUBCASE("3 inputs") {
        // Test 3 zipped iterations
        CountingRanges actual_ranges_a{};
        CountingRanges actual_ranges_b{};
        CountingRanges actual_ranges_c{};
        Idx current_index{};
        for (auto [index, element_range_1, element_range_2, element_range_3] :
             enumerated_zip_sequence(idx_vector_a, idx_vector_b, idx_vector_c)) {
            actual_ranges_a.push_back(element_range_1);
            actual_ranges_b.push_back(element_range_2);
            actual_ranges_c.push_back(element_range_3);

            CHECK(index == current_index++);
        }
        CHECK(actual_ranges_a == expected_ranges_a);
        CHECK(actual_ranges_b == expected_ranges_b);
        CHECK(actual_ranges_c == expected_ranges_c);
    }
}

} // namespace power_grid_model
