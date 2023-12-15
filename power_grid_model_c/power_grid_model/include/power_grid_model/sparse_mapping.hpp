// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_MAPPING_HPP
#define POWER_GRID_MODEL_SPARSE_MAPPING_HPP

#include "power_grid_model.hpp"
#include <ranges>
/*
Sparse mapping technique
Given a input idx array: idx_B_in_A[...] to couple an array of A and an array of B,
i.e. idx_B_in_A[i_A] = j_B.

For entry i_A in the array, idx_B_in_A[i_A] is the idx of B which couples the A object #i_A.

This sparse mapping tries to build a CSC sparse matrix so that
only entry (i, idx_B_in_A[i_A] = j_B) is filled with the sequence of A, i.e. #i_A.

In CSC format, the entries are ordered by the idx of B
therefore, in indptr, each range indptr[j_B:j_B+1]
represents the entries of A where B object #j_B is coupled.

The indices array is not interesting here.
The data array is original index of A. We call it re-order array.
This can be used to reorder A objects by the coupling of B idx.

Example.
For original idx_B_in_A == [3, 5, 2, 1, 1, 2]
size of A is 6
size of B is 7
Result matrix
indptr = [0, 0, 2, 4, 5, 5, 6, 6]
data/reorder = [3, 4, 2, 5, 0, 1]
to read:
    nothing		coupled to B 0
    A 3, 4		coupled to B 1
    A 2, 5		coupled to B 2
    A 0			coupled to B 3
    nothing		coupled to B 4
    A 1			coupled	to B 5
    nothing	coupled to B 6
*/

namespace power_grid_model {

struct SparseMapping {
    IdxVector indptr;
    IdxVector reorder;
};

inline SparseMapping build_sparse_mapping(IdxVector const& idx_B_in_A, Idx const n_B) {
    auto const n_A = static_cast<Idx>(idx_B_in_A.size());
    using SparseEntry = std::pair<Idx, Idx>;
    std::vector<SparseEntry> entries(n_A);

    std::transform(idx_B_in_A.cbegin(), idx_B_in_A.cend(), IdxCount{0}, entries.begin(), [](Idx j_B, Idx i_A) {
        return SparseEntry{i_A, j_B};
    });

    SparseMapping sparse_mapping;
    sparse_mapping.indptr.resize(n_B + 1);
    sparse_mapping.reorder.resize(n_A);

    IdxVector counter(n_B, 0);
    for (auto const& entry : entries) {
        ++counter[entry.second];
    }

    std::inclusive_scan(counter.cbegin(), counter.cend(), sparse_mapping.indptr.begin() + 1);

    std::copy(sparse_mapping.indptr.cbegin() + 1, sparse_mapping.indptr.cend(), counter.begin());

    for (auto it_entry = entries.crbegin(); it_entry != entries.crend(); ++it_entry) {
        sparse_mapping.reorder[--counter[it_entry->second]] = it_entry->first;
    }

    assert(sparse_mapping.indptr[0] == 0);
    assert(sparse_mapping.indptr.back() == n_A);
    return sparse_mapping;
}

struct DenseMapping {
    IdxVector indvector;
    IdxVector reorder;
};

inline DenseMapping build_dense_mapping(IdxVector const& idx_B_in_A, Idx const n_B) {
    auto const n_A = static_cast<Idx>(idx_B_in_A.size());

    using DenseEntry = std::pair<Idx, Idx>;
    DenseMapping dense_mapping;

    if (n_A * log(n_A) < n_A + n_B) {

        std::vector<DenseEntry> mapping_to_from;

        auto index_range = std::ranges::views::iota(0, static_cast<Idx>(idx_B_in_A.size()));

        std::ranges::transform(idx_B_in_A, index_range, std::back_inserter(mapping_to_from),
                               [](Idx value, Idx orig_idx) {
                                   return std::pair{value, orig_idx};
                               });

        std::ranges::sort(mapping_to_from);

        std::ranges::transform(mapping_to_from, std::back_inserter(dense_mapping.indvector),
                               [](DenseEntry const& to_from) { return to_from.first; });

        std::ranges::transform(mapping_to_from, std::back_inserter(dense_mapping.reorder),
                               [](DenseEntry const& to_from) { return to_from.second; });
    } else {
        std::vector<DenseEntry> entries(n_A);

        std::transform(idx_B_in_A.cbegin(), idx_B_in_A.cend(), IdxCount{0}, entries.begin(), [](Idx j_B, Idx i_A) {
            return DenseEntry{i_A, j_B};
        });

        dense_mapping.indvector.resize(n_A);

        dense_mapping.reorder.resize(n_A);

        IdxVector xndvector;

        xndvector.resize(n_B + 1);

        IdxVector counter(n_B + 1, 0);

        for (auto const& entry : entries) {
            ++counter[entry.second];
        }

        std::inclusive_scan(counter.cbegin(), counter.cend(), xndvector.begin());

        for (auto it_entry = entries.crbegin(); it_entry != entries.crend(); ++it_entry) {
            dense_mapping.indvector[xndvector[idx_B_in_A[it_entry->first]] - 1] = idx_B_in_A[it_entry->first];
            dense_mapping.reorder[--xndvector[it_entry->second]] = it_entry->first;
        }
    }

    return dense_mapping;
}

} // namespace power_grid_model
#endif
