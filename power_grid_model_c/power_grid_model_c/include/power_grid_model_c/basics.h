// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @brief Header file which includes basic type definitions
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_BASICS_H
#define POWER_GRID_MODEL_C_BASICS_H

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
#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif

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
 * @brief Opaque struct for the attribute meta class
 *
 * The attribute class contains all the meta information of a single attribute.
 *
 */
typedef struct PGM_MetaAttribute PGM_MetaAttribute;

/**
 * @brief Opaque struct for the component meta class
 *
 * The component class contains all the meta information of a single component.
 *
 */
typedef struct PGM_MetaComponent PGM_MetaComponent;

/**
 * @brief Opaque struct for the dataset meta class
 *
 * The dataset class contains all the meta information of a single dataset.
 *
 */
typedef struct PGM_MetaDataset PGM_MetaDataset;

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
    PGM_power_flow = 0,       /**< power flow calculation */
    PGM_state_estimation = 1, /**< state estimation calculation */
    PGM_short_circuit = 2     /**< short circuit calculation */
};

/**
 * @brief Enumeration for calculation method
 *
 */
enum PGM_CalculationMethod {
    PGM_default_method = -128, /**< the default method for each calculation type, e.g. Newton-Raphson for power flow */
    PGM_linear = 0,            /**< linear constant impedance method for power flow */
    PGM_newton_raphson = 1,    /**< Newton-Raphson method for power flow */
    PGM_iterative_linear = 2,  /**< iterative linear method for state estimation */
    PGM_iterative_current = 3, /**< linear current method for power flow */
    PGM_linear_current = 4,    /**< iterative constant impedance method for power flow */
    PGM_iec60909 = 5           /**< fault analysis for short circuits using the iec60909 standard */
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
 * @brief Enumeration of C basic data types
 *
 */
enum PGM_CType {
    PGM_int32 = 0,   /**< int32_t */
    PGM_int8 = 1,    /**< int8_t */
    PGM_double = 2,  /**< double */
    PGM_double3 = 3, /**< double[3] */
};

#ifdef __cplusplus
}
#endif

#endif
