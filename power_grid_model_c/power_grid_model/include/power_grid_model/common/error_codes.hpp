// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off

#pragma once

#include "common.hpp"

#include <cstdint>

namespace power_grid_model {
enum class PowerGridErrorCode : int16_t {
    // basic
    no_error = 0, // No error occurred.
    regular_error = 1, // Generic power grid error.
    batch_error = 2, // Error occurs in batch calculation.
    serialization_error = 3, // Error occurs during (de-)serialization.
    dataset_error = 4, // Error occurs during dataset handling.
    calculation_error = 5, // Error occurs during the actual calculation.

    // API
    invalid_arguments = 101, // A (combination of) input arguments is not valid.
    missing_case_for_enum = 102, // An enum value is not covered in a for loop.
    invalid_calculation_method = 103, // Invalid calculation method provided.
    tap_search_strategy_incompatible = 104, // Search method is incompatible with optimization strategy.

    // cross-component mapping
    conflict_id = 201, // Conflicting IDs found.
    id_not_found = 202, // A reference to a non-existent ID was provided.
    id_wrong_type = 203, // A referenced ID points to a component that cannot be referenced here.
    invalid_measured_object = 204, // A provided measured object is invalid.
    invalid_regulated_object = 205, // A provided regulated object is invalid.
    duplicatively_regulated_object = 206, // A regulated object is regulated by multiple sources.

    // intra-component
    conflict_voltage = 301, // There is a conflicting voltage.
    invalid_transformer_clock = 302, // Invalid transformer clock found.
    invalid_short_circuit_phase_or_type = 303, // Invalid (combination of) short circuit types and phase(s) provided.
    automatic_tap_input_error = 304, // Automatic tap changer has invalid configuration.

    // limitations
    conflicting_angle_measurement_type = 401, // Conflicting angle measurement types found.
    conflicting_voltage_regulator_u_ref = 402, // Conflicting voltage regulator Uref found.
    unsupported_regulator_combination = 403, // The combination of voltage regulators and transformer tap regulators is not supported in the same model.
    unsupported_load_gen_type_for_voltage_regulator = 404, // Unsupported load_gen type for voltage regulators.
    unsupported_voltage_regulator_source_combination = 405, // Nodes with a source and a voltage regulated load/generator are not supported when both are enabled.
    automatic_tap_calculation_error = 406, // Automatic tap changer with tap at LV side is unsupported for automatic tap changing calculation.

    // calculation
    sparse_matrix_error = 501, // Attempting to invert a non-invertible matrix.
    iteration_diverge = 502, // Unable to iteratively converge to an optimum within the set number of iterations and precision.
    not_observable = 503, // Attempting to solve a non-observable system.
    max_iteration_reached = 504, // Maximum number of iterations reached.

    // expert mode
    not_implemented = 1001, // The functionality is either not supported or not yet implemented.
    experimental_feature = 1002, // An experimental feature was used but experimental features are not enabled.
    unreachable_hit = 1003, // Supposedly unreachable code was hit. This is always a bug in PGM. Please file a bug report.
    unknown_error = 1004, // An unknown error occurred.
    illegal_operation = 1005, // An illegal operation was attempted to the C API, such as accessing a null pointer.
};
} // namespace power_grid_model

// clang-format on
