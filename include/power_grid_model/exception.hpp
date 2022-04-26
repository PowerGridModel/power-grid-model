// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_EXCEPTION_HPP
#define POWER_GRID_MODEL_EXCEPTION_HPP

#include <exception>
#include <string>

#include "power_grid_model.hpp"

namespace power_grid_model {

class PowerGridError : public std::exception {
   public:
    void append_msg(std::string const &msg) {
        msg_ += msg;
    }
    char const *what() const noexcept final {
        return msg_.c_str();
    }

   private:
    std::string msg_;
};

template <typename T>
class MissingCaseForEnumError : public PowerGridError {
   public:
    MissingCaseForEnumError(const std::string &method, const T &value) {
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

class InvalidTransformerClock : public PowerGridError {
   public:
    InvalidTransformerClock(ID id, IntS clock) {
        append_msg("Invalid clock for transformer " + std::to_string(id) + ", clock  " + std::to_string(clock) + '\n');
    }
};

class SparseMatrixError : public PowerGridError {
   public:
    SparseMatrixError(Idx err, std::string const &msg = "") {
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
    ConflictID(ID id) {
        append_msg("Conflicting id detected: " + std::to_string(id) + '\n');
    }
};

class IDNotFound : public PowerGridError {
   public:
    IDNotFound(ID id) {
        append_msg("The id cannot be found: " + std::to_string(id) + '\n');
    }
};

class InvalidMeasuredObject : public PowerGridError {
   public:
    InvalidMeasuredObject(const std::string &object, const std::string &sensor) {
        append_msg(sensor + " is not supported for " + object);
    }
};

class IDWrongType : public PowerGridError {
   public:
    IDWrongType(ID id) {
        append_msg("Wrong type for object with id " + std::to_string(id) + '\n');
    }
};

class CalculationError : public PowerGridError {
   public:
    CalculationError(std::string const &msg) {
        append_msg(msg);
    }
};

class BatchCalculationError : public CalculationError {
   public:
    BatchCalculationError(std::string const &msg) : CalculationError(msg) {
    }
};

class InvalidCalculationMethod : public CalculationError {
   public:
    InvalidCalculationMethod() : CalculationError("The calculation method is invalid for this calculation!") {
    }
};

class UnknownAttributeName : public PowerGridError {
   public:
    UnknownAttributeName(std::string const &attr_name) {
        append_msg("Unknown attribute name!" + attr_name + "\n");
    }
};

}  // namespace power_grid_model

#endif
