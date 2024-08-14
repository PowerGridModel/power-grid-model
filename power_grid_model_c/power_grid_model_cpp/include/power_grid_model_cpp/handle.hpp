// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_HANDLE_HPP
#define POWER_GRID_MODEL_CPP_HANDLE_HPP

#include "power_grid_model_c/handle.h"

#include "basics.hpp"

namespace power_grid_model_cpp {
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
    static constexpr Idx error_code() { return PGM_regular_error; }
};

class PowerGridSerializationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    static constexpr Idx error_code() { return PGM_serialization_error; }
};

class PowerGridBatchError : public PowerGridError {
  public:
    struct FailedScenario {
        Idx scenario;
        std::string error_message;
    };

    PowerGridBatchError(std::string const& message, std::vector<FailedScenario> failed_scenarios_c)
        : PowerGridError{message}, failed_scenarios_{std::move(failed_scenarios_c)} {}
    static constexpr Idx error_code() { return PGM_batch_error; }
    std::vector<FailedScenario> const& failed_scenarios() const { return failed_scenarios_; }

  private:
    std::vector<FailedScenario> failed_scenarios_;
};

class Handle {
  public:
    Handle() : handle_{PGM_create_handle()} {}

    PGM_Handle* get() const { return handle_.get(); }

    void check_error() const {
        Idx error_code = PGM_error_code(handle_.get());
        std::string error_message = error_code == PGM_no_error ? "" : PGM_error_message(handle_.get());
        switch (error_code) {
        case 0:
            return;
        case PGM_regular_error:
            throw PowerGridRegularError{error_message};
        case PGM_batch_error: {
            Idx const n_failed_scenarios = PGM_n_failed_scenarios(handle_.get());
            std::vector<PowerGridBatchError::FailedScenario> failed_scenarios(n_failed_scenarios);
            auto const failed_scenario_seqs = PGM_failed_scenarios(handle_.get());
            auto const failed_scenario_messages = PGM_batch_errors(handle_.get());
            for (Idx i = 0; i < n_failed_scenarios; ++i) {
                failed_scenarios[i] =
                    PowerGridBatchError::FailedScenario{failed_scenario_seqs[i], failed_scenario_messages[i]};
            }
            throw PowerGridBatchError{error_message, std::move(failed_scenarios)};
        }
        case PGM_serialization_error:
            throw PowerGridSerializationError{error_message};
        }
    }

  private:
    UniquePtr<PGM_Handle, PGM_destroy_handle> handle_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_HANDLE_HPP
