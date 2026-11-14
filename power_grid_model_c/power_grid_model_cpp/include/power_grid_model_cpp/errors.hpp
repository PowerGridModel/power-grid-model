// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_ERRORS_HPP
#define POWER_GRID_MODEL_CPP_ERRORS_HPP

#include "basics.hpp"

#include "power_grid_model_c/error_codes.h"

#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace power_grid_model_cpp {

// This file contains exceptions that correspond to a specific C API error code.
// These exceptions are deliberately not code-generated, so that e.g. backwards compatibility can be guaranteed.
// Instead, the mapping from error code to type is generated into error_codes.hpp and kept up-to-date.

class PowerGridError : public std::exception {
  public:
    PowerGridError(std::string message) : message_(std::move(message)) {}
    const char* what() const noexcept override { return message_.c_str(); }
    virtual Idx error_code() const noexcept { return PGM_regular_error; }

  private:
    std::string message_;
};

class PowerGridSerializationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_serialization_error; }
};

class PowerGridBatchError : public PowerGridError {
  public:
    struct FailedScenario {
        Idx scenario{};
        std::string error_message;
        Idx error_code{PGM_regular_error};
    };

    PowerGridBatchError(std::string message, std::vector<FailedScenario> failed_scenarios_c)
        : PowerGridError{std::move(message)}, failed_scenarios_{std::move(failed_scenarios_c)} {}
    Idx error_code() const noexcept override { return PGM_batch_error; }
    std::vector<FailedScenario> const& failed_scenarios() const { return failed_scenarios_; }

  private:
    std::vector<FailedScenario> failed_scenarios_;
};

class PowerGridDataError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_data_error; }
};

class PowerGridComponentDataError : public PowerGridDataError {
  public:
    using PowerGridDataError::PowerGridDataError;
    Idx error_code() const noexcept override { return PGM_component_data_error; }
};

class PowerGridMultiComponentDataError : public PowerGridDataError {
  public:
    using PowerGridDataError::PowerGridDataError;
    Idx error_code() const noexcept override { return PGM_multi_component_data_error; }
};

class PowerGridCalculationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_calculation_error; }
};

class PowerGridBadRequest : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_bad_request; }
};

class PowerGridInternalError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_internal_error; }
};

class PowerGridUnreachableHitError : public PowerGridInternalError {
  public:
    using PowerGridInternalError::PowerGridInternalError;
    Idx error_code() const noexcept override { return PGM_unreachable_hit; }
};

class InvalidArguments : public PowerGridBadRequest {
  public:
    using PowerGridBadRequest::PowerGridBadRequest;
    Idx error_code() const noexcept override { return PGM_invalid_arguments; }
};

class ConflictVoltage : public PowerGridComponentDataError {
  public:
    using PowerGridComponentDataError::PowerGridComponentDataError;
    Idx error_code() const noexcept override { return PGM_conflict_voltage; }
};

class InvalidTransformerClock : public PowerGridComponentDataError {
  public:
    using PowerGridComponentDataError::PowerGridComponentDataError;
    Idx error_code() const noexcept override { return PGM_invalid_transformer_clock; }
};

class SparseMatrixError : public PowerGridCalculationError {
  public:
    using PowerGridCalculationError::PowerGridCalculationError;
    Idx error_code() const noexcept override { return PGM_sparse_matrix_error; }
};

class [[deprecated("This exception is no longer relevant and may be removed in a future version")]]
MissingCaseForEnumError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_invalid_arguments; }
};

class ConflictID : public PowerGridDataError {
  public:
    using PowerGridDataError::PowerGridDataError;
    Idx error_code() const noexcept override { return PGM_conflict_id; }
};

class IDNotFound : public PowerGridDataError {
  public:
    using PowerGridDataError::PowerGridDataError;
    Idx error_code() const noexcept override { return PGM_id_not_found; }
};

class IDWrongType : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_id_wrong_type; }
};

class PowerGridDatasetError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_dataset_error; }
};

class InvalidMeasuredObject : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_invalid_measured_object; }
};

class ConflictingAngleMeasurementType : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_conflicting_angle_measurement_type; }
};

class InvalidRegulatedObject : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_invalid_regulated_object; }
};

class DuplicativelyRegulatedObject : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_duplicatively_regulated_object; }
};

class UnsupportedRegulatorCombinationError : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_unsupported_regulator_combination; }
};

class ConflictingVoltageRegulatorURef : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_conflicting_voltage_regulator_u_ref; }
};

class UnsupportedLoadGenTypeForVoltageRegulator : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_unsupported_load_gen_type_for_voltage_regulator; }
};

class UnsupportedVoltageRegulatorSourceCombinationError : public PowerGridMultiComponentDataError {
  public:
    using PowerGridMultiComponentDataError::PowerGridMultiComponentDataError;
    Idx error_code() const noexcept override { return PGM_unsupported_voltage_regulator_source_combination; }
};

class AutomaticTapInputError : public PowerGridCalculationError {
  public:
    using PowerGridCalculationError::PowerGridCalculationError;
    Idx error_code() const noexcept override { return PGM_automatic_tap_input_error; }
};

class TapSearchStrategyIncompatibleError : public PowerGridUnreachableHitError {
  public:
    using PowerGridUnreachableHitError::PowerGridUnreachableHitError;
    Idx error_code() const noexcept override { return PGM_tap_search_strategy_incompatible; }
};

class InvalidCalculationMethod : public PowerGridBadRequest {
  public:
    using PowerGridBadRequest::PowerGridBadRequest;
    Idx error_code() const noexcept override { return PGM_invalid_calculation_method; }
};

class IterationDiverge : public PowerGridCalculationError {
  public:
    using PowerGridCalculationError::PowerGridCalculationError;
    Idx error_code() const noexcept override { return PGM_iteration_diverge; }
};

class MaxIterationReached : public PowerGridCalculationError {
  public:
    using PowerGridCalculationError::PowerGridCalculationError;
    Idx error_code() const noexcept override { return PGM_max_iteration_reached; }
};

class NotObservableError : public PowerGridCalculationError {
  public:
    using PowerGridCalculationError::PowerGridCalculationError;
    Idx error_code() const noexcept override { return PGM_not_observable; }
};

class InvalidShortCircuitPhaseOrType : public PowerGridComponentDataError {
  public:
    using PowerGridComponentDataError::PowerGridComponentDataError;
    Idx error_code() const noexcept override { return PGM_invalid_short_circuit_phase_or_type; }
};

class PowerGridNotImplementedError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_not_implemented; }
};

class ExperimentalFeature : public PowerGridBadRequest {
  public:
    using PowerGridBadRequest::PowerGridBadRequest;
    Idx error_code() const noexcept override { return PGM_experimental_feature; }
};

class PowerGridUnknownError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_unknown_error; }
};

class PowerGridIllegalOperationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_illegal_operation; }
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_ERRORS_HPP
