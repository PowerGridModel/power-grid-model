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
    // no_error
    PGM_no_error = 0x00000000, /**< No error occurred. */

    // regular_error
    PGM_regular_error = 0x00000001, /**< Generic power grid error. */

    // batch_error
    PGM_batch_error = 0x00000002, /**< Error occurs in batch calculation. */

    // serialization_error
    PGM_serialization_error = 0x00000003, /**< Error occurs during (de-)serialization. */

    // dataset_error
    PGM_dataset_error = 0x00000004, /**< Error occurs during dataset handling. */

    // data_error
    PGM_data_error = 0x00000005, /**< There was a problem with the contents of the provided data. */
    PGM_conflict_id = 0x00000105, /**< Conflicting IDs found. */
    PGM_id_not_found = 0x00000205, /**< A reference to a non-existent ID was provided. */
    PGM_component_data_error = 0x00000305, /**< A component data error was found. */
    PGM_conflict_voltage = 0x00010305, /**< There is a conflicting voltage. */
    PGM_invalid_transformer_clock = 0x00020305, /**< Invalid transformer clock found. */
    PGM_invalid_short_circuit_phase_or_type = 0x00030305, /**< Invalid (combination of) short circuit types and phase(s) provided. */
    PGM_multi_component_data_error = 0x00000405, /**< A data error was found that affects multiple components. */
    PGM_id_wrong_type = 0x00010405, /**< A referenced ID points to a component that cannot be referenced here. */
    PGM_invalid_measured_object = 0x00020405, /**< A provided measured object is invalid. */
    PGM_invalid_regulated_object = 0x00030405, /**< A provided regulated object is invalid. */
    PGM_duplicatively_regulated_object = 0x00040405, /**< A regulated object is regulated by multiple sources. */
    PGM_conflicting_voltage_regulator_u_ref = 0x00050405, /**< Conflicting voltage regulator Uref found. */
    PGM_unsupported_voltage_regulator_source_combination = 0x00060405, /**< Cannot regulate nodes with both a source and a voltage regulated load/generator. */
    PGM_conflicting_angle_measurement_type = 0x00070405, /**< Different current angle measurement types on the same node are not supported. */
    PGM_unsupported_regulator_combination = 0x00080405, /**< The combination of voltage regulators and transformer tap regulators is not supported in the same model. */
    PGM_unsupported_load_gen_type_for_voltage_regulator = 0x00090405, /**< Unsupported load_gen type for voltage regulators. */

    // calculation_error
    PGM_calculation_error = 0x00000006, /**< Error occurs during the actual calculation. */
    PGM_sparse_matrix_error = 0x00000106, /**< Attempting to invert a non-invertible matrix. */
    PGM_iteration_diverge = 0x00000206, /**< Unable to iteratively converge to an optimum within the set number of iterations and precision. */
    PGM_not_observable = 0x00000306, /**< Attempting to solve a non-observable system. */
    PGM_max_iteration_reached = 0x00000406, /**< Maximum number of iterations reached. */
    PGM_automatic_tap_input_error = 0x00000506, /**< Automatic tap changer attempts to control the source side for automatic tap changing calculation. */

    // illegal_operation
    PGM_illegal_operation = 0x00000007, /**< An illegal operation was attempted to the C API, such as accessing a null pointer. */

    // bad_request
    PGM_bad_request = 0x00000008, /**< A bad request was attempted to the C API, such as providing the wrong dataset type. */
    PGM_experimental_feature = 0x00000108, /**< An experimental feature was used but experimental features are not enabled. */
    PGM_invalid_arguments = 0x00000208, /**< A (combination of) input arguments is not valid. */
    PGM_invalid_calculation_method = 0x00000308, /**< Invalid calculation method provided. */

    // not_implemented
    PGM_not_implemented = 0x00000009, /**< The functionality is either not supported or not yet implemented. */

    // internal_error
    PGM_internal_error = 0x0000000A, /**< There was an internal error. */
    PGM_unreachable_hit = 0x0000010A, /**< Supposedly unreachable code was hit. This is always a bug in PGM. Please file a bug report. */
    PGM_tap_search_strategy_incompatible = 0x0001010A, /**< Search method is incompatible with optimization strategy. */

    // unknown_error
    PGM_unknown_error = 0x000000FF, /**< An unknown error occurred. */
};

// NOLINTEND(performance-enum-size,cppcoreguidelines-use-enum-class)

#endif
// clang-format on
