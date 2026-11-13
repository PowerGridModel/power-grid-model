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
#include <type_traits>

namespace power_grid_model_cpp {

TEST_CASE("C API error codes") {
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

TEST_CASE("C++ API error codes") {
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
    SUBCASE("Every API error type derives from itself and from PowerGridError") {
        CHECK(std::is_base_of_v<PowerGridError, PowerGridError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridError>);
        CHECK(std::is_base_of_v<PowerGridBatchError, PowerGridBatchError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridBatchError>);
        CHECK(std::is_base_of_v<PowerGridSerializationError, PowerGridSerializationError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridSerializationError>);
        CHECK(std::is_base_of_v<PowerGridDatasetError, PowerGridDatasetError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridDatasetError>);
        CHECK(std::is_base_of_v<PowerGridDataError, PowerGridDataError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridDataError>);
        CHECK(std::is_base_of_v<ConflictID, ConflictID>);
        CHECK(std::is_base_of_v<PowerGridError, ConflictID>);
        CHECK(std::is_base_of_v<IDNotFound, IDNotFound>);
        CHECK(std::is_base_of_v<PowerGridError, IDNotFound>);
        CHECK(std::is_base_of_v<PowerGridComponentDataError, PowerGridComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridComponentDataError>);
        CHECK(std::is_base_of_v<ConflictVoltage, ConflictVoltage>);
        CHECK(std::is_base_of_v<PowerGridError, ConflictVoltage>);
        CHECK(std::is_base_of_v<InvalidTransformerClock, InvalidTransformerClock>);
        CHECK(std::is_base_of_v<PowerGridError, InvalidTransformerClock>);
        CHECK(std::is_base_of_v<InvalidShortCircuitPhaseOrType, InvalidShortCircuitPhaseOrType>);
        CHECK(std::is_base_of_v<PowerGridError, InvalidShortCircuitPhaseOrType>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<IDWrongType, IDWrongType>);
        CHECK(std::is_base_of_v<PowerGridError, IDWrongType>);
        CHECK(std::is_base_of_v<InvalidMeasuredObject, InvalidMeasuredObject>);
        CHECK(std::is_base_of_v<PowerGridError, InvalidMeasuredObject>);
        CHECK(std::is_base_of_v<InvalidRegulatedObject, InvalidRegulatedObject>);
        CHECK(std::is_base_of_v<PowerGridError, InvalidRegulatedObject>);
        CHECK(std::is_base_of_v<DuplicativelyRegulatedObject, DuplicativelyRegulatedObject>);
        CHECK(std::is_base_of_v<PowerGridError, DuplicativelyRegulatedObject>);
        CHECK(std::is_base_of_v<ConflictingVoltageRegulatorURef, ConflictingVoltageRegulatorURef>);
        CHECK(std::is_base_of_v<PowerGridError, ConflictingVoltageRegulatorURef>);
        CHECK(std::is_base_of_v<UnsupportedVoltageRegulatorSourceCombinationError, UnsupportedVoltageRegulatorSourceCombinationError>);
        CHECK(std::is_base_of_v<PowerGridError, UnsupportedVoltageRegulatorSourceCombinationError>);
        CHECK(std::is_base_of_v<ConflictingAngleMeasurementType, ConflictingAngleMeasurementType>);
        CHECK(std::is_base_of_v<PowerGridError, ConflictingAngleMeasurementType>);
        CHECK(std::is_base_of_v<UnsupportedRegulatorCombinationError, UnsupportedRegulatorCombinationError>);
        CHECK(std::is_base_of_v<PowerGridError, UnsupportedRegulatorCombinationError>);
        CHECK(std::is_base_of_v<UnsupportedLoadGenTypeForVoltageRegulator, UnsupportedLoadGenTypeForVoltageRegulator>);
        CHECK(std::is_base_of_v<PowerGridError, UnsupportedLoadGenTypeForVoltageRegulator>);
        CHECK(std::is_base_of_v<PowerGridCalculationError, PowerGridCalculationError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridCalculationError>);
        CHECK(std::is_base_of_v<SparseMatrixError, SparseMatrixError>);
        CHECK(std::is_base_of_v<PowerGridError, SparseMatrixError>);
        CHECK(std::is_base_of_v<IterationDiverge, IterationDiverge>);
        CHECK(std::is_base_of_v<PowerGridError, IterationDiverge>);
        CHECK(std::is_base_of_v<NotObservableError, NotObservableError>);
        CHECK(std::is_base_of_v<PowerGridError, NotObservableError>);
        CHECK(std::is_base_of_v<MaxIterationReached, MaxIterationReached>);
        CHECK(std::is_base_of_v<PowerGridError, MaxIterationReached>);
        CHECK(std::is_base_of_v<AutomaticTapInputError, AutomaticTapInputError>);
        CHECK(std::is_base_of_v<PowerGridError, AutomaticTapInputError>);
        CHECK(std::is_base_of_v<PowerGridIllegalOperationError, PowerGridIllegalOperationError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridIllegalOperationError>);
        CHECK(std::is_base_of_v<PowerGridBadRequest, PowerGridBadRequest>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridBadRequest>);
        CHECK(std::is_base_of_v<ExperimentalFeature, ExperimentalFeature>);
        CHECK(std::is_base_of_v<PowerGridError, ExperimentalFeature>);
        CHECK(std::is_base_of_v<InvalidArguments, InvalidArguments>);
        CHECK(std::is_base_of_v<PowerGridError, InvalidArguments>);
        CHECK(std::is_base_of_v<InvalidCalculationMethod, InvalidCalculationMethod>);
        CHECK(std::is_base_of_v<PowerGridError, InvalidCalculationMethod>);
        CHECK(std::is_base_of_v<PowerGridNotImplementedError, PowerGridNotImplementedError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridNotImplementedError>);
        CHECK(std::is_base_of_v<PowerGridInternalError, PowerGridInternalError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridInternalError>);
        CHECK(std::is_base_of_v<PowerGridUnreachableHitError, PowerGridUnreachableHitError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridUnreachableHitError>);
        CHECK(std::is_base_of_v<TapSearchStrategyIncompatibleError, TapSearchStrategyIncompatibleError>);
        CHECK(std::is_base_of_v<PowerGridError, TapSearchStrategyIncompatibleError>);
        CHECK(std::is_base_of_v<PowerGridUnknownError, PowerGridUnknownError>);
        CHECK(std::is_base_of_v<PowerGridError, PowerGridUnknownError>);
    }
    SUBCASE("Every API error type with a base type derives from, but is not a base of, its base type") {
        CHECK(std::is_base_of_v<PowerGridDataError, ConflictID>);
        CHECK_FALSE(std::is_base_of_v<ConflictID, PowerGridDataError>);
        CHECK(std::is_base_of_v<PowerGridDataError, IDNotFound>);
        CHECK_FALSE(std::is_base_of_v<IDNotFound, PowerGridDataError>);
        CHECK(std::is_base_of_v<PowerGridDataError, PowerGridComponentDataError>);
        CHECK_FALSE(std::is_base_of_v<PowerGridComponentDataError, PowerGridDataError>);
        CHECK(std::is_base_of_v<PowerGridComponentDataError, ConflictVoltage>);
        CHECK_FALSE(std::is_base_of_v<ConflictVoltage, PowerGridComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridComponentDataError, InvalidTransformerClock>);
        CHECK_FALSE(std::is_base_of_v<InvalidTransformerClock, PowerGridComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridComponentDataError, InvalidShortCircuitPhaseOrType>);
        CHECK_FALSE(std::is_base_of_v<InvalidShortCircuitPhaseOrType, PowerGridComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridDataError, PowerGridMultiComponentDataError>);
        CHECK_FALSE(std::is_base_of_v<PowerGridMultiComponentDataError, PowerGridDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, IDWrongType>);
        CHECK_FALSE(std::is_base_of_v<IDWrongType, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, InvalidMeasuredObject>);
        CHECK_FALSE(std::is_base_of_v<InvalidMeasuredObject, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, InvalidRegulatedObject>);
        CHECK_FALSE(std::is_base_of_v<InvalidRegulatedObject, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, DuplicativelyRegulatedObject>);
        CHECK_FALSE(std::is_base_of_v<DuplicativelyRegulatedObject, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, ConflictingVoltageRegulatorURef>);
        CHECK_FALSE(std::is_base_of_v<ConflictingVoltageRegulatorURef, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, UnsupportedVoltageRegulatorSourceCombinationError>);
        CHECK_FALSE(std::is_base_of_v<UnsupportedVoltageRegulatorSourceCombinationError, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, ConflictingAngleMeasurementType>);
        CHECK_FALSE(std::is_base_of_v<ConflictingAngleMeasurementType, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, UnsupportedRegulatorCombinationError>);
        CHECK_FALSE(std::is_base_of_v<UnsupportedRegulatorCombinationError, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridMultiComponentDataError, UnsupportedLoadGenTypeForVoltageRegulator>);
        CHECK_FALSE(std::is_base_of_v<UnsupportedLoadGenTypeForVoltageRegulator, PowerGridMultiComponentDataError>);
        CHECK(std::is_base_of_v<PowerGridCalculationError, SparseMatrixError>);
        CHECK_FALSE(std::is_base_of_v<SparseMatrixError, PowerGridCalculationError>);
        CHECK(std::is_base_of_v<PowerGridCalculationError, IterationDiverge>);
        CHECK_FALSE(std::is_base_of_v<IterationDiverge, PowerGridCalculationError>);
        CHECK(std::is_base_of_v<PowerGridCalculationError, NotObservableError>);
        CHECK_FALSE(std::is_base_of_v<NotObservableError, PowerGridCalculationError>);
        CHECK(std::is_base_of_v<PowerGridCalculationError, MaxIterationReached>);
        CHECK_FALSE(std::is_base_of_v<MaxIterationReached, PowerGridCalculationError>);
        CHECK(std::is_base_of_v<PowerGridCalculationError, AutomaticTapInputError>);
        CHECK_FALSE(std::is_base_of_v<AutomaticTapInputError, PowerGridCalculationError>);
        CHECK(std::is_base_of_v<PowerGridBadRequest, ExperimentalFeature>);
        CHECK_FALSE(std::is_base_of_v<ExperimentalFeature, PowerGridBadRequest>);
        CHECK(std::is_base_of_v<PowerGridBadRequest, InvalidArguments>);
        CHECK_FALSE(std::is_base_of_v<InvalidArguments, PowerGridBadRequest>);
        CHECK(std::is_base_of_v<PowerGridBadRequest, InvalidCalculationMethod>);
        CHECK_FALSE(std::is_base_of_v<InvalidCalculationMethod, PowerGridBadRequest>);
        CHECK(std::is_base_of_v<PowerGridInternalError, PowerGridUnreachableHitError>);
        CHECK_FALSE(std::is_base_of_v<PowerGridUnreachableHitError, PowerGridInternalError>);
        CHECK(std::is_base_of_v<PowerGridUnreachableHitError, TapSearchStrategyIncompatibleError>);
        CHECK_FALSE(std::is_base_of_v<TapSearchStrategyIncompatibleError, PowerGridUnreachableHitError>);
    }
}
} // namespace power_grid_model_cpp

// clang-format on
