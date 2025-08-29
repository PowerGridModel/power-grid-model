// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging.hpp"

#include <string_view>
#include <vector>

namespace power_grid_model::common::logging {
class NoLogger : public Logger {
  public:
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
};

} // namespace power_grid_model::common::logging
