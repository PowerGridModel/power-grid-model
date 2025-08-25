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

class LogDispatcher final : public LogDispatch {
  public:
    void log(LogEvent tag) override { log_impl(tag); }
    void log(LogEvent tag, std::string_view message) override { log_impl(tag, message); }
    void log(LogEvent tag, double value) override { log_impl(tag, value); }
    void log(LogEvent tag, Idx value) override { log_impl(tag, value); }
    void registrar(Logger* logger) override {
        if (logger != nullptr && std::ranges::find(loggers_, logger) == loggers_.end()) {
            loggers_.push_back(logger);
        }
    }
    void deregistrar(Logger* logger) override {
        if (logger != nullptr) {
            std::erase(loggers_, logger);
        }
    }
    std::unique_ptr<Logger> clone() const override { return std::make_unique<LogDispatcher>(); }

    LogDispatcher(const LogDispatcher&) = delete;
    LogDispatcher& operator=(const LogDispatcher&) = delete;
    LogDispatcher(LogDispatcher&&) = default;
    LogDispatcher& operator=(LogDispatcher&&) = default;
    ~LogDispatcher() override = default;

  private:
    std::vector<Logger*> loggers_;

    template <typename... T>
    constexpr void log_impl(LogEvent tag, T&&... values)
        requires requires(Logger log_) {
            { log_.log(tag, values...) };
        }
    {
        if (loggers_.size() == 1 && loggers_.front() != nullptr) { // allows perfect forwarding to log messages
            loggers_.front()->log(tag, std::forward<T>(values)...);
        } else {
            for (auto& logger : loggers_) {
                if (logger != nullptr) {
                    logger->log(tag, values...);
                }
            }
            capturing::into_the_void(std::forward<T>(values)...);
        }
    }
};

} // namespace power_grid_model::common::logging
