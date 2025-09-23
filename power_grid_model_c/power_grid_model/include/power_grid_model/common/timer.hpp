// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "logging.hpp"

#include <chrono>
#include <sstream>

namespace power_grid_model {

using Clock = std::chrono::high_resolution_clock;
using Duration = std::chrono::duration<double>;

class Timer {
  private:
    Logger* log_;
    LogEvent code_;
    Clock::time_point start_;

  public:
    Timer() : log_{nullptr}, code_{LogEvent::unknown} {};
    Timer(Logger& log, LogEvent code) : log_{&log}, code_{code}, start_{Clock::now()} {}

    Timer(Timer const&) = delete;
    Timer(Timer&& other) noexcept
        : log_{std::exchange(other.log_, nullptr)}, code_{other.code_}, start_{other.start_} {}
    Timer& operator=(Timer const&) = delete;

    Timer& operator=(Timer&& timer) noexcept {
        // Stop the current timer
        stop();

        // Copy/move members
        log_ = timer.log_;
        code_ = timer.code_;
        start_ = timer.start_;

        // Disable original timer
        timer.log_ = nullptr;

        // Return reference
        return *this;
    }

    ~Timer() {
        if (log_ != nullptr) {
            stop();
        }
    }

    void stop() {
        if (log_ != nullptr) {
            auto const now = Clock::now();
            auto const duration = Duration(now - start_);
            log_->log(code_, duration.count());
            log_ = nullptr;
        }
    }
};

} // namespace power_grid_model
