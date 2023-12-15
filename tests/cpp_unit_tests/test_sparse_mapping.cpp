// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/sparse_mapping.hpp>

#include <doctest/doctest.h>
#include <random>

namespace power_grid_model {

TEST_CASE("Test sparse mapping") {
    IdxVector const idx_B_in_A{3, 5, 2, 1, 1, 2};
    SparseMapping mapping{{0, 0, 2, 4, 5, 5, 6, 6}, {3, 4, 2, 5, 0, 1}};
    SparseMapping mapping_2 = build_sparse_mapping(idx_B_in_A, 7);

    CHECK(mapping.indptr == mapping_2.indptr);
    CHECK(mapping.reorder == mapping_2.reorder);
}

TEST_CASE("Test dense mapping - n log n sort") {
    IdxVector const idx_B_in_A{3, 5, 2, 1, 1, 2};
    DenseMapping mapping{{1, 1, 2, 2, 3, 5}, {3, 4, 2, 5, 0, 1}};
    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 7);

    CHECK(mapping.indvector == mapping_2.indvector);
    CHECK(mapping.reorder == mapping_2.reorder);
    CHECK(*(mapping_2.indvector.begin()) == *std::min_element(mapping_2.indvector.begin(), mapping_2.indvector.end()));
    CHECK(*(mapping_2.indvector.end() - 1) ==
          *std::max_element(mapping_2.indvector.begin(), mapping_2.indvector.end()));
}

// TEST_CASE("Test dense mapping - comparison sort") {

//     IdxVector idx_B_in_A(1000000);
//     std::mt19937 eng(16);
//     std::uniform_int_distribution<> distr(0, 1000000);

//     for (auto& val : idx_B_in_A) {
//         val = distr(eng);
//     }

//     IdxVector sorted_idx_B_in_A = idx_B_in_A;
//     std::ranges::sort(sorted_idx_B_in_A);

//     DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 1000001);

//     CHECK(mapping_2.indvector == sorted_idx_B_in_A);
//     CHECK(*(mapping_2.indvector.begin()) == *std::min_element(mapping_2.indvector.begin(),
//     mapping_2.indvector.end())); CHECK(*(mapping_2.indvector.end() - 1) ==
//           *std::max_element(mapping_2.indvector.begin(), mapping_2.indvector.end()));
// }

TEST_CASE("Test dense mapping - comparison sort") {

    IdxVector idx_B_in_A(1000000);

    int j = 0;
    for (Idx i = 999999; i >= 0; i--) {
        idx_B_in_A[j] = i;
        j++;
    }

    IdxVector sorted_idx_B_in_A = idx_B_in_A;
    std::ranges::sort(sorted_idx_B_in_A);

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 1000001);

    CHECK(mapping_2.indvector == sorted_idx_B_in_A);
    CHECK(*(mapping_2.indvector.begin()) == *std::min_element(mapping_2.indvector.begin(), mapping_2.indvector.end()));
    CHECK(*(mapping_2.indvector.end() - 1) ==
          *std::max_element(mapping_2.indvector.begin(), mapping_2.indvector.end()));
}

} // namespace power_grid_model
