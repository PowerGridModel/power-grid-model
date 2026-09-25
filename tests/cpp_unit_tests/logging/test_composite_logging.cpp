// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/composite_logging.hpp>

#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/text_logger.hpp>

#include <doctest/doctest.h>

#include <memory>
#include <string>

namespace power_grid_model::common::logging {
namespace {
using LoggerPtr = std::shared_ptr<MultiThreadedTextLogger>;

LoggerPtr make_text_logger() { return std::make_shared<MultiThreadedTextLogger>(); }

// CalculationInfo only records numeric events, so its should_log is always false.
std::shared_ptr<MultiThreadedCalculationInfo> make_silent_logger() {
    return std::make_shared<MultiThreadedCalculationInfo>();
}
} // namespace

TEST_CASE("Test MultiThreadedCompositeLogger") {
    MultiThreadedCompositeLogger composite;

    SUBCASE("Empty composite has no output and is empty") { CHECK(composite.empty()); }

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

TEST_CASE("Test MultiThreadedCompositeLogger::should_log") {
    MultiThreadedCompositeLogger composite;

    SUBCASE("Empty composite never wants to log") {
        CHECK_FALSE(composite.should_log(LogEvent::unknown));
        CHECK_FALSE(composite.should_log(LogEvent::total));
    }

    SUBCASE("Single verbose logger opts in") {
        composite.add(make_text_logger());
        CHECK(composite.should_log(LogEvent::unknown));
        CHECK(composite.should_log(LogEvent::total));
    }

    SUBCASE("Single silent logger opts out") {
        composite.add(make_silent_logger());
        CHECK_FALSE(composite.should_log(LogEvent::unknown));
        CHECK_FALSE(composite.should_log(LogEvent::total));
    }

    SUBCASE("A single verbose logger among silent ones is enough") {
        composite.add(make_silent_logger());
        composite.add(make_text_logger());
        composite.add(make_silent_logger());
        CHECK(composite.should_log(LogEvent::total));
    }

    SUBCASE("Removing the only verbose logger opts the composite out") {
        auto verbose = make_text_logger();
        composite.add(make_silent_logger());
        composite.add(verbose);
        REQUIRE(composite.should_log(LogEvent::total));

        composite.remove(verbose.get());
        CHECK_FALSE(composite.should_log(LogEvent::total));
    }

    SUBCASE("Reset opts the composite out") {
        composite.add(make_text_logger());
        REQUIRE(composite.should_log(LogEvent::total));

        composite.reset();
        CHECK_FALSE(composite.should_log(LogEvent::total));
    }

    SUBCASE("Lazy messages are not evaluated when no logger opts in") {
        composite.add(make_silent_logger());

        Idx call_count{};
        composite.log(LogEvent::total, [&call_count] {
            ++call_count;
            return std::string{"expensive"};
        });

        CHECK(call_count == 0);
    }

    SUBCASE("Lazy messages are evaluated exactly once and fan out when a logger opts in") {
        auto verbose = make_text_logger();
        composite.add(make_silent_logger());
        composite.add(verbose);

        Idx call_count{};
        composite.log(LogEvent::total, [&call_count] {
            ++call_count;
            return std::string{"expensive"};
        });

        CHECK(call_count == 1);
        CHECK(verbose->report().find("expensive") != std::string::npos);
    }
}

TEST_CASE("Test CompositeChildLogger::should_log") {
    MultiThreadedCompositeLogger composite;

    SUBCASE("Child of an empty composite never wants to log") {
        auto child = composite.create_child();
        CHECK_FALSE(child->should_log(LogEvent::unknown));
        CHECK_FALSE(child->should_log(LogEvent::total));
    }

    SUBCASE("Child of a verbose composite opts in") {
        auto logger = make_text_logger();
        composite.add(logger);

        auto child = composite.create_child();
        CHECK(child->should_log(LogEvent::total));
    }

    SUBCASE("Child of a silent composite opts out") {
        composite.add(make_silent_logger());

        auto child = composite.create_child();
        CHECK_FALSE(child->should_log(LogEvent::total));
    }

    SUBCASE("Child opts in if any of its own children opts in") {
        composite.add(make_silent_logger());
        composite.add(make_text_logger());

        auto child = composite.create_child();
        CHECK(child->should_log(LogEvent::total));
    }

    SUBCASE("Lazy messages on a child are not evaluated when no child logger opts in") {
        composite.add(make_silent_logger());
        auto child = composite.create_child();

        Idx call_count{};
        child->log(LogEvent::total, [&call_count] {
            ++call_count;
            return std::string{"expensive"};
        });

        CHECK(call_count == 0);
    }

    SUBCASE("Lazy messages on a child are evaluated once and fan out when a child logger opts in") {
        auto logger = make_text_logger();
        composite.add(make_silent_logger());
        composite.add(logger);

        Idx call_count{};
        {
            auto child = composite.create_child();
            child->log(LogEvent::total, [&call_count] {
                ++call_count;
                return std::string{"expensive"};
            });
        } // child destroyed here; TextLogger children merge into their parent on destruction

        CHECK(call_count == 1);
        CHECK(logger->report().find("expensive") != std::string::npos);
    }

    SUBCASE("Child does not track loggers added to the composite after its creation") {
        auto child = composite.create_child();
        composite.add(make_text_logger());

        CHECK(composite.should_log(LogEvent::total));
        CHECK_FALSE(child->should_log(LogEvent::total));
    }
}
} // namespace power_grid_model::common::logging
