// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off

#pragma once

#include "common.hpp"

namespace power_grid_model {
enum class PowerGridErrorCode : IntS {
    no_error = 0, // No error occurred.
    regular_error = 1, // Generic power grid error.
    batch_error = 2, // Error occurs in batch calculation.
    serialization_error = 3, // Error occurs during (de-)serialization.
    invalid_arguments = 10, // A (combination of) input arguments is not valid.
    conflict_voltage = 11, // There is a conflicting voltage.
    invalid_transformer_clock = 12, // Invalid transformer clock found.
    sparse_matrix_error = 13, // Attempting to invert a non-invertible matrix.
    missing_case_for_enum = 14, // An enum value is not covered in a for loop.
    conflict_id = 15, // Conflicting IDs found.
    id_not_found = 16, // A reference to a non-existent ID was provided.
    id_wrong_type = 17, // A referenced ID points to a component that cannot be referenced here.
    dataset_error = 18, // Error occurs during dataset handling.
    invalid_measured_object = 30, // A provided measured object is invalid.
    conflicting_angle_measurement_type = 32, // Conflicting angle measurement types found.
    invalid_regulated_object = 40, // A provided regulated object is invalid.
    duplicatively_regulated_object = 41, // A regulated object is regulated by multiple sources.
    unsupported_regulator_combination = 42, // The combination of voltage regulators and transformer tap regulators is not supported in the same model.
    conflicting_voltage_regulator_u_ref = 43, // Conflicting voltage regulator Uref found.
    unsupported_load_gen_type_for_voltage_regulator = 44, // Unsupported load_gen type for voltage regulators.
    unsupported_voltage_regulator_source_combination = 45, // Nodes with a source and a voltage regulated load/generator are not supported when both are enabled.
    automatic_tap_calculation_error = 46, // Automatic tap changer with tap at LV side is unsupported for automatic tap changing calculation.
    automatic_tap_input_error = 47, // Automatic tap changer has invalid configuration.
    tap_search_strategy_incompatible = 48, // Search method is incompatible with optimization strategy.
    invalid_calculation_method = 61, // Invalid calculation method provided.
    iteration_diverge = 62, // Unable to iteratively converge to an optimum within the set number of iterations and precision.
    max_iteration_reached = 63, // Maximum number of iterations reached.
    not_observable = 64, // Attempting to solve a non-observable system.
    invalid_short_circuit_phase_or_type = 82, // Invalid (combination of) short circuit types and phase(s) provided.
    not_implemented = 100, // The functionality is either not supported or not yet implemented.
    experimental_feature = 101, // An experimental feature was used but experimental features are not enabled.
    unreachable_hit = 102, // Supposedly unreachable code was hit.
    unknown_error = 103, // An unknown error occurred.
    illegal_operation = 104, // An illegal operation was attempted to the C API, such as accessing a null pointer.
};
} // namespace power_grid_model

// clang-format on
