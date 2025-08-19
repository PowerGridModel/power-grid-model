// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging.hpp"

#include <string_view>
#include <vector>

namespace power_grid_model::common::logging {
class DefaultLogger : public Logger {
    void log(LogEvent /*tag*/, std::string_view /*message*/) override {}
    void log(LogEvent /*tag*/, double /*value*/) override {}
    void log(LogEvent /*tag*/, Idx /*value*/) override {}
};

class LogDispatcher : public LogDispatch {
  public:
    void log(LogEvent tag, std::string_view message) override { log_impl(tag, message); }
    void log(LogEvent tag, double value) override { log_impl(tag, value); }
    void log(LogEvent tag, Idx value) override { log_impl(tag, value); }
    void registrar(Logger* logger) override {
        if (logger) {
            loggers_.push_back(logger);
        }
    }
    void deregistrar(Logger* logger) override {
        if (logger) {
            std::erase(loggers_, logger);
        }
    }

    ~LogDispatcher() override = default;

  private:
    std::vector<Logger*> loggers_;

    template <typename T> void log_impl(LogEvent tag, T value) {
        for (auto& logger : loggers_) {
            if (logger) {
                logger->log(tag, value);
            }
        }
    }
};

} // namespace power_grid_model::common::logging
