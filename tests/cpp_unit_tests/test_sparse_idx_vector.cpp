// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/sparse_idx_vector.hpp>

namespace power_grid_model::detail {

TEST_CASE("Sparse idx data strucuture for topology") {
    IdxVector const sample_indptr{0, 3, 6, 7};
    SparseIdxVector sparse_idx_vector{sample_indptr};
    IdxVector expected_groups{0, 0, 0, 1, 1, 1, 2};
    IdxVector expected_keys{0, 1, 2, 3, 4, 5, 6};

    SUBCASE("Element iterator") {
        auto elm_iter = sparse_idx_vector.values();
        CHECK(elm_iter[0] == 0);
        CHECK(elm_iter[1] == 0);
        CHECK(elm_iter[2] == 0);
        CHECK(elm_iter[3] == 1);
        CHECK(elm_iter[4] == 1);
        CHECK(elm_iter[5] == 1);
        CHECK(elm_iter[6] == 2);

        IdxVector actual_groups{};
        for (Idx i : elm_iter) {
            actual_groups.push_back(i);
        }
        CHECK(actual_groups == expected_groups);
        CHECK(elm_iter.size() == 7);

        auto elm_iter2 = sparse_idx_vector.values();
        ++elm_iter2;
        ++elm_iter;
        CHECK(elm_iter == elm_iter2);
    }

    // TODO Implement items
    // SUBCASE("Element iterator items") {
    //     auto elm_iter_items = sparse_idx_vector.element_iter_items();
    //     IdxVector actual_groups{};
    //     actual_groups.resize(expected_groups.size());
    //     for (auto [key, value] : elm_iter_items) {
    //         actual_groups[value] = key;
    //     }
    //     CHECK(actual_groups == expected_groups);
    // }

    SUBCASE("Element Range") {
        IdxVector expected_range{0, 0, 1, 1, 1};
        auto elm_range = sparse_idx_vector.value_range(1, 6);
        IdxVector actual_range{};
        for (Idx i : elm_range) {
            actual_range.push_back(i);
        }
        CHECK(actual_range == expected_range);
    }

    SUBCASE("Group iterator") {
        auto groups = sparse_idx_vector.groups();
        SparseIdxVector<Idx>::ElementRange expected_range_0{sample_indptr, 0, 1, 0, 0};
        SparseIdxVector<Idx>::ElementRange expected_range_1{sample_indptr, 1, 2, 0, 0};
        SparseIdxVector<Idx>::ElementRange expected_range_2{sample_indptr, 2, 3, 0, 0};

        IdxVector actual_groups{};
        for (auto element_range : groups) {
            for (auto element : element_range) {
                actual_groups.push_back(element);
            }
        }
        CHECK(actual_groups == expected_groups);

        CHECK(groups[0] == expected_range_0);
        CHECK(groups[1] == expected_range_1);
        CHECK(groups[2] == expected_range_2);

        CHECK(groups.size() == 3);
    }
}

} // namespace power_grid_model::detail
