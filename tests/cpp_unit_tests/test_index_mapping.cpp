// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/index_mapping.hpp>

#include <doctest/doctest.h>
#include <random>

namespace power_grid_model {

TEST_CASE("Test sparse mapping") {
    IdxVector const idx_B_in_A{3, 5, 2, 1, 1, 2};
    SparseIndexMapping mapping{{0, 0, 2, 4, 5, 5, 6, 6}, {3, 4, 2, 5, 0, 1}};
    SparseIndexMapping mapping_2 = build_sparse_mapping(idx_B_in_A, 7);

    CHECK(mapping.indptr == mapping_2.indptr);
    CHECK(mapping.reorder == mapping_2.reorder);
}

TEST_CASE("Test dense mapping - counting sort") {
    IdxVector const idx_B_in_A{3, 5, 2, 1, 1, 2};
    DenseIndexMapping const mapping{{1, 1, 2, 2, 3, 5}, {3, 4, 2, 5, 0, 1}};
    DenseIndexMapping const mapping_2 = build_dense_mapping(idx_B_in_A, 7);

    CHECK(mapping.indvector == mapping_2.indvector);
    CHECK(mapping.reorder == mapping_2.reorder);
    CHECK(mapping_2.indvector.begin() == std::ranges::min_element(mapping_2.indvector));
    CHECK(mapping_2.indvector.end() - 1 == std::ranges::max_element(mapping_2.indvector));
}

TEST_CASE("Test dense mapping - comparison sort") {
    constexpr Idx count{1'000'000};

    IdxVector idx_B_in_A;
    std::ranges::copy((std::views::iota(Idx{0}, count) | std::views::reverse), std::back_inserter(idx_B_in_A));

    IdxVector sorted_idx_B_in_A = idx_B_in_A;
    std::ranges::sort(sorted_idx_B_in_A);

    DenseIndexMapping const mapping = build_dense_mapping(idx_B_in_A, count);

    CHECK(mapping.indvector == sorted_idx_B_in_A);
    CHECK(mapping.indvector.begin() == std::ranges::min_element(mapping.indvector));
    CHECK(mapping.indvector.end() - 1 == std::ranges::max_element(mapping.indvector));
}

} // namespace power_grid_model
