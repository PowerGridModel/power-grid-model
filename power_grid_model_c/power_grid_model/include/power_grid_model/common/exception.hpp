// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "enum.hpp"
#include "error_codes.hpp"

#include <concepts>
#include <exception>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace power_grid_model {
class PowerGridError : public std::exception {
  public:
    char const* what() const noexcept final { return msg_.c_str(); }
    constexpr PowerGridErrorCode error_code() const noexcept { return error_code_; }

  protected:
    PowerGridError() = default;
    explicit PowerGridError(std::string msg, PowerGridErrorCode error_code = PowerGridErrorCode::regular_error)
        : msg_{std::move(msg)}, error_code_{error_code} {}

    void append_msg(std::string_view msg) { msg_ = std::format("{} {}", msg_, msg); }

  private:
    std::string msg_;
    PowerGridErrorCode error_code_{PowerGridErrorCode::regular_error};
};

namespace detail {
template <std::derived_from<PowerGridError> Exception> constexpr PowerGridErrorCode get_error_code();
template <std::derived_from<PowerGridError> Exception>
constexpr PowerGridErrorCode get_error_code(Exception const& /*exception*/);
} // namespace detail

class InvalidArguments : public PowerGridError {
  public:
    struct TypeValuePair {
        std::string name;
        std::string value;
    };

    template <std::same_as<TypeValuePair>... Options>
    InvalidArguments(std::string error_msg, PowerGridErrorCode error_code = PowerGridErrorCode::invalid_arguments)
        : PowerGridError{std::move(error_msg), error_code} {}

    template <std::same_as<TypeValuePair>... Options>
    InvalidArguments(std::string_view method, std::string_view arguments,
                     PowerGridErrorCode error_code = PowerGridErrorCode::invalid_arguments)
        : PowerGridError{std::format("{} is not implemented for {}!\n", method, arguments), error_code} {}

    template <class... Options>
        requires((std::same_as<std::remove_cvref_t<Options>, TypeValuePair> && ...) && sizeof...(Options) > 0)
    InvalidArguments(std::string_view method, Options const&... options)
        : InvalidArguments{method, "the following combination of options", detail::get_error_code(*this)} {
        (append_msg(std::format("{}: {}\n", options.name, options.value)), ...);
    }
};

class MissingCaseForEnumError : public InvalidArguments {
  public:
    template <typename T>
    MissingCaseForEnumError(std::string_view method, T const& value)
        : InvalidArguments{method, std::format("{} #{:d}", typeid(T).name(), value), detail::get_error_code(*this)} {}

    template <typename T>
        requires std::is_enum_v<T>
    MissingCaseForEnumError(std::string_view method, T const& value)
        : InvalidArguments{method, std::format("{} #{:d}", typeid(T).name(), std::to_underlying(value)),
                           detail::get_error_code(*this)} {}
};

class ConflictVoltage : public PowerGridError {
  public:
    ConflictVoltage(ID id, ID id1, ID id2, double u1, double u2)
        : PowerGridError{
              std::format(
                  "Conflicting voltage for line {}\n voltage at from node {} is {}\n voltage at to node {} is {}\n", id,
                  id1, u1, id2, u2),
              detail::get_error_code(*this)} {}
};

class InvalidTransformerClock : public PowerGridError {
  public:
    InvalidTransformerClock(ID id, IntS clock)
        : PowerGridError{std::format("Invalid clock for transformer {}, clock {}\n", id, clock),
                         detail::get_error_code(*this)} {}
};

class SparseMatrixError : public PowerGridError {
  public:
    SparseMatrixError(Idx err, std::string_view msg = "")
        : PowerGridError{
              std::format(
                  "Sparse matrix error with error code #{} (possibly singular)\n{}If you get this error from state "
                  "estimation, it usually means the system is not fully observable, i.e. not enough measurements.",
                  err, msg.empty() ? "" : std::format("{}\n", msg)),
              detail::get_error_code(*this)} {}
    SparseMatrixError()
        : PowerGridError{"Sparse matrix error, possibly singular matrix!\n"
                         "If you get this error from state estimation, "
                         "it might mean the system is not fully observable, i.e. not enough measurements.\n"
                         "It might also mean that you are running into a corner case where PGM cannot resolve yet.\n"
                         "See https://github.com/PowerGridModel/power-grid-model/issues/864.",
                         detail::get_error_code(*this)} {}
};

class NotObservableError : public PowerGridError {
  public:
    NotObservableError()
        : PowerGridError{"Not enough measurements available for state estimation.\n", detail::get_error_code(*this)} {}
    NotObservableError(std::string_view msg)
        : PowerGridError{std::format("Not enough measurements available for state estimation.\n{}\n", msg),
                         detail::get_error_code(*this)} {}
};

class IterationDiverge : public PowerGridError {
  public:
    IterationDiverge(std::string msg, PowerGridErrorCode error_code = PowerGridErrorCode::iteration_diverge)
        : PowerGridError{std::move(msg), error_code} {}
    IterationDiverge(Idx num_iter, double max_dev, double err_tol,
                     PowerGridErrorCode error_code = PowerGridErrorCode::iteration_diverge)
        : IterationDiverge{
              std::format("Iteration failed to converge after {} iterations! Max deviation: {}, error tolerance: {}.\n",
                          num_iter, max_dev, err_tol),
              error_code} {}
};

class MaxIterationReached : public IterationDiverge {
  public:
    MaxIterationReached(std::string_view msg = "")
        : IterationDiverge{std::format("Maximum number of iterations reached! {}\n", msg),
                           detail::get_error_code(*this)} {}
};

class ConflictID : public PowerGridError {
  public:
    explicit ConflictID(ID id)
        : PowerGridError{std::format("Conflicting id detected: {}\n", id), detail::get_error_code(*this)} {}
};

class IDNotFound : public PowerGridError {
  public:
    explicit IDNotFound(ID id)
        : PowerGridError{std::format("The id cannot be found: {}\n", id), detail::get_error_code(*this)} {}
};
class Idx2DNotFound : public PowerGridError {
  public:
    explicit Idx2DNotFound(Idx2D id)
        : PowerGridError{std::format("The idx 2d cannot be found: {{{}, {}}}.\n", id.group, id.pos),
                         detail::get_error_code(*this)} {}
};

class InvalidMeasuredObject : public PowerGridError {
  public:
    InvalidMeasuredObject(std::string_view object, std::string_view sensor)
        : PowerGridError{std::format("{} measurement is not supported for object of type {}", sensor, object),
                         detail::get_error_code(*this)} {}
};

class InvalidMeasuredTerminalType : public PowerGridError {
  public:
    InvalidMeasuredTerminalType(MeasuredTerminalType const terminal_type, std::string_view sensor)
        : PowerGridError{std::format("{} measurement is not supported for object of type {}", sensor,
                                     std::to_underlying(terminal_type)),
                         detail::get_error_code(*this)} {}
};

class InvalidRegulatedObject : public PowerGridError {
  public:
    InvalidRegulatedObject(std::string_view object, std::string_view regulator)
        : PowerGridError{std::format("{} regulator is not supported for object of type {}", regulator, object),
                         detail::get_error_code(*this)} {}
    InvalidRegulatedObject(ID id, std::string_view regulator)
        : PowerGridError{std::format("{} regulator is not supported for object with ID {}", regulator, id),
                         detail::get_error_code(*this)} {}
};

class DuplicativelyRegulatedObject : public PowerGridError {
  public:
    DuplicativelyRegulatedObject()
        : PowerGridError{"There are objects regulated by more than one regulator. Maximum one regulator is allowed.",
                         detail::get_error_code(*this)} {}
};

class UnsupportedRegulatorCombinationError : public PowerGridError {
  public:
    UnsupportedRegulatorCombinationError()
        : PowerGridError{"The combination of voltage regulators and transformer tap regulators is not supported in the "
                         "same model.",
                         detail::get_error_code(*this)} {}
};

class ConflictingVoltageRegulatorURef : public PowerGridError {
  public:
    ConflictingVoltageRegulatorURef(std::string const& regulator_ids)
        : PowerGridError{std::format("Conflicting u_ref values detected for voltage regulators {}.", regulator_ids),
                         detail::get_error_code(*this)} {}
};

class UnsupportedLoadGenTypeForVoltageRegulator : public PowerGridError {
  public:
    UnsupportedLoadGenTypeForVoltageRegulator(std::string const& regulator_ids)
        : PowerGridError{std::format("Unsupported load_gen type for voltage regulators {}.", regulator_ids),
                         detail::get_error_code(*this)} {}
};

class UnsupportedVoltageRegulatorSourceCombinationError : public PowerGridError {
  public:
    UnsupportedVoltageRegulatorSourceCombinationError(ID id)
        : PowerGridError{std::format("Nodes with a source and a voltage regulated load/generator are not supported "
                                     "when both are enabled. Found at node with id {}",
                                     id),
                         detail::get_error_code(*this)} {}
};

class AutomaticTapCalculationError : public PowerGridError {
  public:
    AutomaticTapCalculationError(ID id)
        : PowerGridError{
              std::format("Automatic tap changing regulator with tap_side at LV side is not supported. Found at id {}",
                          id),
              detail::get_error_code(*this)} {}
};

class AutomaticTapInputError : public PowerGridError {
  public:
    AutomaticTapInputError(std::string_view msg)
        : PowerGridError{std::format("Automatic tap changer has invalid configuration. {}", msg),
                         detail::get_error_code(*this)} {}
};

class IDWrongType : public PowerGridError {
  public:
    explicit IDWrongType(ID id)
        : PowerGridError{std::format("Wrong type for object with id {}\n", id), detail::get_error_code(*this)} {}
};

class ConflictingAngleMeasurementType : public PowerGridError {
  public:
    ConflictingAngleMeasurementType(std::string_view msg)
        : PowerGridError{std::format("Conflicting angle measurement type. {}", msg), detail::get_error_code(*this)} {}
};

class CalculationError : public PowerGridError {
  public:
    explicit CalculationError(std::string msg, PowerGridErrorCode error_code = PowerGridErrorCode::calculation_error)
        : PowerGridError{std::move(msg), error_code} {}
};

class BatchCalculationError : public CalculationError {
  public:
    BatchCalculationError(std::string msg, IdxVector failed_scenarios, std::vector<PowerGridErrorCode> err_codes,
                          std::vector<std::string> err_msgs)
        : CalculationError{std::move(msg), detail::get_error_code(*this)},
          failed_scenarios_{std::move(failed_scenarios)},
          err_codes_{std::move(err_codes)},
          err_msgs_(std::move(err_msgs)) {}

    IdxVector const& failed_scenarios() const { return failed_scenarios_; }

    std::vector<PowerGridErrorCode> const& err_codes() const { return err_codes_; }
    std::vector<std::string> const& err_msgs() const { return err_msgs_; }

  private:
    IdxVector failed_scenarios_;
    std::vector<PowerGridErrorCode> err_codes_;
    std::vector<std::string> err_msgs_;
};

class InvalidCalculationMethod : public CalculationError {
  public:
    InvalidCalculationMethod()
        : CalculationError{"The calculation method is invalid for this calculation!", detail::get_error_code(*this)} {}
};

class InvalidShortCircuitType : public PowerGridError {
  public:
    explicit InvalidShortCircuitType(FaultType short_circuit_type)
        : PowerGridError{
              std::format("The short circuit type ({}) is invalid!\n", std::to_underlying(short_circuit_type)),
              detail::get_error_code(*this)} {}
    InvalidShortCircuitType(bool sym, FaultType short_circuit_type)
        : PowerGridError{std::format("The short circuit type ({}) does not match the calculation type (symmetric={})\n",
                                     std::to_underlying(short_circuit_type), static_cast<int>(sym)),
                         detail::get_error_code(*this)} {}
};

class InvalidShortCircuitPhases : public PowerGridError {
  public:
    InvalidShortCircuitPhases(FaultType short_circuit_type, FaultPhase short_circuit_phases)
        : PowerGridError{std::format("The short circuit phases ({}) do not match the short circuit type ({})\n",
                                     std::to_underlying(short_circuit_phases), std::to_underlying(short_circuit_type)),
                         detail::get_error_code(*this)} {}
};

class InvalidShortCircuitPhaseOrType : public PowerGridError {
  public:
    InvalidShortCircuitPhaseOrType()
        : PowerGridError{"During one calculation the short circuit types phases should be similar for all faults\n",
                         detail::get_error_code(*this)} {}
};

class SerializationError : public PowerGridError {
  public:
    explicit SerializationError(std::string msg) : PowerGridError{std::move(msg), detail::get_error_code(*this)} {}
};

class DatasetError : public PowerGridError {
  public:
    explicit DatasetError(std::string_view msg)
        : PowerGridError{std::format("Dataset error: {}", msg), detail::get_error_code(*this)} {}
};

class ExperimentalFeature : public InvalidArguments {
  public:
    explicit ExperimentalFeature(std::string error_msg)
        : InvalidArguments{std::move(error_msg), detail::get_error_code(*this)} {}

    ExperimentalFeature(std::string_view method, std::string_view arguments)
        : InvalidArguments{method, arguments, detail::get_error_code(*this)} {}

    template <class... Options>
        requires((std::same_as<std::remove_cvref_t<Options>, TypeValuePair> && ...) && sizeof...(Options) > 0)
    ExperimentalFeature(std::string_view method, Options const&... options)
        : InvalidArguments{method, "the following combination of options", detail::get_error_code(*this)} {
        (append_msg(std::format("{}: {}\n", options.name, options.value)), ...);
    }
};

class NotImplementedError : public PowerGridError {
  public:
    NotImplementedError() : PowerGridError{"Function not yet implemented", detail::get_error_code(*this)} {}
};

class UnreachableHit : public PowerGridError {
  public:
    UnreachableHit(std::string_view method, std::string_view reason_for_assumption)
        : PowerGridError{
              std::format("Unreachable code hit when executing {}.\n The following assumption for unreachability "
                          "was not met: {}.\n This may be a bug in the library\n",
                          method, reason_for_assumption),
              detail::get_error_code(*this)} {}
};

class TapSearchStrategyIncompatibleError : public InvalidArguments {
  public:
    template <typename T1, typename T2>
    TapSearchStrategyIncompatibleError(std::string_view method, T1 const& value1, T2 const& value2)
        : InvalidArguments{method,
                           std::format("{} #{} and {} #{}", typeid(T1).name(), std::to_underlying(value1),
                                       typeid(T2).name(), std::to_underlying(value2)),
                           detail::get_error_code(*this)} {}
};

namespace detail {
template <std::derived_from<PowerGridError> Exception> constexpr PowerGridErrorCode get_error_code() {
    using enum PowerGridErrorCode;
    if constexpr (std::same_as<Exception, InvalidArguments>) {
        return invalid_arguments;
    } else if constexpr (std::same_as<Exception, ExperimentalFeature>) {
        return experimental_feature;
    } else if constexpr (std::same_as<Exception, MissingCaseForEnumError>) {
        return missing_case_for_enum;
    } else if constexpr (std::same_as<Exception, ConflictVoltage>) {
        return conflict_voltage;
    } else if constexpr (std::same_as<Exception, InvalidTransformerClock>) {
        return invalid_transformer_clock;
    } else if constexpr (std::same_as<Exception, SparseMatrixError>) {
        return sparse_matrix_error;
    } else if constexpr (std::same_as<Exception, ConflictID>) {
        return conflict_id;
    } else if constexpr (std::same_as<Exception, IDNotFound> || std::same_as<Exception, Idx2DNotFound>) {
        return id_not_found;
    } else if constexpr (std::same_as<Exception, IDWrongType>) {
        return id_wrong_type;
    } else if constexpr (std::same_as<Exception, DatasetError>) {
        return dataset_error;
    } else if constexpr (std::same_as<Exception, InvalidMeasuredObject> ||
                         std::same_as<Exception, InvalidMeasuredTerminalType>) {
        return invalid_measured_object;
    } else if constexpr (std::same_as<Exception, ConflictingAngleMeasurementType>) {
        return conflicting_angle_measurement_type;
    } else if constexpr (std::same_as<Exception, InvalidRegulatedObject>) {
        return invalid_regulated_object;
    } else if constexpr (std::same_as<Exception, DuplicativelyRegulatedObject>) {
        return duplicatively_regulated_object;
    } else if constexpr (std::same_as<Exception, UnsupportedRegulatorCombinationError>) {
        return unsupported_regulator_combination;
    } else if constexpr (std::same_as<Exception, ConflictingVoltageRegulatorURef>) {
        return conflicting_voltage_regulator_u_ref;
    } else if constexpr (std::same_as<Exception, UnsupportedLoadGenTypeForVoltageRegulator>) {
        return unsupported_load_gen_type_for_voltage_regulator;
    } else if constexpr (std::same_as<Exception, UnsupportedVoltageRegulatorSourceCombinationError>) {
        return unsupported_voltage_regulator_source_combination;
    } else if constexpr (std::same_as<Exception, AutomaticTapCalculationError>) {
        return automatic_tap_calculation_error;
    } else if constexpr (std::same_as<Exception, AutomaticTapInputError>) {
        return automatic_tap_input_error;
    } else if constexpr (std::same_as<Exception, TapSearchStrategyIncompatibleError>) {
        return tap_search_strategy_incompatible;
    } else if constexpr (std::same_as<Exception, InvalidCalculationMethod>) {
        return invalid_calculation_method;
    } else if constexpr (std::same_as<Exception, IterationDiverge>) {
        return iteration_diverge;
    } else if constexpr (std::same_as<Exception, MaxIterationReached>) {
        return max_iteration_reached;
    } else if constexpr (std::same_as<Exception, NotObservableError>) {
        return not_observable;
    } else if constexpr (std::same_as<Exception, InvalidShortCircuitType> ||
                         std::same_as<Exception, InvalidShortCircuitPhases> ||
                         std::same_as<Exception, InvalidShortCircuitPhaseOrType>) {
        return invalid_short_circuit_phase_or_type;
    } else if constexpr (std::same_as<Exception, BatchCalculationError>) {
        return batch_error;
    } else if constexpr (std::same_as<Exception, CalculationError>) {
        return calculation_error;
    } else if constexpr (std::same_as<Exception, SerializationError>) {
        return serialization_error;
    } else if constexpr (std::same_as<Exception, NotImplementedError>) {
        return not_implemented;
    } else if constexpr (std::same_as<Exception, UnreachableHit>) {
        return unreachable_hit;
    } else {
        // PowerGridError itself and exceptions derived outside this header default to a regular error
        return regular_error;
    }
}

template <std::derived_from<PowerGridError> Exception>
constexpr PowerGridErrorCode get_error_code(Exception const& /*exception*/) {
    return get_error_code<Exception>();
}
} // namespace detail

} // namespace power_grid_model
