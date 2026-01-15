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
    auto const sleep_duration = std::chrono::microseconds(10); // small delay to ensure measurable time
    double const time_reference = std::chrono::duration<double>(sleep_duration).count();

    auto check_report = [&time_reference](auto const& report, LogEvent const event, Idx const idx, Idx const size) {
        CHECK(report.size() == size);
        CHECK(report[idx].first == event);
        double const time_stamp = report[idx].second;
        CHECK(time_stamp >= time_reference);
    };

    SUBCASE("Default constructor") {
        static_assert(std::is_default_constructible_v<Timer>);
        auto timer_tester = []() {
            auto timer = Timer{};
            CHECK_NOTHROW(timer.stop());
        };
        CHECK_NOTHROW(timer_tester();); // destructor should not throw
    }

    SUBCASE("Log time") {
        auto const& event_1 = LogEvent::prepare;
        auto const& event_2 = LogEvent::create_math_solver;
        auto const& event_3 = LogEvent::math_solver;
        auto time_event = [&test_logger, sleep_duration](LogEvent event) {
            auto timer = Timer{test_logger, event};
            std::this_thread::sleep_for(sleep_duration);
        };

        time_event(event_1);
        report = test_logger.report();
        check_report(report, event_1, Idx{0}, Idx{1});

        time_event(event_2);
        report = test_logger.report();
        check_report(report, event_2, Idx{1}, Idx{2});

        time_event(event_3);
        report = test_logger.report();
        check_report(report, event_3, Idx{2}, Idx{3});
    }

    SUBCASE("Stop timer") {
        auto const& event = LogEvent::prepare;
        auto timer = Timer{test_logger, event};
        std::this_thread::sleep_for(sleep_duration);
        timer.stop();
        CHECK_NOTHROW(timer.stop()); // second stop should have no effect
        report = test_logger.report();
        check_report(report, event, 0, 1);
        CHECK_NOTHROW(timer.stop()); // third stop should have no effect
        check_report(report, event, 0, 1);
    }

    SUBCASE("Timer is not copyable") {
        static_assert(!std::is_copy_constructible_v<Timer>);
        static_assert(!std::is_copy_assignable_v<Timer>);
    }

    SUBCASE("Move constructor") {
        auto const& event = LogEvent::prepare;
        auto timer_1 = Timer{test_logger, event};
        auto timer_2 = Timer{std::move(timer_1)};
        std::this_thread::sleep_for(sleep_duration);
        timer_2.stop();

        report = test_logger.report();
        check_report(report, event, 0, 1);
    }

    SUBCASE("Move assignment") {
        auto const& event_1 = LogEvent::prepare;
        auto const& event_2 = LogEvent::create_math_solver;
        auto timer_1 = Timer{test_logger, event_1};
        auto timer_2 = Timer{test_logger, event_2};
        std::this_thread::sleep_for(sleep_duration);
        timer_2 = std::move(timer_1);
        std::this_thread::sleep_for(sleep_duration);
        timer_2.stop();

        report = test_logger.report();
        check_report(report, event_2, 0, 2);
        check_report(report, event_1, 1, 2);
    }
}

} // namespace power_grid_model::common::logging
