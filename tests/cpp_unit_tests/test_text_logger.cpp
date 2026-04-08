// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/text_logger.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/logging.hpp>

#include <doctest/doctest.h>

#include <string>
#include <string_view>

namespace power_grid_model::common::logging {
namespace {
void logger_helper(TextLogger& logger) {
    using enum LogEvent;
    logger.log(total);
    logger.log(build_model, 1.0);
    logger.log(total_single_calculation_in_thread, Idx{2});
    logger.log(total_batch_calculation_in_thread, "4");
    logger.log("5");
    logger.log(copy_model, []() { return "6"; });
    logger.log([]() { return "7.0"; });
}

void report_checker_helper(std::string_view report) {
    auto string_matcher = [](LogEvent tag, std::string_view message) -> std::string {
        return std::format("ns] Tag:{}: {}\n", std::to_underlying(tag), message);
    };

    using enum LogEvent;
    CHECK(report.find(string_matcher(total, "")) != std::string_view::npos);
    CHECK(report.find(string_matcher(build_model, "1.000000")) != std::string_view::npos);
    CHECK(report.find(string_matcher(total_single_calculation_in_thread, "2")) != std::string_view::npos);
    CHECK(report.find(string_matcher(total_batch_calculation_in_thread, "4")) != std::string_view::npos);
    CHECK(report.find(string_matcher(unknown, "5")) != std::string_view::npos);
    CHECK(report.find(string_matcher(copy_model, "6")) != std::string_view::npos);
    CHECK(report.find(string_matcher(unknown, "7.0")) != std::string_view::npos);
}
} // namespace

TEST_CASE("Test TextLogger") {
    SUBCASE("No flush-handler") {
        TextLogger txt_logger{};

        SUBCASE("Log and report") {
            auto report = txt_logger.report();
            CHECK(report.empty());

            logger_helper(txt_logger);

            report = txt_logger.report();
            report_checker_helper(report);
        }

        SUBCASE("Clear report") {
            txt_logger.clear();
            auto clean_report = txt_logger.report();
            CHECK(clean_report.empty());

            logger_helper(txt_logger);
            txt_logger.clear();
            clean_report = txt_logger.report();
            CHECK(clean_report.empty());
        }

        SUBCASE("Flush report") {
            // flushing empty reports without a handler is okay
            auto report = txt_logger.report();
            CHECK(report.empty());
            txt_logger.flush();
            CHECK(report.empty());

            // flushinig non-empty report without a handler should clear the report
            logger_helper(txt_logger);
            report = txt_logger.report();
            report_checker_helper(report);
            txt_logger.flush();
            report = txt_logger.report();
            CHECK(report.empty());
        }

        SUBCASE("Merge into itself") {
            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            // merging into itself should not change the report
            txt_logger.merge_into(txt_logger);
            report = txt_logger.report();
            report_checker_helper(report);
        }

        SUBCASE("Merge into empty TextLogger") {
            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            TextLogger other_txt_logger{};
            auto other_report = other_txt_logger.report();
            CHECK(other_report.empty());

            txt_logger.merge_into(other_txt_logger);
            other_report = other_txt_logger.report();
            // new logger should have the same report as original logger
            report_checker_helper(other_report);

            // original logger should be cleared
            report = txt_logger.report();
            CHECK(report.empty());
        }

        SUBCASE("Merge into non-empty-different TextLogger") {
            using enum LogEvent;

            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            TextLogger other_txt_logger{};
            auto other_report = other_txt_logger.report();
            CHECK(other_report.empty());
            other_txt_logger.log("other");
            other_report = other_txt_logger.report();
            CHECK(other_report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), "other")) !=
                  std::string_view::npos);

            txt_logger.merge_into(other_txt_logger);
            report = txt_logger.report();
            CHECK(report.empty()); // original logger should be cleared after merging into
            other_report = other_txt_logger.report();
            CHECK(other_report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), "other")) !=
                  std::string_view::npos);
            report_checker_helper(other_report);
        }

        SUBCASE("Destructor") {
            // destructor should not throw without handler
            {
                TextLogger scopped_txt_logger{};
                logger_helper(scopped_txt_logger);
            }
        }

        SUBCASE("Lazy vs eager logging") {
            // TODO(figueroa1395): I'm not sure if this is worth testing at the moment
            // as the difference is not really apparent at the right now, but this is an enabler for extension
            auto call_count = Idx{0};
            auto expensive = [&call_count]() {
                ++call_count;
                return "expensive";
            };

            // lazy logging: expensive is not triggered
            if (false) {
                txt_logger.log(expensive);
            }
            CHECK(call_count == 0);

            // lazy logging: expensive is triggered when we want it to
            txt_logger.log(expensive);
            CHECK(call_count == 1);

            // eager logging: expensive is triggered immediately
            txt_logger.log(expensive());
            CHECK(call_count == 2);
        }
    }

    SUBCASE("With flush-handler") {
        auto flushed_report = std::string{};
        auto flush_handler = [&flushed_report](std::string buffer) { flushed_report = buffer; };
        auto throwing_flush_handler = [](std::string /*buffer*/) { throw std::runtime_error("flush failed"); };

        SUBCASE("Flush report with handler that doesn't throw") {
            TextLogger txt_logger{flush_handler};
            CHECK(flushed_report.empty());

            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            txt_logger.flush();
            report = txt_logger.report();
            CHECK(report.empty());                 // report should be cleared after flush
            report_checker_helper(flushed_report); // flushed report should have the contents of the original report
        }

        SUBCASE("Flush report with handler that throws") {
            TextLogger txt_logger{throwing_flush_handler};

            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            // flushing should throw and report should be cleared even if handler throws
            CHECK_THROWS_AS(txt_logger.flush(), std::runtime_error);
            report = txt_logger.report();
            CHECK(report.empty()); // report should be cleared even if handler throws
        }

        SUBCASE("Destructor with handler that doesn't throw") {
            {
                TextLogger scopped_txt_logger{flush_handler};
                CHECK(flushed_report.empty());
                logger_helper(scopped_txt_logger);
                report_checker_helper(scopped_txt_logger.report());
            }
            // after destruction, flushed report should have the contents of the original report
            report_checker_helper(flushed_report);
        }

        SUBCASE("Destructor with handler that throws") {
            // throwing destructor should be handled gracefully, report cleared and flushed report kept empty
            {
                TextLogger scopped_txt_logger{throwing_flush_handler};
                CHECK(flushed_report.empty());
                logger_helper(scopped_txt_logger);
                report_checker_helper(scopped_txt_logger.report());
            }
            CHECK(flushed_report.empty());
        }
    }
}
} // namespace power_grid_model::common::logging
