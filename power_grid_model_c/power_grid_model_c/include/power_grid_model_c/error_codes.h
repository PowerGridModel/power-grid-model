// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off

/**
 * @brief Header file which includes the error codes that are returned by PGM_error_code().
 *
 * The numeric values are part of the public API and never change.
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_ERROR_CODES_H
#define POWER_GRID_MODEL_C_ERROR_CODES_H

// NOLINTBEGIN(performance-enum-size,cppcoreguidelines-use-enum-class)

/**
 * @brief Enumeration of error codes.
 *
 */
enum PGM_ErrorCode {
    // basic
    PGM_no_error = 0, /**< No error occurred. */
    PGM_regular_error = 1, /**< Generic power grid error. */
    PGM_batch_error = 2, /**< Error occurs in batch calculation. */
    PGM_serialization_error = 3, /**< Error occurs during (de-)serialization. */
    PGM_dataset_error = 4, /**< Error occurs during dataset handling. */
    PGM_calculation_error = 5, /**< Error occurs during the actual calculation. */

    // API
    PGM_invalid_arguments = 101, /**< A (combination of) input arguments is not valid. */
    PGM_missing_case_for_enum = 102, /**< An enum value is not covered in a for loop. */
    PGM_invalid_calculation_method = 103, /**< Invalid calculation method provided. */
    PGM_tap_search_strategy_incompatible = 104, /**< Search method is incompatible with optimization strategy. */

    // cross-component mapping
    PGM_conflict_id = 201, /**< Conflicting IDs found. */
    PGM_id_not_found = 202, /**< A reference to a non-existent ID was provided. */
    PGM_id_wrong_type = 203, /**< A referenced ID points to a component that cannot be referenced here. */
    PGM_invalid_measured_object = 204, /**< A provided measured object is invalid. */
    PGM_invalid_regulated_object = 205, /**< A provided regulated object is invalid. */
    PGM_duplicatively_regulated_object = 206, /**< A regulated object is regulated by multiple sources. */

    // intra-component
    PGM_conflict_voltage = 301, /**< There is a conflicting voltage. */
    PGM_invalid_transformer_clock = 302, /**< Invalid transformer clock found. */
    PGM_invalid_short_circuit_phase_or_type = 303, /**< Invalid (combination of) short circuit types and phase(s) provided. */
    PGM_automatic_tap_input_error = 304, /**< Automatic tap changer has invalid configuration. */

    // limitations
    PGM_conflicting_angle_measurement_type = 401, /**< Conflicting angle measurement types found. */
    PGM_conflicting_voltage_regulator_u_ref = 402, /**< Conflicting voltage regulator Uref found. */
    PGM_unsupported_regulator_combination = 403, /**< The combination of voltage regulators and transformer tap regulators is not supported in the same model. */
    PGM_unsupported_load_gen_type_for_voltage_regulator = 404, /**< Unsupported load_gen type for voltage regulators. */
    PGM_unsupported_voltage_regulator_source_combination = 405, /**< Nodes with a source and a voltage regulated load/generator are not supported when both are enabled. */
    PGM_automatic_tap_calculation_error = 406, /**< Automatic tap changer with tap at LV side is unsupported for automatic tap changing calculation. */

    // calculation
    PGM_sparse_matrix_error = 501, /**< Attempting to invert a non-invertible matrix. */
    PGM_iteration_diverge = 502, /**< Unable to iteratively converge to an optimum within the set number of iterations and precision. */
    PGM_not_observable = 503, /**< Attempting to solve a non-observable system. */
    PGM_max_iteration_reached = 504, /**< Maximum number of iterations reached. */

    // expert mode
    PGM_not_implemented = 1001, /**< The functionality is either not supported or not yet implemented. */
    PGM_experimental_feature = 1002, /**< An experimental feature was used but experimental features are not enabled. */
    PGM_unreachable_hit = 1003, /**< Supposedly unreachable code was hit. This is always a bug in PGM. Please file a bug report. */
    PGM_unknown_error = 1004, /**< An unknown error occurred. */
    PGM_illegal_operation = 1005, /**< An illegal operation was attempted to the C API, such as accessing a null pointer. */
};

// NOLINTEND(performance-enum-size,cppcoreguidelines-use-enum-class)

#endif
// clang-format on
