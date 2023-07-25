// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_TIMER_HPP
#define POWER_GRID_MODEL_TIMER_HPP

#include "power_grid_model.hpp"

#include <iomanip>
#include <sstream>

namespace power_grid_model {

class Timer {
   private:
    CalculationInfo *info_;
    int code_;
    std::string name_;
    Clock::time_point start_;

   public:
    Timer() : info_(nullptr){};

    Timer(CalculationInfo &info, int code, std::string name)
        : info_(&info), code_(code), name_(std::move(name)), start_(Clock::now()) {
    }

    Timer(const Timer &) = delete;
    Timer(Timer &&) = default;
    Timer &operator=(const Timer &) = delete;

    Timer &operator=(Timer &&timer) noexcept {
        // Stop the current timer
        stop();

        // Copy/move members
        info_ = timer.info_;
        code_ = timer.code_;
        name_ = std::move(timer.name_);
        start_ = timer.start_;

        // Disable original timer
        timer.info_ = nullptr;

        // Return reference
        return *this;
    }

    ~Timer() {
        if (info_) {
            stop();
        }
    }

    void stop() {
        if (info_) {
            auto const now = Clock::now();
            auto const duration = Duration(now - start_);
            info_->operator[](Timer::make_key(code_, name_)) += (double)duration.count();
            info_ = nullptr;
        }
    }

    static std::string make_key(int code, const std::string &name) {
        std::stringstream ss;
        ss << std::setw(4) << std::setfill('0') << code << ".";
        auto key = ss.str();
        for (size_t i = 0, n = key.length() - 1; i < n; ++i) {
            if (key[i] == '0') {
                break;
            }
            key += "\t";
        }
        key += name;
        return key;
    }
};

}  // namespace power_grid_model

#endif  // POWER_GRID_MODEL_TIMER_HPP
