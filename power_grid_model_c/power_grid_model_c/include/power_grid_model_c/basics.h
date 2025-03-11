// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
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
#ifdef PGM_DLL_EXPORTS // defined if we are building the POWER_GRID_MODEL DLL (instead of using it)
#define PGM_API PGM_HELPER_DLL_EXPORT
#else
#define PGM_API PGM_HELPER_DLL_IMPORT
#endif // PGM_DLL_EXPORTS
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

// NOLINTBEGIN(modernize-use-using)

// index type
typedef int64_t PGM_Idx;
typedef int32_t PGM_ID;

/**
 * @brief Opaque struct for the PowerGridModel class.
 *
 */
typedef struct PGM_PowerGridModel PGM_PowerGridModel;

/**
 * @brief Opaque struct for the handle class.
 *
 * The handle class is used to store error and information.
 *
 */
typedef struct PGM_Handle PGM_Handle;

/**
 * @brief Opaque struct for the option class.
 *
 * The option class is used to set calculation options like calculation method.
 *
 */
typedef struct PGM_Options PGM_Options;

// Only enable the opaque struct definition if this header is consumed by the C-API user.
// If this header is included when compiling the C-API, the structs below are decleared/defined in the C++ files.
#ifndef PGM_DLL_EXPORTS
/**
 * @brief Opaque struct for the attribute meta class.
 *
 * The attribute class contains all the meta information of a single attribute.
 *
 */
typedef struct PGM_MetaAttribute PGM_MetaAttribute;

/**
 * @brief Opaque struct for the component meta class.
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
 * @brief Opaque struct for the serializer class.
 */
typedef struct PGM_Serializer PGM_Serializer;

/**
 * @brief Opaque struct for the deserializer class.
 */
typedef struct PGM_Deserializer PGM_Deserializer;

/**
 * @brief Opaque struct for the const dataset class.
 */
typedef struct PGM_ConstDataset PGM_ConstDataset;

/**
 * @brief Opaque struct for the multable dataset class.
 * The mutable dataset is meant for the user to provide buffers to store the output of calculations.
 */
typedef struct PGM_MutableDataset PGM_MutableDataset;

/**
 * @brief Opaque struct for the writable dataset class.
 * The writable dataset is meant for the user to provide buffers for the deserializer.
 */
typedef struct PGM_WritableDataset PGM_WritableDataset;

/**
 * @brief Opaque struct for the information of the dataset.
 */
typedef struct PGM_DatasetInfo PGM_DatasetInfo;
#endif

// NOLINTEND(modernize-use-using)

// NOLINTBEGIN(performance-enum-size)

// enums
/**
 * @brief Enumeration for calculation type.
 *
 */
enum PGM_CalculationType {
    PGM_power_flow = 0,       /**< power flow calculation */
    PGM_state_estimation = 1, /**< state estimation calculation */
    PGM_short_circuit = 2     /**< short circuit calculation */
};

/**
 * @brief Enumeration for calculation method.
 *
 */
enum PGM_CalculationMethod {
    PGM_default_method = -128, /**< the default method for each calculation type, e.g. Newton-Raphson for power flow */
    PGM_linear = 0,            /**< linear constant impedance method for power flow */
    PGM_newton_raphson = 1,    /**< Newton-Raphson method for power flow or state estimation */
    PGM_iterative_linear = 2,  /**< iterative linear method for state estimation */
    PGM_iterative_current = 3, /**< linear current method for power flow */
    PGM_linear_current = 4,    /**< iterative constant impedance method for power flow */
    PGM_iec60909 = 5           /**< fault analysis for short circuits using the iec60909 standard */
};

/**
 * @brief Enumeration for calculation and/or component symmetry
 */
enum PGM_SymmetryType {
    PGM_asymmetric = 0, /** < asymmetric calculation and/or component */
    PGM_symmetric = 1   /** < symmetric calculation and/or component */
};

/**
 * @brief Enumeration of error codes.
 *
 */
enum PGM_ErrorCode {
    PGM_no_error = 0,           /**< no error occurred */
    PGM_regular_error = 1,      /**< some error occurred which is not in the batch calculation */
    PGM_batch_error = 2,        /**< some error occurred which is in the batch calculation */
    PGM_serialization_error = 3 /**< some error occurred which is in the (de)serialization process */
};

/**
 * @brief Enumeration of C basic data types.
 *
 */
enum PGM_CType {
    PGM_int32 = 0,   /**< int32_t */
    PGM_int8 = 1,    /**< int8_t */
    PGM_double = 2,  /**< double */
    PGM_double3 = 3, /**< double[3] */
};

/**
 * @brief Enumeration of serialization types.
 *
 */
enum PGM_SerializationFormat {
    PGM_json = 0,    /**< JSON serialization format */
    PGM_msgpack = 1, /**< msgpack serialization format */
};

/**
 * @brief Enumeration of short circuit voltage scaling.
 *
 */
enum PGM_ShortCircuitVoltageScaling {
    PGM_short_circuit_voltage_scaling_minimum = 0, /**< voltage scaling for minimum short circuit currents */
    PGM_short_circuit_voltage_scaling_maximum = 1, /**< voltage scaling for maximum short circuit currents */
};

/**
 * @brief Enumeration of tap changing strategies.
 *
 */
enum PGM_TapChangingStrategy {
    PGM_tap_changing_strategy_disabled = 0, /**< disable automatic tap adjustment */
    PGM_tap_changing_strategy_any_valid_tap =
        1, /**< adjust tap position automatically; optimize for any value in the voltage band */
    PGM_tap_changing_strategy_min_voltage_tap =
        2, /**< adjust tap position automatically; optimize for the lower end of the voltage band */
    PGM_tap_changing_strategy_max_voltage_tap =
        3, /**< adjust tap position automatically; optimize for the higher end of the voltage band */
    PGM_tap_changing_strategy_fast_any_tap =
        4, /**< adjust tap position automatically; optimize for any value in the voltage band; binary search */
};

/**
 * @brief Enumeration of experimental features.
 *
 * [Danger mode]
 *
 * The behavior of experimental features may not be final and no stability guarantees are made to the users.
 * Which features (if any) are enabled in experimental mode may change over time.
 *
 */
enum PGM_ExperimentalFeatures {
    PGM_experimental_features_disabled = 0, /**< disable experimental features */
    PGM_experimental_features_enabled = 1,  /**< enable experimental features */
};

// NOLINTEND(performance-enum-size)

#ifdef __cplusplus
}
#endif

#endif
