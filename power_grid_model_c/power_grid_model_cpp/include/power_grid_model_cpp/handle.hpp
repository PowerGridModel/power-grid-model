// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_HANDLE_HPP
#define POWER_GRID_MODEL_CPP_HANDLE_HPP

#include "handle.h"

#include "basics.hpp"

namespace power_grid_model_cpp {
class Handle {

  public:
    Handle() : handle_{PGM_create_handle()} {}

    ~Handle() = default;

    Idx error_code() const { return PGM_error_code(handle_.get()); }

    char const* error_message() const { return PGM_error_message(handle_.get()); }

    Idx n_failed_scenarios() const { return PGM_n_failed_scenarios(handle_.get()); }

    Idx const* failed_scenarios() const { return PGM_failed_scenarios(handle_.get()); }

    char const** batch_errors() const { return PGM_batch_errors(handle_.get()); }

    void clear_error() const { PGM_clear_error(handle_.get()); }

    PGM_Handle* get() const { return handle_.get(); }

  private:
    UniquePtr<PGM_Handle, PGM_destroy_handle> handle_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_HANDLE_HPP