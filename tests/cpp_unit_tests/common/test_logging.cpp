// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.common;
import power_grid_model.common.logging;
#else
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/logging.hpp>
#endif

#include <doctest/doctest.h>

#include <concepts>
#include <memory>
#include <string>
#include <string_view>

namespace power_grid_model::common::logging {
namespace {
static_assert(std::same_as<power_grid_model::LogEvent, LogEvent>);
static_assert(std::same_as<power_grid_model::Logger, Logger>);

class RecordingLogger : public Logger {
  public:
    using Logger::log;

    void log(LogEvent tag) override { last_tag = tag; }
    void log(LogEvent tag, std::string_view message) override {
        last_tag = tag;
        last_message = message;
    }
    void log(LogEvent tag, double /*value*/) override { last_tag = tag; }
    void log(LogEvent tag, Idx /*value*/) override { last_tag = tag; }
    [[nodiscard]] bool should_log(LogEvent tag) const override { return tag != LogEvent::debug; }

    LogEvent last_tag{LogEvent::total};
    std::string last_message;
};
} // namespace

TEST_CASE("Logger") {
    RecordingLogger logger;

    SUBCASE("Message without tag") {
        logger.log("message");
        CHECK(logger.last_tag == LogEvent::unknown);
        CHECK(logger.last_message == "message");
    }
    SUBCASE("Lazy message is only constructed when it should be logged") {
        bool called{false};
        auto const fn = [&called] {
            called = true;
            return std::string{"lazy"};
        };
        static_assert(LazyLoggingFn<decltype(fn)>);

        logger.log(LogEvent::debug, fn);
        CHECK_FALSE(called);

        logger.log(LogEvent::info, fn);
        CHECK(called);
        CHECK(logger.last_tag == LogEvent::info);
        CHECK(logger.last_message == "lazy");
    }
}

TEST_CASE("MultiThreadedLogger defaults") {
    class DefaultMultiThreadedLogger : public MultiThreadedLogger {
      public:
        void log(LogEvent /*tag*/) override {}
        void log(LogEvent /*tag*/, std::string_view /*message*/) override {}
        void log(LogEvent /*tag*/, double /*value*/) override {}
        void log(LogEvent /*tag*/, Idx /*value*/) override {}
    };

    DefaultMultiThreadedLogger logger;
    CHECK(logger.create_child() == nullptr);

    std::string output{"not empty"};
    logger.get_output([&output](std::string_view view) { output = view; });
    CHECK(output.empty());
}
} // namespace power_grid_model::common::logging
