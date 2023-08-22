// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
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
 * @brief Get the name of the dataset
 * @param handle
 * @param info pointer to the info object
 * @return  a pointer to null null-terminated string of the dataset name.
 *     The pointer is permanently valid.
 */
PGM_API char const* PGM_dataset_info_name(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the flag whether the dataset is a batch dataset
 * @param handle
 * @param info pointer to the info object
 * @return 1 if the dataset is a batch, 0 if it is not.
 */
PGM_API PGM_Idx PGM_dataset_info_is_batch(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the batch size of the dataset
 * @param handle
 * @param info pointer to the info object
 * @return Size of the batch. For a single-dataset, the batch size is always one.
 */
PGM_API PGM_Idx PGM_dataset_info_batch_size(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the number of components in the dataset
 * @param handle
 * @param info pointer to the info object
 * @return Number of components
 */
PGM_API PGM_Idx PGM_dataset_info_n_components(PGM_Handle* handle, PGM_DatasetInfo const* info);

/**
 * @brief Get the name of i-th component
 * @param handle
 * @param info pointer to the info object
 * @param component_idx idx number of the component
 * @return a pointer to null-terminated string of the component name
 *     The pointer is permanently valid.
 */
PGM_API char const* PGM_dataset_info_component_name(PGM_Handle* handle, PGM_DatasetInfo const* info,
                                                    PGM_Idx component_idx);

/**
 * @brief Get the elements per scenario for the i-th component
 * @param handle
 * @param info pointer to the info object
 * @param component_idx idx number of the component
 * @return Number of elements per scenario for that component.
 *     Or -1 if the scenario is not uniform (different number per scenario)
 */
PGM_API PGM_Idx PGM_dataset_info_elements_per_scenario(PGM_Handle* handle, PGM_DatasetInfo const* info,
                                                       PGM_Idx component_idx);

/**
 * @brief Get the total number of elements for the i-th component
 * @param handle
 * @param info pointer to the info object
 * @param component_idx idx number of the component
 * @return Total number of elements of that component
 *     If the number of elements per scenario is uniform,
 *     this value should equal to PGM_dataset_info_component_name() * PGM_dataset_info_elements_per_scenario()
 */
PGM_API PGM_Idx PGM_dataset_info_total_elements(PGM_Handle* handle, PGM_DatasetInfo const* info, PGM_Idx component_idx);

#ifdef __cplusplus
}
#endif

#endif
