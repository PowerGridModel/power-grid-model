// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"
#include "common/grouped_index_vector.hpp"

#include <numeric>
#include <ranges>

namespace power_grid_model {

struct SparseIndexMapping {
    IdxVector indptr;
    IdxVector reorder;
};

/// Sparse mapping technique
///
/// using counting sort: https://en.wikipedia.org/wiki/Counting_sort
/// Given a input idx array: idx_B_in_A[...] to couple an array of A and an array of B,
/// i.e. idx_B_in_A[i_A] = j_B.
///
/// For entry i_A in the array, idx_B_in_A[i_A] is the idx of B which couples the A object #i_A.
///
/// This sparse mapping tries to build a CSC sparse matrix so that
/// only entry (i, idx_B_in_A[i_A] = j_B) is filled with the sequence of A, i.e. #i_A.
///
/// In CSC format, the entries are ordered by the idx of B
/// therefore, in indptr, each range indptr[j_B:j_B+1]
/// represents the entries of A where B object #j_B is coupled.
///
/// The indices array is not interesting here.
/// The data array is original index of A. We call it re-order array.
/// This can be used to reorder A objects by the coupling of B idx.
///
/// Example.
/// For original idx_B_in_A == [3, 5, 2, 1, 1, 2]
/// size of A is 6
/// size of B is 7
/// Result matrix
/// indptr = [0, 0, 2, 4, 5, 5, 6, 6]
/// data/reorder = [3, 4, 2, 5, 0, 1]
/// to read:
///     nothing		coupled to B 0
///     A 3, 4		coupled to B 1
///     A 2, 5		coupled to B 2
///     A 0			coupled to B 3
///     nothing		coupled to B 4
///     A 1			coupled	to B 5
///     nothing	    coupled to B 6
inline SparseIndexMapping build_sparse_mapping(IdxVector const& idx_B_in_A, Idx const n_B) {
    using SparseEntry = std::pair<Idx, Idx>;

    auto const n_A = static_cast<Idx>(idx_B_in_A.size());

    std::vector<SparseEntry> entries(n_A);
    std::ranges::transform(idx_B_in_A, IdxRange{static_cast<Idx>(idx_B_in_A.size())}, entries.begin(),
                           [](Idx j_B, Idx i_A) { return SparseEntry{i_A, j_B}; });

    SparseIndexMapping sparse_mapping;
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

struct DenseIndexMapping {
    IdxVector indvector;
    IdxVector reorder;
};

namespace index_mapping::detail {

inline auto build_dense_mapping_comparison_sort(IdxVector const& idx_B_in_A, Idx const /* n_B */) {
    using DenseEntry = std::pair<Idx, Idx>;

    std::vector<DenseEntry> mapping_to_from;
    mapping_to_from.reserve(idx_B_in_A.size());
    std::ranges::transform(idx_B_in_A, IdxRange{static_cast<Idx>(idx_B_in_A.size())},
                           std::back_inserter(mapping_to_from),
                           [](Idx value, Idx orig_idx) { return std::pair{value, orig_idx}; });

    std::ranges::sort(mapping_to_from);

    DenseIndexMapping result;
    result.indvector.reserve(mapping_to_from.size());
    result.reorder.reserve(mapping_to_from.size());
    std::ranges::transform(mapping_to_from, std::back_inserter(result.indvector),
                           [](DenseEntry const& to_from) { return to_from.first; });

    std::ranges::transform(mapping_to_from, std::back_inserter(result.reorder),
                           [](DenseEntry const& to_from) { return to_from.second; });

    return result;
}

inline auto build_dense_mapping_counting_sort(IdxVector const& idx_B_in_A, Idx const n_B) {
    auto sparse_result = build_sparse_mapping(idx_B_in_A, n_B);

    return DenseIndexMapping{.indvector = power_grid_model::detail::sparse_decode(sparse_result.indptr),
                             .reorder = std::move(sparse_result.reorder)};
}

struct IndexMappingApproachCriterion {
    double n_a_prefactor{};
    double n_a_log_n_a_prefactor{};
    double constant{};

    constexpr bool operator()(std::integral auto n_A, std::integral auto n_B) const {
        auto const n_A_ = static_cast<double>(n_A);
        auto const n_B_ = static_cast<double>(n_B);

        return n_B_ < n_a_prefactor * n_A_ + n_a_log_n_a_prefactor * n_A_ * log(n_A_) + constant;
    }
};

constexpr IndexMappingApproachCriterion index_mapping_criterion_gcc{.n_a_prefactor = -0.00733595283054587,
                                                                    .n_a_log_n_a_prefactor = 0.01888288636738604,
                                                                    .constant = 20.338844396105696};

} // namespace index_mapping::detail

inline DenseIndexMapping build_dense_mapping(IdxVector const& idx_B_in_A, Idx const n_B) {
    if (index_mapping::detail::index_mapping_criterion_gcc(idx_B_in_A.size(), n_B)) {
        return index_mapping::detail::build_dense_mapping_counting_sort(idx_B_in_A, n_B);
    }
    return index_mapping::detail::build_dense_mapping_comparison_sort(idx_B_in_A, n_B);
}

} // namespace power_grid_model
