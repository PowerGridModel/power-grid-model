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
    Logger* info_;
    LogEvent code_;
    Clock::time_point start_;

  public:
    Timer() : info_{nullptr}, code_{LogEvent::unknown} {};
    Timer(Logger& info, LogEvent code) : info_{&info}, code_{code}, start_{Clock::now()} {}

    Timer(Timer const&) = delete;
    Timer(Timer&&) = default;
    Timer& operator=(Timer const&) = delete;

    Timer& operator=(Timer&& timer) noexcept {
        // Stop the current timer
        stop();

        // Copy/move members
        info_ = timer.info_;
        code_ = timer.code_;
        start_ = timer.start_;

        // Disable original timer
        timer.info_ = nullptr;

        // Return reference
        return *this;
    }

    ~Timer() {
        if (info_ != nullptr) {
            stop();
        }
    }

    void stop() {
        if (info_ != nullptr) {
            auto const now = Clock::now();
            auto const duration = Duration(now - start_);
            info_->log(code_, duration.count());
            info_ = nullptr;
        }
    }
};

} // namespace power_grid_model
