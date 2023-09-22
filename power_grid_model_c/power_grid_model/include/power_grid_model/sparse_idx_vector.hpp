// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/range.hpp>

namespace power_grid_model::detail {

struct element_view_t {};
struct items_view_t {};

template <class T>
concept ElementView = std::same_as<element_view_t, T>; // TODO Replace Idx

template <class T>
concept ItemsView = std::same_as<items_view_t, T>;

template <typename ValueType> class SparseIdxVector {
  public:
    using ValueTypeVector = std::vector<ValueType>; // Can be Idx, can be array<Idx, 2>
    // The keys of vector are size_t. Outside this class, the indexing is with Idx instead of size_t, hence conversion.
    using KeyType = Idx;

  public:
    explicit SparseIdxVector(ValueTypeVector const& indptr) : indptr_(indptr) {}

    template <typename IteratorViewType> class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;

      public:
        Iterator(ValueTypeVector const& indptr, size_t const& group, size_t const& element)
            : indptr_(indptr), group_(group), element_(element) {}

        friend bool operator==(Iterator const& lhs, Iterator const& rhs) {
            return lhs.group_ == rhs.group_ && lhs.element_ == rhs.element_;
        }
        friend bool operator!=(Iterator const& lhs, Iterator const& rhs) { return !(lhs == rhs); }

        auto operator*()
            requires ElementView<IteratorViewType>
        {
            return static_cast<KeyType>(group_);
        }

        auto operator*()
            requires ItemsView<IteratorViewType>
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
        ValueTypeVector const& indptr_{};
        size_t group_{};
        size_t element_{};

        auto group_size(size_t const& group_number) { return indptr_[group_number + 1] - indptr_[group_number]; }

        auto search_idx(ValueType const& index) {
            // insert search algorithm here
            for (size_t group = 0; group != indptr_.size(); ++group) {
                if (index >= indptr_[group] && index < indptr_[group + 1]) {
                    return group;
                }
            }
            throw std::out_of_range{"Element not found on index"};
        }
    };

    template <typename IteratorViewType> class ElementRange {
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

        Iterator<IteratorViewType> begin() { return Iterator<IteratorViewType>{indptr_, group_begin_, element_begin_}; }
        Iterator<IteratorViewType> end() { return Iterator<IteratorViewType>{indptr_, group_end_, element_end_}; }

      protected:
        ValueTypeVector const& indptr_;
        size_t group_begin_;
        size_t group_end_;
        size_t element_begin_;
        size_t element_end_;
    };

    template <typename IteratorViewType> class GroupIterator : public ElementRange<IteratorViewType> {
      public:
        using ElementRangeType = ElementRange<IteratorViewType>;

        GroupIterator(ValueTypeVector const& indptr, size_t group_begin, size_t group_end, size_t element_begin,
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
    ValueTypeVector const& indptr_;
};

} // namespace power_grid_model::detail

#endif
