// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_HANDLE_HPP
#define POWER_GRID_MODEL_CPP_HANDLE_HPP

#include "basics.hpp"

#include "power_grid_model_c/handle.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <vector>

namespace power_grid_model_cpp {
class PowerGridError : public std::exception {
  public:
    PowerGridError(std::string message) : message_(std::move(message)) {}
    const char* what() const noexcept override { return message_.c_str(); }
    virtual Idx error_code() const noexcept { return PGM_regular_error; };

  private:
    std::string message_;
};

class PowerGridRegularError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_regular_error; }
};

class PowerGridSerializationError : public PowerGridError {
  public:
    using PowerGridError::PowerGridError;
    Idx error_code() const noexcept override { return PGM_serialization_error; }
};

class PowerGridBatchError : public PowerGridError {
  public:
    struct FailedScenario {
        Idx scenario{};
        std::string error_message;
    };

    PowerGridBatchError(std::string message, std::vector<FailedScenario> failed_scenarios_c)
        : PowerGridError{std::move(message)}, failed_scenarios_{std::move(failed_scenarios_c)} {}
    Idx error_code() const noexcept override { return PGM_batch_error; }
    std::vector<FailedScenario> const& failed_scenarios() const { return failed_scenarios_; }

  private:
    std::vector<FailedScenario> failed_scenarios_;
};

class Handle {
  public:
    RawHandle* get() const { return handle_.get(); }

    void clear_error() const { PGM_clear_error(get()); }

    void check_error() const {
        RawHandle const* handle_ptr = get();
        Idx const error_code = PGM_error_code(handle_ptr);
        std::string const error_message = error_code == PGM_no_error ? "" : PGM_error_message(handle_ptr);
        switch (error_code) {
        case PGM_no_error:
            return;
        case PGM_regular_error:
            clear_error();
            throw PowerGridRegularError{error_message};
        case PGM_batch_error: {
            Idx const n_failed_scenarios = PGM_n_failed_scenarios(handle_ptr);
            auto const failed_scenario_seqs =
                std::span{PGM_failed_scenarios(handle_ptr), static_cast<size_t>(n_failed_scenarios)};
            auto const failed_scenario_messages =
                std::span{PGM_batch_errors(handle_ptr), static_cast<size_t>(n_failed_scenarios)};
            auto failed_scenarios = std::views::zip(failed_scenario_seqs, failed_scenario_messages) |
                    std::views::transform([](auto const& zipped) {
                        return PowerGridBatchError::FailedScenario{// NOLINT(modernize-use-designated-initializers)
                                                                std::get<0>(zipped), std::get<1>(zipped)};
                    }) |
                    std::ranges::to<std::vector>();
            clear_error();
            throw PowerGridBatchError{error_message, std::move(failed_scenarios)};
        }
        case PGM_serialization_error:
            clear_error();
            throw PowerGridSerializationError{error_message};
        default:
            clear_error();
            throw PowerGridError{error_message};
        }
    }

    template <typename Func, typename... Args> auto call_with(Func&& func, Args&&... args) const {
        if constexpr (std::is_void_v<decltype(std::forward<Func>(func)(get(), std::forward<Args>(args)...))>) {
            std::forward<Func>(func)(get(), std::forward<Args>(args)...);
            check_error();
        } else {
            auto result = std::forward<Func>(func)(get(), std::forward<Args>(args)...);
            check_error();
            return result;
        }
    }

  private:
    // For handle the const semantics are not needed.
    // It is meant to be mutated even in const situations.
    std::unique_ptr<RawHandle, detail::DeleterFunctor<&PGM_destroy_handle>> handle_{PGM_create_handle()};
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_HANDLE_HPP
