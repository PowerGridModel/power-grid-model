// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging.hpp"

#include <string_view>
#include <vector>

namespace power_grid_model::common::logging {
class NoLogger : public Logger {
    void log(LogEvent /*tag*/) override {}
    void log(LogEvent /*tag*/, std::string_view /*message*/) override {}
    void log(LogEvent /*tag*/, double /*value*/) override {}
    void log(LogEvent /*tag*/, Idx /*value*/) override {}
    std::unique_ptr<Logger> clone() const override { return std::make_unique<NoLogger>(); }
};

} // namespace power_grid_model::common::logging
