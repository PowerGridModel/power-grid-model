// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#include <power_grid_model/common/timer.hpp>

#include <span>
#include <thread>
#include <variant>

#include <doctest/doctest.h>

namespace power_grid_model::common::logging {
namespace {
using common::logging::MultiThreadedLogger;

class MiniLogger : public common::logging::Logger {
  private:
    using Entry = std::pair<LogEvent, double>;
    std::vector<Entry> data_{};

  public:
    void log(LogEvent /*event*/) override { /* not used in Timer */ }
    void log(LogEvent /*event*/, std::string_view /*message*/) override { /* not used in Timer */ }
    void log(LogEvent /*event*/, Idx /*value*/) override { /* not used in Timer */ }
    void log(LogEvent event, double value) override { data_.emplace_back(event, value); }

    auto report() const { return std::span(data_); }
};
} // namespace

TEST_CASE("Test Timer") {
    auto test_logger = MiniLogger{};
    auto report = test_logger.report();
    CHECK(report.size() == 0);

    SUBCASE("Default constructor") {
        static_assert(std::is_default_constructible_v<Timer>);
        auto timer_tester = []() {
            auto timer = Timer{};
            CHECK_NOTHROW(timer.stop());
        };
        CHECK_NOTHROW(timer_tester();); // destructor should not throw
    }

    SUBCASE("Logging time") {
        auto const& event_1 = LogEvent::prepare;
        auto const& event_2 = LogEvent::create_math_solver;
        auto const& event_3 = LogEvent::math_solver;
        auto time_event = [&test_logger](LogEvent event) {
            auto timer = Timer{test_logger, event};
            std::this_thread::sleep_for(
                std::chrono::milliseconds(10)); // abitrary (reasonable) sleep to have measurable
        };

        time_event(event_1);
        report = test_logger.report();
        CHECK(report.size() == 1);
        CHECK(report[0].first == event_1);
        CAPTURE(report[0].second);
        CHECK((report[0].second >= 0.01 && report[0].second < 0.015)); // allow some margin

        time_event(event_2);
        report = test_logger.report();
        CHECK(report.size() == 2);
        CHECK(report[1].first == event_2);
        CAPTURE(report[1].second);
        CHECK((report[1].second >= 0.01 && report[1].second < 0.015)); // allow some margin

        time_event(event_3);
        report = test_logger.report();
        CHECK(report.size() == 3);
        CHECK(report[2].first == event_3);
        CAPTURE(report[2].second);
        CHECK((report[2].second >= 0.01 && report[2].second < 0.015)); // allow some margin
    }

    SUBCASE("Stop timer") {
        auto const& event = LogEvent::prepare;
        auto timer = Timer{test_logger, event};
        auto check_report = [&test_logger, &event]() {
            auto scoped_report = test_logger.report();
            CHECK(scoped_report.size() == 1);
            CHECK(scoped_report[0].first == event);
            CAPTURE(scoped_report[0].second);
            CHECK((scoped_report[0].second >= 0.01 && scoped_report[0].second < 0.015)); // allow some margin
        };

        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // abitrary (reasonable) sleep to have measurable time
        timer.stop();
        CHECK_NOTHROW(timer.stop()); // second stop should have no effect
        check_report();
        CHECK_NOTHROW(timer.stop()); // third stop should have no effect
        check_report();
    }

    SUBCASE("Timer is not copyable") {
        static_assert(!std::is_copy_constructible_v<Timer>);
        static_assert(!std::is_copy_assignable_v<Timer>);
    }

    SUBCASE("Move constructor") {
        auto const& event = LogEvent::prepare;
        auto timer_1 = Timer{test_logger, event};
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // abitrary (reasonable) sleep to have measurable time

        auto timer_2 = Timer{std::move(timer_1)};
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // abitrary (reasonable) sleep to have measurable time
        timer_2.stop();

        auto report = test_logger.report();
        CHECK(report.size() == 1);
        CHECK(report[0].first == event);
        CAPTURE(report[0].second);
        CHECK((report[0].second >= 0.02 && report[0].second < 0.03)); // allow some margin
    }

    SUBCASE("Move assignment") {
        auto const& event_1 = LogEvent::prepare;
        auto const& event_2 = LogEvent::create_math_solver;
        auto timer_1 = Timer{test_logger, event_1};
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // abitrary (reasonable) sleep to have measurable time

        auto timer_2 = Timer{test_logger, event_2};
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // abitrary (reasonable) sleep to have measurable time

        timer_2 = std::move(timer_1);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // abitrary (reasonable) sleep to have measurable time
        timer_2.stop();

        auto report = test_logger.report();
        CHECK(report.size() == 2);
        CHECK(report[0].first == event_2);
        CAPTURE(report[0].second);
        CHECK((report[0].second >= 0.01 && report[0].second < 0.015)); // allow some margin
        CHECK(report[1].first == event_1);
        CAPTURE(report[1].second);
        CHECK((report[1].second >= 0.03 && report[1].second < 0.04)); // allow some margin
    }
}

} // namespace power_grid_model::common::logging
