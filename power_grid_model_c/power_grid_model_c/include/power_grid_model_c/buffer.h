// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes buffer functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_BUFFER_H
#define POWER_GRID_MODEL_C_BUFFER_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a buffer with certain size and component type
 *
 * You can use this function to allocate a buffer.
 * You can also use your own allocation function to do that
 * with size and alignment got from PGM_meta_component_size() and PGM_meta_component_alignment().
 * The buffer created by this function should be freed by PGM_destroy_buffer().
 *
 * It is recommended to call PGM_buffer_set_nan() after you create an input or update buffer.
 * In this way all the attributes will be set to NaN.
 * And if there is a new optional attribute added in the future.
 * You have garantee that your code is still compatible
 * because that optional attribute will be set to NaN and the default value will be used.
 *
 * @param handle
 * @param component component pointer
 * @param size size of the buffer in terms of number of elements
 * @return  Pointer to the buffer. Or NULL if your input is invalid.
 */
PGM_API void* PGM_create_buffer(PGM_Handle* handle, PGM_MetaComponent const* component, PGM_Idx size);

/**
 * @brief Destroy the buffer you created using PGM_create_buffer().
 *
 * NOTE: do not call this function on the buffer you created by your own function.
 *
 * @param ptr Pointer to the buffer created using PGM_create_buffer()
 */
PGM_API void PGM_destroy_buffer(void* ptr);

/**
 * @brief Set all the attributes of a buffer to NaN
 *
 * @param handle
 * @param component component pointer
 * @param ptr pointer to buffer, created either by PGM_create_buffer() or your own function.
 * @param buffer_offset offset in the buffer where you begin to set nan, in terms of number of elements
 * @param size size of the buffer in terms of number of elements
 */
PGM_API void PGM_buffer_set_nan(PGM_Handle* handle, PGM_MetaComponent const* component, void* ptr,
                                PGM_Idx buffer_offset, PGM_Idx size);

/**
 * @brief Set value of a certain attribute from an array to the component buffer
 *
 * You can use this function to set value.
 * You can also set value by proper pointer arithmetric and casting,
 * using the offset information returned by PGM_meta_attribute_offset().
 *
 * @param handle
 * @param attribute attribute pointer
 * @param buffer_ptr pointer to the buffer
 * @param src_ptr pointer to the source array you want to retrieve the value from
 * @param buffer_offset offset in the buffer where you begin to set value, in terms of number of elements
 * @param size size of the buffer in terms of number of elements
 * @param src_stride  stride of the source array in bytes.
 * You can set it to -1, the default stride of the size of the attribute type (like sizeof(double)).
 * If you set it to a positive number, the i-th set-value will retrieve the source data at
 * (void const*)((char const*)src_ptr + i * src_stride)
 */
PGM_API void PGM_buffer_set_value(PGM_Handle* handle, PGM_MetaAttribute const* attribute, void* buffer_ptr,
                                  void const* src_ptr, PGM_Idx buffer_offset, PGM_Idx size, PGM_Idx src_stride);

/**
 * @brief Get value of a certain attribute from the component buffer to an array
 *
 * You can use this function to get value.
 * You can also get value by proper pointer arithmetric and casting,
 * using the offset information returned by PGM_meta_attribute_offset().
 *
 * @param handle
 * @param attribute attribute pointer
 * @param buffer_ptr pointer to the buffer
 * @param dest_ptr pointer to the destination array you want to save the value to
 * @param buffer_offset offset in the buffer where you begin to get value, in terms of number of elements
 * @param size size of the buffer in terms of number of elements
 * @param dest_stride stride of the destination array in bytes.
 * You can set it to -1, the default stride of the size of the attribute type (like sizeof(double)).
 * If you set it to a positive number, the i-th get-value will retrieve the source data at
 * (void*)((char*)dest_ptr + i * dest_stride)
 */
PGM_API void PGM_buffer_get_value(PGM_Handle* handle, PGM_MetaAttribute const* attribute, void const* buffer_ptr,
                                  void* dest_ptr, PGM_Idx buffer_offset, PGM_Idx size, PGM_Idx dest_stride);

#ifdef __cplusplus
}
#endif

#endif
