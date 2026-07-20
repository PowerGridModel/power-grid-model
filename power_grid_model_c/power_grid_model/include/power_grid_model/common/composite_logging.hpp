// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging.hpp"

#include <algorithm>
#include <memory>
#include <ranges>
#include <string_view>
#include <vector>

namespace power_grid_model::common::logging {

// Owns a list of child loggers (created by MultiThreadedCompositeLogger::create_child) and fans all log calls out to
// each of them. The children are owned by this logger; their lifetimes are tied to this object.
class CompositeChildLogger : public Logger {
  public:
    explicit CompositeChildLogger(std::vector<std::unique_ptr<Logger>> children) : children_{std::move(children)} {}

    void log(LogEvent tag) override                       { log_all(tag); }
    void log(LogEvent tag, std::string_view message) override { log_all(tag, message); }
    void log(LogEvent tag, double value) override             { log_all(tag, value); }
    void log(LogEvent tag, Idx value) override                { log_all(tag, value); }

    using Logger::log;

  private:
    std::vector<std::unique_ptr<Logger>> children_;

    template <typename... Args> void log_all(Args&&... args) {
        for (auto& child : children_) {
            child->log(std::forward<Args>(args)...);
        }
    }
};

// Non-owning fan-out MultiThreadedLogger. Holds non-owning pointers to MultiThreadedLogger instances and forwards
// all log calls to each. create_child() creates a CompositeChildLogger that owns one child per registered logger.
//
// Lifetime contract: all loggers in the list must outlive this composite.
// Dedupe: registering the same logger twice is a no-op (idempotent, consistent with logging conventions).
// UB: modifying the logger list while a calculation is in progress.
class MultiThreadedCompositeLogger : public MultiThreadedLogger {
  public:
    MultiThreadedCompositeLogger() = default;
    explicit MultiThreadedCompositeLogger(std::vector<MultiThreadedLogger*> loggers) : loggers_{std::move(loggers)} {}

    // Add/remove a logger. The object address is unchanged so any existing reference_wrapper
    // pointing to this composite remains valid. Do not call while a calculation is in progress.
    void add(MultiThreadedLogger* logger) {
        if (std::ranges::contains(loggers_, logger)) {
            return; // already registered — dedupe silently, consistent with logging API conventions
        }
        loggers_.push_back(logger);
    }
    void remove(MultiThreadedLogger* logger) {
        if (auto it = std::ranges::find(loggers_, logger); it != loggers_.end()) {
            loggers_.erase(it);
        }
    }
    void reset() { loggers_.clear(); }

    std::unique_ptr<Logger> create_child() override {
        std::vector<std::unique_ptr<Logger>> child_loggers;
        child_loggers.reserve(loggers_.size());
        for (auto* logger : loggers_) {
            child_loggers.push_back(logger->create_child());
        }
        return std::make_unique<CompositeChildLogger>(std::move(child_loggers));
    }

    void log(LogEvent tag) override                           { log_all(tag); }
    void log(LogEvent tag, std::string_view message) override { log_all(tag, message); }
    void log(LogEvent tag, double value) override             { log_all(tag, value); }
    void log(LogEvent tag, Idx value) override                { log_all(tag, value); }

    using MultiThreadedLogger::log;

    // Fan out clear() to every registered logger.
    void clear() override {
        for (auto* logger : loggers_) {
            logger->clear();
        }
    }

    [[nodiscard]] bool empty() const { return loggers_.empty(); }

  private:
    std::vector<MultiThreadedLogger*> loggers_; // non-owning

    template <typename... Args> void log_all(Args&&... args) {
        for (auto* logger : loggers_) {
            logger->log(std::forward<Args>(args)...);
        }
    }
};

} // namespace power_grid_model::common::logging
