// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
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
 * @brief Create a deserializer from msgpack byte stream
 * @param handle
 * @param data pointer to the byte stream
 * @param size of the byte stream
 * @return a pointer to the deserializer instance (should be freed by PGM_destroy_deserializer());
 *     or NULL if errors occured (check the handle for error information)
 */
PGM_API PGM_Deserializer* PGM_create_deserializer_from_msgpack(PGM_Handle* handle, char const* data, PGM_Idx size);

/**
 * @brief Create a deserializer from msgpack byte stream
 * @param handle
 * @param json_string pointer to a null-terminated json string
 * @return a pointer to the deserializer instance. Should be freed by PGM_destroy_deserializer()
 *     Or NULL if errors occured. Check the handle for error information.
 */
PGM_API PGM_Deserializer* PGM_create_deserializer_from_json(PGM_Handle* handle, char const* json_string);

/**
 * @brief Get the PGM_WritableDataset object from the deserializer
 * @param handle
 * @param deserializer pointer to deserializer
 * @return a pointer the instance of PGM_WritableDataset.
 *     The pointer has the same lifetime as the deserializer.
 *     Use PGM_writable_dataset_get_info() to get the information of the dataset.
 *     Use PGM_writable_dataset_set_buffer() to set buffer
 */
PGM_API PGM_WritableDataset* PGM_deserializer_get_dataset(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Parse the dataset and write to the user-provided buffers
 *    The buffers must be set through PGM_writable_dataset_set_buffer().
 * @param handle
 * @param deserializer pointer to deserializer
 * @return The parsed data are written into the buffer. Check handle for possible errors.
 */
PGM_API void PGM_deserializer_parse_to_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Destory deserializer
 * @param deserializer pointer to deserializer
 * @return
 */
PGM_API void PGM_destroy_deserializer(PGM_Deserializer* deserializer);

/**
 * @brief Create a serializer object based on the buffer data
 * @param handle
 * @param dataset pointer to null-terminated string of dataset name
 * @param is_batch 1 for batch dataset, 0 for single dataset
 * @param batch_size size of the batch. For single dataset, this should be one.
 * @param n_components number of components in the dataset.
 * @param components components pointer to array of component names
 *     components[i] is a pointer to null-terminated string of i-th component name
 * @param elements_per_scenario pointer to array of numbers of the elements per scenario for all components
 *     If i-th component is uniform, elements_per_scenario[i] is the number of elements per scenario for i-th component
 *     If i-th component is not uniform, elements_per_scenario[i] must be one
 * @param indptrs indptrs pointer to array of indptrs per component
 *     If i-th component is not uniform, indptrs[i] points to an array of length batch_size + 1
 *     If i-th component is uniform, indptrs[i] should be NULL
 *     If all components are uniform, indptrs itself should be NULL
 * @param data data pointer to array of void pointers of buffers
 *     data[i] is a void pointer to the buffer of i-th component
 * @return pointer to a serializer object, should be freed by PGM_destroy_serializer()
 *     If errors occur, return NULL. Check handle for error.
 */
PGM_API PGM_Serializer* PGM_create_serializer(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                              PGM_Idx batch_size, PGM_Idx n_components, char const** components,
                                              PGM_Idx const* elements_per_scenario, PGM_Idx const** indptrs,
                                              void const** data);

/**
 * @brief Serialize the dataset into msgpack format.
 * @param handle
 * @param serializer pointer to serializer
 * @param use_compact_list 1 for use compact list per element of serialization, 0 for use dictionary per element.
 * @param data output argument, the data pointer of msgpack bytes will be written to *data.
 * @param size output argument, the length of the msgpack bytes will be written to *size.
 * @return no return value, check handle for error.
 */
PGM_API void PGM_get_msgpack(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list,
                             char const** data, PGM_Idx* size);

/**
 * @brief Serialize the dataset into json format.
 * @param handle
 * @param serializer pointer to serializer
 * @param use_compact_list 1 for use compact list per element of serialization, 0 for use dictionary per element.
 * @param indent indentation of json, use -1 for no indent and no new line.
 * @return a NULL-terminated json string.
 *     If errors occur, return NULL. Check handle for error.
 */
PGM_API char const* PGM_get_json(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list,
                                 PGM_Idx indent);

/**
 * @brief Destroy serializer
 * @param serializer pointer to serializer
 * @return
 */
PGM_API void PGM_destroy_serializer(PGM_Serializer* serializer);

#ifdef __cplusplus
}
#endif

#endif
