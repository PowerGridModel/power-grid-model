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
//     and the size indicates the number of *data points* in the set
// the dataset can also be a batch of sets
//     the indptr is an integer array, for i-th sets, the set data is in the range [ indptr[i], indptr[i + 1] )
//     the size indicates the number of *batches* in the whole set
template <bool is_const>
class DataPointer {
    template <class T>
    using ptr_t = std::conditional_t<is_const, T const*, T*>;

   public:
    DataPointer() : ptr_{nullptr}, indptr_{nullptr}, batch_size_{}, length_per_batch_{} {
    }

    // single batch dataset
    DataPointer(ptr_t<void> ptr, Idx single_length)
        : ptr_{ptr}, indptr_{nullptr}, batch_size_{1}, length_per_batch_{single_length} {
    }

    // fix batch length
    DataPointer(ptr_t<void> ptr, Idx batch_size, Idx length_per_batch)
        : ptr_{ptr}, indptr_{nullptr}, batch_size_{batch_size}, length_per_batch_{length_per_batch} {
    }

    // variable batches
    DataPointer(ptr_t<void> ptr, Idx const* indptr, Idx batch_size)
        : ptr_{ptr}, indptr_{indptr}, batch_size_{batch_size}, length_per_batch_{-1} {
    }

    // copy to const constructor
    DataPointer(ptr_t<void> ptr, Idx const* indptr, Idx batch_size, Idx length_per_batch)
        : ptr_{ptr}, indptr_{indptr}, batch_size_{batch_size}, length_per_batch_{length_per_batch} {
    }

    template <class T>
    std::pair<ptr_t<T>, ptr_t<T>> get_iterators(Idx pos) const {
        ;
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
                return std::make_pair(ptr, ptr + length_per_batch_ * batch_size_);
            }
            else {
                return std::make_pair(ptr + length_per_batch_ * pos, ptr + length_per_batch_ * (pos + 1));
            }
        }
    }

    Idx batch_size() const {
        return batch_size_;
    }

    Idx length_per_batch(Idx pos) const {
        assert(pos >= 0);
        assert(pos < batch_size_);
        if (indptr_) {
            return indptr_[pos + 1] - indptr_[pos];
        }
        else {
            return length_per_batch_;
        }
    }

    ptr_t<void> raw_ptr() const {
        return ptr_;
    }

    // check if the dataset is one empty batch
    // the batch size should be one
    // the length of data should be zero
    bool is_empty() const {
        if (indptr_) {
            return (indptr_[batch_size_] == 0);
        }
        else {
            return batch_size_ == 0 || length_per_batch_ == 0;
        }
    }

    // conversion to const iterator
    template <class UX = DataPointer<true>>
    operator std::enable_if_t<!is_const, UX>() const {
        return DataPointer<true>{ptr_, indptr_, batch_size_, length_per_batch_};
    }

   private:
    ptr_t<void> ptr_;
    Idx const* indptr_;
    Idx batch_size_;       // number of batches
    Idx length_per_batch_;  // number of data points per batch, -1 for variable batches
};

using MutableDataPointer = DataPointer<false>;
using ConstDataPointer = DataPointer<true>;

using Dataset = std::map<std::string, MutableDataPointer>;
using ConstDataset = std::map<std::string, ConstDataPointer>;

}  // namespace power_grid_model

#endif
