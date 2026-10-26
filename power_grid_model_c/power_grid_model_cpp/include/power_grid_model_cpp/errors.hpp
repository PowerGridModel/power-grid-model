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

// Errors that correspond to a specific C API error code. The mapping from error code to type is generated into
// error_codes.hpp from code_generation/data/error_codes/error_codes.json; keep the type names in sync with it.
class InvalidArguments : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_invalid_arguments; }
};

class ConflictVoltage : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_conflict_voltage; }
};

class InvalidTransformerClock : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_invalid_transformer_clock; }
};

class SparseMatrixError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_sparse_matrix_error; }
};

class MissingCaseForEnumError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_missing_case_for_enum; }
};

class ConflictID : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_conflict_id; }
};

class IDNotFound : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_id_not_found; }
};

class IDWrongType : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_id_wrong_type; }
};

class PowerGridDatasetError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_dataset_error; }
};

class PowerGridCalculationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_calculation_error; }
};

class InvalidMeasuredObject : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_invalid_measured_object; }
};

class ConflictingAngleMeasurementType : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_conflicting_angle_measurement_type; }
};

class InvalidRegulatedObject : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_invalid_regulated_object; }
};

class DuplicativelyRegulatedObject : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_duplicatively_regulated_object; }
};

class UnsupportedRegulatorCombinationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_unsupported_regulator_combination; }
};

class ConflictingVoltageRegulatorURef : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_conflicting_voltage_regulator_u_ref; }
};

class UnsupportedLoadGenTypeForVoltageRegulator : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_unsupported_load_gen_type_for_voltage_regulator; }
};

class UnsupportedVoltageRegulatorSourceCombinationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_unsupported_voltage_regulator_source_combination; }
};

class AutomaticTapCalculationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_automatic_tap_calculation_error; }
};

class AutomaticTapInputError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_automatic_tap_input_error; }
};

class TapSearchStrategyIncompatibleError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_tap_search_strategy_incompatible; }
};

class InvalidCalculationMethod : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_invalid_calculation_method; }
};

class IterationDiverge : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_iteration_diverge; }
};

class MaxIterationReached : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_max_iteration_reached; }
};

class NotObservableError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_not_observable; }
};

class InvalidShortCircuitPhaseOrType : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_invalid_short_circuit_phase_or_type; }
};

class PowerGridNotImplementedError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_not_implemented; }
};

class ExperimentalFeature : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_experimental_feature; }
};

class PowerGridUnreachableHitError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_unreachable_hit; }
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
