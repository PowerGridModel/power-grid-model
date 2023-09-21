// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/range.hpp>

namespace power_grid_model::detail {

template <typename ValueType> class SparseIdxVector {
  public:
    using ValueTypeVector = std::vector<ValueType>; // Can be Idx, can be array<Idx, 2>
    using KeyType = Idx; // Question: Should we assume this as size_t? This would eliminate need for this. Type needed
                         // at for eg. n_load_gen. Can cast at output.

  public:
    explicit SparseIdxVector(ValueTypeVector const& indptr) : indptr_(indptr) {}

    class IteratorBase {
      public:
        using difference_type = std::ptrdiff_t; // Verify if correct
        using pointer = ValueType*;
        using reference = ValueType&;
        using iterator_category = std::forward_iterator_tag;

        bool operator!=(IteratorBase const& other) { return !(*this == other); }

        ValueTypeVector const& indptr_;
    };

    class Iterator {
      public:
        using difference_type = std::ptrdiff_t; // Verify if correct
        using value_pointer = ValueType*;
        using value_reference = ValueType&;
        using key_pointer = KeyType*;
        using key_reference = KeyType&;
        using iterator_category = std::forward_iterator_tag;

      public:
        Iterator(ValueTypeVector const& indptr, size_t const& group, size_t const& element)
            : indptr_(indptr), group_(group), element_(element) {}

        friend bool operator==(Iterator const& lhs, Iterator const& rhs) {
            return lhs.group_ == rhs.group_ && lhs.element_ == rhs.element_;
        }
        friend bool operator!=(Iterator const& lhs, Iterator const& rhs) { return !(lhs == rhs); }

        KeyType operator*() {
            return convert_size_type(group_);
        } // TODO Should be reference but its not the element being returned, but conversion
        // auto operator*() const { return std::make_tuple(group_, indptr_[group_] + element_); }

        KeyType size() { return convert_size_type(indptr_.back()); }

        Iterator& operator++() {
            ++element_;
            if (element_ == group_size(group_)) {
                element_ = 0;
                ++group_;
            }
            return *this;
        }

        KeyType operator[](ValueType const& index) { return convert_size_type(search_idx(index)); }

        Iterator begin() { return Iterator{indptr_, 0, 0}; }
        Iterator end() { return Iterator{indptr_, indptr_.size() - 1, 0}; }

      private:
        ValueTypeVector const& indptr_{};
        size_t group_{};
        size_t element_{};

        KeyType convert_size_type(size_t const& size_t_to_convert) { return static_cast<KeyType>(size_t_to_convert); }

        size_t group_size(size_t const& group_number) { return indptr_[group_number + 1] - indptr_[group_number]; }

        size_t search_idx(ValueType const& index) {
            // insert search algorithm here
            for (size_t group = 0; group != indptr_.size(); ++group) {
                if (index >= indptr_[group] && index < indptr_[group + 1]) {
                    return group;
                }
            }
            throw std::out_of_range{"Element not found on index"};
        }
    };

    class ElementRange {
      public:
        ElementRange(ValueTypeVector const& indptr, size_t group_begin, size_t group_end, size_t element_begin,
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

        Iterator begin() { return Iterator{indptr_, group_begin_, element_begin_}; }
        Iterator end() { return Iterator{indptr_, group_end_, element_end_}; }

      protected:
        ValueTypeVector const& indptr_;
        size_t group_begin_;
        size_t group_end_;
        size_t element_begin_;
        size_t element_end_;
    };

    class GroupIterator : public ElementRange {
      public:
        GroupIterator(ValueTypeVector const& indptr, size_t group_begin, size_t group_end, size_t element_begin,
                      size_t element_end)
            : ElementRange{indptr, group_begin, group_end, element_begin, element_end} {}

        GroupIterator& operator++() {
            ++this->group_begin_;
            ++this->group_end_;
            return *this;
        }

        ElementRange operator*() { return *this; }

        ElementRange operator[](size_t const group) { return ElementRange{this->indptr_, group, group + 1, 0, 0}; }

        size_t size() { return this->indptr_.size() - 1; }
        GroupIterator begin() { return GroupIterator{this->indptr_, 0, 1, 0, 0}; }
        GroupIterator end() {
            return GroupIterator{this->indptr_, this->indptr_.size() - 1, this->indptr_.size(), 0, 0};
        }
    };

    Iterator values() { return Iterator(indptr_, 0, 0); }
    // ElementIteratorItems items() { return ElementIteratorItems(indptr_, 0, 0); }

    ElementRange value_range(ValueType const& value_begin, ValueType const& value_end) {
        auto group_begin = values()[value_begin];
        auto group_end = values()[value_end];
        auto element_begin = value_begin - indptr_[group_begin];
        auto element_end = value_end - indptr_[group_end];
        return ElementRange(indptr_, group_begin, group_end, element_begin, element_end);
    }

    GroupIterator groups() { return GroupIterator(indptr_, 0, indptr_.size() - 1, 0, 0); }

    // private:
    ValueTypeVector const& indptr_;
};

} // namespace power_grid_model::detail

#endif
