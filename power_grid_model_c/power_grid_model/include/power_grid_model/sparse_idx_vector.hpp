// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

namespace power_grid_model::detail {

class SparseIdxVector {
  public:
    explicit SparseIdxVector(IdxVector const& indptr) : indptr_(indptr), previous_{indptr.front()} {}

    auto at(Idx const index) { return search_idx_(index); }
    auto operator[](Idx const index) { return search_idx_(index); }
    auto data_size() { return indptr_.back(); }

  private:
    IdxVector indptr_;
    Idx previous_; // previously searched index

    Idx search_idx_(Idx const index) {
        // insert search algorithm here
        for (Idx value = 0; value != indptr_.size(); ++value) {
            if (index >= indptr_[value] && index < indptr_[value + 1]) {
                previous_ = value;
                return value;
            }
        }
        throw std::out_of_range{"Element not found on index"};
    }
};

} // namespace power_grid_model::detail

#endif
