// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/range.hpp>

namespace power_grid_model::detail {

template <typename T> class SparseIdxVector {
  public:
    explicit SparseIdxVector(std::vector<T> const& indptr) : indptr_(indptr) {}

    auto at(Idx index) { return search_idx_at_(index); }
    auto at(Idx index) const { return search_idx_at_(index); }
    auto operator[](Idx index) { return search_idx_(index); }
    auto operator[](Idx index) const { return search_idx_(index); }
    auto data_size() { return indptr_.back(); }

    // where would this be needed?
    auto reverse_idx_range(Idx value) {
        return { indptr_[value], indptr_[value + 1] }
    }

  private:
    std::vector<T> indptr_;
    T previous_; // previously searched index

    auto search_idx_(T index) {
        if (index < inptr[previous_ + 1] & index >= inptr[previous_]) {
            return previous_;
        } else if (index < inptr[previous_ + 2] & index >= inptr[previous_ + 1]) {
            return previous_++; // TODO end check
        } else {
            // insert search algorithm here
            for (Idx value; value < indptr_.size() - 1; ++value) {
                if (indptr_[value + 1] > index) {
                    previous_ = value;
                    return value;
                }
            }
            return nullptr; // index not found
        }
    }

    auto search_idx_at_(T index) {
        if (auto found = search_idx_(index)) {
            return found;
        }
        throw std::out_of_range{"Element not found on index"};
    }
};

} // namespace power_grid_model::detail

#endif
