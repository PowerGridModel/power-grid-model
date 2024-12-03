// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_BUFFER_HPP
#define POWER_GRID_MODEL_CPP_BUFFER_HPP

#include "basics.hpp"
#include "handle.hpp"

#include "power_grid_model_c/buffer.h"

namespace power_grid_model_cpp {
class Buffer {
  public:
    Buffer(MetaComponent const* component, Idx size)
        : component_{component}, size_{size}, buffer_{handle_.call_with(PGM_create_buffer, component, size)} {};

    RawDataConstPtr get() const { return buffer_.get(); }
    RawDataPtr get() { return buffer_.get(); }

    Idx size() const { return size_; }

    void set_nan() { set_nan(0, size_); }
    void set_nan(Idx buffer_offset) { set_nan(buffer_offset, 1); }
    void set_nan(Idx buffer_offset, Idx size) {
        handle_.call_with(PGM_buffer_set_nan, component_, get(), buffer_offset, size);
    }

    void set_value(MetaAttribute const* attribute, RawDataConstPtr src_ptr, Idx src_stride) {
        set_value(attribute, src_ptr, 0, size_, src_stride);
    }
    void set_value(MetaAttribute const* attribute, RawDataConstPtr src_ptr, Idx buffer_offset, Idx src_stride) {
        set_value(attribute, src_ptr, buffer_offset, 1, src_stride);
    }
    void set_value(MetaAttribute const* attribute, RawDataConstPtr src_ptr, Idx buffer_offset, Idx size,
                   Idx src_stride) {
        handle_.call_with(PGM_buffer_set_value, attribute, get(), src_ptr, buffer_offset, size, src_stride);
    }

    void get_value(MetaAttribute const* attribute, RawDataPtr dest_ptr, Idx dest_stride) const {
        get_value(attribute, dest_ptr, 0, size_, dest_stride);
    }
    void get_value(MetaAttribute const* attribute, RawDataPtr dest_ptr, Idx buffer_offset, Idx dest_stride) const {
        get_value(attribute, dest_ptr, buffer_offset, 1, dest_stride);
    }
    void get_value(MetaAttribute const* attribute, RawDataPtr dest_ptr, Idx buffer_offset, Idx size,
                   Idx dest_stride) const {
        handle_.call_with(PGM_buffer_get_value, attribute, get(), dest_ptr, buffer_offset, size, dest_stride);
    }

  private:
    Handle handle_{};
    MetaComponent const* component_;
    Idx size_;
    detail::UniquePtr<void, &PGM_destroy_buffer> buffer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_BUFFER_HPP
