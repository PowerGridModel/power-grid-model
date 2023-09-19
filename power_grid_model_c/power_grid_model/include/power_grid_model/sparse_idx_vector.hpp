// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/range.hpp>

namespace power_grid_model::detail {

class SparseIdxVector {
  public:
    explicit SparseIdxVector(IdxVector const& indptr) : indptr_(indptr) {}

    class Iterator {
      public:
        Iterator(IdxVector const& indptr, Idx group, Idx element)
            : indptr_(indptr), size_(static_cast<Idx>(indptr.size())), group_(group), element_(element) {}

        bool operator!=(Iterator const& other) { return !(group_ == other.group_ && element_ == other.element_); }

        bool operator==(Iterator const& other) { return (group_ == other.group_ && element_ == other.element_); }

      protected:
        IdxVector const& indptr_;
        Idx size_;
        Idx group_;
        Idx element_;
    };

    class ElementIterator : public Iterator {
      public:
        ElementIterator(IdxVector const& indptr, Idx group, Idx element) : Iterator(indptr, group, element) {}

        Idx operator*() const { return group_; }

        Idx size() { return indptr_.back(); }

        ElementIterator& operator++() {
            ++element_;
            auto const group_size = indptr_[group_ + 1] - indptr_[group_];
            if (element_ == group_size) {
                element_ = 0;
                ++group_;
            }
            return *this;
        }

        ElementIterator begin() { return ElementIterator{indptr_, 0, 0}; }
        ElementIterator end() { return ElementIterator{indptr_, size_ - 1, 0}; }

        Idx operator[](Idx const& index) { return search_idx_(index); }

      private:
        Idx search_idx_(Idx const& element) {
            // insert search algorithm here
            for (Idx group = 0; group != size_; ++group) {
                if (element >= indptr_[group] && element < indptr_[group + 1]) {
                    return group;
                }
            }
            throw std::out_of_range{"Element not found on index"};
        }
    };

    class ElementIteratorItems : public ElementIterator {
      public:
        ElementIteratorItems(IdxVector const& indptr, Idx group, Idx element)
            : ElementIterator(indptr, group, element) {}
        auto operator*() const { return std::make_pair(group_, indptr_[group_] + element_); }
        auto operator[](Idx const& index) const { return std::make_pair(*this[index], index); }
    };

    class GroupIterator : public Iterator {
      public:
        GroupIterator(IdxVector const& indptr, Idx group, Idx element) : Iterator(indptr, group, element) {}

        ElementIterator operator*() const {
            IdxVector subset_indptr{indptr_[group_], indptr_[group_ + 1]};
            return ElementIterator(subset_indptr, group_, 0);
        }

        Idx size() { return size_ - 1; }

        GroupIterator& operator++() {
            ++group_;
            return *this;
        }

        GroupIterator begin() { return GroupIterator{indptr_, 0, 0}; }
        GroupIterator end() { return GroupIterator{indptr_, size_, 0}; }

        ElementIterator operator[](Idx const group) {
            IdxVector subset_indptr{indptr_[group], indptr_[group + 1]};
            return ElementIterator(subset_indptr, group, 0);
        }
    };

    ElementIterator element_iter() { return ElementIterator(indptr_, 0, 0); }
    ElementIterator element_iter_items() { return ElementIteratorItems(indptr_, 0, 0); }
    GroupIterator group_iter() { return GroupIterator(indptr_, 0, 0); }

  private:
    IdxVector const& indptr_;
};

} // namespace power_grid_model::detail

#endif
