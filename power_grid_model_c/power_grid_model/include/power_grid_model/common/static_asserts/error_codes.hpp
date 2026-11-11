// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once

#include "../error_codes.hpp"
#include "../exception.hpp"

#include <utility>

namespace power_grid_model::test {

// The numeric error codes are public contract (e.g. read by the wrappers) and must never change.
// no_error
static_assert(std::to_underlying(PowerGridErrorCode::no_error) == 0x00000000);

// regular_error
static_assert(std::to_underlying(PowerGridErrorCode::regular_error) == 0x00000001);

// batch_error
static_assert(std::to_underlying(PowerGridErrorCode::batch_error) == 0x00000002);

// serialization_error
static_assert(std::to_underlying(PowerGridErrorCode::serialization_error) == 0x00000003);

// dataset_error
static_assert(std::to_underlying(PowerGridErrorCode::dataset_error) == 0x00000004);

// data_error
static_assert(std::to_underlying(PowerGridErrorCode::data_error) == 0x00000005);
static_assert(std::to_underlying(PowerGridErrorCode::conflict_id) == 0x00000105);
static_assert(std::to_underlying(PowerGridErrorCode::id_not_found) == 0x00000205);
static_assert(std::to_underlying(PowerGridErrorCode::component_data_error) == 0x00000305);
static_assert(std::to_underlying(PowerGridErrorCode::conflict_voltage) == 0x00010305);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_transformer_clock) == 0x00020305);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_short_circuit_phase_or_type) == 0x00030305);
static_assert(std::to_underlying(PowerGridErrorCode::multi_component_data_error) == 0x00000405);
static_assert(std::to_underlying(PowerGridErrorCode::id_wrong_type) == 0x00010405);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_measured_object) == 0x00020405);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_regulated_object) == 0x00030405);
static_assert(std::to_underlying(PowerGridErrorCode::duplicatively_regulated_object) == 0x00040405);
static_assert(std::to_underlying(PowerGridErrorCode::conflicting_voltage_regulator_u_ref) == 0x00050405);
static_assert(std::to_underlying(PowerGridErrorCode::unsupported_voltage_regulator_source_combination) == 0x00060405);
static_assert(std::to_underlying(PowerGridErrorCode::unsupported_data_error) == 0x00000505);
static_assert(std::to_underlying(PowerGridErrorCode::conflicting_angle_measurement_type) == 0x00010505);
static_assert(std::to_underlying(PowerGridErrorCode::unsupported_regulator_combination) == 0x00020505);
static_assert(std::to_underlying(PowerGridErrorCode::unsupported_load_gen_type_for_voltage_regulator) == 0x00030505);

// calculation_error
static_assert(std::to_underlying(PowerGridErrorCode::calculation_error) == 0x00000006);
static_assert(std::to_underlying(PowerGridErrorCode::sparse_matrix_error) == 0x00000106);
static_assert(std::to_underlying(PowerGridErrorCode::iteration_diverge) == 0x00000206);
static_assert(std::to_underlying(PowerGridErrorCode::not_observable) == 0x00000306);
static_assert(std::to_underlying(PowerGridErrorCode::max_iteration_reached) == 0x00000406);
static_assert(std::to_underlying(PowerGridErrorCode::automatic_tap_input_error) == 0x00000506);

// illegal_operation
static_assert(std::to_underlying(PowerGridErrorCode::illegal_operation) == 0x00000007);

// bad_request
static_assert(std::to_underlying(PowerGridErrorCode::bad_request) == 0x00000008);
static_assert(std::to_underlying(PowerGridErrorCode::experimental_feature) == 0x00000108);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_arguments) == 0x00000208);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_calculation_method) == 0x00000308);

// not_implemented
static_assert(std::to_underlying(PowerGridErrorCode::not_implemented) == 0x00000009);

// internal_error
static_assert(std::to_underlying(PowerGridErrorCode::internal_error) == 0x0000000A);
static_assert(std::to_underlying(PowerGridErrorCode::unreachable_hit) == 0x0000010A);
static_assert(std::to_underlying(PowerGridErrorCode::tap_search_strategy_incompatible) == 0x0001010A);

// unknown_error
static_assert(std::to_underlying(PowerGridErrorCode::unknown_error) == 0x000000FF);

// The error code that each exception reports is public contract as well.
static_assert(detail::get_error_code<BatchCalculationError>() == PowerGridErrorCode::batch_error);
static_assert(detail::get_error_code<SerializationError>() == PowerGridErrorCode::serialization_error);
static_assert(detail::get_error_code<DatasetError>() == PowerGridErrorCode::dataset_error);
static_assert(detail::get_error_code<CalculationError>() == PowerGridErrorCode::calculation_error);
static_assert(detail::get_error_code<InvalidArguments>() == PowerGridErrorCode::invalid_arguments);
static_assert(detail::get_error_code<MissingCaseForEnumError>() == PowerGridErrorCode::invalid_arguments);
static_assert(detail::get_error_code<InvalidCalculationMethod>() == PowerGridErrorCode::invalid_calculation_method);
static_assert(detail::get_error_code<TapSearchStrategyIncompatibleError>() == PowerGridErrorCode::tap_search_strategy_incompatible);
static_assert(detail::get_error_code<ConflictID>() == PowerGridErrorCode::conflict_id);
static_assert(detail::get_error_code<IDNotFound>() == PowerGridErrorCode::id_not_found);
static_assert(detail::get_error_code<Idx2DNotFound>() == PowerGridErrorCode::id_not_found);
static_assert(detail::get_error_code<IDWrongType>() == PowerGridErrorCode::id_wrong_type);
static_assert(detail::get_error_code<InvalidMeasuredObject>() == PowerGridErrorCode::invalid_measured_object);
static_assert(detail::get_error_code<InvalidMeasuredTerminalType>() == PowerGridErrorCode::invalid_measured_object);
static_assert(detail::get_error_code<InvalidRegulatedObject>() == PowerGridErrorCode::invalid_regulated_object);
static_assert(detail::get_error_code<DuplicativelyRegulatedObject>() == PowerGridErrorCode::duplicatively_regulated_object);
static_assert(detail::get_error_code<ConflictVoltage>() == PowerGridErrorCode::conflict_voltage);
static_assert(detail::get_error_code<InvalidTransformerClock>() == PowerGridErrorCode::invalid_transformer_clock);
static_assert(detail::get_error_code<InvalidShortCircuitType>() == PowerGridErrorCode::invalid_short_circuit_phase_or_type);
static_assert(detail::get_error_code<InvalidShortCircuitPhases>() == PowerGridErrorCode::invalid_short_circuit_phase_or_type);
static_assert(detail::get_error_code<InvalidShortCircuitPhaseOrType>() == PowerGridErrorCode::invalid_short_circuit_phase_or_type);
static_assert(detail::get_error_code<AutomaticTapInputError>() == PowerGridErrorCode::automatic_tap_input_error);
static_assert(detail::get_error_code<ConflictingAngleMeasurementType>() == PowerGridErrorCode::conflicting_angle_measurement_type);
static_assert(detail::get_error_code<ConflictingVoltageRegulatorURef>() == PowerGridErrorCode::conflicting_voltage_regulator_u_ref);
static_assert(detail::get_error_code<UnsupportedRegulatorCombinationError>() == PowerGridErrorCode::unsupported_regulator_combination);
static_assert(detail::get_error_code<UnsupportedLoadGenTypeForVoltageRegulator>() == PowerGridErrorCode::unsupported_load_gen_type_for_voltage_regulator);
static_assert(detail::get_error_code<UnsupportedVoltageRegulatorSourceCombinationError>() == PowerGridErrorCode::unsupported_voltage_regulator_source_combination);
static_assert(detail::get_error_code<SparseMatrixError>() == PowerGridErrorCode::sparse_matrix_error);
static_assert(detail::get_error_code<IterationDiverge>() == PowerGridErrorCode::iteration_diverge);
static_assert(detail::get_error_code<NotObservableError>() == PowerGridErrorCode::not_observable);
static_assert(detail::get_error_code<MaxIterationReached>() == PowerGridErrorCode::max_iteration_reached);
static_assert(detail::get_error_code<NotImplementedError>() == PowerGridErrorCode::not_implemented);
static_assert(detail::get_error_code<ExperimentalFeature>() == PowerGridErrorCode::experimental_feature);
static_assert(detail::get_error_code<UnreachableHit>() == PowerGridErrorCode::unreachable_hit);
static_assert(detail::get_error_code<IllegalOperationError>() == PowerGridErrorCode::illegal_operation);
static_assert(detail::get_error_code<BadRequest>() == PowerGridErrorCode::bad_request);

} // namespace power_grid_model::test

// clang-format on
