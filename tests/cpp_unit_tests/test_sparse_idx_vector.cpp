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
        auto elm_iter = sparse_idx_vector.element_iter();
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

        SparseIdxVector::ElementIterator elm_iter2 = sparse_idx_vector.element_iter();
        ++elm_iter2;
        ++elm_iter;
        CHECK(elm_iter == elm_iter2);
    }

    // TODO Implement items
    SUBCASE("Element iterator items") {
        auto elm_iter_items = sparse_idx_vector.element_iter_items();

        IdxVector actual_groups{};
        actual_groups.resize(expected_groups.size());
        for (auto [key, value] : elm_iter_items) {
            actual_groups[value] = key;
        }
        CHECK(actual_groups == expected_groups);
    }

    SUBCASE("Group iterator") {
        auto group_iter = sparse_idx_vector.group_iter();
        auto sample_begin = sample_indptr.begin();
        SparseIdxVector::ElementIterator expected_range_0{IdxVector{0, 3}, 0, 0};
        SparseIdxVector::ElementIterator expected_range_1{IdxVector{3, 6}, 1, 0};
        SparseIdxVector::ElementIterator expected_range_2{IdxVector{6, 7}, 2, 0};

        // TODO Implement group iterator. Check what it is supposed to do.

        // CHECK(group_iter[0] == expected_range_0);
        // CHECK(group_iter[1] == expected_range_1);
        // CHECK(group_iter[2] == expected_range_2);

        // CHECK(*(++group_iter) == expected_range_0);
        // CHECK(*(++group_iter) == expected_range_1);
        // CHECK(*(group_iter) == expected_range_2);

        // CHECK(actual_groups == expected_groups);
        CHECK(group_iter.size() == 3);

        auto group_iter2 = sparse_idx_vector.element_iter();
        auto group_iter3 = sparse_idx_vector.element_iter();
        ++group_iter2;
        ++group_iter3;
        CHECK(group_iter2 == group_iter3);
    }
}

} // namespace power_grid_model::detail
