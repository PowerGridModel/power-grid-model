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
class Logger {
  public:
    explicit Logger(Idx logger_type) : logger_{handle_.call_with(PGM_create_logger, logger_type)} {}

    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;
    Logger(Logger&&) noexcept = default;
    Logger& operator=(Logger&&) noexcept = default;
    ~Logger() = default;

    PGM_Logger* get() { return logger_.get(); }
    PGM_Logger const* get() const { return logger_.get(); }

    void register_logger(Handle& handle) { handle.call_with(PGM_register_logger, get()); }

    void unregister_logger(Handle& handle) { handle.call_with(PGM_unregister_logger, get()); }

    std::string get_output(Handle& handle) {
        std::string output;
        PGM_LogOutputCallback const cb = [](char const* data, PGM_Idx size, void* user_data) {
            *static_cast<std::string*>(user_data) = std::string(data, static_cast<std::size_t>(size));
        };
        handle.call_with(PGM_logger_get_output, get(), cb, static_cast<void*>(&output));
        return output;
    }

    void clear(Handle& handle) { handle.call_with(PGM_logger_clear, get()); }

    static void unregister_all(Handle& handle) { handle.call_with(PGM_unregister_all_loggers); }

  private:
    Handle handle_{};
    detail::UniquePtr<PGM_Logger, &PGM_destroy_logger> logger_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_LOGGER_HPP
