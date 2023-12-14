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

// input from the test: (const idx_B_in_A{3, 5, 2, 1, 1, 2}, 7)
inline SparseMapping build_sparse_mapping(IdxVector const& idx_B_in_A, Idx const n_B) {
    // n_A becomes the size of idx_B_in_A, thus it is 6
    auto const n_A = static_cast<Idx>(idx_B_in_A.size());
    using SparseEntry = std::pair<Idx, Idx>;
    // entries vector with size n_A
    std::vector<SparseEntry> entries(n_A);
    // idx_B_in_A{3, 5, 2, 1, 1, 2}, then count = {0, 1, 2, 3, 4, 5} since count is affected by the length of
    // idx_B_in_A j_B is an element from idx_B_in_A, and i_A is the current value from IdxCount{0} returns
    // entries[{0,3},{1,5},{2,2},{3,1},{4,1},{5,2}]
    std::transform(idx_B_in_A.cbegin(), idx_B_in_A.cend(), IdxCount{0}, entries.begin(), [](Idx j_B, Idx i_A) {
        return SparseEntry{i_A, j_B};
    });
    // initialize result
    SparseMapping sparse_mapping;
    // size of indpntr will become 8
    sparse_mapping.indptr.resize(n_B + 1);
    // size of reorder will be 6
    sparse_mapping.reorder.resize(n_A);
    // counting sort column, keep row as sorted
    // counter = {n_Bx0} = {0, 0, 0, 0, 0, 0, 0}
    IdxVector counter(n_B, 0); // amount of load gens connected to each bus
    // For every entry, ex. {0,3}, we add 1 to counter at entry.second place in this case 3rd, so it becomes {0, 0, 0,
    // 1, 0, 0, 0} counter should become {0, 2, 2, 1, 0, 1, 0}
    for (auto const& entry : entries) {
        ++counter[entry.second];
    }
    // accumulate value to indptr
    // [a, b, c, d], the inclusive scan would produce [a, a+b, a+b+c, a+b+c+d]
    // counter = {0, 2, 2, 1, 0, 1, 0}, indptr becomes {0, 0, 2, 4, 5, 5, 6, 6}
    std::inclusive_scan(counter.cbegin(), counter.cend(), sparse_mapping.indptr.begin() + 1);
    // copy back
    // copy everything except the indptr[0] to counter.
    // counter becomes {0, 2, 4, 5, 5, 6, 6}
    std::copy(sparse_mapping.indptr.cbegin() + 1, sparse_mapping.indptr.cend(), counter.begin());
    // sort
    // crbegin() points to the element at the end of the container
    // crend() points to the position before the first element
    // at this point: entries[{0,3},{1,5},{2,2},{3,1},{4,1},{5,2}], counter {0, 2, 4, 5, 5, 6, 6}
    // Reverse order the first entry in entries: {5,2}, it_entry->second = 2
    // counter[2] is 4, decremented to 3(counter becomes {0, 2, 3, 5, 5, 6, 6}), thus sparse_mapping.reorder[3] is set
    // to it_entry->first which is 5. sparse_mapping.reorder = {0, 0, 0, 5, 0, 0},
    // Final result: sparse_mapping.reorder = {3, 4, 2, 5, 0, 1}
    for (auto it_entry = entries.crbegin(); it_entry != entries.crend(); ++it_entry) {
        sparse_mapping.reorder[--counter[it_entry->second]] = it_entry->first;
    }
    // the first is the index before reordering of load_gen
    // it entry second is the index of the bus
    // reorder at the end contains for each new index of the load gen the old index of the load gen in a linear way
    assert(sparse_mapping.indptr[0] == 0);
    assert(sparse_mapping.indptr.back() == n_A);
    return sparse_mapping;
}

struct DenseMapping {
    IdxVector indvector;
    IdxVector reorder;
};

inline DenseMapping build_dense_mapping(IdxVector const& idx_B_in_A, Idx const n_B) {
    // auto const n_A = static_cast<Idx>(idx_B_in_A.size());
    // auto const r = std::max_element(idx_B_in_A.begin(), idx_B_in_A.end())[0];
    // for counting sort nr_of_operations = 4*n_A + 2*n_B + 2*n_A + 2*n_B - 1 + 1 + n_A + 1 + 14 * n_A
    // nr_of_operations = 20*n_A + 4*n_B + 2
    // auto const c = 21 * n_A + 4 * n_B + 1;
    // for n log n sort nr of operations = 1 + 7*n_A + n_A*log(n_A)?
    using DenseEntry = std::pair<Idx, Idx>;
    DenseMapping dense_mapping;

    // if (n_A * log(n_A) < 6.97e-5 * r) {
    // n_A
    std::vector<DenseEntry> mapping_to_from;

    // n_A operations
    auto index_range = std::ranges::views::iota(0, static_cast<Idx>(idx_B_in_A.size()));

    // n_A * 2 operations?
    std::ranges::transform(idx_B_in_A, index_range, std::back_inserter(mapping_to_from), [](Idx value, Idx orig_idx) {
        return std::pair{value, orig_idx};
    });
    // n_A log n_A?
    std::ranges::sort(mapping_to_from); // runs in O(n log n)

    // n_A * 2 operations?
    std::ranges::transform(mapping_to_from, std::back_inserter(dense_mapping.indvector),
                           [](DenseEntry const& to_from) { return to_from.first; });
    // n_A * 2 operations?
    std::ranges::transform(mapping_to_from, std::back_inserter(dense_mapping.reorder),
                           [](DenseEntry const& to_from) { return to_from.second; });
    // } else {
    //     // n_A
    //     std::vector<DenseEntry> entries(n_A);

    //     // n_A, since the transform operation will be performed n_A times and it only does one operation - returning
    //     the
    //     // dense entry
    //     std::transform(idx_B_in_A.cbegin(), idx_B_in_A.cend(), IdxCount{0}, entries.begin(), [](Idx j_B, Idx i_A) {
    //         return DenseEntry{i_A, j_B};
    //     });

    //     // n_A
    //     dense_mapping.indvector.resize(n_A);
    //     // n_A
    //     dense_mapping.reorder.resize(n_A);

    //     IdxVector xndvector;
    //     // n_B
    //     xndvector.resize(n_B + 1);
    //     // n_B
    //     IdxVector counter(n_B + 1, 0);

    //     // entries have the same size as idx_B_in_A, thus n_A * 2. One for access, one for incrementing
    //     for (auto const& entry : entries) {
    //         ++counter[entry.second];
    //     }

    //     // Loop over counter is n_B, but the addition does not happen on the first element so n_B - 1?
    //     // Placing the values in xndvector is n_B, thus 2n_B - 1?
    //     std::inclusive_scan(counter.cbegin(), counter.cend(), xndvector.begin());

    //     // {0,3},{1,5},{2,2},{3,1},{4,1},{5,2}

    //     // auto it_entry = entries.crbegin() should be 1
    //     // it_entry != entries.crend() should be n_A + 1, since it does all the operations including
    //     // the final one to check whether the condition does not hold anymore
    //     // ++it_entry will be n_A

    //     // Inside the loop:
    //     // dense_mapping.indvector[xndvector[idx_B_in_A[it_entry->first]] - 1] will be 5 * n_A:
    //     // first one for it_entry->first
    //     // second one for idx_B_in_A[it_entry->first]
    //     // third one for xndvector[idx_B_in_A[it_entry->first]]
    //     // fourth one for xndvector[idx_B_in_A[it_entry->first]] - 1
    //     // fifth one for dense_mapping.indvector[xndvector[idx_B_in_A[it_entry->first]] - 1]

    //     // idx_B_in_A[it_entry->first] should then be 2 * n_A

    //     // the first line becomes 5 * n_A + n_A(for assigning) + 2 * n_A = 8 * n_A

    //     // Second line first part: dense_mapping.reorder[--xndvector[it_entry->second]] will be 4 * n_A
    //     // first one for it_entry->second
    //     // second one for xndvector[it_entry->second]
    //     // third one for --xndvector[it_entry->second]
    //     // fourth one fpr dense_mapping.reorder[--xndvector[it_entry->second]]

    //     // Second line second part:it_entry->first will be n_A

    //     // total second line will be 6 * n_A

    //     // total both lines = 8 * n_A + 6 * n_A = 14 * n_A
    //     for (auto it_entry = entries.crbegin(); it_entry != entries.crend(); ++it_entry) {
    //         dense_mapping.indvector[xndvector[idx_B_in_A[it_entry->first]] - 1] = idx_B_in_A[it_entry->first];
    //         dense_mapping.reorder[--xndvector[it_entry->second]] = it_entry->first;
    //     }
    //     // }

    return dense_mapping;
}

} // namespace power_grid_model
#endif
