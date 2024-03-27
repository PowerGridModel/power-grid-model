// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes dataset handling functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_DATASET_H
#define POWER_GRID_MODEL_C_DATASET_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the name of the dataset.
 * @param handle
 * @param info A pointer to the info object.
 * @return A pointer to null null-terminated string of the dataset name.
 *     The pointer is permanently valid.
 */
PGM_API char const* PGM_dataset_info_name(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the flag whether the dataset is a batch dataset.
 * @param handle
 * @param info A pointer to the info object.
 * @return 1 if the dataset is a batch, 0 if it is not.
 */
PGM_API PGM_Idx PGM_dataset_info_is_batch(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the batch size of the dataset.
 * @param handle
 * @param info A pointer to the info object.
 * @return Size of the batch. For a single-dataset, the batch size is always 1.
 */
PGM_API PGM_Idx PGM_dataset_info_batch_size(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the number of components in the dataset.
 * @param handle
 * @param info A pointer to the info object.
 * @return The number of components.
 */
PGM_API PGM_Idx PGM_dataset_info_n_components(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the name of i-th component.
 * @param handle
 * @param info A pointer to the info object.
 * @param component_idx The index number of the component.
 * @return A pointer to the null-terminated string of the component name.
 *     The pointer is permanently valid.
 */
PGM_API char const* PGM_dataset_info_component_name(PGM_Handle* handle, PGM_DatasetInfo const* info,
                                                    PGM_Idx component_idx);

/**
 * @brief Get the elements per scenario for the i-th component.
 * @param handle
 * @param info A pointer to the info object.
 * @param component_idx The index number of the component.
 * @return Number of elements per scenario for that component,
 *     or -1 if the scenario is not uniform (different number per scenario).
 */
PGM_API PGM_Idx PGM_dataset_info_elements_per_scenario(PGM_Handle* handle, PGM_DatasetInfo const* info,
                                                       PGM_Idx component_idx);

/**
 * @brief Get the total number of elements for the i-th component.
 * @param handle
 * @param info A pointer to the info object.
 * @param component_idx The index number of the component.
 * @return The total number of elements of that component.
 *     If the number of elements per scenario is uniform,
 *     this value must equal to PGM_dataset_info_batch_size() * PGM_dataset_info_elements_per_scenario().
 */
PGM_API PGM_Idx PGM_dataset_info_total_elements(PGM_Handle* handle, PGM_DatasetInfo const* info, PGM_Idx component_idx);

/**
 * @brief Create an instance of PGM_ConstDataset.
 * @param handle
 * @param dataset The name of the dataset.
 * @param is_batch 1 if the dataset is a batch, 0 if the dataset is single.
 * @param batch_size The size of the batch. For single datasets, this must be 1.
 * @return A pointer to the created PGM_ConstDataset, or NULL if errors occur. Check the handle for error.
 *    The instance must be freed by PGM_destroy_dataset_const().
 */
PGM_API PGM_ConstDataset* PGM_create_dataset_const(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                                   PGM_Idx batch_size);

/**
 * @brief Create an instance of PGM_ConstDataset from a PGM_WritableDataset
 *     They share the same buffer memory.
 * @param handle
 * @param writable_dataset pointer to an instance of PGM_WritableDataset
 * @return A pointer to the created PGM_ConstDataset, or NULL if errors occur. Check the handle for error.
 *    The instance must be freed by PGM_destroy_dataset_const().
 */
PGM_API PGM_ConstDataset* PGM_create_dataset_const_from_writable(PGM_Handle* handle,
                                                                 PGM_WritableDataset const* writable_dataset);

/**
 * @brief Create an instance of PGM_ConstDataset from a PGM_MutableDataset
 *     They share the same buffer memory.
 * @param handle
 * @param mutable_dataset pointer to an instance of PGM_MutableDataset
 * @return A pointer to the created PGM_ConstDataset, or NULL if errors occur. Check the handle for error.
 *    The instance must be freed by PGM_destroy_dataset_const().
 */
PGM_API PGM_ConstDataset* PGM_create_dataset_const_from_mutable(PGM_Handle* handle,
                                                                PGM_MutableDataset const* mutable_dataset);

/**
 * @brief Destroy an instance of PGM_ConstDataset.
 * @param dataset The pointer to the PGM_ConstDataset created by PGM_create_dataset_const(),
 * PGM_create_dataset_const_from_writable(), or PGM_create_dataset_const_from_mutable().
 * @return
 */
PGM_API void PGM_destroy_dataset_const(PGM_ConstDataset* dataset);

/**
 * @brief Add a component buffer to an instance of PGM_ConstDataset.
 * @param handle
 * @param dataset The pointer to the PGM_ConstDataset.
 * @param component The name of the component.
 * @param elements_per_scenario The number of the elements per scenario.
 *     If the component is uniform, elements_per_scenario must be >= 0
 *     If the component is not uniform, elements_per_scenario must be -1
 * @param total_elements The total number of elements for all scenarios.
 *     If elements_per_scenario >= 0, we must have elements_per_scenario * batch_size = total_elements.
 * @param indptr A pointer to an array of indptr of a non-uniform component.
 *     If the component is uniform, indptr must be NULL.
 *     If the component is not uniform, indptr must point to an array of size (batch_size + 1).
 *         The values in the array must be not decreasing.
 *         And we must have indptr[0] = 0, indptr[batch_size] = total_elements.
 * @param data A void pointer to the buffer data.
 * @return
 */
PGM_API void PGM_dataset_const_add_buffer(PGM_Handle* handle, PGM_ConstDataset* dataset, char const* component,
                                          PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                                          void const* data);

/**
 * @brief Get the dataset info of the instance PGM_ConstDataset.
 * @param handle
 * @param dataset A pointer to the PGM_ConstDataset.
 * @return A pointer to the instance of PGM_DatasetInfo.
 *     The pointer has the same lifetime as the input dataset pointer.
 */
PGM_API PGM_DatasetInfo const* PGM_dataset_const_get_info(PGM_Handle* handle, PGM_ConstDataset const* dataset);

/**
 * @brief Get the dataset info of the instance PGM_WritableDataset.
 * @param handle
 * @param dataset A pointer to the PGM_WritableDataset.
 * @return A pointer to the instance of PGM_DatasetInfo.
 *     The pointer has the same lifetime as the input dataset pointer.
 */
PGM_API PGM_DatasetInfo const* PGM_dataset_writable_get_info(PGM_Handle* handle, PGM_WritableDataset const* dataset);

/**
 * @brief Set buffer into the instance PGM_WritableDataset.
 * @param handle
 * @param dataset A pointer to the PGM_WritableDataset.
 * @param component The name of the component.
 * @param indptr A pointer to an array of indptr of a non-uniform component.
 *     If the component is uniform, indptr must be NULL.
 *     If the component is not uniform, indptr must point to an array of size (batch_size + 1).
 * @param data A void pointer to the buffer data.
 * @return
 */
PGM_API void PGM_dataset_writable_set_buffer(PGM_Handle* handle, PGM_WritableDataset* dataset, char const* component,
                                             PGM_Idx* indptr, void* data);

/**
 * @brief Create an instance of PGM_MutableDataset.
 * @param handle
 * @param dataset The name of the dataset.
 * @param is_batch 1 if the dataset is a batch, 0 if the dataset is single.
 * @param batch_size The size of the batch. For single datasets, this must be 1.
 * @return A pointer to the created PGM_MutableDataset, or NULL if errors occur. Check the handle for error.
 *    The instance must be freed by PGM_destroy_dataset_mutable().
 */
PGM_API PGM_MutableDataset* PGM_create_dataset_mutable(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                                       PGM_Idx batch_size);

/**
 * @brief Destroy an instance of PGM_MutableDataset.
 * @param dataset The pointer to the PGM_MutableDataset created by PGM_create_dataset_mutable().
 * @return
 */
PGM_API void PGM_destroy_dataset_mutable(PGM_MutableDataset* dataset);

/**
 * @brief Add a component buffer to an instance of PGM_MutableDataset.
 * @param handle
 * @param dataset The pointer to the PGM_MutableDataset.
 * @param component The name of the component.
 * @param elements_per_scenario The number of the elements per scenario.
 *     If the component is uniform, elements_per_scenario must be >= 0
 *     If the component is not uniform, elements_per_scenario must be -1
 * @param total_elements The total number of elements for all scenarios.
 *     If elements_per_scenario >= 0, we must have elements_per_scenario * batch_size = total_elements.
 * @param indptr A pointer to an array of indptr of a non-uniform component.
 *     If the component is uniform, indptr must be NULL.
 *     If the component is not uniform, indptr must point to an array of size (batch_size + 1).
 *         The values in the array must be not decreasing.
 *         And we must have indptr[0] = 0, indptr[batch_size] = total_elements.
 * @param data A void pointer to the buffer data.
 * @return
 */
PGM_API void PGM_dataset_mutable_add_buffer(PGM_Handle* handle, PGM_MutableDataset* dataset, char const* component,
                                            PGM_Idx elements_per_scenario, PGM_Idx total_elements,
                                            PGM_Idx const* indptr, void* data);

/**
 * @brief Get the dataset info of the instance PGM_MutableDataset.
 * @param handle
 * @param dataset A pointer to the PGM_MutableDataset.
 * @return A pointer to the instance of PGM_DatasetInfo.
 *     The pointer has the same lifetime as the input dataset pointer.
 */
PGM_API PGM_DatasetInfo const* PGM_dataset_mutable_get_info(PGM_Handle* handle, PGM_MutableDataset const* dataset);

#ifdef __cplusplus
}
#endif

#endif
