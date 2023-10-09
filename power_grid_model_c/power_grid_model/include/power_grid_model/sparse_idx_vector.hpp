// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range.hpp>
#include <boost/range/combine.hpp>
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
    explicit SparseIdxVector(IdxVector indptr) : indptr_(indptr) {}

    template <class Value>
    class Iterator : public boost::iterator_facade<Iterator<Value>, Value, boost::random_access_traversal_tag,
                                                   boost::iterator_range<IdxCount>, Idx> {
      public:
        explicit Iterator(IdxVector indptr, Idx idx) : indptr_(indptr), idx_(idx) {}

      private:
        IdxVector indptr_;
        Idx idx_;
        friend class boost::iterator_core_access;

        boost::iterator_range<IdxCount> dereference() const {
            return boost::counting_range(indptr_[idx_], indptr_[idx_ + 1]);
        }
        bool equal(Iterator const& other) const { return idx_ == other.idx_; }
        void increment() { ++idx_; }
        void decrement() { --idx_; }
        void advance(Idx n) { idx_ += n; }
        Idx distance_to(Iterator const& other) const { return other.idx_ - idx_; }
    };

    auto get_element_range(Idx group) { return boost::iterator_range<IdxCount>(indptr_[group], indptr_[group + 1]); }

    Idx get_group(Idx element) {
        return static_cast<Idx>(std::upper_bound(indptr_.begin(), indptr_.end(), element) - indptr_.begin());
    }

    auto begin() { return Iterator<Idx>(indptr_, 0); }
    auto end() { return Iterator<Idx>(indptr_, indptr_.size() - 1); }
    auto size() { return indptr_.back();  }


  private:
    IdxVector indptr_;
};

class DenseIdxVector {
  public:
    explicit DenseIdxVector(IdxVector dense_idx_vector) : dense_idx_vector_(dense_idx_vector) {}

    auto get_group(Idx element) { return dense_idx_vector_[element]; }

    auto get_element_range(Idx group) {
        auto dense_begin = dense_idx_vector_.begin();
        auto dense_end = dense_idx_vector_.end();
        auto range_pair = std::equal_range(dense_begin, dense_end, group);
        return boost::iterator_range<IdxCount>(range_pair.first - dense_begin, range_pair.second - dense_begin);
    }

  private:
    IdxVector dense_idx_vector_;
};

template<typename T>
concept sparse_type = std::is_same<T, SparseIdxVector>::value;

template <sparse_type... T>
auto zip_idx_vector(T&... sparse_vectors) {
    // assert(all(sparse_vectors.size()...))

    // template<bool...> struct bool_pack;
    // template<bool... bs> 
    // using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;

    // static_assert(all_true<(Numbers == 0 || Numbers == 1)...>::value, "");
    // Check if all are of same size
    // Add index at the start position

    auto zip_begin = boost::make_zip_iterator(boost::make_tuple(sparse_vectors.begin()...));
    auto zip_end = boost::make_zip_iterator(boost::make_tuple(sparse_vectors.end()...));
    return boost::make_iterator_range(zip_begin, zip_end);
}

} // namespace power_grid_model::detail

#endif
