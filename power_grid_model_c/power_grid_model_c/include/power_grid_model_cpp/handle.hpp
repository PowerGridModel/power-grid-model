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

class PowerGridRegularError : public std::exception {};

class PowerGridBatchError : public std::exception {};

class PowerGridSerializationError : public std::exception {};

class Handle {
  public:
    Handle() : handle_(PGM_create_handle()) {}

    void check_error() const {
        Idx error_code = PGM_error_code(handle_.get());
        switch (error_code) {
        case 0:
            break;
        case PGM_regular_error:
            throw PowerGridRegularError();
        case PGM_batch_error:
            throw PowerGridBatchError();
        case PGM_serialization_error:
            throw PowerGridSerializationError();
        }
        if (PGM_error_code(handle_.get()) != 0) {
            throw PowerGridRegularError();
        }
    }

    PGM_Handle* get_ptr() const { return handle_.get(); }

  private:
    UniquePtr<PGM_Handle, &PGM_destroy_handle> handle_;
};

} // namespace power_grid_model

#endif // POWER_GRID_MODEL_CPP_HANDLE_HPP