// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/composite_logging.hpp>

#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/text_logger.hpp>

#include <doctest/doctest.h>

#include <memory>

namespace power_grid_model::common::logging {
namespace {
using LoggerPtr = std::shared_ptr<MultiThreadedTextLogger>;

LoggerPtr make_text_logger() { return std::make_shared<MultiThreadedTextLogger>(); }
} // namespace

TEST_CASE("Test MultiThreadedCompositeLogger") {
    MultiThreadedCompositeLogger composite;

    SUBCASE("Empty composite has no output and is empty") {
        CHECK(composite.empty());
    }

    SUBCASE("Adding a null logger is a no-op") {
        composite.add(nullptr);
        CHECK(composite.empty());
    }

    SUBCASE("Logging fans out to a single registered logger") {
        auto logger = make_text_logger();
        composite.add(logger);
        CHECK_FALSE(composite.empty());

        composite.log(LogEvent::total, Idx{1});

        CHECK(logger->report().find("Tag:0") != std::string::npos);
    }

    SUBCASE("Logging fans out to multiple registered loggers") {
        auto logger_a = make_text_logger();
        auto logger_b = make_text_logger();
        composite.add(logger_a);
        composite.add(logger_b);

        composite.log(LogEvent::total, Idx{1});

        CHECK_FALSE(logger_a->report().empty());
        CHECK_FALSE(logger_b->report().empty());
    }

    SUBCASE("Registering the same logger twice is idempotent") {
        auto logger = make_text_logger();
        composite.add(logger);
        composite.add(logger); // second add — silent no-op

        composite.log(LogEvent::total, Idx{1});

        // Only one entry should be logged, i.e. exactly one occurrence of the tag.
        auto const report = logger->report();
        auto const first = report.find("Tag:0");
        CHECK(first != std::string::npos);
        CHECK(report.find("Tag:0", first + 1) == std::string::npos);
    }

    SUBCASE("Remove detaches a specific logger without affecting others") {
        auto logger_a = make_text_logger();
        auto logger_b = make_text_logger();
        composite.add(logger_a);
        composite.add(logger_b);

        composite.remove(logger_a.get());
        composite.log(LogEvent::total, Idx{1});

        CHECK(logger_a->report().empty());
        CHECK_FALSE(logger_b->report().empty());
    }

    SUBCASE("Remove of an unregistered logger is a no-op") {
        auto logger = make_text_logger();
        composite.remove(logger.get()); // never added
        CHECK(composite.empty());
    }

    SUBCASE("Reset detaches all loggers") {
        auto logger_a = make_text_logger();
        auto logger_b = make_text_logger();
        composite.add(logger_a);
        composite.add(logger_b);

        composite.reset();
        CHECK(composite.empty());

        composite.log(LogEvent::total, Idx{1});
        CHECK(logger_a->report().empty());
        CHECK(logger_b->report().empty());
    }

    SUBCASE("clear() fans out to every registered logger") {
        auto logger = make_text_logger();
        composite.add(logger);
        composite.log(LogEvent::total, Idx{1});
        CHECK_FALSE(logger->report().empty());

        composite.clear();
        CHECK(logger->report().empty());
    }

    SUBCASE("Registered logger implementation stays alive after the caller drops its own shared_ptr") {
        MultiThreadedTextLogger const* raw_logger{};
        {
            auto logger = make_text_logger();
            raw_logger = logger.get();
            composite.add(logger);
        } // caller's shared_ptr is dropped here; the composite keeps its own shared_ptr alive.
        CHECK_FALSE(composite.empty());

        // The composite still owns the implementation, so logging must not crash and must produce output.
        // Observing through raw_logger is not UB: the composite's shared_ptr keeps the object alive.
        composite.log(LogEvent::total, Idx{1});
        CHECK_FALSE(raw_logger->report().empty());
    }

    SUBCASE("create_child fans out to a child of every registered logger") {
        auto logger_a = make_text_logger();
        auto logger_b = make_text_logger();
        composite.add(logger_a);
        composite.add(logger_b);

        {
            auto child = composite.create_child();
            child->log(LogEvent::total, Idx{1});
        } // child destroyed here; TextLogger children merge into their parent on destruction

        CHECK_FALSE(logger_a->report().empty());
        CHECK_FALSE(logger_b->report().empty());
    }
}
} // namespace power_grid_model::common::logging
