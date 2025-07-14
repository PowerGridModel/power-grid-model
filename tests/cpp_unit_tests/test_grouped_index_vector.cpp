// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/grouped_index_vector.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

namespace {
using IdxRanges = std::vector<IdxRange>;

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

} // namespace

} // namespace power_grid_model

namespace {
template <typename first, typename second> struct TypePair {
    using A = first;
    using B = second;
};
} // namespace

TYPE_TO_STRING_AS("SparseGroupedIdxVector, from_sparse_t",
                  TypePair<power_grid_model::SparseGroupedIdxVector, power_grid_model::from_sparse_t>);
TYPE_TO_STRING_AS("SparseGroupedIdxVector, from_dense_t",
                  TypePair<power_grid_model::SparseGroupedIdxVector, power_grid_model::from_dense_t>);
TYPE_TO_STRING_AS("SparseGroupedIdxVector, from_natural_t",
                  TypePair<power_grid_model::SparseGroupedIdxVector, power_grid_model::from_natural_t>);
TYPE_TO_STRING_AS("DenseGroupedIdxVector, from_sparse_t",
                  TypePair<power_grid_model::DenseGroupedIdxVector, power_grid_model::from_sparse_t>);
TYPE_TO_STRING_AS("DenseGroupedIdxVector, from_dense_t",
                  TypePair<power_grid_model::DenseGroupedIdxVector, power_grid_model::from_dense_t>);
TYPE_TO_STRING_AS("DenseGroupedIdxVector, from_natural_t",
                  TypePair<power_grid_model::DenseGroupedIdxVector, power_grid_model::from_natural_t>);

TYPE_TO_STRING_AS("SparseGroupedIdxVector, SparseGroupedIdxVector",
                  TypePair<power_grid_model::SparseGroupedIdxVector, power_grid_model::SparseGroupedIdxVector>);
TYPE_TO_STRING_AS("SparseGroupedIdxVector, DenseGroupedIdxVector",
                  TypePair<power_grid_model::SparseGroupedIdxVector, power_grid_model::DenseGroupedIdxVector>);
TYPE_TO_STRING_AS("DenseGroupedIdxVector, SparseGroupedIdxVector",
                  TypePair<power_grid_model::DenseGroupedIdxVector, power_grid_model::SparseGroupedIdxVector>);
TYPE_TO_STRING_AS("DenseGroupedIdxVector, DenseGroupedIdxVector",
                  TypePair<power_grid_model::DenseGroupedIdxVector, power_grid_model::DenseGroupedIdxVector>);

namespace power_grid_model {

TEST_CASE_TEMPLATE("Grouped idx data structure", IdxVectorConstructor, TypePair<SparseGroupedIdxVector, from_sparse_t>,
                   TypePair<SparseGroupedIdxVector, from_dense_t>, TypePair<SparseGroupedIdxVector, from_natural_t>,
                   TypePair<DenseGroupedIdxVector, from_sparse_t>, TypePair<DenseGroupedIdxVector, from_dense_t>,
                   TypePair<DenseGroupedIdxVector, from_natural_t>) {
    using IdxVectorType = typename IdxVectorConstructor::A;
    using ConstructFromTag = typename IdxVectorConstructor::B;

    IdxVector const groups{1, 1, 1, 3, 3, 3, 4};
    Idx const num_groups{6};
    IdxRanges expected_ranges{IdxRange{0, 0}, IdxRange{0, 3}, IdxRange{3, 3},
                              IdxRange{3, 6}, IdxRange{6, 7}, IdxRange{7, 7}};
    std::vector<Idx> const expected_elements{0, 1, 2, 3, 4, 5, 6};

    auto const idx_vector = construct_from<IdxVectorType, ConstructFromTag>(groups, num_groups);

    SUBCASE("Empty grouped idx vector - no explicit initialization") {
        IdxVectorType const indices;
        CHECK(indices.element_size() == 0);
        CHECK(indices.size() == 0);
    }

    SUBCASE("Empty grouped idx vector - explicit initialization") {
        IdxVectorType const indices{};
        CHECK(indices.element_size() == 0);
        CHECK(indices.size() == 0);
    }

    SUBCASE("Element range") {
        std::vector<Idx> const actual_idx_counts{};
        for (size_t group_number = 0; group_number < num_groups; group_number++) {
            CHECK(std::ranges::equal(idx_vector.get_element_range(group_number), expected_ranges[group_number]));
        }
    }

    SUBCASE("get_group") {
        for (size_t element = 0; element < groups.size(); element++) {
            CHECK(idx_vector.get_group(element) == groups[element]);
        }
    }

    SUBCASE("sizes") {
        CHECK(idx_vector.size() == num_groups);
        CHECK(idx_vector.element_size() == expected_elements.size());
    }

    SUBCASE("iteration") {
        std::vector<Idx> actual_elements{};
        IdxRanges actual_ranges{};
        for (auto const& element_range : idx_vector) {
            actual_ranges.emplace_back(element_range.begin(), element_range.end());
            for (auto element : element_range) {
                actual_elements.emplace_back(element);
            }
        }
        CHECK(actual_elements == expected_elements);
        CHECK(std::ranges::equal(actual_ranges, expected_ranges,
                                 [](auto const& lhs, auto const& rhs) { return std::ranges::equal(lhs, rhs); }));
    }
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
    IdxRanges const expected_ranges_a{IdxRange{0, 0}, IdxRange{0, 3}, IdxRange{3, 3},
                                      IdxRange{3, 6}, IdxRange{6, 7}, IdxRange{7, 7}};
    std::vector<Idx> const expected_elements_a{0, 1, 2, 3, 4, 5, 6};

    // Second grouped idx vector and its expeceted elements and groups
    IdxVector const groups_b{0, 1, 1, 3, 3, 4, 5, 5};
    IdxRanges const expected_ranges_b{IdxRange{0, 1}, IdxRange{1, 3}, IdxRange{3, 3},
                                      IdxRange{3, 5}, IdxRange{5, 6}, IdxRange{6, 8}};
    std::vector<Idx> const expected_elements_b{0, 1, 2, 3, 4, 5, 6, 7};

    // reuse for brevity
    auto const& groups_c = groups_a;
    auto const& expected_ranges_c = expected_ranges_a;

    // Construct both grouped idx vectors
    auto const idx_vector_a = construct_from<A, from_natural_t>(groups_a, num_groups);
    auto const idx_vector_b = construct_from<B, from_natural_t>(groups_b, num_groups);
    auto const idx_vector_c = construct_from<C, from_natural_t>(groups_c, num_groups);

    SUBCASE("empty input") {
        auto const empty_idx_vector = A{};
        for ([[maybe_unused]] auto [index, element_range] : enumerated_zip_sequence(empty_idx_vector)) {
            FAIL("this code should not be reached");
        }
    }

    SUBCASE("1 input") {
        // Test single zipped iteration
        IdxRanges actual_ranges_a{};
        Idx current_index{};
        for (auto const& [index, element_range] : enumerated_zip_sequence(idx_vector_a)) {
            actual_ranges_a.emplace_back(element_range.begin(), element_range.end());

            CHECK(index == current_index++);
        }
        CHECK(std::ranges::equal(actual_ranges_a, expected_ranges_a,
                                 [](auto const& lhs, auto const& rhs) { return std::ranges::equal(lhs, rhs); }));
    }

    SUBCASE("2 inputs") {
        std::vector<Idx> actual_idx_counts_a{};
        std::vector<Idx> actual_idx_counts_b{};
        IdxRanges actual_ranges_a{};
        IdxRanges actual_ranges_b{};
        Idx current_index{};
        for (auto const& [index, first_group, second_group] : enumerated_zip_sequence(idx_vector_a, idx_vector_b)) {
            for (auto const& element : first_group) {
                actual_idx_counts_a.push_back(element);
            }
            for (auto const& element : second_group) {
                actual_idx_counts_b.push_back(element);
            }
            actual_ranges_a.emplace_back(first_group.begin(), first_group.end());
            actual_ranges_b.emplace_back(second_group.begin(), second_group.end());

            CHECK(index == current_index++);
        }

        CHECK(actual_idx_counts_a == expected_elements_a);
        CHECK(actual_idx_counts_b == expected_elements_b);
        CHECK(std::ranges::equal(actual_ranges_a, expected_ranges_a,
                                 [](auto const& lhs, auto const& rhs) { return std::ranges::equal(lhs, rhs); }));
        CHECK(std::ranges::equal(actual_ranges_b, expected_ranges_b,
                                 [](auto const& lhs, auto const& rhs) { return std::ranges::equal(lhs, rhs); }));
    }

    SUBCASE("3 inputs") {
        // Test 3 zipped iterations
        IdxRanges actual_ranges_a{};
        IdxRanges actual_ranges_b{};
        IdxRanges actual_ranges_c{};
        Idx current_index{};
        for (auto const& [index, element_range_1, element_range_2, element_range_3] :
             enumerated_zip_sequence(idx_vector_a, idx_vector_b, idx_vector_c)) {
            actual_ranges_a.emplace_back(element_range_1.begin(), element_range_1.end());
            actual_ranges_b.emplace_back(element_range_2.begin(), element_range_2.end());
            actual_ranges_c.emplace_back(element_range_3.begin(), element_range_3.end());

            CHECK(index == current_index++);
        }
        CHECK(std::ranges::equal(actual_ranges_a, expected_ranges_a,
                                 [](auto const& lhs, auto const& rhs) { return std::ranges::equal(lhs, rhs); }));
        CHECK(std::ranges::equal(actual_ranges_b, expected_ranges_b,
                                 [](auto const& lhs, auto const& rhs) { return std::ranges::equal(lhs, rhs); }));
        CHECK(std::ranges::equal(actual_ranges_c, expected_ranges_c,
                                 [](auto const& lhs, auto const& rhs) { return std::ranges::equal(lhs, rhs); }));
    }
}

} // namespace power_grid_model
