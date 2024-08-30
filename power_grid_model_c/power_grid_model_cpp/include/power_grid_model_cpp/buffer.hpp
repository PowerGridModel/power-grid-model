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

    RawDataPtr get() const { return buffer_.get(); }

    Idx size() const { return size_; }

    static void set_nan(Buffer& buffer, Idx buffer_offset, Idx size) {
        buffer.handle_.call_with(PGM_buffer_set_nan, buffer.component_, buffer.buffer_.get(), buffer_offset, size);
    }
    void set_nan() { set_nan(*this, 0, size_); }
    void set_nan(Idx buffer_offset) { set_nan(*this, buffer_offset, 1); }
    void set_nan(Idx buffer_offset, Idx size) { set_nan(*this, buffer_offset, size); }

    static void set_value(MetaAttribute const* attribute, Buffer const& buffer, RawDataConstPtr src_ptr,
                          Idx buffer_offset, Idx size, Idx src_stride) {
        buffer.handle_.call_with(PGM_buffer_set_value, attribute, buffer.buffer_.get(), src_ptr, buffer_offset, size,
                                 src_stride);
    }
    void set_value(MetaAttribute const* attribute, RawDataConstPtr src_ptr, Idx src_stride) {
        set_value(attribute, *this, src_ptr, 0, size_, src_stride);
    }
    void set_value(MetaAttribute const* attribute, RawDataConstPtr src_ptr, Idx buffer_offset, Idx src_stride) {
        set_value(attribute, *this, src_ptr, buffer_offset, 1, src_stride);
    }
    void set_value(MetaAttribute const* attribute, RawDataConstPtr src_ptr, Idx buffer_offset, Idx size,
                   Idx src_stride) {
        set_value(attribute, *this, src_ptr, buffer_offset, size, src_stride);
    }

    static void get_value(MetaAttribute const* attribute, Buffer const& buffer, RawDataPtr dest_ptr, Idx buffer_offset,
                          Idx size, Idx dest_stride) {
        buffer.handle_.call_with(PGM_buffer_get_value, attribute, buffer.buffer_.get(), dest_ptr, buffer_offset, size,
                                 dest_stride);
    }
    void get_value(MetaAttribute const* attribute, RawDataPtr dest_ptr, Idx dest_stride) const {
        get_value(attribute, *this, dest_ptr, 0, size_, dest_stride);
    }
    void get_value(MetaAttribute const* attribute, RawDataPtr dest_ptr, Idx buffer_offset, Idx dest_stride) const {
        get_value(attribute, *this, dest_ptr, buffer_offset, 1, dest_stride);
    }
    void get_value(MetaAttribute const* attribute, RawDataPtr dest_ptr, Idx buffer_offset, Idx size,
                   Idx dest_stride) const {
        get_value(attribute, *this, dest_ptr, buffer_offset, size, dest_stride);
    }

  private:
    Handle handle_{};
    MetaComponent const* component_;
    Idx size_;
    detail::UniquePtr<void, &PGM_destroy_buffer> buffer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_BUFFER_HPP
