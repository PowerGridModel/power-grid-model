// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logging_impl.hpp"

#include <mutex>

namespace power_grid_model::common::logging {

template <std::derived_from<Logger> LoggerType>
    requires requires(LoggerType& destination, LoggerType const& source) {
        { merge_into(destination, source) };
    }
class MultiThreadedLoggerImpl : public MultiThreadedLogger {
    using SubThreadLogger = LoggerType;

  public:
    class ThreadLogger : public SubThreadLogger {
      public:
        ThreadLogger(MultiThreadedLoggerImpl& parent) : parent_{&parent} {}
        ~ThreadLogger() override { sync(); }
        void sync() const { parent_->sync(*this); }

      private:
        MultiThreadedLoggerImpl* parent_;
    };

    virtual std::unique_ptr<Logger> create_child() override { return std::make_unique<ThreadLogger>(*this); }
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
        std::lock_guard lock{mutex_};
        merge_into(log_, static_cast<SubThreadLogger const&>(logger));
    }
};

using NoMultiThreadedLogger = MultiThreadedLoggerImpl<NoLogger>;

} // namespace power_grid_model::common::logging
