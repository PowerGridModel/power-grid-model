// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "logging.hpp"
#include "multi_threaded_logging.hpp"

#include <chrono>
#include <concepts>
#include <format>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace power_grid_model {
namespace common::logging {
class TextLogger : public Logger {
    using FlushHandler = std::function<void(std::string)>;

    std::stringstream data_;
    FlushHandler flush_handler_;

  public:
    TextLogger() = default;
    TextLogger(FlushHandler flush_handler) : flush_handler_{std::move(flush_handler)} {}

    TextLogger(TextLogger const&) = delete;
    TextLogger(TextLogger&& other) noexcept
        : data_(std::exchange(other.data_, {})), flush_handler_(std::exchange(other.flush_handler_, {})) {}

    TextLogger& operator=(TextLogger const&) = delete;
    TextLogger& operator=(TextLogger&& other) noexcept {
        if (this != &other) {
            data_ = std::exchange(other.data_, {});
            flush_handler_ = std::exchange(other.flush_handler_, {});
        }
        return *this;
    }
    ~TextLogger() override {
        // exception swallowing: we try to flush if possible. If not possible, clear data and ignore log
        try {
            // only flush if there is a handler and there are contents to flush
            if (flush_handler_ && !data_.view().empty()) { // only flush if there are contents and a handler
                flush();
                return;
            }
        } catch (...) { // NOSONAR(S2738) // NOLINT
            // fallthrough to clear. log is ignored
        }
        clear();
    };

    void log(LogEvent tag) override { log_impl(tag, ""); }
    void log(LogEvent tag, double value) override { log_impl(tag, std::to_string(value)); }
    void log(LogEvent tag, Idx value) override { log_impl(tag, std::to_string(value)); }
    void log(LogEvent tag, std::string_view message) override { log_impl(tag, message); }

    using Logger::log;

  private:
    static auto timestamp() {
        using namespace std::chrono;
        const auto now = system_clock::now();
        const auto sec = floor<seconds>(now);
        const auto ms = duration_cast<milliseconds>(now - sec).count();

        // Z stands for UTC time zone
        // format example: 2026-04-25 14:00:00.000Z
        return std::format("{:%F %T}.{:03}Z", sec, ms);
    }

    template <typename T> void log_impl(LogEvent tag, T&& value) {
        data_ << std::format("[{}] Tag:{}: {}\n", timestamp(), std::to_underlying(tag), std::forward<T>(value));
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
        destination.data_ << data_.view();
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
