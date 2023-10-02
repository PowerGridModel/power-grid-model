// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP
#define POWER_GRID_MODEL_SPARSE_IDX_VECTOR_HPP

#include "power_grid_model.hpp"

#include <boost/iterator/iterator_facade.hpp>
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

*/

namespace power_grid_model::detail {

class SparseIdxVector {
  public:
    explicit SparseIdxVector(IdxVector indptr) : indptr_(indptr) {}

    auto group(Idx group) { return boost::iterator_range<IdxCount>(indptr_[group], indptr_[group + 1]); }

  private:
    IdxVector indptr_;
};

} // namespace power_grid_model::detail

#endif
