// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
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
 * @brief Create a new instance of Power Grid Model
 *
 * This is the main function to create a new model.
 * You need to prepare the buffer data for input.
 * The returned model need to be freed by PGM_destroy_model()
 *
 * @param handle
 * @param system_frequency frequency of the system, usually 50 or 60 Hz
 * @param n_components number of component types in the input data
 * @param components Pointer to a char const* array consisting the name of each component.
 * For i-th component, components[i] should be a char const* string of the component name
 * @param component_sizes Pointer to an integer array specifying the size of each component.
 * For i-th component, component_sizes[i] specifies how many elements there are for this component.
 * @param input_data Pointer to a void const* array consisting the input data buffers.
 * For i-th component, input_data[i] is a void const* pointer to the data buffer for this component.
 * @return  A opaque pointer to the created model.
 * If there are errors during the creation, a NULL is returned.
 * Use PGM_error_code() and PGM_error_message() to check the error.
 */
PGM_API PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency, PGM_Idx n_components,
                                             char const** components, PGM_Idx const* component_sizes,
                                             void const** input_data);

/**
 * @brief Update the model by changing mutable attributes of some elements
 *
 * All the elements you supply in the update dataset should have valid ids
 * which exist in the original model.
 *
 * Use PGM_error_code() and PGM_error_message() to check if there are errors in the update.
 *
 * @param handle
 * @param model Pointer to the existing model
 * @param n_components number of component types you want to update
 * @param components Pointer to a char const* array consisting the name of each component.
 * For i-th component, components[i] should be a char const* string of the component name
 * @param component_sizes Pointer to an integer array specifying the size of each component.
 * For i-th component, component_sizes[i] specifies how many elements you want to update for this component.
 * NOTE: You do not need to update all the elemenmts for this component.
 * @param update_data Pointer to a void const* array consisting the update data buffers.
 * For i-th component, update_data[i] is a void const* pointer to the update data buffer for this component.
 */
PGM_API void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Idx n_components,
                              char const** components, PGM_Idx const* component_sizes, void const** update_data);

/**
 * @brief Make a copy of an existing model
 *
 * The returned model need to be freed by PGM_destroy_model()
 *
 * @param handle
 * @param model Pointer to an existing model
 * @return  A opaque pointer to the new copy.
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
 * @param model Pointer to model
 * @param component A char const* string as component name
 * @param size Size of the ID array
 * @param ids Pointer to #PGM_ID array buffer, this should be at least length of size.
 * @param indexer Pointer to a #PGM_Idx array buffer. The results will be written to this array.
 * The array should be pre-allocated with at least length of size.
 */
PGM_API void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                             PGM_ID const* ids, PGM_Idx* indexer);

/**
 * @brief Execute a one-time or batch calculation.
 *
 * This is the main function to execute calculation.
 * You can choose to execute one-time calculation or batch calculation,
 * by controlling the n_update_components argument.
 * If n_update_components == 0, it is a one-time calculation. n_scenario will be ignored and assumed to be 1.
 * If n_update_components > 0, it is a batch calculation.
 *
 * You need to pre-allocate all output buffer.
 *
 * Use PGM_error_code() and PGM_error_message() to check the error.
 *
 * @param handle
 * @param model Pointer to model
 * @param opt Pointer to options, you need to pre-set all the calculation options you want
 * @param n_output_components Number of components you want in the output.
 * Note you do not need to get all the components as in the input in the output.
 * You can select only certain component (like, node, line) in the output.
 * @param output_components Pointer to a char const* array consisting the name of each output component.
 * For i-th component, output_components[i] should be a char const* string of the output component name
 * @param output_data Pointer to a void* array consisting the output data buffers.
 * For i-th component, output_data[i] is a void* pointer to the output data buffer for this component.
 * NOTE: Once you decide to select one component to be in the output. You always get all the elements for this
 * component. This means the output data buffer should be at least the size of
 * n_scenarios * n_element_for_this_component_in_the_model.
 * Otherwise you get buffer overflow!
 * This is also the reason you do not specify the number of elements per component in output buffer.
 * @param n_scenarios Number of scenarios in a batch calculation.
 * Note this argument is ignored if you set n_update_components = 0. Then the n_scenarios is always 1.
 * @param n_update_components Number of components you want to update in a batch calculation.
 * If you set n_update_components = 0, it means it is a one-time calculation instead of batch calculation.
 * @param update_components Pointer to a char const* array consisting the name of each update component.
 * For i-th component, update_components[i] should be a char const* string of the update component name.
 * @param n_component_elements_per_scenario Pointer to an integer array specifying
 * number of elements to be updated per component per scenario.
 * For i-th component, n_component_elements_per_scenario[i] specifies
 * how many elements you want to update for this component per scenario.
 * If you want to update different number of elements per scenario for i-th component, you can set
 * n_component_elements_per_scenario[i] = -1.
 * In this case the program will look at indptrs_per_component[i].
 * @param indptrs_per_component Pointer to an integer pointer array
 * specifying different number of updated elements per scenario per component.
 * For i-th component, if n_component_elements_per_scenario[i] == -1,
 * indptrs_per_component[i] should be a pointer to a #PGM_Idx array with at least length of n_scenarios + 1.
 * For j-th scenario, the number of updated elements for i-th components is specified by
 * indptrs_per_component[i][j + 1] - indptrs_per_component[i][j].
 * <br>
 * If n_component_elements_per_scenario[i] > 0, indptrs_per_component[i] should be a NULL.
 * If all entries in n_component_elements_per_scenario are larger than zero,
 * indptrs_per_component can be a NULL as a whole.
 * @param update_data Pointer to a void const* array consisting the update data buffers for batch calculation.
 * For i-th component, update_data[i] is a void const* pointer to the update data buffer for this component.
 * <br><br>
 * If n_component_elements_per_scenario[i] > 0, the begin and end (excluding) pointer iterator
 * for j-th scenario is
 * <br>
 * begin = (UpdateType const*)update_data[i] + j * n_component_elements_per_scenario[i]
 * <br>
 * end = (UpdateType const*)update_data[i] + (j + 1) * n_component_elements_per_scenario[i]
 * <br><br>
 * If n_component_elements_per_scenario[i] == -1, the begin and end (excluding) pointer iterator
 * for j-th scenario is
 * <br>
 * begin = (UpdateType const*)update_data[i] + indptrs_per_component[i][j]
 * <br>
 * end = (UpdateType const*)update_data[i] + indptrs_per_component[i][j + 1]
 */
PGM_API void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt,
                           PGM_Idx n_output_components, char const** output_components, void** output_data,
                           PGM_Idx n_scenarios, PGM_Idx n_update_components, char const** update_components,
                           PGM_Idx const* n_component_elements_per_scenario, PGM_Idx const** indptrs_per_component,
                           void const** update_data);

/**
 * @brief Destroy the model returned by PGM_create_model() or PGM_copy_model()
 *
 * @param model pointer to the model
 */
PGM_API void PGM_destroy_model(PGM_PowerGridModel* model);

#ifdef __cplusplus
}
#endif

#endif
