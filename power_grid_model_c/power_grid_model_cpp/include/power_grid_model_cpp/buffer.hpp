// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_BUFFER_HPP
#define POWER_GRID_MODEL_CPP_BUFFER_HPP

#include "buffer.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Buffer {
  public:
    power_grid_model_cpp::Handle handle;

    Buffer(PGM_MetaComponent const* component, Idx size)
        : component_{component}, size_{size}, handle(), buffer_{PGM_create_buffer(handle.get(), component, size)} {};

    ~Buffer() = default;

    static void set_nan(PGM_Handle* provided_handle, PGM_MetaComponent const* component, void* ptr, Idx buffer_offset,
                        Idx size) {
        PGM_buffer_set_nan(provided_handle, component, ptr, buffer_offset, size);
    }
    void set_nan(Idx buffer_offset) {
        PGM_buffer_set_nan(handle.get(), component_, buffer_.get(), buffer_offset, size_);
    }

    static void set_value(PGM_Handle* provided_handle, PGM_MetaAttribute const* attribute, void* buffer_ptr,
                          void const* src_ptr, Idx buffer_offset, Idx size, Idx src_stride) {
        PGM_buffer_set_value(provided_handle, attribute, buffer_ptr, src_ptr, buffer_offset, size, src_stride);
    }
    void set_value(PGM_MetaAttribute const* attribute, void const* src_ptr, Idx buffer_offset, Idx src_stride) {
        PGM_buffer_set_value(handle.get(), attribute, buffer_.get(), src_ptr, buffer_offset, size_, src_stride);
    }

    static void get_value(PGM_Handle* provided_handle, PGM_MetaAttribute const* attribute, void const* buffer_ptr,
                          void* dest_ptr, Idx buffer_offset, Idx size, Idx dest_stride) {
        PGM_buffer_get_value(provided_handle, attribute, buffer_ptr, dest_ptr, buffer_offset, size, dest_stride);
    }
    void get_value(PGM_MetaAttribute const* attribute, void* dest_ptr, Idx buffer_offset, Idx dest_stride) const {
        PGM_buffer_get_value(handle.get(), attribute, buffer_.get(), dest_ptr, buffer_offset, size_, dest_stride);
    }

  private:
    PGM_MetaComponent const* component_;
    Idx size_;
    UniquePtr<void, PGM_destroy_buffer> buffer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_BUFFER_HPP