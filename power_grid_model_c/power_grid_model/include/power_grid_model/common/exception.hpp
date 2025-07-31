// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "enum.hpp"

#include <exception>
#include <format>
#include <sstream>
#include <string>

namespace power_grid_model {
class PowerGridError : public std::exception {
  public:
    char const* what() const noexcept final { return msg_.c_str(); }

  protected:
    PowerGridError() = default;
    PowerGridError(std::string msg) : msg_{std::move(msg)} {}

    void append_msg(std::string_view msg) { msg_ = std::format("{} {}", msg_, msg); }

  private:
    std::string msg_;
};

class InvalidArguments : public PowerGridError {
  public:
    struct TypeValuePair {
        std::string name;
        std::string value;
    };

    template <std::same_as<TypeValuePair>... Options>
    InvalidArguments(std::string error_msg) : PowerGridError{std::move(error_msg)} {}

    template <std::same_as<TypeValuePair>... Options>
    InvalidArguments(std::string_view method, std::string_view arguments)
        : PowerGridError{std::format("{} is not implemented for {}!\n", method, arguments)} {}

    template <class... Options>
        requires((std::same_as<std::remove_cvref_t<Options>, TypeValuePair> && ...) && sizeof...(Options) > 0)
    InvalidArguments(std::string_view method, Options const&... options)
        : InvalidArguments{method, "the following combination of options"} {
        (append_msg(std::format("{}: {}\n", options.name, options.value)), ...);
    }
};

class MissingCaseForEnumError : public InvalidArguments {
  public:
    template <typename T>
    MissingCaseForEnumError(std::string_view method, T const& value)
        : InvalidArguments{method, std::format("{} #{:d}", typeid(T).name(), static_cast<IntS>(value))} {}
};

class ConflictVoltage : public PowerGridError {
  public:
    ConflictVoltage(ID id, ID id1, ID id2, double u1, double u2)
        : PowerGridError{std::format(
              "Conflicting voltage for line {}\n voltage at from node {} is {}\n voltage at to node {} is {}\n", id,
              id1, u1, id2, u2)} {}
};

class InvalidBranch : public PowerGridError {
  public:
    InvalidBranch(ID branch_id, ID node_id)
        : PowerGridError{std::format("Branch {} has the same from- and to-node {},\n This is not allowed!\n", branch_id,
                                     node_id)} {}
};

class InvalidBranch3 : public PowerGridError {
  public:
    InvalidBranch3(ID branch3_id, ID node_1_id, ID node_2_id, ID node_3_id)
        : PowerGridError{std::format(
              "Branch3 {} is connected to the same node at least twice. Node 1/2/3: {}/{}/{},\n This is not allowed!\n",
              branch3_id, node_1_id, node_2_id, node_3_id)} {}
};

class InvalidTransformerClock : public PowerGridError {
  public:
    InvalidTransformerClock(ID id, IntS clock)
        : PowerGridError{std::format("Invalid clock for transformer {}, clock {}\n", id, clock)} {}
};

class SparseMatrixError : public PowerGridError {
  public:
    SparseMatrixError(Idx err, std::string_view msg = "")
        : PowerGridError{std::format(
              "Sparse matrix error with error code #{} (possibly singular)\n{}If you get this error from state "
              "estimation, it usually means the system is not fully observable, i.e. not enough measurements.",
              err, msg.empty() ? "" : std::format("{}\n", msg))} {}
    SparseMatrixError()
        : PowerGridError{"Sparse matrix error, possibly singular matrix!\n"
                         "If you get this error from state estimation, "
                         "it might mean the system is not fully observable, i.e. not enough measurements.\n"
                         "It might also mean that you are running into a corner case where PGM cannot resolve yet.\n"
                         "See https://github.com/PowerGridModel/power-grid-model/issues/864."} {}
};

class NotObservableError : public PowerGridError {
  public:
    NotObservableError() : PowerGridError{"Not enough measurements available for state estimation.\n"} {}
    NotObservableError(std::string_view msg)
        : PowerGridError{std::format("Not enough measurements available for state estimation.\n{}\n", msg)} {}
};

class IterationDiverge : public PowerGridError {
  public:
    IterationDiverge(std::string msg) : PowerGridError{std::move(msg)} {}
    IterationDiverge(Idx num_iter, double max_dev, double err_tol)
        : PowerGridError{
              std::format("Iteration failed to converge after {} iterations! Max deviation: {}, error tolerance: {}.\n",
                          num_iter, max_dev, err_tol)} {}
};

class MaxIterationReached : public IterationDiverge {
  public:
    MaxIterationReached(std::string_view msg = "")
        : IterationDiverge{std::format("Maximum number of iterations reached! {}\n", msg)} {}
};

class ConflictID : public PowerGridError {
  public:
    explicit ConflictID(ID id) : PowerGridError{std::format("Conflicting id detected: {}\n", id)} {}
};

class IDNotFound : public PowerGridError {
  public:
    explicit IDNotFound(ID id) : PowerGridError{std::format("The id cannot be found: {}\n", id)} {}
};
class Idx2DNotFound : public PowerGridError {
  public:
    explicit Idx2DNotFound(Idx2D id)
        : PowerGridError{std::format("The idx 2d cannot be found: {{{}, {}}}.\n", id.group, id.pos)} {}
};

class InvalidMeasuredObject : public PowerGridError {
  public:
    InvalidMeasuredObject(std::string_view object, std::string_view sensor)
        : PowerGridError{std::format("{} measurement is not supported for object of type {}", sensor, object)} {}
};

class InvalidMeasuredTerminalType : public PowerGridError {
  public:
    InvalidMeasuredTerminalType(MeasuredTerminalType const terminal_type, std::string_view sensor)
        : PowerGridError{std::format("{} measurement is not supported for object of type {}", sensor,
                                     static_cast<IntS>(terminal_type))} {}
};

class InvalidRegulatedObject : public PowerGridError {
  public:
    InvalidRegulatedObject(std::string_view object, std::string_view regulator)
        : PowerGridError{std::format("{} regulator is not supported for object of type {}", regulator, object)} {}
    InvalidRegulatedObject(ID id, std::string_view regulator)
        : PowerGridError{std::format("{} regulator is not supported for object with ID {}", regulator, id)} {}
};

class DuplicativelyRegulatedObject : public PowerGridError {
  public:
    DuplicativelyRegulatedObject()
        : PowerGridError{"There are objects regulated by more than one regulator. Maximum one regulator is allowed."} {}
};

class AutomaticTapCalculationError : public PowerGridError {
  public:
    AutomaticTapCalculationError(ID id)
        : PowerGridError{std::format(
              "Automatic tap changing regulator with tap_side at LV side is not supported. Found at id {}", id)} {}
};

class AutomaticTapInputError : public PowerGridError {
  public:
    AutomaticTapInputError(std::string_view msg)
        : PowerGridError{std::format("Automatic tap changer has invalid configuration. {}", msg)} {}
};

class IDWrongType : public PowerGridError {
  public:
    explicit IDWrongType(ID id) : PowerGridError{std::format("Wrong type for object with id {}\n", id)} {}
};

class ConflictingAngleMeasurementType : public PowerGridError {
  public:
    ConflictingAngleMeasurementType(std::string_view msg)
        : PowerGridError{std::format("Conflicting angle measurement type. {}", msg)} {}
};

class CalculationError : public PowerGridError {
  public:
    explicit CalculationError(std::string msg) : PowerGridError{std::move(msg)} {}
};

class BatchCalculationError : public CalculationError {
  public:
    BatchCalculationError(std::string msg, IdxVector failed_scenarios, std::vector<std::string> err_msgs)
        : CalculationError{std::move(msg)},
          failed_scenarios_{std::move(failed_scenarios)},
          err_msgs_(std::move(err_msgs)) {}

    IdxVector const& failed_scenarios() const { return failed_scenarios_; }

    std::vector<std::string> const& err_msgs() const { return err_msgs_; }

  private:
    IdxVector failed_scenarios_;
    std::vector<std::string> err_msgs_;
};

class InvalidCalculationMethod : public CalculationError {
  public:
    InvalidCalculationMethod() : CalculationError{"The calculation method is invalid for this calculation!"} {}
};

class InvalidShortCircuitType : public PowerGridError {
  public:
    explicit InvalidShortCircuitType(FaultType short_circuit_type)
        : PowerGridError{
              std::format("The short circuit type ({}) is invalid!\n", static_cast<IntS>(short_circuit_type))} {}
    InvalidShortCircuitType(bool sym, FaultType short_circuit_type)
        : PowerGridError{std::format("The short circuit type ({}) does not match the calculation type (symmetric={})\n",
                                     static_cast<IntS>(short_circuit_type), static_cast<int>(sym))} {}
};

class InvalidShortCircuitPhases : public PowerGridError {
  public:
    InvalidShortCircuitPhases(FaultType short_circuit_type, FaultPhase short_circuit_phases)
        : PowerGridError{std::format("The short circuit phases ({}) do not match the short circuit type ({})\n",
                                     static_cast<IntS>(short_circuit_phases), static_cast<IntS>(short_circuit_type))} {}
};

class InvalidShortCircuitPhaseOrType : public PowerGridError {
  public:
    InvalidShortCircuitPhaseOrType()
        : PowerGridError{"During one calculation the short circuit types phases should be similar for all faults\n"} {}
};

class SerializationError : public PowerGridError {
  public:
    explicit SerializationError(std::string msg) : PowerGridError{std::move(msg)} {}
};

class DatasetError : public PowerGridError {
  public:
    explicit DatasetError(std::string_view msg) : PowerGridError{std::format("Dataset error: {}", msg)} {}
};

class ExperimentalFeature : public InvalidArguments {
    using InvalidArguments::InvalidArguments;
};

class NotImplementedError : public PowerGridError {
  public:
    NotImplementedError() : PowerGridError{"Function not yet implemented"} {}
};

class UnreachableHit : public PowerGridError {
  public:
    UnreachableHit(std::string_view method, std::string_view reason_for_assumption)
        : PowerGridError{
              std::format("Unreachable code hit when executing {}.\n The following assumption for unreachability "
                          "was not met: {}.\n This may be a bug in the library\n",
                          method, reason_for_assumption)} {}
};

class TapSearchStrategyIncompatibleError : public InvalidArguments {
  public:
    template <typename T1, typename T2>
    TapSearchStrategyIncompatibleError(std::string_view method, T1 const& value1, T2 const& value2)
        : InvalidArguments{method, std::format("{} #{} and {} #{}", typeid(T1).name(), static_cast<IntS>(value1),
                                               typeid(T2).name(), static_cast<IntS>(value2))} {}
};

} // namespace power_grid_model
