// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging.hpp"

#include <string_view>
#include <vector>

namespace power_grid_model::common::logging {
class NoLogger : public Logger {
    void log(LogEvent /*tag*/, std::string_view /*message*/) override {}
    void log(LogEvent /*tag*/, double /*value*/) override {}
    void log(LogEvent /*tag*/, Idx /*value*/) override {}
    std::unique_ptr<Logger> clone() const override { return std::make_unique<NoLogger>(); }
};

class LogDispatcher : public LogDispatch {
  public:
    void log(LogEvent tag, std::string_view message) override { log_impl(tag, message); }
    void log(LogEvent tag, double value) override { log_impl(tag, value); }
    void log(LogEvent tag, Idx value) override { log_impl(tag, value); }
    void registrar(Logger* logger) override {
        if (logger && std::ranges::find(loggers_, logger) == loggers_.end()) {
            loggers_.push_back(logger);
        }
    }
    void deregistrar(Logger* logger) override {
        if (logger) {
            std::erase(loggers_, logger);
        }
    }
    std::unique_ptr<Logger> clone() const override { return std::make_unique<LogDispatcher>(); }

    ~LogDispatcher() override = default;

  private:
    std::vector<Logger*> loggers_;

    template <typename T>
        requires std::same_as<T, std::string_view> || std::same_as<T, double> || std::same_as<T, Idx>
    void log_impl(LogEvent tag, T value) {
        for (auto& logger : loggers_) {
            if (logger) {
                logger->log(tag, value);
            }
        }
    }
};

} // namespace power_grid_model::common::logging
