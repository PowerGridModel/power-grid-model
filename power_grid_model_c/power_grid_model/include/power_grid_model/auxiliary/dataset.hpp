// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_DATASET_HPP
#define POWER_GRID_MODEL_AUXILIARY_DATASET_HPP

// define dataset classes with void pointers

#include "../power_grid_model.hpp"

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

template <bool is_const>
class DataPointer {
    template <class T>
    using ptr_t = std::conditional_t<is_const, T const*, T*>;

   public:
    DataPointer() : ptr_{nullptr}, indptr_{nullptr}, batch_size_{}, elements_per_scenario_{} {
    }

    // single batch dataset
    DataPointer(ptr_t<void> ptr, Idx single_length)
        : ptr_{ptr}, indptr_{nullptr}, batch_size_{1}, elements_per_scenario_{single_length} {
    }

    // fix batch length
    DataPointer(ptr_t<void> ptr, Idx batch_size, Idx elements_per_scenario)
        : ptr_{ptr}, indptr_{nullptr}, batch_size_{batch_size}, elements_per_scenario_{elements_per_scenario} {
    }

    // variable batches
    DataPointer(ptr_t<void> ptr, Idx const* indptr, Idx batch_size)
        : ptr_{ptr}, indptr_{indptr}, batch_size_{batch_size}, elements_per_scenario_{-1} {
    }

    // copy to const constructor
    DataPointer(ptr_t<void> ptr, Idx const* indptr, Idx batch_size, Idx elements_per_scenario)
        : ptr_{ptr}, indptr_{indptr}, batch_size_{batch_size}, elements_per_scenario_{elements_per_scenario} {
    }

    template <class T>
    std::pair<ptr_t<T>, ptr_t<T>> get_iterators(Idx pos) const {
        assert(pos < batch_size_);
        ptr_t<T> const ptr = reinterpret_cast<ptr_t<T>>(ptr_);
        if (indptr_) {
            if (pos < 0) {
                return std::make_pair(ptr, ptr + indptr_[batch_size_]);
            }
            else {
                return std::make_pair(ptr + indptr_[pos], ptr + indptr_[pos + 1]);
            }
        }
        else {
            if (pos < 0) {
                return std::make_pair(ptr, ptr + elements_per_scenario_ * batch_size_);
            }
            else {
                return std::make_pair(ptr + elements_per_scenario_ * pos, ptr + elements_per_scenario_ * (pos + 1));
            }
        }
    }

    Idx batch_size() const {
        return (Idx)batch_size_;
    }

    Idx elements_per_scenario(Idx pos) const {
        assert(pos >= 0);
        assert(pos < batch_size_);
        if (indptr_) {
            return indptr_[pos + 1] - indptr_[pos];
        }
        else {
            return (Idx)elements_per_scenario_;
        }
    }

    ptr_t<void> raw_ptr() const {
        return ptr_;
    }

    // check if the dataset is one empty batch
    // the length of data should be zero
    bool is_empty() const {
        if (indptr_) {
            return (indptr_[batch_size_] == 0);
        }
        else {
            return batch_size_ == 0 || elements_per_scenario_ == 0;
        }
    }

    // conversion to const iterator
    template <class UX = DataPointer<true>>
    requires(!is_const) explicit operator UX() const {
        return DataPointer<true>{ptr_, indptr_, batch_size_, elements_per_scenario_};
    }

   private:
    ptr_t<void> ptr_;
    Idx const* indptr_;
    Idx batch_size_;             // number of batches
    Idx elements_per_scenario_;  // number of data points per batch, -1 for variable batches
};

using MutableDataPointer = DataPointer<false>;
using ConstDataPointer = DataPointer<true>;

using Dataset = std::map<std::string, MutableDataPointer>;
using ConstDataset = std::map<std::string, ConstDataPointer>;

}  // namespace power_grid_model

#endif