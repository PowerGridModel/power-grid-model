// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "enum.hpp"

#include <exception>
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
    MissingCaseForEnumError(const std::string& method, const T& value) {
        append_msg(method + " is not implemented for " + typeid(T).name() + " #" + std::to_string(IntS(value)) + "!\n");
    }
};

class ConflictVoltage : public PowerGridError {
  public:
    ConflictVoltage(ID id, ID id1, ID id2, double u1, double u2) {
        append_msg("Conflicting voltage for line " + std::to_string(id) + "\n voltage at from node " +
                   std::to_string(id1) + " is " + std::to_string(u1) + "\n voltage at to node " + std::to_string(id2) +
                   " is " + std::to_string(u2) + '\n');
    }
};

class InvalidBranch : public PowerGridError {
  public:
    InvalidBranch(ID branch_id, ID node_id) {
        append_msg("Branch " + std::to_string(branch_id) + " has the same from- and to-node " +
                   std::to_string(node_id) + ",\n This is not allowed!\n");
    }
};

class InvalidBranch3 : public PowerGridError {
  public:
    InvalidBranch3(ID branch3_id, ID node_1_id, ID node_2_id, ID node_3_id) {
        append_msg("Branch3 " + std::to_string(branch3_id) +
                   " is connected to the same node at least twice. Node 1/2/3: " + std::to_string(node_1_id) + "/" +
                   std::to_string(node_2_id) + "/" + std::to_string(node_3_id) + ",\n This is not allowed!\n");
    }
};

class InvalidTransformerClock : public PowerGridError {
  public:
    InvalidTransformerClock(ID id, IntS clock) {
        append_msg("Invalid clock for transformer " + std::to_string(id) + ", clock  " + std::to_string(clock) + '\n');
    }
};

class SparseMatrixError : public PowerGridError {
  public:
    SparseMatrixError(Idx err, std::string const& msg = "") {
        append_msg("Sparse matrix error with error code #" + std::to_string(err) + " (possibly singular)\n");
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

class IterationDiverge : public PowerGridError {
  public:
    IterationDiverge(Idx num_iter, double max_dev, double err_tol) {
        append_msg("Iteration failed to converge after " + std::to_string(num_iter) + " iterations! Max deviation: " +
                   std::to_string(max_dev) + ", error tolerance: " + std::to_string(err_tol) + ".\n");
    }
};

class ConflictID : public PowerGridError {
  public:
    explicit ConflictID(ID id) { append_msg("Conflicting id detected: " + std::to_string(id) + '\n'); }
};

class IDNotFound : public PowerGridError {
  public:
    explicit IDNotFound(ID id) { append_msg("The id cannot be found: " + std::to_string(id) + '\n'); }
};

class InvalidMeasuredObject : public PowerGridError {
  public:
    InvalidMeasuredObject(const std::string& object, const std::string& sensor) {
        append_msg(sensor + " is not supported for " + object);
    }
};

class IDWrongType : public PowerGridError {
  public:
    explicit IDWrongType(ID id) { append_msg("Wrong type for object with id " + std::to_string(id) + '\n'); }
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

class UnknownAttributeName : public PowerGridError {
  public:
    explicit UnknownAttributeName(std::string const& attr_name) {
        append_msg("Unknown attribute name!" + attr_name + "\n");
    }
};

class InvalidShortCircuitType : public PowerGridError {
  public:
    explicit InvalidShortCircuitType(FaultType short_circuit_type) {
        append_msg("The short circuit type (" + std::to_string(static_cast<IntS>(short_circuit_type)) +
                   ") is invalid!\n");
    }
    InvalidShortCircuitType(bool sym, FaultType short_circuit_type) {
        append_msg("The short circuit type (" + std::to_string(static_cast<IntS>(short_circuit_type)) +
                   ") does not match the calculation type (symmetric=" + std::to_string(static_cast<int>(sym)) + ")\n");
    }
};

class InvalidShortCircuitPhases : public PowerGridError {
  public:
    InvalidShortCircuitPhases(FaultType short_circuit_type, FaultPhase short_circuit_phases) {
        append_msg("The short circuit phases (" + std::to_string(static_cast<IntS>(short_circuit_phases)) +
                   ") do not match the short circuit type (" + std::to_string(static_cast<IntS>(short_circuit_type)) +
                   ")\n");
    }
};

class InvalidShortCircuitPhaseOrType : public PowerGridError {
  public:
    InvalidShortCircuitPhaseOrType() {
        append_msg("During one calculation the short circuit types phases should be similar for all faults \n");
    }
};

class SerializationError : public PowerGridError {
  public:
    explicit SerializationError(std::string const& msg) { append_msg(msg); }
};

class DatasetError : public PowerGridError {
  public:
    explicit DatasetError(std::string const& msg) { append_msg(msg); }
};

} // namespace power_grid_model
