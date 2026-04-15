// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "logging.hpp"

#include <concepts>
#include <string_view>

namespace power_grid_model::common::logging {
class NoLogger : public Logger {
  public:
    using Logger::log;

    void log(LogEvent /*tag*/) override {
        // no logging
    }
    void log(LogEvent /*tag*/, std::string_view /*message*/) override {
        // no logging
    }
    void log(LogEvent /*tag*/, double /*value*/) override {
        // no logging
    }
    void log(LogEvent /*tag*/, Idx /*value*/) override {
        // no logging
    }
    void log(std::string_view /*message*/) const {
        // no logging
    }
    template <LazyLoggingFn Fn>
    void log(LogEvent /*tag*/, Fn&& /*fn*/) const { // NOLINT(cppcoreguidelines-missing-std-forward) // intentional
        // no logging
    }
    template <LazyLoggingFn Fn>
    void log(Fn&& /*fn*/) const { // NOLINT(cppcoreguidelines-missing-std-forward) // intentional
        // no logging
    }

    template <std::derived_from<Logger> T> T& merge_into(T& destination) const { return destination; }
};

} // namespace power_grid_model::common::logging
