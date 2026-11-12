// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off

#pragma once

#include <power_grid_model_cpp/basics.hpp>

#include <power_grid_model_c/error_codes.h>

#include <map>
#include <string>

namespace power_grid_model_cpp {

// Error codes by the name of the error type that is exposed to users; error types shared by several codes are omitted.
inline std::map<std::string, Idx, std::less<>> const& error_codes_by_user_type() {
    static std::map<std::string, Idx, std::less<>> const error_codes = {
        {"PowerGridError", PGM_regular_error},
        {"PowerGridBatchError", PGM_batch_error},
        {"PowerGridSerializationError", PGM_serialization_error},
        {"PowerGridDatasetError", PGM_dataset_error},
        {"PowerGridDataError", PGM_data_error},
        {"ConflictID", PGM_conflict_id},
        {"IDNotFound", PGM_id_not_found},
        {"PowerGridComponentDataError", PGM_component_data_error},
        {"ConflictVoltage", PGM_conflict_voltage},
        {"InvalidTransformerClock", PGM_invalid_transformer_clock},
        {"InvalidShortCircuitPhaseOrType", PGM_invalid_short_circuit_phase_or_type},
        {"PowerGridMultiComponentDataError", PGM_multi_component_data_error},
        {"IDWrongType", PGM_id_wrong_type},
        {"InvalidMeasuredObject", PGM_invalid_measured_object},
        {"InvalidRegulatedObject", PGM_invalid_regulated_object},
        {"DuplicativelyRegulatedObject", PGM_duplicatively_regulated_object},
        {"ConflictingVoltageRegulatorURef", PGM_conflicting_voltage_regulator_u_ref},
        {"UnsupportedVoltageRegulatorSourceCombinationError", PGM_unsupported_voltage_regulator_source_combination},
        {"ConflictingAngleMeasurementType", PGM_conflicting_angle_measurement_type},
        {"UnsupportedRegulatorCombinationError", PGM_unsupported_regulator_combination},
        {"UnsupportedLoadGenTypeForVoltageRegulator", PGM_unsupported_load_gen_type_for_voltage_regulator},
        {"PowerGridCalculationError", PGM_calculation_error},
        {"SparseMatrixError", PGM_sparse_matrix_error},
        {"IterationDiverge", PGM_iteration_diverge},
        {"NotObservableError", PGM_not_observable},
        {"MaxIterationReached", PGM_max_iteration_reached},
        {"AutomaticTapInputError", PGM_automatic_tap_input_error},
        {"PowerGridIllegalOperationError", PGM_illegal_operation},
        {"PowerGridBadRequest", PGM_bad_request},
        {"ExperimentalFeature", PGM_experimental_feature},
        {"InvalidArguments", PGM_invalid_arguments},
        {"InvalidCalculationMethod", PGM_invalid_calculation_method},
        {"PowerGridNotImplementedError", PGM_not_implemented},
        {"PowerGridInternalError", PGM_internal_error},
        {"PowerGridUnreachableHitError", PGM_unreachable_hit},
        {"TapSearchStrategyIncompatibleError", PGM_tap_search_strategy_incompatible},
        {"PowerGridUnknownError", PGM_unknown_error},
    };
    return error_codes;
}
} // namespace power_grid_model_cpp

// clang-format on
