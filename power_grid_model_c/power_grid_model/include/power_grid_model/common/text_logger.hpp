// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "logging.hpp"
#include "multi_threaded_logging.hpp"

#include <chrono>
#include <concepts>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace power_grid_model {
namespace common::logging {
template <typename Fn>
concept LazyLoggingFn = std::invocable<Fn> && std::convertible_to<std::invoke_result_t<Fn>, std::string> &&
                        (!std::convertible_to<Fn, std::string_view>);

class TextLogger : public Logger {
    using Clock = std::chrono::high_resolution_clock;
    using FlushHandler = std::function<void(std::string)>;

    Clock::time_point start_time_;
    std::stringstream data_;
    FlushHandler flush_handler_;

  public:
    TextLogger() : start_time_{Clock::now()} {};
    TextLogger(FlushHandler flush_handler) : start_time_{Clock::now()}, flush_handler_{std::move(flush_handler)} {}

    TextLogger(TextLogger const&) = delete;
    TextLogger(TextLogger&& other) noexcept = default;
    TextLogger& operator=(TextLogger const&) = delete;
    TextLogger& operator=(TextLogger&& other) noexcept = default;
    ~TextLogger() override {
        // exception swallowing: we try to flush if possible. If not possible, clear data and ignore log
        try {
            // only flush if there is a handler and there are contents to flush
            if (flush_handler_ && data_.rdbuf()->in_avail() > 0) { // only flush if there are contents and a handler
                flush();
                return;
            }
        } catch (...) { // NOSONAR(S2738)
            // fallthrough to clear. log is ignored
        }
        clear();
    };

    void log(LogEvent tag) override { log_impl(tag, ""); }
    void log(LogEvent tag, double value) override { log_impl(tag, std::to_string(value)); }
    void log(LogEvent tag, Idx value) override { log_impl(tag, std::to_string(value)); }
    void log(LogEvent tag, std::string_view message) override { log_impl(tag, message); }
    void log(std::string_view message) { log_impl(LogEvent::unknown, message); }
    template <LazyLoggingFn Fn> void log(LogEvent tag, Fn&& fn) { log_impl(tag, std::invoke(std::forward<Fn>(fn))); }
    template <LazyLoggingFn Fn> void log(Fn&& fn) { log_impl(LogEvent::unknown, std::invoke(std::forward<Fn>(fn))); }

  private:
    void log_impl(LogEvent tag, std::string_view message) {
        data_ << std::format("[{} ns] Tag:{}: {}\n",
                             std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start_time_).count(),
                             std::to_underlying(tag), message);
    }

  public:
    void clear() {
        data_.str(""); // clear content
        data_.clear(); // reset error flags
    }
    std::string report() const { return data_.str(); }
    void flush() {
        if (flush_handler_) {
            // exception swallowing: if the handler throws, we leave the logger in valid state and the caller handles it
            try {
                auto buffer = data_.str();
                flush_handler_(std::move(buffer));
            } catch (...) { // NOSONAR(S2738)
                clear();    // leave logger in valid state and throw away report
                throw;      // rethrow to let caller handle it
            }
        }
        clear(); // if no handler, discard log
    }

    TextLogger& merge_into(TextLogger& destination) const {
        if (&destination == this) {
            return destination; // nothing to do
        }
        destination.data_ << data_.str();
        return destination;
    }
};

class MultiThreadedTextLogger : public MultiThreadedLoggerImpl<TextLogger> {
  public:
    using MultiThreadedLoggerImpl<TextLogger>::MultiThreadedLoggerImpl;

    std::string report() const { return get().report(); }
    void clear() { get().clear(); }
    void flush() { get().flush(); }
};
} // namespace common::logging

using common::logging::MultiThreadedTextLogger;
using common::logging::TextLogger;
} // namespace power_grid_model
