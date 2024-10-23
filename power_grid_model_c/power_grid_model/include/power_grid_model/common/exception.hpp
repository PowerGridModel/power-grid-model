// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "enum.hpp"

#include <exception>
#include <sstream>
#include <string>

namespace power_grid_model {
namespace detail {
inline auto to_string(std::floating_point auto x) {
    std::ostringstream sstr{}; // NOLINT(misc-const-correctness) // https://github.com/llvm/llvm-project/issues/57297
    sstr << x;
    return sstr.str();
}
inline auto to_string(std::integral auto x) { return std::to_string(x); }
} // namespace detail

class PowerGridError : public std::exception {
  public:
    void append_msg(std::string_view msg) { msg_ += msg; }
    char const* what() const noexcept final { return msg_.c_str(); }

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
    InvalidArguments(std::string const& method, std::string const& arguments) {
        append_msg(method + " is not implemented for " + arguments + "!\n");
    }

    template <class... Options>
        requires(std::same_as<std::remove_cvref_t<Options>, TypeValuePair> && ...)
    InvalidArguments(std::string const& method, Options&&... options)
        : InvalidArguments{method, "the following combination of options"} {
        (append_msg(" " + std::forward<Options>(options).name + ": " + std::forward<Options>(options).value + "\n"),
         ...);
    }
};

class MissingCaseForEnumError : public InvalidArguments {
  public:
    template <typename T>
    MissingCaseForEnumError(std::string const& method, const T& value)
        : InvalidArguments{method, std::string{typeid(T).name()} + " #" + detail::to_string(static_cast<IntS>(value))} {
    }
};

class ConflictVoltage : public PowerGridError {
  public:
    ConflictVoltage(ID id, ID id1, ID id2, double u1, double u2) {
        append_msg("Conflicting voltage for line " + detail::to_string(id) + "\n voltage at from node " +
                   detail::to_string(id1) + " is " + detail::to_string(u1) + "\n voltage at to node " +
                   detail::to_string(id2) + " is " + detail::to_string(u2) + '\n');
    }
};

class InvalidBranch : public PowerGridError {
  public:
    InvalidBranch(ID branch_id, ID node_id) {
        append_msg("Branch " + detail::to_string(branch_id) + " has the same from- and to-node " +
                   detail::to_string(node_id) + ",\n This is not allowed!\n");
    }
};

class InvalidBranch3 : public PowerGridError {
  public:
    InvalidBranch3(ID branch3_id, ID node_1_id, ID node_2_id, ID node_3_id) {
        append_msg("Branch3 " + detail::to_string(branch3_id) +
                   " is connected to the same node at least twice. Node 1/2/3: " + detail::to_string(node_1_id) + "/" +
                   detail::to_string(node_2_id) + "/" + detail::to_string(node_3_id) + ",\n This is not allowed!\n");
    }
};

class InvalidTransformerClock : public PowerGridError {
  public:
    InvalidTransformerClock(ID id, IntS clock) {
        append_msg("Invalid clock for transformer " + detail::to_string(id) + ", clock " + detail::to_string(clock) +
                   '\n');
    }
};

class SparseMatrixError : public PowerGridError {
  public:
    SparseMatrixError(Idx err, std::string const& msg = "") {
        append_msg("Sparse matrix error with error code #" + detail::to_string(err) + " (possibly singular)\n");
        if (!msg.empty()) {
            append_msg(msg + "\n");
        }
        append_msg("If you get this error from state estimation, ");
        append_msg("it usually means the system is not fully observable, i.e. not enough measurements.");
    }
    SparseMatrixError() {
        append_msg("Sparse matrix error, possibly singular matrix!\n" +
                   std::string("If you get this error from state estimation, ") +
                   "it usually means the system is not fully observable, i.e. not enough measurements.");
    }
};

class NotObservableError : public PowerGridError {
  public:
    NotObservableError() { append_msg("Not enough measurements available for state estimation.\n"); }
};

class IterationDiverge : public PowerGridError {
  public:
    IterationDiverge() = default;
    IterationDiverge(Idx num_iter, double max_dev, double err_tol) {
        append_msg("Iteration failed to converge after " + detail::to_string(num_iter) +
                   " iterations! Max deviation: " + detail::to_string(max_dev) +
                   ", error tolerance: " + detail::to_string(err_tol) + ".\n");
    }
};

class MaxIterationReached : public IterationDiverge {
  public:
    MaxIterationReached(std::string const& msg = "") {
        append_msg("Maximum iterations reached, no solution. " + msg + "\n");
    }
};

class ConflictID : public PowerGridError {
  public:
    explicit ConflictID(ID id) { append_msg("Conflicting id detected: " + detail::to_string(id) + '\n'); }
};

class IDNotFound : public PowerGridError {
  public:
    explicit IDNotFound(ID id) { append_msg("The id cannot be found: " + detail::to_string(id) + '\n'); }
};
class Idx2DNotFound : public PowerGridError {
  public:
    explicit Idx2DNotFound(Idx2D id) {
        append_msg("The idx 2d cannot be found: {" + detail::to_string(id.group) + ", " + detail::to_string(id.pos) +
                   "}.\n");
    }
};

class InvalidMeasuredObject : public PowerGridError {
  public:
    InvalidMeasuredObject(std::string const& object, std::string const& sensor) {
        append_msg(sensor + " measurement is not supported for object of type " + object);
    }
};

class InvalidRegulatedObject : public PowerGridError {
  public:
    InvalidRegulatedObject(std::string const& object, std::string const& regulator) {
        append_msg(regulator + " regulator is not supported for object of type " + object);
    }
    InvalidRegulatedObject(ID id, std::string const& regulator) {
        append_msg(regulator + " regulator is not supported for object with ID " + detail::to_string(id));
    }
};

class DuplicativelyRegulatedObject : public PowerGridError {
  public:
    DuplicativelyRegulatedObject() {
        append_msg("There are objects regulated by more than one regulator. Maximum one regulator is allowed.");
    }
};

class AutomaticTapCalculationError : public PowerGridError {
  public:
    AutomaticTapCalculationError(ID id) {
        append_msg("Automatic tap changing regulator with tap_side at LV side is not supported. Found at id " +
                   detail::to_string(id)); // NOSONAR
    }
};

class IDWrongType : public PowerGridError {
  public:
    explicit IDWrongType(ID id) { append_msg("Wrong type for object with id " + detail::to_string(id) + '\n'); }
};

class CalculationError : public PowerGridError {
  public:
    explicit CalculationError(std::string const& msg) { append_msg(msg); }
};

class BatchCalculationError : public CalculationError {
  public:
    BatchCalculationError(std::string const& msg, IdxVector failed_scenarios, std::vector<std::string> err_msgs)
        : CalculationError(msg), failed_scenarios_{std::move(failed_scenarios)}, err_msgs_(std::move(err_msgs)) {}

    IdxVector const& failed_scenarios() const { return failed_scenarios_; }

    std::vector<std::string> const& err_msgs() const { return err_msgs_; }

  private:
    IdxVector failed_scenarios_;
    std::vector<std::string> err_msgs_;
};

class InvalidCalculationMethod : public CalculationError {
  public:
    InvalidCalculationMethod() : CalculationError("The calculation method is invalid for this calculation!") {}
};

class InvalidShortCircuitType : public PowerGridError {
  public:
    explicit InvalidShortCircuitType(FaultType short_circuit_type) {
        append_msg("The short circuit type (" + detail::to_string(static_cast<IntS>(short_circuit_type)) +
                   ") is invalid!\n");
    }
    InvalidShortCircuitType(bool sym, FaultType short_circuit_type) {
        append_msg("The short circuit type (" + detail::to_string(static_cast<IntS>(short_circuit_type)) +
                   ") does not match the calculation type (symmetric=" + detail::to_string(static_cast<int>(sym)) +
                   ")\n");
    }
};

class InvalidShortCircuitPhases : public PowerGridError {
  public:
    InvalidShortCircuitPhases(FaultType short_circuit_type, FaultPhase short_circuit_phases) {
        append_msg("The short circuit phases (" + detail::to_string(static_cast<IntS>(short_circuit_phases)) +
                   ") do not match the short circuit type (" +
                   detail::to_string(static_cast<IntS>(short_circuit_type)) + ")\n");
    }
};

class InvalidShortCircuitPhaseOrType : public PowerGridError {
  public:
    InvalidShortCircuitPhaseOrType() {
        append_msg("During one calculation the short circuit types phases should be similar for all faults\n");
    }
};

class SerializationError : public PowerGridError {
  public:
    explicit SerializationError(std::string const& msg) { append_msg(msg); }
};

class DatasetError : public PowerGridError {
  public:
    explicit DatasetError(std::string const& msg) { append_msg("Dataset error: " + msg); }
};

class ExperimentalFeature : public InvalidArguments {
    using InvalidArguments::InvalidArguments;
};

class NotImplementedError : public PowerGridError {
  public:
    NotImplementedError() { append_msg("Function not yet implemented"); }
};

class UnreachableHit : public PowerGridError {
  public:
    UnreachableHit(std::string const& method, std::string const& reason_for_assumption) {
        append_msg("Unreachable code hit when executing " + method +
                   ".\n The following assumption for unreachability was not met: " + reason_for_assumption +
                   ".\n This may be a bug in the library\n");
    }
};

class TapSearchStrategyIncompatibleError : public InvalidArguments {
  public:
    template <typename T1, typename T2>
    TapSearchStrategyIncompatibleError(std::string const& method, const T1& value1, const T2& value2)
        : InvalidArguments{
              method, std::string{typeid(T1).name()} + " #" + detail::to_string(static_cast<IntS>(value1)) + " and " +
                          std::string{typeid(T2).name()} + " #" + detail::to_string(static_cast<IntS>(value2))} {}
};

} // namespace power_grid_model
