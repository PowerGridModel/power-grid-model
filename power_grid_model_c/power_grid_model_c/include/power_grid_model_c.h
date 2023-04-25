// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @mainpage Power Grid Model C API Documentation
 * This gives an overview of C API.
 * All the exported functions are declared in power_grid_model_c.h
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_H
#define POWER_GRID_MODEL_C_H

// Generic helper definitions for shared library support
// API_MACRO_BLOCK
#if defined _WIN32
#define PGM_HELPER_DLL_IMPORT __declspec(dllimport)
#define PGM_HELPER_DLL_EXPORT __declspec(dllexport)
#define PGM_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define PGM_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define PGM_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#define PGM_HELPER_DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define PGM_HELPER_DLL_IMPORT
#define PGM_HELPER_DLL_EXPORT
#define PGM_HELPER_DLL_LOCAL
#endif
#endif
// Now we use the generic helper definitions above to define PGM_API and PGM_LOCAL.
#ifdef PGM_DLL_EXPORTS  // defined if we are building the POWER_GRID_MODEL DLL (instead of using it)
#define PGM_API PGM_HELPER_DLL_EXPORT
#else
#define PGM_API PGM_HELPER_DLL_IMPORT
#endif  // PGM_DLL_EXPORTS
#define PGM_LOCAL PGM_HELPER_DLL_LOCAL
// API_MACRO_BLOCK

// integers
#include <stddef.h>
#include <stdint.h>

// C linkage
#ifdef __cplusplus
extern "C" {
#endif

// index type
typedef int64_t PGM_Idx;
typedef int32_t PGM_ID;

/**
 * @brief Opaque struct for the PowerGridModel class
 *
 */
typedef struct PGM_PowerGridModel PGM_PowerGridModel;

/**
 * @brief Opaque struct for the handle class
 *
 * The handle class is used to store error and information
 *
 */
typedef struct PGM_Handle PGM_Handle;

/**
 * @brief Opaque struct for the option class
 *
 * The option class is used to set calculation options like calculation method.
 *
 */
typedef struct PGM_Options PGM_Options;

// enums
/**
 * @brief Enumeration for calculation type
 *
 */
enum PGM_CalculationType {
    PGM_power_flow = 0,      /**< power flow calculation */
    PGM_state_estimation = 1 /**< state estimation calculation */
};
/**
 * @brief Enumeration for calculation method
 *
 */
enum PGM_CalculationMethod {
    PGM_linear = 0,            /**< linear constant impedance method for power flow */
    PGM_newton_raphson = 1,    /**< Newton-Raphson method for power flow */
    PGM_iterative_linear = 2,  /**< iterative linear method for state estimation */
    PGM_iterative_current = 3, /**< linear current method for power flow */
    PGM_linear_current = 4     /**< iterative constant impedance method for power flow */
};
/**
 * @brief Enumeration of error codes
 *
 */
enum PGM_ErrorCode {
    PGM_no_error = 0,      /**< no error occurred */
    PGM_regular_error = 1, /**< some error occurred which is not in the batch calculation */
    PGM_batch_error = 2    /**< some error occurred which is in the batch calculation */
};

/**
 * @brief Create a new handle
 *
 * A handle object is needed to store error information.
 * If you run it in multi-threading at user side, each thread should have unique handle.
 * The handle should be destroyed by PGM_destroy_handle()
 *
 * @return Pointer to the created handle
 */
PGM_API PGM_Handle* PGM_create_handle(void);

/**
 * @brief Destroy the handle
 *
 * @param handle Pointer to the handle created by PGM_create_handle()
 */
PGM_API void PGM_destroy_handle(PGM_Handle* handle);

/**
 * @brief Get error code of last operation
 *
 * @param handle Pointer to the handle you just used for an operation
 * @return  The error code, see #PGM_ErrorCode
 */
PGM_API PGM_Idx PGM_error_code(PGM_Handle const* handle);

/**
 * @brief Get error message of last operation
 *
 * If the error code is PGM_batch_error.
 * Use PGM_n_failed_scenarios(), PGM_failed_scenarios(), and PGM_batch_errors() to retrieve the detail.
 *
 * @param handle Pointer to the handle you just used for an operation
 * @return  A char const* poiner to a zero terminated string.
 * The pointer is not valid if you execute another operation.
 * You need to copy the string in your own data.
 */
PGM_API char const* PGM_error_message(PGM_Handle const* handle);

/**
 * @brief Get the number of failed scenarios, only applicable when you just execute a batch calculation
 *
 * @param handle Pointer to the handle you just used for a batch calculation
 * @return  The number of failed scenarios
 */
PGM_API PGM_Idx PGM_n_failed_scenarios(PGM_Handle const* handle);

/**
 * @brief Get the list of failed scenarios, only applicable when you just execute a batch calculation
 *
 * @param handle Pointer to the handle you just used for a batch calculation
 * @return  A pointer to a PGM_Idx array with length returned by PGM_n_failed_scenarios().
 * The pointer is not valid if you execute another operation.
 * You need to copy the array in your own data.
 */
PGM_API PGM_Idx const* PGM_failed_scenarios(PGM_Handle const* handle);

/**
 * @brief Get the list of batch errors, only applicable when you just execute a batch calculation
 *
 * @param handle Pointer to the handle you just used for a batch calculation
 * @return  A pointer to a char const* array with length returned by PGM_n_failed_scenarios().
 * Each entry is a zero terminated string.
 * The pointer is not valid if you execute another operation.
 * You need to copy the array (and the string) in your own data.
 */
PGM_API char const** PGM_batch_errors(PGM_Handle const* handle);

/**
 * @brief Clear and reset the handle
 *
 * @param handle Pointer to the handle
 */
PGM_API void PGM_clear_error(PGM_Handle* handle);

/**
 * @brief Get number of datasets
 *
 * @param handle
 * @return  The number of datasets
 */
PGM_API PGM_Idx PGM_meta_n_datasets(PGM_Handle* handle);

/**
 * @brief Get name of idx-th dataset
 *
 * @param handle
 * @param idx the sequence number, should be between [0, PGM_meta_n_datasets())
 * @return  The name of idx-th dataset in a char const*. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API char const* PGM_meta_dataset_name(PGM_Handle* handle, PGM_Idx idx);

/**
 * @brief Get the number of components for a dataset
 *
 * You will get one of the following depending on the idx
 *   - input
 *   - update
 *   - sym_output
 *   - asym_output
 *
 * @param handle
 * @param dataset name of dataset
 * @return  Number of components, or zero if your input is invalid
 */
PGM_API PGM_Idx PGM_meta_n_components(PGM_Handle* handle, char const* dataset);

/**
 * @brief Get name of idx-th component
 *
 * @param handle
 * @param dataset name of dataset
 * @param idx sequence number of component, should be between [0, PGM_meta_n_components())
 * @return  The name of idx-th component in a char const*. The pointer is permanantly valid.
 * Or a NULL if your input is out of bound.
 */
PGM_API char const* PGM_meta_component_name(PGM_Handle* handle, char const* dataset, PGM_Idx idx);

/**
 * @brief Get size of the component
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @return  Size of the component. Or zero if your input is invalid.
 */
PGM_API size_t PGM_meta_component_size(PGM_Handle* handle, char const* dataset, char const* component);

/**
 * @brief Get alignment of the component
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @return  Alignment of the component. Or zero if your input is invalid.
 */
PGM_API size_t PGM_meta_component_alignment(PGM_Handle* handle, char const* dataset, char const* component);

/**
 * @brief Get number of attributes of the component
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @return  Number of attributes. Or zero if your input is invalid.
 */
PGM_API PGM_Idx PGM_meta_n_attributes(PGM_Handle* handle, char const* dataset, char const* component);

/**
 * @brief Get idx-th of attribute name
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @param idx sequence number of attribute, should be between [0, PGM_meta_n_attributes())
 * @return  Name of the attribute in char const*. The pointer is permanantly valid.
 * Or NULL if your input is invalid.
 */
PGM_API char const* PGM_meta_attribute_name(PGM_Handle* handle, char const* dataset, char const* component,
                                            PGM_Idx idx);

/**
 * @brief Get the type of an attribute
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @param attribute attribute name
 * @return  Type of the attribute in char const*. The string is a valid C type name. The pointer is permanantly valid.
 * Or NULL if your input is invalid.
 *
 * Valid types are:
 *   - int32_t
 *   - int8_t
 *   - double
 *   - double[3]
 */
PGM_API char const* PGM_meta_attribute_ctype(PGM_Handle* handle, char const* dataset, char const* component,
                                             char const* attribute);

/**
 * @brief Get the ofsset of an attribute in a component
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @param attribute attribute name
 * @return  Offset of this attribute. Or zero if your input is invalid.
 */
PGM_API size_t PGM_meta_attribute_offset(PGM_Handle* handle, char const* dataset, char const* component,
                                         char const* attribute);

/**
 * @brief Get if the system is little endian
 *
 * @param handle
 * @return  One if the system is litten endian. Zero if the system is big endian.
 */
PGM_API int PGM_is_little_endian(PGM_Handle* handle);

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
 * @param dataset dataset name
 * @param component component name
 * @param size size of the buffer in terms of number of elements
 * @return  Pointer to the buffer. Or NULL if your input is invalid.
 */
PGM_API void* PGM_create_buffer(PGM_Handle* handle, char const* dataset, char const* component, PGM_Idx size);

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
 * @param dataset dataset name
 * @param component component name
 * @param ptr pointer to buffer, created either by PGM_create_buffer() or your own function.
 * @param size size of the buffer in terms of number of elements
 */
PGM_API void PGM_buffer_set_nan(PGM_Handle* handle, char const* dataset, char const* component, void* ptr,
                                PGM_Idx size);

/**
 * @brief Set value of a certain attribute from an array to the component buffer
 *
 * You can use this function to set value.
 * You can also set value by proper pointer arithmetric and casting,
 * using the offset information returned by PGM_meta_attribute_offset().
 * If your input is invalid, the handle will contain the error message.
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @param attribute attribute name
 * @param buffer_ptr pointer to the buffer
 * @param src_ptr pointer to the source array you want to retrieve the value from
 * @param size size of the buffer in terms of number of elements
 * @param src_stride  stride of the source array in bytes.
 * You can set it to -1, the default stride of the size of the attribute type (like sizeof(double)).
 * If you set it to a positive number, the i-th set-value will retrieve the source data at
 * (void const*)((char const*)src_ptr + i * src_stride)
 */
PGM_API void PGM_buffer_set_value(PGM_Handle* handle, char const* dataset, char const* component, char const* attribute,
                                  void* buffer_ptr, void const* src_ptr, PGM_Idx size, PGM_Idx src_stride);

/**
 * @brief Get value of a certain attribute from the component buffer to an array
 *
 * You can use this function to get value.
 * You can also get value by proper pointer arithmetric and casting,
 * using the offset information returned by PGM_meta_attribute_offset().
 * If your input is invalid, the handle will contain the error message.
 *
 * @param handle
 * @param dataset dataset name
 * @param component component name
 * @param attribute attribute name
 * @param buffer_ptr pointer to the buffer
 * @param dest_ptr pointer to the destination array you want to save the value to
 * @param size size of the buffer in terms of number of elements
 * @param dest_stride stride of the destination array in bytes.
 * You can set it to -1, the default stride of the size of the attribute type (like sizeof(double)).
 * If you set it to a positive number, the i-th get-value will retrieve the source data at
 * (void*)((char*)dest_ptr + i * dest_stride)
 */
PGM_API void PGM_buffer_get_value(PGM_Handle* handle, char const* dataset, char const* component, char const* attribute,
                                  void const* buffer_ptr, void* dest_ptr, PGM_Idx size, PGM_Idx dest_stride);

/**
 * @brief Create an option instance.
 *
 * The option is needed to run calculations.
 * This function create a new option instance with the following default values:
 *   - calculation_type: PGM_power_flow
 *   - calculation_method: PGM_newton_raphson
 *   - symmetric: 1
 *   - err_tol: 1e-8
 *   - max_iter: 20
 *   - threading: -1
 *
 * @param handle
 * @return a pointer to the option instance. Should be freed by PGM_destroy_options()
 */
PGM_API PGM_Options* PGM_create_options(PGM_Handle* handle);

/**
 * @brief Free the option instance
 *
 * @param opt pointer to an option instance created by PGM_create_options()
 */
PGM_API void PGM_destroy_options(PGM_Options* opt);

/**
 * @brief Specify type of calculation
 *
 * @param handle
 * @param opt pointer to option instance
 * @param type See #PGM_CalculationType
 */
PGM_API void PGM_set_calculation_type(PGM_Handle* handle, PGM_Options* opt, PGM_Idx type);

/**
 * @brief Specify method of calculation
 *
 * @param handle
 * @param opt pointer to option instance
 * @param method See #PGM_CalculationMethod
 */
PGM_API void PGM_set_calculation_method(PGM_Handle* handle, PGM_Options* opt, PGM_Idx method);

/**
 * @brief Specify if we are calculating symmetrically or asymmetrically
 *
 * @param handle
 * @param opt pointer to option instance
 * @param sym One for symmetric calculation. Zero for asymmetric calculation
 */
PGM_API void PGM_set_symmetric(PGM_Handle* handle, PGM_Options* opt, PGM_Idx sym);

/**
 * @brief Specify the error tolerance to stop iterations. Only applicable if using iterative method.
 *
 * It is in terms of voltage deviation per iteration in p.u.
 *
 * @param handle
 * @param opt pointer to option instance
 * @param err_tol Relative votlage deviation tolerance
 */
PGM_API void PGM_set_err_tol(PGM_Handle* handle, PGM_Options* opt, double err_tol);

/**
 * @brief Specify maximum number of iterations. Only applicable if using iterative method.
 *
 * @param handle
 * @param opt pointer to option instance
 * @param max_iter Maximum number of iterations
 */
PGM_API void PGM_set_max_iter(PGM_Handle* handle, PGM_Options* opt, PGM_Idx max_iter);

/**
 * @brief Specify the multi-threading strategy. Only applicable for batch calculation.
 *
 * @param handle
 * @param opt pointer to option instance
 * @param threading Threading option. See below
 *   - -1: No multi-threading, calculate sequentially
 *   - 0: use number of machine available threads
 *   - >0: specify number of threads you want to calculate in parallel
 */
PGM_API void PGM_set_threading(PGM_Handle* handle, PGM_Options* opt, PGM_Idx threading);

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

#endif  // POWER_GRID_MODEL_C_H
