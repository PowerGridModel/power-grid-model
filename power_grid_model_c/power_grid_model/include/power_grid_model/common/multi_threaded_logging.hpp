// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "dummy_logging.hpp"

#include <cassert>
#include <mutex>

namespace power_grid_model::common::logging {

template <std::derived_from<Logger> LoggerType>
    requires requires(LoggerType& destination, LoggerType const& source) {
        { source.merge_into(destination) };
    }
class MultiThreadedLoggerImpl : public MultiThreadedLogger {
  public:
    class ThreadLogger : public LoggerType {
      public:
        ThreadLogger(MultiThreadedLoggerImpl& parent) : parent_{&parent} {}
        ThreadLogger(ThreadLogger const&) = default;
        ThreadLogger& operator=(ThreadLogger const&) = default;
        ThreadLogger(ThreadLogger&&) noexcept = default;
        ThreadLogger& operator=(ThreadLogger&&) noexcept = default;
        ~ThreadLogger() noexcept override { sync(); }
        void sync() const { parent_->sync(*this); }

      private:
        MultiThreadedLoggerImpl* parent_;
    };

    std::unique_ptr<Logger> create_child() override { return std::make_unique<ThreadLogger>(*this); }
    LoggerType& get() { return log_; }
    LoggerType const& get() const { return log_; }

    void log(LogEvent tag) override { log_.log(tag); }
    void log(LogEvent tag, std::string_view message) override { log_.log(tag, message); }
    void log(LogEvent tag, double value) override { log_.log(tag, value); }
    void log(LogEvent tag, Idx value) override { log_.log(tag, value); }

  private:
    friend class ThreadLogger;

    LoggerType log_;
    std::mutex mutex_;

    void sync(ThreadLogger const& logger) {
        assert(&logger != &log_);

        std::lock_guard const lock{mutex_};
        logger.merge_into(log_);
    }
};

using NoMultiThreadedLogger = MultiThreadedLoggerImpl<NoLogger>;

} // namespace power_grid_model::common::logging
