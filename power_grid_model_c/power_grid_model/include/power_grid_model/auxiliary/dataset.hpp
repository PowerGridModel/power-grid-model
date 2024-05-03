// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0
/*
#pragma once

// define dataset classes with void pointers

#include "dataset_fwd.hpp"

#include "../common/common.hpp"

#include <cassert>
#include <map>

namespace power_grid_model {

// a dataset wrapper using void pointers
// the void pointers can be cast to relevant types
// the dataset is either one set of data
//     the indptr is a nullptr
//     the batch_size is one
// the dataset can also be a batch of sets with different length per batch
//     the indptr is an integer array, for i-th sets,
//          the set data is in the range [ indptr[i], indptr[i + 1] )
// the dataset can also be a batch of sets with the same length per batch
//     the indptr is a nullptr, for i-th sets,
//          the set data is in the range [ i * elements_per_scenario, (i + 1) * elements_per_scenario )

template <dataset_type_tag T> constexpr bool is_const_dataset_v = std::same_as<T, const_dataset_t>;

static_assert(dataset_type_tag<const_dataset_t>);
static_assert(dataset_type_tag<mutable_dataset_t>);
static_assert(is_const_dataset_v<const_dataset_t>);
static_assert(!is_const_dataset_v<mutable_dataset_t>);

template <dataset_type_tag dataset_type_> class DataPointer {
  public:
    using dataset_type = dataset_type_;

    template <class T> std::pair<ptr_t<T>, ptr_t<T>> get_iterators(Idx pos) const {
        assert(pos < batch_size_);
        auto* const ptr = reinterpret_cast<ptr_t<T>>(ptr_);
        if (indptr_) {
            if (pos < 0) {
                return std::make_pair(ptr, ptr + indptr_[batch_size_]);
            }
            return std::make_pair(ptr + indptr_[pos], ptr + indptr_[pos + 1]);
        }
        if (pos < 0) {
            return std::make_pair(ptr, ptr + elements_per_scenario_ * batch_size_);
        }
        return std::make_pair(ptr + elements_per_scenario_ * pos, ptr + elements_per_scenario_ * (pos + 1));
    }


    Idx elements_per_scenario(Idx pos) const {
        assert(pos >= 0);
        assert(pos < batch_size_);
        if (indptr_ != nullptr) {
            return indptr_[pos + 1] - indptr_[pos];
        }
        return elements_per_scenario_;
    }

    ptr_t<void> raw_ptr() const { return ptr_; }

    // check if the dataset is one empty batch
    // the length of data should be zero
    bool is_empty() const {
        if (indptr_ != nullptr) {
            return (indptr_[batch_size_] == 0);
        }
        return batch_size_ == 0 || elements_per_scenario_ == 0;
    }

  private:
    ptr_t<void> ptr_;
    Idx const* indptr_;
    Idx batch_size_;            // number of batches
    Idx elements_per_scenario_; // number of data points per batch, -1 for variable batches
};

} // namespace power_grid_model
*/