// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_LOGGER_HPP
#define POWER_GRID_MODEL_CPP_LOGGER_HPP

#include "basics.hpp"
#include "handle.hpp"

#include "power_grid_model_c/logger.h"

#include <string>

namespace power_grid_model_cpp {
// Opt-in diagnostic logger. Create a Logger, attach it to one or more Model instances via
// Model::add_logger(), then read output with get_output().
//
// Lifetime: the underlying logging implementation is shared with every Model/Handle this
// logger is registered to (see Model::add_logger()). Destroying this wrapper while still
// registered is safe: the implementation stays alive and keeps collecting output for those
// registrations, but this specific Logger object can no longer be used to target that
// registration individually. Use Model::remove_all_loggers() or destroy the Model to release it.
//
// Concurrency: do not register, unregister, destroy, read, or clear a logger while a
// calculation using it is in progress on any thread other than the calculation's own
// internal batch threads (which are always safe).
class Logger {
  public:
    explicit Logger(PGM_LoggerType logger_type)
        : logger_{handle_.call_with(PGM_create_logger, static_cast<Idx>(logger_type))} {}

    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;
    Logger(Logger&&) noexcept = default;
    Logger& operator=(Logger&&) noexcept = default;
    ~Logger() = default;

    PGM_Logger* get() { return logger_.get(); }
    PGM_Logger const* get() const { return logger_.get(); }

    std::string get_output() {
        std::string output;
        PGM_LogOutputCallback const cb = [](char const* data, PGM_Idx size, void* user_data) {
            *static_cast<std::string*>(user_data) = std::string(data, static_cast<std::size_t>(size));
        };
        handle_.call_with(PGM_logger_get_output, get(), cb, static_cast<void*>(&output));
        return output;
    }

    void clear() { handle_.call_with(PGM_logger_clear, get()); }

  private:
    Handle handle_{}; // used only for exception propagation from calls on this logger
    detail::UniquePtr<PGM_Logger, &PGM_destroy_logger> logger_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_LOGGER_HPP
