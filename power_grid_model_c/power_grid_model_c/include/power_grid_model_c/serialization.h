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
 * @return a pointer to the deserializer instance. Should be freed by PGM_destroy_deserializer()
 *     Or NULL if errors occured. Check the handle for error information.
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
 * @brief Get the name of the dataset
 * @param handle
 * @param deserializer pointer to deserializer
 * @return a pointer to null null-terminated string of the dataset name
 */
PGM_API char const* PGM_deserializer_dataset_name(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Get if the dataset is a batch dataset
 * @param handle
 * @param deserializer pointer to deserializer
 * @return 1 if the dataset is a batch, 0 if it is not.
 */
PGM_API PGM_Idx PGM_deserializer_is_batch(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Get the batch size of the dataset
 * @param handle
 * @param deserializer deserializer pointer to deserializer
 * @return Size of the batch. For a single-dataset, the batch size is always one.
 */
PGM_API PGM_Idx PGM_deserializer_batch_size(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Get the number of components in the dataset
 * @param handle
 * @param deserializer pointer to deserializer
 * @return Number of components
 */
PGM_API PGM_Idx PGM_deserializer_n_components(PGM_Handle* handle, PGM_Deserializer* deserializer);

/**
 * @brief Get the name of i-th component
 * @param handle
 * @param deserializer pointer to deserializer
 * @param component_idx idx number of the component
 * @return a pointer to null-terminated string of the component name
 */
PGM_API char const* PGM_deserializer_component_name(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                    PGM_Idx component_idx);

/**
 * @brief Get the elements per scenario for the i-th component
 * @param handle
 * @param deserializer pointer to deserializer
 * @param component_idx idx number of the component * @param component_idx
 * @return Number of elements per scenario for that component.
 *     Or -1 if the scenario is not uniform (different number per scenario)
 */
PGM_API PGM_Idx PGM_deserializer_component_elements_per_scenario(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                                 PGM_Idx component_idx);

/**
 * @brief Get the total number of elements for the i-th component
 * @param handle
 * @param deserializer pointer to deserializer
 * @param component_idx idx number of the component * @param component_idx
 * @return Total number of elements of that component
 *     If the number of elements per scenario is uniform,
 *     this value should equal to PGM_deserializer_batch_size() * PGM_deserializer_component_elements_per_scenario()
 */
PGM_API PGM_Idx PGM_deserializer_component_total_elements(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                          PGM_Idx component_idx);

/**
 * @brief Parse the dataset and write to the user-provided buffers
 * @param handle
 * @param deserializer pointer to deserializer
 * @param components pointer to array of component names
 *     components[i] is a pointer to null-terminated string of i-th component name
 * @param data pointer to array of void pointers of buffers
 *     data[i] is a void pointer to the buffer of i-th component
 * @param indptrs pointer to array of indptrs per component
 *     if i-th component is not uniform, indptrs[i] points to an array of length batch_size + 1
 *     if i-th component is uniform, indptrs[i] should be NULL
 *     if all components are uniform, indptrs itself should be NULL
 * @return The parsed data are written into the buffer. Check handle for possible errors.
 */
PGM_API void PGM_deserializer_parse_to_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                              char const** components, void** data, PGM_Idx** indptrs);

/**
 * @brief Destory deserializer
 * @param deserializer pointer to deserializer
 * @return 
*/
PGM_API void PGM_destroy_deserializer(PGM_Deserializer* deserializer);

#ifdef __cplusplus
}
#endif

#endif
