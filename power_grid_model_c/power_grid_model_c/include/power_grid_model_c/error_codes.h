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
    PGM_no_error = 0, /**< No error occurred. */
    PGM_regular_error = 1, /**< Generic power grid error. */
    PGM_batch_error = 2, /**< Error occurs in batch calculation. */
    PGM_serialization_error = 3, /**< Error occurs during (de-)serialization. */
    PGM_invalid_arguments = 10, /**< A (combination of) input arguments is not valid. */
    PGM_conflict_voltage = 11, /**< There is a conflicting voltage. */
    PGM_invalid_transformer_clock = 12, /**< Invalid transformer clock found. */
    PGM_sparse_matrix_error = 13, /**< Attempting to invert a non-invertible matrix. */
    PGM_missing_case_for_enum = 14, /**< An enum value is not covered in a for loop. */
    PGM_conflict_id = 15, /**< Conflicting IDs found. */
    PGM_id_not_found = 16, /**< A reference to a non-existent ID was provided. */
    PGM_id_wrong_type = 17, /**< A referenced ID points to a component that cannot be referenced here. */
    PGM_dataset_error = 18, /**< Error occurs during dataset handling. */
    PGM_invalid_measured_object = 30, /**< A provided measured object is invalid. */
    PGM_conflicting_angle_measurement_type = 32, /**< Conflicting angle measurement types found. */
    PGM_invalid_regulated_object = 40, /**< A provided regulated object is invalid. */
    PGM_duplicatively_regulated_object = 41, /**< A regulated object is regulated by multiple sources. */
    PGM_unsupported_regulator_combination = 42, /**< The combination of voltage regulators and transformer tap regulators is not supported in the same model. */
    PGM_conflicting_voltage_regulator_u_ref = 43, /**< Conflicting voltage regulator Uref found. */
    PGM_unsupported_load_gen_type_for_voltage_regulator = 44, /**< Unsupported load_gen type for voltage regulators. */
    PGM_unsupported_voltage_regulator_source_combination = 45, /**< Nodes with a source and a voltage regulated load/generator are not supported when both are enabled. */
    PGM_automatic_tap_calculation_error = 46, /**< Automatic tap changer with tap at LV side is unsupported for automatic tap changing calculation. */
    PGM_automatic_tap_input_error = 47, /**< Automatic tap changer has invalid configuration. */
    PGM_tap_search_strategy_incompatible = 48, /**< Search method is incompatible with optimization strategy. */
    PGM_invalid_calculation_method = 61, /**< Invalid calculation method provided. */
    PGM_iteration_diverge = 62, /**< Unable to iteratively converge to an optimum within the set number of iterations and precision. */
    PGM_max_iteration_reached = 63, /**< Maximum number of iterations reached. */
    PGM_not_observable = 64, /**< Attempting to solve a non-observable system. */
    PGM_invalid_short_circuit_phase_or_type = 82, /**< Invalid (combination of) short circuit types and phase(s) provided. */
    PGM_not_implemented = 100, /**< The functionality is either not supported or not yet implemented. */
    PGM_experimental_feature = 101, /**< An experimental feature was used but experimental features are not enabled. */
    PGM_unreachable_hit = 102, /**< Supposedly unreachable code was hit. */
    PGM_unknown_error = 103, /**< An unknown error occurred. */
    PGM_illegal_operation = 104, /**< An illegal operation was attempted to the C API, such as accessing a null pointer. */
};

// NOLINTEND(performance-enum-size,cppcoreguidelines-use-enum-class)

#endif
// clang-format on
