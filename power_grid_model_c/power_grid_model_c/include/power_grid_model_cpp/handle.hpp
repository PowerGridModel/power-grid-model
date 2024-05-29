// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef POWER_GRID_MODEL_CPP_HANDLE_HPP
#define POWER_GRID_MODEL_CPP_HANDLE_HPP

#include "../power_grid_model_c/handle.h"
#include "basics.hpp"

#include <exception>
#include <string>

namespace power_grid_model {

class PowerGridError : public std::exception {
  public:
    PowerGridError(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

  private:
    std::string message_;
};

class PowerGridRegularError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
};

class PowerGridBatchError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
};

class PowerGridSerializationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
};

class Handle {
  public:
    Handle() : handle_{PGM_create_handle()} {}

    void check_error() const {
        Idx error_code = PGM_error_code(handle_.get());
        std::string error_message = error_code == PGM_no_error ? "" : PGM_error_message(handle_.get());
        switch (error_code) {
        case 0:
            break;
        case PGM_regular_error:
            throw PowerGridRegularError{error_message};
        case PGM_batch_error:
            throw PowerGridBatchError{error_message};
        case PGM_serialization_error:
            throw PowerGridSerializationError{error_message};
        }
    }

    PGM_Handle* get_ptr() const { return handle_.get(); }

  private:
    UniquePtr<PGM_Handle, &PGM_destroy_handle> handle_;
};

} // namespace power_grid_model

#endif // POWER_GRID_MODEL_CPP_HANDLE_HPP