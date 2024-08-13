// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_BUFFER_HPP
#define POWER_GRID_MODEL_CPP_BUFFER_HPP

#include "power_grid_model_c/buffer.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Buffer {
  public:
    Buffer(PGM_MetaComponent const* component, Idx size)
        : component_{component}, size_{size}, buffer_{PGM_create_buffer(handle_.get(), component, size)} {};

    void* get() const { return buffer_.get(); }

    static void set_nan(Handle const& handle, PGM_MetaComponent const* component, Buffer& buffer, Idx buffer_offset,
                        Idx size) {
        PGM_buffer_set_nan(handle.get(), component, buffer.get(), buffer_offset, size);
        handle.check_error();
    }
    void set_nan(Idx buffer_offset) { set_nan(handle_, component_, *this, buffer_offset, size_); }

    static void set_value(Handle const& handle, PGM_MetaAttribute const* attribute, Buffer& buffer, void const* src_ptr,
                          Idx buffer_offset, Idx size, Idx src_stride) {
        PGM_buffer_set_value(handle.get(), attribute, buffer.get(), src_ptr, buffer_offset, size, src_stride);
        handle.check_error();
    }
    void set_value(PGM_MetaAttribute const* attribute, void const* src_ptr, Idx buffer_offset, Idx src_stride) {
        set_value(handle_, attribute, *this, src_ptr, buffer_offset, size_, src_stride);
    }

    static void get_value(Handle const& handle, PGM_MetaAttribute const* attribute, Buffer const& buffer,
                          void* dest_ptr, Idx buffer_offset, Idx size, Idx dest_stride) {
        PGM_buffer_get_value(handle.get(), attribute, buffer.get(), dest_ptr, buffer_offset, size, dest_stride);
        handle.check_error();
    }
    void get_value(PGM_MetaAttribute const* attribute, void* dest_ptr, Idx buffer_offset, Idx dest_stride) const {
        get_value(handle_, attribute, *this, dest_ptr, buffer_offset, size_, dest_stride);
    }

  private:
    PGM_MetaComponent const* component_;
    Idx size_;
    Handle handle_{};
    UniquePtr<void, PGM_destroy_buffer> buffer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_BUFFER_HPP
