// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/range.hpp>

/*
A data-structure for iterating through the indptr, ie. sparse representation of data.
Indptr can be eg: [0, 3, 6, 7]
This means that:
objects 0, 1, 2 are coupled to index 0
objects 3, 4, 5 are coupled to index 1
objects 6 is coupled to index 2

Another intuitive way to look at this for python developers is like list of lists: [[0, 1, 2], [3, 4, 5], [6]].

SparseIdxVector will help iterate over this indptr like a map via the Iterator interface directly.

While using iterator operations, the 'values' view dereferences the index of outer list. And the 'items' view
derferences [index of outer list, value]

The 'items' and 'values' functions iterate through each element.
The 'groups_items' and 'groups_values' functions iterates through the outer index.

*/

namespace power_grid_model::detail {

// Tags for items or values iterator.
struct element_view_t {};
struct items_view_t {};

template <class T>
concept ElementView = std::same_as<element_view_t, T>;

template <class T>
concept ItemsView = std::same_as<items_view_t, T>;

template <class T>
concept IteratorView = ElementView<T> || ItemsView<T>;

template <typename ValueType> class SparseIdxVector {
  public:
    using ValueVector = std::vector<ValueType>; // ValueType Can be Idx, can be array<Idx, 2>
    // The keys of vector are size_t. Outside this class, the indexing is with Idx instead of size_t, hence conversion.
    using KeyType = Idx;

  public:
    explicit SparseIdxVector(ValueVector const& indptr) : indptr_(indptr) {}

    template <IteratorView ViewTag> class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;

      public:
        Iterator(ValueVector const& indptr, size_t const& group, size_t const& element)
            : indptr_(indptr), group_(group), element_(element) {}

        friend bool operator==(Iterator const& lhs, Iterator const& rhs) {
            return lhs.group_ == rhs.group_ && lhs.element_ == rhs.element_;
        }
        friend bool operator!=(Iterator const& lhs, Iterator const& rhs) { return !(lhs == rhs); }

        auto operator*()
            requires ElementView<ViewTag>
        {
            return static_cast<KeyType>(group_);
        }

        auto operator*()
            requires ItemsView<ViewTag>
        {
            return std::make_tuple(group_, indptr_[group_] + element_);
        }

        auto size() { return static_cast<KeyType>(indptr_.back()); }

        auto& operator++() {
            ++element_;
            if (element_ == group_size(group_)) {
                element_ = 0;
                ++group_;
            }
            return *this;
        }

        auto operator[](ValueType const& index) { return static_cast<KeyType>(search_idx(index)); }

        auto begin() { return Iterator{indptr_, 0, 0}; }
        auto end() { return Iterator{indptr_, indptr_.size() - 1, 0}; }

      private:
        ValueVector const& indptr_{};
        size_t group_{};
        size_t element_{};

        auto group_size(size_t const& group_number) const { return indptr_[group_number + 1] - indptr_[group_number]; }

        auto search_idx(ValueType const& index) {
            // insert search algorithm here
            for (size_t group = 0; group != indptr_.size(); ++group) {
                if (index >= indptr_[group] && index < indptr_[group + 1]) {
                    return group;
                }
            }
            throw std::out_of_range{"Element not found on index"};
        }

        // TODO Improve search function
        // auto search_idx(ValueType const& index)  const {
        //     KeyType lower{0};
        //     auto upper = static_cast<KeyType>(indptr_.size() - 1);
        //     auto lower_value = indptr_[lower];
        //     auto upper_value = indptr_[upper];

        //     while (lower < upper) {
        //         auto test = (upper - lower) / 2;
        //         auto test_value = indptr_[test];
        //         if (test_value < index) {
        //             lower = test;
        //             lower_value = test_value;
        //         } else {
        //             upper = test;
        //             upper_value = test_value;
        //         }
        //     }
        //     return lower;
        // }
    };

    template <typename ViewTag> class ElementRange {
      public:
        ElementRange(ValueVector const& indptr, size_t group_begin, size_t group_end, size_t element_begin,
                     size_t element_end)
            : indptr_(indptr),
              group_begin_(group_begin),
              group_end_(group_end),
              element_begin_(element_begin),
              element_end_(element_end) {}

        friend bool operator==(ElementRange const& lhs, ElementRange const& rhs) {
            return lhs.group_begin_ == rhs.group_begin_ && lhs.element_begin_ == rhs.element_begin_ &&
                   lhs.group_end_ == rhs.group_end_ && lhs.element_end_ == rhs.element_end_;
        }
        friend bool operator!=(ElementRange const& lhs, ElementRange const& rhs) { return !(lhs == rhs); }

        Iterator<ViewTag> begin() { return Iterator<ViewTag>{indptr_, group_begin_, element_begin_}; }
        Iterator<ViewTag> end() { return Iterator<ViewTag>{indptr_, group_end_, element_end_}; }

      protected:
        ValueVector const& indptr_;
        size_t group_begin_;
        size_t group_end_;
        size_t element_begin_;
        size_t element_end_;
    };

    template <typename ViewTag> class GroupIterator : public ElementRange<ViewTag> {
      public:
        using ElementRangeType = ElementRange<ViewTag>;

        GroupIterator(ValueVector const& indptr, size_t group_begin, size_t group_end, size_t element_begin,
                      size_t element_end)
            : ElementRangeType{indptr, group_begin, group_end, element_begin, element_end} {}

        GroupIterator& operator++() {
            ++this->group_begin_;
            ++this->group_end_;
            return *this;
        }

        auto operator*() { return static_cast<ElementRangeType>(*this); }

        auto operator[](size_t const group) { return ElementRangeType{this->indptr_, group, group + 1, 0, 0}; }

        size_t size() { return this->indptr_.size() - 1; }
        auto begin() { return GroupIterator{this->indptr_, 0, 1, 0, 0}; }
        auto end() { return GroupIterator{this->indptr_, this->indptr_.size() - 1, this->indptr_.size(), 0, 0}; }
    };

    auto groups_values() { return GroupIterator<element_view_t>(indptr_, 0, indptr_.size() - 1, 0, 0); }
    auto groups_items() { return GroupIterator<items_view_t>(indptr_, 0, indptr_.size() - 1, 0, 0); }

    auto values() { return Iterator<element_view_t>(indptr_, 0, 0); }
    auto items() { return Iterator<items_view_t>(indptr_, 0, 0); }

  private:
    ValueVector const& indptr_;
};

} // namespace power_grid_model::detail

#endif
