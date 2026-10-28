// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off

#pragma once
#ifndef POWER_GRID_MODEL_CPP_ERROR_CODES_HPP
#define POWER_GRID_MODEL_CPP_ERROR_CODES_HPP

#include "basics.hpp"
#include "errors.hpp"

#include <power_grid_model_c/error_codes.h>

#include <string>
#include <utility>

namespace power_grid_model_cpp {

// Throw the exception that corresponds to a specific error code; unknown error codes become a PowerGridError.
[[noreturn]] inline void throw_coded_error(Idx error_code, std::string message) {
    switch (error_code) {
    case PGM_regular_error:
        throw PowerGridError{std::move(message)};
    case PGM_serialization_error:
        throw PowerGridSerializationError{std::move(message)};
    case PGM_dataset_error:
        throw PowerGridDatasetError{std::move(message)};
    case PGM_calculation_error:
        throw PowerGridCalculationError{std::move(message)};
    case PGM_invalid_arguments:
        throw InvalidArguments{std::move(message)};
    case PGM_missing_case_for_enum:
        throw MissingCaseForEnumError{std::move(message)};
    case PGM_invalid_calculation_method:
        throw InvalidCalculationMethod{std::move(message)};
    case PGM_tap_search_strategy_incompatible:
        throw TapSearchStrategyIncompatibleError{std::move(message)};
    case PGM_conflict_id:
        throw ConflictID{std::move(message)};
    case PGM_id_not_found:
        throw IDNotFound{std::move(message)};
    case PGM_id_wrong_type:
        throw IDWrongType{std::move(message)};
    case PGM_invalid_measured_object:
        throw InvalidMeasuredObject{std::move(message)};
    case PGM_invalid_regulated_object:
        throw InvalidRegulatedObject{std::move(message)};
    case PGM_duplicatively_regulated_object:
        throw DuplicativelyRegulatedObject{std::move(message)};
    case PGM_conflict_voltage:
        throw ConflictVoltage{std::move(message)};
    case PGM_invalid_transformer_clock:
        throw InvalidTransformerClock{std::move(message)};
    case PGM_invalid_short_circuit_phase_or_type:
        throw InvalidShortCircuitPhaseOrType{std::move(message)};
    case PGM_automatic_tap_input_error:
        throw AutomaticTapInputError{std::move(message)};
    case PGM_conflicting_angle_measurement_type:
        throw ConflictingAngleMeasurementType{std::move(message)};
    case PGM_conflicting_voltage_regulator_u_ref:
        throw ConflictingVoltageRegulatorURef{std::move(message)};
    case PGM_unsupported_regulator_combination:
        throw UnsupportedRegulatorCombinationError{std::move(message)};
    case PGM_unsupported_load_gen_type_for_voltage_regulator:
        throw UnsupportedLoadGenTypeForVoltageRegulator{std::move(message)};
    case PGM_unsupported_voltage_regulator_source_combination:
        throw UnsupportedVoltageRegulatorSourceCombinationError{std::move(message)};
    case PGM_automatic_tap_calculation_error:
        throw AutomaticTapCalculationError{std::move(message)};
    case PGM_sparse_matrix_error:
        throw SparseMatrixError{std::move(message)};
    case PGM_iteration_diverge:
        throw IterationDiverge{std::move(message)};
    case PGM_not_observable:
        throw NotObservableError{std::move(message)};
    case PGM_max_iteration_reached:
        throw MaxIterationReached{std::move(message)};
    case PGM_not_implemented:
        throw PowerGridNotImplementedError{std::move(message)};
    case PGM_experimental_feature:
        throw ExperimentalFeature{std::move(message)};
    case PGM_unreachable_hit:
        throw PowerGridUnreachableHitError{std::move(message)};
    case PGM_unknown_error:
        throw PowerGridUnknownError{std::move(message)};
    case PGM_illegal_operation:
        throw PowerGridIllegalOperationError{std::move(message)};
    case PGM_bad_request:
        throw PowerGridBadRequest{std::move(message)};
    default:
        throw PowerGridUnknownError{std::move(message)};
    }
}
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_ERROR_CODES_HPP
// clang-format on
