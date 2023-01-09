// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_C_H
#define POWER_GRID_MODEL_C_H

// Generic helper definitions for shared library support
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

// integers
#include <stddef.h>
#include <stdint.h>

// C linkage
#ifdef __cplusplus
extern "C" {
#endif

// index type
typedef int64_t PGM_Idx;

#ifndef PGM_DLL_EXPORTS
// else define opaque pointer
typedef struct PGM_PowerGridModel PGM_PowerGridModel;
// context handle
typedef struct PGM_Handle PGM_Handle;
// options
typedef struct PGM_Options PGM_Options;
#endif

// enums
enum PGM_CalculationType { PGM_power_flow = 0, PGM_state_estimation = 1 };
enum PGM_CalculationMethod {
    PGM_linear = 0,
    PGM_newton_raphson = 1,
    PGM_PGM_iterative_linear = 2,
    PGM_iterative_current = 3,
    PGM_linear_current = 4
};

// create and release handle
PGM_API PGM_Handle* PGM_create_handle();
PGM_API void PGM_destroy_handle(PGM_Handle* handle);

// get error code and error messsage
PGM_API PGM_Idx PGM_err_code(PGM_Handle const* handle);
PGM_API char const* PGM_err_msg(PGM_Handle const* handle);
PGM_API void PGM_clear_error(PGM_Handle* handle);

// retrieve meta data
PGM_API PGM_Idx PGM_meta_n_datasets(PGM_Handle* handle);
PGM_API char const* PGM_meta_dataset_name(PGM_Handle* handle, PGM_Idx idx);
PGM_API PGM_Idx PGM_meta_n_classes(PGM_Handle* handle, char const* dataset);
PGM_API char const* PGM_meta_class_name(PGM_Handle* handle, char const* dataset, PGM_Idx idx);
PGM_API size_t PGM_meta_class_size(PGM_Handle* handle, char const* dataset, char const* class_name);
PGM_API size_t PGM_meta_class_alignment(PGM_Handle* handle, char const* dataset, char const* class_name);
PGM_API PGM_Idx PGM_meta_n_attributes(PGM_Handle* handle, char const* dataset, char const* class_name);
PGM_API char const* PGM_meta_attribute_name(PGM_Handle* handle, char const* dataset, char const* class_name,
                                            PGM_Idx idx);
PGM_API char const* PGM_meta_attribute_ctype(PGM_Handle* handle, char const* dataset, char const* class_name,
                                             char const* attribute);
PGM_API size_t PGM_meta_attribute_offset(PGM_Handle* handle, char const* dataset, char const* class_name,
                                         char const* attribute);
PGM_API int PGM_is_little_endian(PGM_Handle* handle);

// buffer control
PGM_API void* PGM_create_buffer(PGM_Handle* handle, char const* dataset, char const* class_name, PGM_Idx size);
PGM_API void PGM_destroy_buffer(PGM_Handle* handle, void* ptr);

// options
PGM_API PGM_Options* PGM_create_options(PGM_Handle* handle);
PGM_API void PGM_destroy_options(PGM_Handle* handle, PGM_Options* opt);
PGM_API void PGM_set_calculation_type(PGM_Handle* handle, PGM_Options* opt, PGM_Idx type);
PGM_API void PGM_set_calculation_method(PGM_Handle* handle, PGM_Options* opt, PGM_Idx method);
PGM_API void PGM_set_symmetric(PGM_Handle* handle, PGM_Options* opt, PGM_Idx sym);
PGM_API void PGM_set_err_tol(PGM_Handle* handle, PGM_Options* opt, double err_tol);
PGM_API void PGM_set_max_iter(PGM_Handle* handle, PGM_Options* opt, PGM_Idx max_iter);
PGM_API void PGM_set_threading(PGM_Handle* handle, PGM_Options* opt, PGM_Idx threading);

// create model
PGM_API PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency, PGM_Idx n_input_types,
                                             char const** type_names, PGM_Idx const* type_sizes,
                                             void const** input_data);

// update model
PGM_API void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Idx n_update_types,
                              char const** type_names, PGM_Idx const* type_sizes, void const** update_data);

// destroy model
PGM_API void PGM_destroy_model(PGM_Handle* handle, PGM_PowerGridModel* model);

#ifdef __cplusplus
}
#endif

#endif  // POWER_GRID_MODEL_C_H