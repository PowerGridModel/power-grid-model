// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief header file which includes model functions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_MODEL_H
#define POWER_GRID_MODEL_C_MODEL_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new instance of Power Grid Model.
 *
 * This is the main function to create a new model.
 * You need to prepare the buffer data for input.
 * The returned model need to be freed by PGM_destroy_model()
 *
 * @param handle
 * @param system_frequency The frequency of the system, usually 50 or 60 Hz
 * @param input_dataset Pointer to an instance of PGM_ConstDataset. It should have data type "input".
 * @return The opaque pointer to the created model.
 * If there are errors during the creation, a NULL is returned.
 * Use PGM_error_code() and PGM_error_message() to check the error. */
PGM_API PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency,
                                             PGM_ConstDataset const* input_dataset);

/**
 * @brief Update the model by changing mutable attributes of some elements.
 *
 * All the elements you supply in the update dataset should have valid ids
 * which exist in the original model.
 *
 * Use PGM_error_code() and PGM_error_message() to check if there are errors in the update.
 *
 * @param handle
 * @param model A pointer to an existing model.
 * @param update_dataset Pointer to an instance of PGM_ConstDataset. It should have data type "update".
 * @return
 */
PGM_API void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_ConstDataset const* update_dataset);

/**
 * @brief Make a copy of an existing model.
 *
 * The returned model need to be freed by PGM_destroy_model()
 *
 * @param handle
 * @param model A pointer to an existing model
 * @return A opaque pointer to the new copy.
 * If there are errors during the creation, a NULL is returned.
 * Use PGM_error_code() and PGM_error_message() to check the error.
 */
PGM_API PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model);

/**
 * @brief Get the sequence numbers based on list of ids in a given component.
 *
 * For example, if there are 5 nodes in the model with id [10, 2, 5, 15, 30].
 * We have a node ID list of [2, 5, 15, 5, 10, 10, 30].
 * We would like to know the sequence number of each element in the model.
 * Calling this function should result in a sequence array of [1, 2, 3, 2, 0, 0, 4].
 *
 * If you supply a non-existing ID in the ID array, an error will be raised.
 * Use PGM_error_code() and PGM_error_message() to check the error.
 *
 * @param handle
 * @param model A pointer to an existing model.
 * @param component A char const* string as component name.
 * @param size The size of the ID array.
 * @param ids A pointer to a #PGM_ID array buffer, this should be at least length of size.
 * @param indexer A pointer to a #PGM_Idx array buffer. The results will be written to this array.
 * The array should be pre-allocated with at least length of size.
 */
PGM_API void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                             PGM_ID const* ids, PGM_Idx* indexer);

/**
 * @brief Execute a one-time or batch calculation.
 *
 * This is the main function to execute calculation.
 * You can choose to execute one-time calculation or batch calculation,
 * by controlling the batch_dataset argument.
 * If batch_dataset == NULL, it is a one-time calculation.
 * If batch_dataset != NULL, it is a batch calculation with batch update in the batch_dataset.
 *
 * You need to pre-allocate all output buffer.
 *
 * Use PGM_error_code() and PGM_error_message() to check the error.
 *
 * @param handle
 * @param model A pointer to an existing model.
 * @param opt A pointer to options, you need to pre-set all the calculation options you want.
 * @param output_dataset A pointer to an instance of PGM_MutableDataset.
 *   The dataset should have type "*_output", depending on the type of dataset.
 *   You need to pre-allocate all output memory buffers.
 *   You do not need to output all the component types as in the input.
 *   For example, you can choose only create output buffers for node, not for line.
 * @param batch_dataset A pointer to an instance of PGM_ConstDataset for batch calculation.
 *   Or NULL for single calculation.
 *   The dataset should have is_batch == true. The type of the dataset should be "update".
 * @return
 */
PGM_API void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                           PGM_MutableDataset const* output_dataset, PGM_ConstDataset const* batch_dataset);

/**
 * @brief Destroy the model returned by PGM_create_model() or PGM_copy_model().
 *
 * @param model The pointer to the model.
 */
PGM_API void PGM_destroy_model(PGM_PowerGridModel* model);

#ifdef __cplusplus
}
#endif

#endif
