// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once

#include "../exception.hpp"

#include <utility>

namespace power_grid_model::test {

// The numeric error codes are public contract (e.g. read by the wrappers) and must never change.
static_assert(std::to_underlying(PowerGridErrorCode::no_error) == 0);
static_assert(std::to_underlying(PowerGridErrorCode::regular_error) == 1);
static_assert(std::to_underlying(PowerGridErrorCode::batch_error) == 2);
static_assert(std::to_underlying(PowerGridErrorCode::serialization_error) == 3);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_arguments) == 10);
static_assert(std::to_underlying(PowerGridErrorCode::conflict_voltage) == 11);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_transformer_clock) == 12);
static_assert(std::to_underlying(PowerGridErrorCode::sparse_matrix_error) == 13);
static_assert(std::to_underlying(PowerGridErrorCode::missing_case_for_enum) == 14);
static_assert(std::to_underlying(PowerGridErrorCode::conflict_id) == 15);
static_assert(std::to_underlying(PowerGridErrorCode::id_not_found) == 16);
static_assert(std::to_underlying(PowerGridErrorCode::id_wrong_type) == 17);
static_assert(std::to_underlying(PowerGridErrorCode::dataset_error) == 18);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_measured_object) == 30);
static_assert(std::to_underlying(PowerGridErrorCode::conflicting_angle_measurement_type) == 32);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_regulated_object) == 40);
static_assert(std::to_underlying(PowerGridErrorCode::unsupported_regulator_combination) == 42);
static_assert(std::to_underlying(PowerGridErrorCode::unsupported_load_gen_type_for_voltage_regulator) == 44);
static_assert(std::to_underlying(PowerGridErrorCode::unsupported_voltage_regulator_source_combination) == 45);
static_assert(std::to_underlying(PowerGridErrorCode::automatic_tap_calculation_error) == 46);
static_assert(std::to_underlying(PowerGridErrorCode::automatic_tap_input_error) == 47);
static_assert(std::to_underlying(PowerGridErrorCode::tap_search_strategy_incompatible) == 48);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_calculation_method) == 61);
static_assert(std::to_underlying(PowerGridErrorCode::iteration_diverge) == 62);
static_assert(std::to_underlying(PowerGridErrorCode::max_iteration_reached) == 63);
static_assert(std::to_underlying(PowerGridErrorCode::not_observable) == 64);
static_assert(std::to_underlying(PowerGridErrorCode::invalid_short_circuit_phase_or_type) == 82);
static_assert(std::to_underlying(PowerGridErrorCode::not_implemented) == 100);
static_assert(std::to_underlying(PowerGridErrorCode::experimental_feature) == 101);
static_assert(std::to_underlying(PowerGridErrorCode::unreachable_hit) == 102);
static_assert(std::to_underlying(PowerGridErrorCode::unknown_error) == 103);

// The error code that each exception reports is public contract as well.
static_assert(detail::get_error_code<InvalidArguments>() == PowerGridErrorCode::invalid_arguments);
static_assert(detail::get_error_code<ExperimentalFeature>() == PowerGridErrorCode::experimental_feature);
static_assert(detail::get_error_code<MissingCaseForEnumError>() == PowerGridErrorCode::missing_case_for_enum);
static_assert(detail::get_error_code<ConflictVoltage>() == PowerGridErrorCode::conflict_voltage);
static_assert(detail::get_error_code<InvalidTransformerClock>() == PowerGridErrorCode::invalid_transformer_clock);
static_assert(detail::get_error_code<SparseMatrixError>() == PowerGridErrorCode::sparse_matrix_error);
static_assert(detail::get_error_code<ConflictID>() == PowerGridErrorCode::conflict_id);
static_assert(detail::get_error_code<IDNotFound>() == PowerGridErrorCode::id_not_found);
static_assert(detail::get_error_code<Idx2DNotFound>() == PowerGridErrorCode::id_not_found);
static_assert(detail::get_error_code<IDWrongType>() == PowerGridErrorCode::id_wrong_type);
static_assert(detail::get_error_code<DatasetError>() == PowerGridErrorCode::dataset_error);
static_assert(detail::get_error_code<InvalidMeasuredObject>() == PowerGridErrorCode::invalid_measured_object);
static_assert(detail::get_error_code<InvalidMeasuredTerminalType>() == PowerGridErrorCode::invalid_measured_object);
static_assert(detail::get_error_code<ConflictingAngleMeasurementType>() == PowerGridErrorCode::conflicting_angle_measurement_type);
static_assert(detail::get_error_code<InvalidRegulatedObject>() == PowerGridErrorCode::invalid_regulated_object);
static_assert(detail::get_error_code<DuplicativelyRegulatedObject>() == PowerGridErrorCode::regular_error);
static_assert(detail::get_error_code<UnsupportedRegulatorCombinationError>() == PowerGridErrorCode::unsupported_regulator_combination);
static_assert(detail::get_error_code<ConflictingVoltageRegulatorURef>() == PowerGridErrorCode::regular_error);
static_assert(detail::get_error_code<UnsupportedLoadGenTypeForVoltageRegulator>() == PowerGridErrorCode::unsupported_load_gen_type_for_voltage_regulator);
static_assert(detail::get_error_code<UnsupportedVoltageRegulatorSourceCombinationError>() == PowerGridErrorCode::unsupported_voltage_regulator_source_combination);
static_assert(detail::get_error_code<AutomaticTapCalculationError>() == PowerGridErrorCode::automatic_tap_calculation_error);
static_assert(detail::get_error_code<AutomaticTapInputError>() == PowerGridErrorCode::automatic_tap_input_error);
static_assert(detail::get_error_code<TapSearchStrategyIncompatibleError>() == PowerGridErrorCode::tap_search_strategy_incompatible);
static_assert(detail::get_error_code<CalculationError>() == PowerGridErrorCode::regular_error);
static_assert(detail::get_error_code<InvalidCalculationMethod>() == PowerGridErrorCode::invalid_calculation_method);
static_assert(detail::get_error_code<IterationDiverge>() == PowerGridErrorCode::iteration_diverge);
static_assert(detail::get_error_code<MaxIterationReached>() == PowerGridErrorCode::max_iteration_reached);
static_assert(detail::get_error_code<NotObservableError>() == PowerGridErrorCode::not_observable);
static_assert(detail::get_error_code<BatchCalculationError>() == PowerGridErrorCode::batch_error);
static_assert(detail::get_error_code<InvalidShortCircuitType>() == PowerGridErrorCode::invalid_short_circuit_phase_or_type);
static_assert(detail::get_error_code<InvalidShortCircuitPhases>() == PowerGridErrorCode::invalid_short_circuit_phase_or_type);
static_assert(detail::get_error_code<InvalidShortCircuitPhaseOrType>() == PowerGridErrorCode::invalid_short_circuit_phase_or_type);
static_assert(detail::get_error_code<SerializationError>() == PowerGridErrorCode::serialization_error);
static_assert(detail::get_error_code<NotImplementedError>() == PowerGridErrorCode::not_implemented);
static_assert(detail::get_error_code<UnreachableHit>() == PowerGridErrorCode::unreachable_hit);

} // namespace power_grid_model::test

// clang-format on
