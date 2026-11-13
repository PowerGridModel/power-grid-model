// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This test file is automatically generated. DO NOT modify it manually!

// clang-format off

#include <power_grid_model_cpp/basics.hpp>
#include <power_grid_model_cpp/error_codes.hpp>
#include <power_grid_model_cpp/errors.hpp>

#include <power_grid_model_c/error_codes.h>
#include <power_grid_model_c/handle.h>

#include <doctest/doctest.h>

#include <cstdint>
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
        check_coded_error<PowerGridError>(PGM_regular_error, PGM_regular_error);
        check_coded_error<PowerGridSerializationError>(PGM_serialization_error, PGM_serialization_error);
        check_coded_error<PowerGridDatasetError>(PGM_dataset_error, PGM_dataset_error);
        check_coded_error<PowerGridDataError>(PGM_data_error, PGM_data_error);
        check_coded_error<ConflictID>(PGM_conflict_id, PGM_conflict_id);
        check_coded_error<IDNotFound>(PGM_id_not_found, PGM_id_not_found);
        check_coded_error<PowerGridComponentDataError>(PGM_component_data_error, PGM_component_data_error);
        check_coded_error<ConflictVoltage>(PGM_conflict_voltage, PGM_conflict_voltage);
        check_coded_error<InvalidTransformerClock>(PGM_invalid_transformer_clock, PGM_invalid_transformer_clock);
        check_coded_error<InvalidShortCircuitPhaseOrType>(PGM_invalid_short_circuit_phase_or_type, PGM_invalid_short_circuit_phase_or_type);
        check_coded_error<PowerGridMultiComponentDataError>(PGM_multi_component_data_error, PGM_multi_component_data_error);
        check_coded_error<IDWrongType>(PGM_id_wrong_type, PGM_id_wrong_type);
        check_coded_error<InvalidMeasuredObject>(PGM_invalid_measured_object, PGM_invalid_measured_object);
        check_coded_error<InvalidRegulatedObject>(PGM_invalid_regulated_object, PGM_invalid_regulated_object);
        check_coded_error<DuplicativelyRegulatedObject>(PGM_duplicatively_regulated_object, PGM_duplicatively_regulated_object);
        check_coded_error<ConflictingVoltageRegulatorURef>(PGM_conflicting_voltage_regulator_u_ref, PGM_conflicting_voltage_regulator_u_ref);
        check_coded_error<UnsupportedVoltageRegulatorSourceCombinationError>(PGM_unsupported_voltage_regulator_source_combination, PGM_unsupported_voltage_regulator_source_combination);
        check_coded_error<ConflictingAngleMeasurementType>(PGM_conflicting_angle_measurement_type, PGM_conflicting_angle_measurement_type);
        check_coded_error<UnsupportedRegulatorCombinationError>(PGM_unsupported_regulator_combination, PGM_unsupported_regulator_combination);
        check_coded_error<UnsupportedLoadGenTypeForVoltageRegulator>(PGM_unsupported_load_gen_type_for_voltage_regulator, PGM_unsupported_load_gen_type_for_voltage_regulator);
        check_coded_error<PowerGridCalculationError>(PGM_calculation_error, PGM_calculation_error);
        check_coded_error<SparseMatrixError>(PGM_sparse_matrix_error, PGM_sparse_matrix_error);
        check_coded_error<IterationDiverge>(PGM_iteration_diverge, PGM_iteration_diverge);
        check_coded_error<NotObservableError>(PGM_not_observable, PGM_not_observable);
        check_coded_error<MaxIterationReached>(PGM_max_iteration_reached, PGM_max_iteration_reached);
        check_coded_error<AutomaticTapInputError>(PGM_automatic_tap_input_error, PGM_automatic_tap_input_error);
        check_coded_error<PowerGridIllegalOperationError>(PGM_illegal_operation, PGM_illegal_operation);
        check_coded_error<PowerGridBadRequest>(PGM_bad_request, PGM_bad_request);
        check_coded_error<ExperimentalFeature>(PGM_experimental_feature, PGM_experimental_feature);
        check_coded_error<InvalidArguments>(PGM_invalid_arguments, PGM_invalid_arguments);
        check_coded_error<InvalidCalculationMethod>(PGM_invalid_calculation_method, PGM_invalid_calculation_method);
        check_coded_error<PowerGridNotImplementedError>(PGM_not_implemented, PGM_not_implemented);
        check_coded_error<PowerGridInternalError>(PGM_internal_error, PGM_internal_error);
        check_coded_error<PowerGridUnreachableHitError>(PGM_unreachable_hit, PGM_unreachable_hit);
        check_coded_error<TapSearchStrategyIncompatibleError>(PGM_tap_search_strategy_incompatible, PGM_tap_search_strategy_incompatible);
        check_coded_error<PowerGridUnknownError>(PGM_unknown_error, PGM_unknown_error);
    }
    SUBCASE("An undeclared error code is mapped to a PowerGridUnknownError") {
        check_coded_error<PowerGridUnknownError>(std::numeric_limits<Idx>::max(), PGM_unknown_error);
    }
    SUBCASE("Every error code is a subcode of itself") {
        CHECK(PGM_error_code_is_subcode_of(PGM_no_error, PGM_no_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_regular_error, PGM_regular_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_batch_error, PGM_batch_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_serialization_error, PGM_serialization_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_dataset_error, PGM_dataset_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_data_error, PGM_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflict_id, PGM_conflict_id) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_id_not_found, PGM_id_not_found) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_component_data_error, PGM_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflict_voltage, PGM_conflict_voltage) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_transformer_clock, PGM_invalid_transformer_clock) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_short_circuit_phase_or_type, PGM_invalid_short_circuit_phase_or_type) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_id_wrong_type, PGM_id_wrong_type) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_measured_object, PGM_invalid_measured_object) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_regulated_object, PGM_invalid_regulated_object) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_duplicatively_regulated_object, PGM_duplicatively_regulated_object) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflicting_voltage_regulator_u_ref, PGM_conflicting_voltage_regulator_u_ref) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_voltage_regulator_source_combination, PGM_unsupported_voltage_regulator_source_combination) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflicting_angle_measurement_type, PGM_conflicting_angle_measurement_type) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_regulator_combination, PGM_unsupported_regulator_combination) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_load_gen_type_for_voltage_regulator, PGM_unsupported_load_gen_type_for_voltage_regulator) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_calculation_error, PGM_calculation_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_sparse_matrix_error, PGM_sparse_matrix_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_iteration_diverge, PGM_iteration_diverge) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_not_observable, PGM_not_observable) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_max_iteration_reached, PGM_max_iteration_reached) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_automatic_tap_input_error, PGM_automatic_tap_input_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_illegal_operation, PGM_illegal_operation) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_bad_request, PGM_bad_request) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_experimental_feature, PGM_experimental_feature) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_arguments, PGM_invalid_arguments) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_calculation_method, PGM_invalid_calculation_method) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_not_implemented, PGM_not_implemented) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_internal_error, PGM_internal_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_unreachable_hit, PGM_unreachable_hit) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_tap_search_strategy_incompatible, PGM_tap_search_strategy_incompatible) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_unknown_error, PGM_unknown_error) == 1);
    }
    SUBCASE("Every error code with a base type is a subcode of, but not a base of, its base type") {
        CHECK(PGM_error_code_is_subcode_of(PGM_conflict_id, PGM_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_data_error, PGM_conflict_id) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_id_not_found, PGM_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_data_error, PGM_id_not_found) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_component_data_error, PGM_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_data_error, PGM_component_data_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflict_voltage, PGM_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_component_data_error, PGM_conflict_voltage) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_transformer_clock, PGM_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_component_data_error, PGM_invalid_transformer_clock) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_short_circuit_phase_or_type, PGM_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_component_data_error, PGM_invalid_short_circuit_phase_or_type) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_data_error, PGM_multi_component_data_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_id_wrong_type, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_id_wrong_type) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_measured_object, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_invalid_measured_object) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_regulated_object, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_invalid_regulated_object) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_duplicatively_regulated_object, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_duplicatively_regulated_object) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflicting_voltage_regulator_u_ref, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_conflicting_voltage_regulator_u_ref) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_voltage_regulator_source_combination, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_unsupported_voltage_regulator_source_combination) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflicting_angle_measurement_type, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_conflicting_angle_measurement_type) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_regulator_combination, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_unsupported_regulator_combination) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_load_gen_type_for_voltage_regulator, PGM_multi_component_data_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_unsupported_load_gen_type_for_voltage_regulator) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_sparse_matrix_error, PGM_calculation_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_calculation_error, PGM_sparse_matrix_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_iteration_diverge, PGM_calculation_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_calculation_error, PGM_iteration_diverge) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_not_observable, PGM_calculation_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_calculation_error, PGM_not_observable) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_max_iteration_reached, PGM_calculation_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_calculation_error, PGM_max_iteration_reached) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_automatic_tap_input_error, PGM_calculation_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_calculation_error, PGM_automatic_tap_input_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_experimental_feature, PGM_bad_request) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_bad_request, PGM_experimental_feature) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_arguments, PGM_bad_request) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_bad_request, PGM_invalid_arguments) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_calculation_method, PGM_bad_request) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_bad_request, PGM_invalid_calculation_method) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unreachable_hit, PGM_internal_error) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_internal_error, PGM_unreachable_hit) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_tap_search_strategy_incompatible, PGM_unreachable_hit) == 1);
        CHECK(PGM_error_code_is_subcode_of(PGM_unreachable_hit, PGM_tap_search_strategy_incompatible) == 0);
    }
    SUBCASE("Only no_error is a subcode of no_error") {
        CHECK(PGM_error_code_is_subcode_of(PGM_regular_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_batch_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_serialization_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_dataset_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_data_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflict_id, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_id_not_found, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_component_data_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflict_voltage, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_transformer_clock, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_short_circuit_phase_or_type, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_multi_component_data_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_id_wrong_type, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_measured_object, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_regulated_object, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_duplicatively_regulated_object, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflicting_voltage_regulator_u_ref, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_voltage_regulator_source_combination, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_conflicting_angle_measurement_type, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_regulator_combination, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unsupported_load_gen_type_for_voltage_regulator, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_calculation_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_sparse_matrix_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_iteration_diverge, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_not_observable, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_max_iteration_reached, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_automatic_tap_input_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_illegal_operation, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_bad_request, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_experimental_feature, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_arguments, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_invalid_calculation_method, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_not_implemented, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_internal_error, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unreachable_hit, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_tap_search_strategy_incompatible, PGM_no_error) == 0);
        CHECK(PGM_error_code_is_subcode_of(PGM_unknown_error, PGM_no_error) == 0);
    }
    SUBCASE("Out-of-range error codes are no subcodes") {
        static_assert(std::numeric_limits<Idx>::max() > std::numeric_limits<std::uint32_t>::max(),
                      "Error codes are uint32_t; this test requires Idx to be wider");
        constexpr auto too_large = static_cast<Idx>(std::numeric_limits<std::uint32_t>::max()) + 1;
        for (auto const invalid : {too_large, std::numeric_limits<Idx>::max(), Idx{-1}}) {
            CHECK(PGM_error_code_is_subcode_of(invalid, PGM_data_error) == 0);
            CHECK(PGM_error_code_is_subcode_of(PGM_data_error, invalid) == 0);
        }
    }
}
} // namespace power_grid_model_cpp

// clang-format on
