// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This test file is automatically generated. DO NOT modify it manually!

// clang-format off

#include <power_grid_model_cpp/basics.hpp>
#include <power_grid_model_cpp/error_codes.hpp>
#include <power_grid_model_cpp/errors.hpp>

#include <power_grid_model_c/error_codes.h>

#include <doctest/doctest.h>

#include <limits>
#include <string>

namespace power_grid_model_cpp {
namespace {
template <class ExpectedError> void check_coded_error(Idx error_code, Idx expected_error_code) {
    try {
        throw_coded_error(error_code, "test message");
        FAIL("throw_coded_error did not throw");
    } catch (PowerGridError const& error) {
        CHECK(typeid(error) == typeid(ExpectedError));
        CHECK(error.error_code() == expected_error_code);
        CHECK(std::string{error.what()} == "test message");
    }
}
} // namespace

TEST_CASE("API error codes") {
    SUBCASE("Every declared error code is mapped to its API error type") {
        check_coded_error<InvalidArguments>(PGM_invalid_arguments, PGM_invalid_arguments);
        check_coded_error<MissingCaseForEnumError>(PGM_missing_case_for_enum, PGM_missing_case_for_enum);
        check_coded_error<InvalidCalculationMethod>(PGM_invalid_calculation_method, PGM_invalid_calculation_method);
        check_coded_error<TapSearchStrategyIncompatibleError>(PGM_tap_search_strategy_incompatible, PGM_tap_search_strategy_incompatible);
        check_coded_error<ConflictID>(PGM_conflict_id, PGM_conflict_id);
        check_coded_error<IDNotFound>(PGM_id_not_found, PGM_id_not_found);
        check_coded_error<IDWrongType>(PGM_id_wrong_type, PGM_id_wrong_type);
        check_coded_error<InvalidMeasuredObject>(PGM_invalid_measured_object, PGM_invalid_measured_object);
        check_coded_error<InvalidRegulatedObject>(PGM_invalid_regulated_object, PGM_invalid_regulated_object);
        check_coded_error<DuplicativelyRegulatedObject>(PGM_duplicatively_regulated_object, PGM_duplicatively_regulated_object);
        check_coded_error<ConflictVoltage>(PGM_conflict_voltage, PGM_conflict_voltage);
        check_coded_error<InvalidTransformerClock>(PGM_invalid_transformer_clock, PGM_invalid_transformer_clock);
        check_coded_error<InvalidShortCircuitPhaseOrType>(PGM_invalid_short_circuit_phase_or_type, PGM_invalid_short_circuit_phase_or_type);
        check_coded_error<AutomaticTapInputError>(PGM_automatic_tap_input_error, PGM_automatic_tap_input_error);
        check_coded_error<ConflictingAngleMeasurementType>(PGM_conflicting_angle_measurement_type, PGM_conflicting_angle_measurement_type);
        check_coded_error<ConflictingVoltageRegulatorURef>(PGM_conflicting_voltage_regulator_u_ref, PGM_conflicting_voltage_regulator_u_ref);
        check_coded_error<UnsupportedRegulatorCombinationError>(PGM_unsupported_regulator_combination, PGM_unsupported_regulator_combination);
        check_coded_error<UnsupportedLoadGenTypeForVoltageRegulator>(PGM_unsupported_load_gen_type_for_voltage_regulator, PGM_unsupported_load_gen_type_for_voltage_regulator);
        check_coded_error<UnsupportedVoltageRegulatorSourceCombinationError>(PGM_unsupported_voltage_regulator_source_combination, PGM_unsupported_voltage_regulator_source_combination);
        check_coded_error<AutomaticTapCalculationError>(PGM_automatic_tap_calculation_error, PGM_automatic_tap_calculation_error);
        check_coded_error<SparseMatrixError>(PGM_sparse_matrix_error, PGM_sparse_matrix_error);
        check_coded_error<IterationDiverge>(PGM_iteration_diverge, PGM_iteration_diverge);
        check_coded_error<NotObservableError>(PGM_not_observable, PGM_not_observable);
        check_coded_error<MaxIterationReached>(PGM_max_iteration_reached, PGM_max_iteration_reached);
        check_coded_error<PowerGridNotImplementedError>(PGM_not_implemented, PGM_not_implemented);
        check_coded_error<ExperimentalFeature>(PGM_experimental_feature, PGM_experimental_feature);
        check_coded_error<PowerGridUnreachableHitError>(PGM_unreachable_hit, PGM_unreachable_hit);
        check_coded_error<PowerGridUnknownError>(PGM_unknown_error, PGM_unknown_error);
        check_coded_error<PowerGridIllegalOperationError>(PGM_illegal_operation, PGM_illegal_operation);
    }
    SUBCASE("An undeclared error code is mapped to a PowerGridError") {
        check_coded_error<PowerGridError>(std::numeric_limits<Idx>::max(), PGM_unknown_error);
    }
}
} // namespace power_grid_model_cpp

// clang-format on
