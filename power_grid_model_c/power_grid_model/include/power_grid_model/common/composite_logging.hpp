// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace power_grid_model::common::logging {

// Owns a list of child loggers (created by MultiThreadedCompositeLogger::create_child) and fans all log calls out to
// each of them. The children are owned by this logger; their lifetimes are tied to this object.
class CompositeChildLogger : public Logger {
  public:
    explicit CompositeChildLogger(std::vector<std::unique_ptr<Logger>> children) : children_{std::move(children)} {}

    void log(LogEvent tag) override {
        for (auto& child : children_) {
            child->log(tag);
        }
    }
    void log(LogEvent tag, std::string_view message) override {
        for (auto& child : children_) {
            child->log(tag, message);
        }
    }
    void log(LogEvent tag, double value) override {
        for (auto& child : children_) {
            child->log(tag, value);
        }
    }
    void log(LogEvent tag, Idx value) override {
        for (auto& child : children_) {
            child->log(tag, value);
        }
    }

    using Logger::log;

  private:
    std::vector<std::unique_ptr<Logger>> children_;
};

// Non-owning fan-out MultiThreadedLogger. Holds non-owning pointers to MultiThreadedLogger instances and forwards
// all log calls to each. create_child() creates a CompositeChildLogger that owns one child per registered logger.
//
// Lifetime contract: all loggers in the list must outlive this composite.
// UB: registering the same logger twice, modifying the logger list while calculate is in progress.
class MultiThreadedCompositeLogger : public MultiThreadedLogger {
  public:
    explicit MultiThreadedCompositeLogger(std::vector<MultiThreadedLogger*> loggers) : loggers_{std::move(loggers)} {}

    std::unique_ptr<Logger> create_child() override {
        std::vector<std::unique_ptr<Logger>> children;
        children.reserve(loggers_.size());
        for (auto* l : loggers_) {
            children.push_back(l->create_child());
        }
        return std::make_unique<CompositeChildLogger>(std::move(children));
    }

    void log(LogEvent tag) override {
        for (auto* l : loggers_) {
            l->log(tag);
        }
    }
    void log(LogEvent tag, std::string_view message) override {
        for (auto* l : loggers_) {
            l->log(tag, message);
        }
    }
    void log(LogEvent tag, double value) override {
        for (auto* l : loggers_) {
            l->log(tag, value);
        }
    }
    void log(LogEvent tag, Idx value) override {
        for (auto* l : loggers_) {
            l->log(tag, value);
        }
    }

    using MultiThreadedLogger::log;

    [[nodiscard]] bool empty() const { return loggers_.empty(); }

  private:
    std::vector<MultiThreadedLogger*> loggers_; // non-owning
};

} // namespace power_grid_model::common::logging
