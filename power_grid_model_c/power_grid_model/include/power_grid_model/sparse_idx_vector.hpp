// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>
#include <boost/range/counting_range.hpp>

/*
A data-structure for iterating through the indptr, ie. sparse representation of data.
Indptr can be eg: [0, 3, 6, 7]
This means that:
objects 0, 1, 2 are coupled to index 0
objects 3, 4, 5 are coupled to index 1
objects 6 is coupled to index 2

Another intuitive way to look at this for python developers is like list of lists: [[0, 1, 2], [3, 4, 5], [6]].

DenseIdxVector is a vector of element to group. ie. [0, 1, 1, 4] would denote that [[0], [1, 2], [], [], [3]]
The input, ie. [0, 1, 3] should be strictly increasing

*/

namespace power_grid_model::detail {

class SparseIdxVector {
  public:
    SparseIdxVector() = default;
    explicit SparseIdxVector(IdxVector indptr) : indptr_(indptr.empty() ? IdxVector{0} : indptr) {
        assert(!indptr.empty());
    }

    template <class Value>
    class GroupIterator : public boost::iterator_facade<GroupIterator<Value>, Value, boost::random_access_traversal_tag,
                                                        boost::iterator_range<IdxCount>, Idx> {
      public:
        explicit GroupIterator(IdxVector const& indptr, Idx group) : indptr_(indptr), group_(group) {}

      private:
        IdxVector const& indptr_;
        Idx group_;
        friend class boost::iterator_core_access;

        boost::iterator_range<IdxCount> dereference() const {
            return boost::counting_range(indptr_[group_], indptr_[group_ + 1]);
        }
        bool equal(GroupIterator const& other) const { return group_ == other.group_; }
        void increment() { ++group_; }
        void decrement() { --group_; }
        void advance(Idx n) { group_ += n; }
        Idx distance_to(GroupIterator const& other) const { return other.group_ - group_; }
    };

    constexpr auto size() { return indptr_.size() - 1; }
    auto begin() { return GroupIterator<Idx>(indptr_, 0); }
    auto end() { return GroupIterator<Idx>(indptr_, size()); }

    constexpr auto element_size() { return indptr_.back(); }
    auto get_element_range(Idx group) { return boost::iterator_range<IdxCount>(indptr_[group], indptr_[group + 1]); }
    auto get_group(Idx element) {
        assert(element < element_size());
        return static_cast<Idx>(std::upper_bound(indptr_.begin(), indptr_.end(), element) - indptr_.begin() - 1);
    }

  private:
    IdxVector indptr_;
};

class DenseIdxVector {
  public:
    DenseIdxVector() = default;
    explicit DenseIdxVector(IdxVector const& dense_vector, Idx groups_size)
        : dense_vector_(dense_vector), groups_size_(groups_size) {}

    template <class Value>
    class GroupIterator : public boost::iterator_facade<GroupIterator<Value>, Value, boost::bidirectional_traversal_tag,
                                                        boost::iterator_range<IdxCount>, Idx> {
      public:
        constexpr GroupIterator() = default;
        explicit constexpr GroupIterator(IdxVector const& dense_vector, Idx const& group)
            : dense_vector_{dense_vector},
              group_{group},
              group_range_{std::equal_range(std::cbegin(dense_vector_), std::cend(dense_vector_), group)} {}

      private:
        using group_iterator = IdxVector::const_iterator;

        IdxVector const& dense_vector_;
        Idx group_;
        std::pair<group_iterator, group_iterator> group_range_;

        friend class boost::iterator_core_access;

        boost::iterator_range<IdxCount> dereference() const {
            return boost::counting_range(std::distance(std::cbegin(dense_vector_), group_range_.first),
                                         std::distance(std::cbegin(dense_vector_), group_range_.second));
        }
        constexpr bool equal(GroupIterator const& other) const { return group_ == other.group_; }
        constexpr void increment() { advance(1); }
        constexpr void decrement() { advance(-1); }
        constexpr void advance(Idx n) {
            auto const start = n > 0 ? group_range_.second : std::cbegin(dense_vector_);
            auto const stop = n > 0 ? std::cend(dense_vector_) : group_range_.first;

            group_ += n;
            group_range_ = std::equal_range(start, stop, group_);
        }
    };

    constexpr auto size() { return groups_size_; }
    constexpr auto begin() { return GroupIterator<Idx>{dense_vector_, 0}; }
    constexpr auto end() { return GroupIterator<Idx>{dense_vector_, size()}; }

    constexpr auto element_size() { return dense_vector_.size(); }
    constexpr auto get_group(Idx element) { return dense_vector_[element]; }
    auto get_element_range(Idx group) {
        auto dense_begin = dense_vector_.begin();
        auto dense_end = dense_vector_.end();
        auto range_pair = std::equal_range(dense_begin, dense_end, group);
        return boost::iterator_range<IdxCount>(range_pair.first - dense_begin, range_pair.second - dense_begin);
    }

  private:
    IdxVector dense_vector_;
    Idx groups_size_;
};

template <typename T>
concept sparse_type = std::is_same<T, SparseIdxVector>::value;

template <sparse_type First, sparse_type... Rest> auto zip_sequence(First& first, Rest&... rest) {

    assert((first.size() == rest.size()) && ...);
    // TODO Add common index as count at the first postiion
    // auto common_idx_counter = boost::iterator_range<IdxCount>(0, first.size());

    auto zip_begin = boost::make_zip_iterator(boost::make_tuple(first.begin(), rest.begin()...));
    auto zip_end = boost::make_zip_iterator(boost::make_tuple(first.end(), rest.end()...));
    return boost::make_iterator_range(zip_begin, zip_end);
}

} // namespace power_grid_model::detail

#endif