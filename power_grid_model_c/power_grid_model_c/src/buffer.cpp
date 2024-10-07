// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/buffer.h"

#include <power_grid_model/auxiliary/meta_data.hpp>

#include <cstdlib>

namespace {
using namespace power_grid_model;

using meta_data::RawDataConstPtr;
using meta_data::RawDataPtr;
} // namespace

// buffer control
RawDataPtr PGM_create_buffer(PGM_Handle* /* handle */, PGM_MetaComponent const* component, PGM_Idx size) {
    // alignment should be maximum of alignment of the component and alignment of void*
    size_t const alignment = std::max(component->alignment, sizeof(void*));
    // total bytes should be multiple of alignment
    size_t const requested_bytes = component->size * size;
    size_t const rounded_bytes = ((requested_bytes + alignment - 1) / alignment) * alignment;
#ifdef _WIN32
    return _aligned_malloc(rounded_bytes, alignment);
#else
    return std::aligned_alloc(alignment, rounded_bytes);
#endif
}
void PGM_destroy_buffer(RawDataPtr ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}
void PGM_buffer_set_nan(PGM_Handle* /* handle */, PGM_MetaComponent const* component, void* ptr, PGM_Idx buffer_offset,
                        PGM_Idx size) {
    component->set_nan(ptr, buffer_offset, size);
}

namespace {
// template for get and set attribute
template <bool is_get, class BufferPtr, class ValuePtr>
void buffer_get_set_value(PGM_MetaAttribute const* attribute, BufferPtr buffer_ptr, ValuePtr value_ptr,
                          PGM_Idx buffer_offset, PGM_Idx size, PGM_Idx stride) {
    // if stride is negative, use the size of the attributes as stride
    if (stride < 0) {
        stride = static_cast<PGM_Idx>(attribute->size);
    }
    for (Idx i = buffer_offset; i != size + buffer_offset; ++i) {
        ValuePtr const shifted_value_ptr =
            reinterpret_cast<std::conditional_t<is_get, char*, char const*>>(value_ptr) + stride * i;
        if constexpr (is_get) {
            attribute->get_value(buffer_ptr, shifted_value_ptr, i);
        } else {
            attribute->set_value(buffer_ptr, shifted_value_ptr, i);
        }
    }
}
} // namespace
void PGM_buffer_set_value(PGM_Handle* /* handle */, PGM_MetaAttribute const* attribute, RawDataPtr buffer_ptr,
                          RawDataConstPtr src_ptr, PGM_Idx buffer_offset, PGM_Idx size, PGM_Idx src_stride) {
    buffer_get_set_value<false>(attribute, buffer_ptr, src_ptr, buffer_offset, size, src_stride);
}
void PGM_buffer_get_value(PGM_Handle* /* handle */, PGM_MetaAttribute const* attribute, RawDataConstPtr buffer_ptr,
                          RawDataPtr dest_ptr, PGM_Idx buffer_offset, PGM_Idx size, PGM_Idx dest_stride) {
    buffer_get_set_value<true>(attribute, buffer_ptr, dest_ptr, buffer_offset, size, dest_stride);
}
