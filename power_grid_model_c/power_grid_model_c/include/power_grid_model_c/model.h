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
 * Use PGM_error_code() and PGM_error_message() to check the error.
 */
PGM_API PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency,
                                             PGM_ConstDataset const* input_dataset);

/**
 * @brief Update the model by changing mutable attributes of some elements.
 *
 * Use PGM_error_code() and PGM_error_message() to check if there are errors in the update.
 * NOTE: The model will be in an undefined state after errors occured during the update and should be destroyed.
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
 * The user can use the function set_next_cartesian_product_dimension() to combine multiple batch datasets
 * to create a multi-dimension batch calculation using a linked list pattern. The calculation core will
 * interpret the combined dataset as a cartesian product on a linked list of all the scenarios.
 * Each batch dataset in the linked list represents one dimension of the cartesian product.
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
 * @brief Per-scenario request for optional native PF state output.
 */
typedef struct PGM_StateOutputRequest {
    PGM_Idx y_bus;
    PGM_Idx jacobian;
} PGM_StateOutputRequest;

/**
 * @brief Borrowed view of one scenario's state output.
 *
 * All pointers are valid until the owning PGM_StateOutput is destroyed. A zero
 * availability flag means the corresponding pointer fields are NULL.
 */
typedef struct PGM_StateScenarioView {
    PGM_Idx has_state;
    PGM_Idx y_bus_requested;
    PGM_Idx jacobian_requested;
    PGM_Idx n_groups;
    PGM_Idx n_input_nodes;
    PGM_Idx const* input_node_group;
    PGM_Idx const* input_node_bus;
    PGM_ID const* input_node_id;
} PGM_StateScenarioView;

/**
 * @brief Borrowed view of one group's sparse matrices and bus-to-input mapping.
 *
 * CSR/value arrays are NULL when the corresponding matrix is absent. Mapping
 * pointers are valid until the owning PGM_StateOutput is destroyed.
 */
typedef struct PGM_StateGroupView {
    PGM_Idx group;
    PGM_Idx n_bus;
    PGM_Idx is_symmetric;
    PGM_Idx n_user_node_refs;
    PGM_Idx const* bus_user_indptr;
    PGM_Idx const* bus_user_sequence;
    PGM_ID const* bus_user_id;
    int8_t const* bus_kind;
    PGM_ID const* origin_branch3_id;
    PGM_Idx has_y_bus;
    PGM_Idx y_bus_nnz;
    PGM_Idx const* y_bus_row_indptr;
    PGM_Idx const* y_bus_col_indices;
    double const* admittance_real;
    double const* admittance_imag;
    PGM_Idx n_admittance_values;
    PGM_Idx has_jacobian;
    PGM_Idx jacobian_nnz;
    PGM_Idx const* jacobian_row_indptr;
    PGM_Idx const* jacobian_col_indices;
    double const* jacobian_h;
    double const* jacobian_n;
    double const* jacobian_m;
    double const* jacobian_l;
    PGM_Idx n_jacobian_values;
} PGM_StateGroupView;

/**
 * @brief Calculate normally and return an owned native state result.
 *
 * request_count must be one for a single calculation or equal the effective
 * flattened batch scenario count. Requests are in output scenario order.
 * The caller owns *state_output and must release it with
 * PGM_destroy_state_output(). The component output dataset remains caller-owned.
 */
PGM_API void PGM_calculate_with_state(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                                      PGM_MutableDataset const* output_dataset,
                                      PGM_ConstDataset const* batch_dataset,
                                      PGM_StateOutputRequest const* requests, PGM_Idx request_count,
                                      PGM_StateOutput** state_output);

/** @brief Get the number of scenarios represented by a state result. */
PGM_API PGM_Idx PGM_state_output_scenario_count(PGM_Handle* handle, PGM_StateOutput const* state_output);

/** @brief Get a borrowed scenario view. */
PGM_API void PGM_state_output_get_scenario(PGM_Handle* handle, PGM_StateOutput const* state_output,
                                           PGM_Idx scenario_idx, PGM_StateScenarioView* view);

/** @brief Get a borrowed group view for one scenario. */
PGM_API void PGM_state_output_get_group(PGM_Handle* handle, PGM_StateOutput const* state_output, PGM_Idx scenario_idx,
                                        PGM_Idx group_idx, PGM_StateGroupView* view);

/** @brief Destroy an owned state result and invalidate all its borrowed views. */
PGM_API void PGM_destroy_state_output(PGM_StateOutput* state_output);

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
