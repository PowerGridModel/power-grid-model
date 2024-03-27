// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes serialization functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_SERIALIZATION_H
#define POWER_GRID_MODEL_C_SERIALIZATION_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a deserializer from binary buffer/byte stream.
 * @param handle
 * @param data The pointer to the byte stream.
 * @param size The size of the byte stream.
 * @param serialization_format The desired data format of the serialization. See #PGM_SerializationFormat .
 * @return A pointer to the deserializer instance. Should be freed by PGM_destroy_deserializer().
 *     Returns NULL if errors occured (check the handle for error information).
 */
PGM_API PGM_Deserializer* PGM_create_deserializer_from_binary_buffer(PGM_Handle* handle, char const* data, PGM_Idx size,
                                                                     PGM_Idx serialization_format);

/**
 * @brief Create a deserializer from a null terminated C string.
 * @param handle
 * @param data_string The pointer to the null-terminated C string.
 * @param serialization_format The desired data format of the serialization. See #PGM_SerializationFormat .
 * @return A pointer to the deserializer instance. Should be freed by PGM_destroy_deserializer().
 *     Returns NULL if errors occured (check the handle for error information).
 */
PGM_API PGM_Deserializer* PGM_create_deserializer_from_null_terminated_string(PGM_Handle* handle,
                                                                              char const* data_string,
                                                                              PGM_Idx serialization_format);

/**
 * @brief Get the PGM_WritableDataset object from the deserializer.
 * @param handle
 * @param deserializer The pointer to the deserializer.
 * @return A pointer the instance of PGM_WritableDataset.
 *     The pointer has the same lifetime as the deserializer.
 *     Use PGM_writable_dataset_get_info() to get the information of the dataset.
 *     Use PGM_writable_dataset_set_buffer() to set buffer.
 */
PGM_API PGM_WritableDataset* PGM_deserializer_get_dataset(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Parse the dataset and write to the user-provided buffers.
 *     The buffers must be set through PGM_writable_dataset_set_buffer().
 * @param handle
 * @param deserializer The pointer to the deserializer
 * @return No return value; check handle for error.
 */
PGM_API void PGM_deserializer_parse_to_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Destory deserializer
 * @param deserializer pointer to deserializer
 * @return
 */
PGM_API void PGM_destroy_deserializer(PGM_Deserializer* deserializer);

/**
 * @brief Create a serializer object based on input dataset, the buffers must be set in advance.
 * @param handle
 * @param dataset A pointer to an instance of PGM_ConstDataset
 * @param serialization_format The desired data format of the serialization. See #PGM_SerializationFormat .
 * @return A pointer to the new serializer object. Should be freed by PGM_destroy_serializer()
 *     Returns NULL if errors occured (check the handle for error information).
 */
PGM_API PGM_Serializer* PGM_create_serializer(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                              PGM_Idx serialization_format);

/**
 * @brief Serialize the dataset into a binary buffer format.
 * @param handle
 * @param serializer A pointer to an existing serializer.
 * @param use_compact_list 1 for use compact list per element of serialization; 0 for use dictionary per element.
 * @param data Output argument: the data pointer of the packed buffer will be written to *data.
 * @param size Output argument: the length of the packed buffer will be written to *size.
 * @return No return value; check handle for error.
 */
PGM_API void PGM_serializer_get_to_binary_buffer(PGM_Handle* handle, PGM_Serializer* serializer,
                                                 PGM_Idx use_compact_list, char const** data, PGM_Idx* size);

/**
 * @brief Serialize the dataset into a zero terminated C string.
 *     Only supported for uncompressed data formats.
 * @param handle
 * @param serializer A pointer to an existing serializer.
 * @param use_compact_list 1 for use compact list per element of serialization; 0 for use dictionary per element.
 * @param indent The indentation of the JSON, use -1 for no indent and no new line (compact format).
 * @return A NULL-terminated json string.
 *     Returns NULL if errors occured (check the handle for error information).
 */
PGM_API char const* PGM_serializer_get_to_zero_terminated_string(PGM_Handle* handle, PGM_Serializer* serializer,
                                                                 PGM_Idx use_compact_list, PGM_Idx indent);

/**
 * @brief Destroy serializer.
 * @param serializer The pointer to the serializer.
 * @return
 */
PGM_API void PGM_destroy_serializer(PGM_Serializer* serializer);

#ifdef __cplusplus
}
#endif

#endif
