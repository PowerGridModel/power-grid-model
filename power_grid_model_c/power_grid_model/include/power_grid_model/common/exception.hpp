// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "enum.hpp"

#include <exception>
#include <format>
#include <string>

namespace power_grid_model {

class PowerGridError : public std::exception {
  public:
    void append_msg(std::string_view msg) { msg_ += msg; }
    char const* what() const noexcept final { return msg_.c_str(); }

  private:
    std::string msg_;
};

template <typename T> class MissingCaseForEnumError : public PowerGridError {
  public:
    MissingCaseForEnumError(std::string const& method, const T& value) {
        append_msg(std::format("{} is not implemented for {} #{}", method, typeid(T).name(), IntS(value)));
    }
};

class ConflictVoltage : public PowerGridError {
  public:
    ConflictVoltage(ID id, ID id1, ID id2, double u1, double u2) {
        append_msg(std::format(
            "Conflicting voltage for line {}\n voltage at from node {} is {}\n voltage at to node {} is {}\n", id, id1,
            u1, id2, u2));
    }
};

class InvalidBranch : public PowerGridError {
  public:
    InvalidBranch(ID branch_id, ID node_id) {
        append_msg(
            std::format("Branch {} has the same from- and to-node {},\n This is not allowed!\n", branch_id, node_id));
    }
};

class InvalidBranch3 : public PowerGridError {
  public:
    InvalidBranch3(ID branch3_id, ID node_1_id, ID node_2_id, ID node_3_id) {
        append_msg(std::format(
            "Branch3 {} is connected to the same node at least twice. Node 1/2/3: {}/{}/{},\n This is not allowed!\n",
            branch3_id, node_1_id, node_2_id, node_3_id));
    }
};

class InvalidTransformerClock : public PowerGridError {
  public:
    InvalidTransformerClock(ID id, IntS clock) {
        append_msg(std::format("Invalid clock for transformer {}, clock {}\n", id, clock));
    }
};

class SparseMatrixError : public PowerGridError {
  public:
    SparseMatrixError(Idx err, std::string const& msg = "") {
        append_msg(std::format("Sparse matrix error with error code #{} (possibly singular)\n", err));
        if (!msg.empty()) {
            append_msg(std::format("{}\n", msg));
        }
        append_msg("If you get this error from state estimation, ");
        append_msg("it usually means the system is not fully observable, i.e. not enough measurements.");
    }
    SparseMatrixError() {
        append_msg("Sparse matrix error, possibly singular matrix!\n");
        append_msg("If you get this error from state estimation, it ");
        append_msg("usually means the system is not fully observable, i.e., not enough measurements.");
    }
};

class NotObservableError : public PowerGridError {
  public:
    NotObservableError() { append_msg("Not enough measurements available for state estimation.\n"); }
};

class IterationDiverge : public PowerGridError {
  public:
    IterationDiverge(Idx num_iter, double max_dev, double err_tol) {
        append_msg(
            std::format("Iteration failed to converge after {} iterations! Max deviation: {}, error tolerance: {}\n",
                        num_iter, max_dev, err_tol));
    }
};

class ConflictID : public PowerGridError {
  public:
    explicit ConflictID(ID id) { append_msg(std::format("Conflicting id detected: {}\n", id)); }
};

class IDNotFound : public PowerGridError {
  public:
    explicit IDNotFound(ID id) { append_msg(std::format("The id cannot be found: {}\n", id)); }
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
        append_msg(std::format("{} regulator is not supported for object of type {}", regulator, object));
    }
    InvalidRegulatedObject(ID id, std::string const& regulator) {
        append_msg(std::format("{} regulator is not supported for object with ID {}", regulator, id));
    }
};

class AutomaticTapCalculationError : public PowerGridError {
  public:
    AutomaticTapCalculationError(ID id) {
        append_msg(std::format("Automatic tap changing regulator is at LV side is not supported. Found at id{}", id));
    }
};

class IDWrongType : public PowerGridError {
  public:
    explicit IDWrongType(ID id) { append_msg(std::format("Wrong type for object with id {}\n", id)); }
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
        append_msg(std::format("The short circuit type ({}) is invalid!\n", static_cast<IntS>(short_circuit_type)));
    }
    InvalidShortCircuitType(bool sym, FaultType short_circuit_type) {
        append_msg(std::format("The short circuit type ({}) does not match the calculation type (symmetric={})\n",
                               static_cast<IntS>(short_circuit_type), static_cast<int>(sym)));
    }
};

class InvalidShortCircuitPhases : public PowerGridError {
  public:
    InvalidShortCircuitPhases(FaultType short_circuit_type, FaultPhase short_circuit_phases) {
        append_msg(std::format("The short circuit phases ({}) do not match the short circuit type ({})\n",
                               static_cast<IntS>(short_circuit_phases), static_cast<IntS>(short_circuit_type)));
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
    explicit DatasetError(std::string const& msg) { append_msg(std::format("Dataset error: {}", msg)); }
};

class UnreachableHit : public PowerGridError {
  public:
    UnreachableHit(std::string const& method, std::string const& reason_for_assumption) {
        append_msg(std::format("Unreachable code hit when executing {}.\n The following assumption for unreachability "
                               "was not met: {}.\n This may be a bug in the library\n",
                               method, reason_for_assumption));
    }
};

} // namespace power_grid_model
