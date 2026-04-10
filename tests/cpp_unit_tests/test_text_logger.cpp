// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/text_logger.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/common/logging.hpp>

#include <doctest/doctest.h>

#include <concepts>
#include <format>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace power_grid_model::common::logging {
namespace {
class RuntimeFlushException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

constexpr Idx one_thread = Idx{1};
constexpr Idx arbitrary_n_threads = Idx{7};

constexpr std::string msg_other{"other"};
constexpr std::string msg_after_move{"after move"};
constexpr std::string msg_flush_failed{"flush failed"};
constexpr std::string msg_before_flush{"before flush"};
constexpr std::string msg_extra_log_one{"extra log 1"};
constexpr std::string msg_extra_log_two{"extra log 2"};

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
    auto string_matcher = [](LogEvent tag, std::string_view message) {
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

void child_logger_helper(Idx current_thread_idx, TextLogger& logger) { logger.log(std::to_string(current_thread_idx)); }

void multi_threaded_report_checker_helper(Idx n_threads, std::string_view report) {
    for (Idx idx = 0; idx < n_threads; ++idx) {
        CHECK(report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(LogEvent::unknown), idx)) !=
              std::string_view::npos);
    }
}

template <typename JobFn>
    requires std::invocable<JobFn, Idx, MultiThreadedTextLogger&>
void run_parallel_jobs(Idx n_threads, MultiThreadedTextLogger& logger, JobFn&& job) {
    std::vector<std::jthread> threads;
    threads.reserve(n_threads);
    for (Idx const idx : IdxRange{n_threads}) {
        threads.emplace_back(job, idx, std::ref(logger));
    }
    capturing::into_the_void(std::forward<JobFn>(job));
}
} // namespace

TEST_CASE("Test TextLogger") {
    using enum LogEvent;
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

            // original logger should be kept the same
            report = txt_logger.report();
            report_checker_helper(report);
        }

        SUBCASE("Merge into non-empty-different TextLogger") {
            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            TextLogger other_txt_logger{};
            auto other_report = other_txt_logger.report();
            CHECK(other_report.empty());
            other_txt_logger.log(msg_other);
            other_report = other_txt_logger.report();
            CHECK(other_report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_other)) !=
                  std::string_view::npos);

            txt_logger.merge_into(other_txt_logger);
            report = txt_logger.report();
            // original logger should be kept the same
            report_checker_helper(report);
            other_report = other_txt_logger.report();
            CHECK(other_report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_other)) !=
                  std::string_view::npos);
            report_checker_helper(other_report);
        }

        SUBCASE("Destructor") {
            // destructor should not throw without handler
            CHECK_NOTHROW({
                TextLogger scopped_txt_logger{};
                logger_helper(scopped_txt_logger);
            });
        }

        SUBCASE("Move constructor") {
            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            // moved-to has the data
            TextLogger other_txt_logger{std::move(txt_logger)};
            auto other_report = other_txt_logger.report();
            report_checker_helper(other_report);

            // moved-from is valid but empty
            report = txt_logger.report();
            CHECK(report.empty());
            CHECK_NOTHROW(txt_logger.log(unknown, msg_after_move));

            // moved-to didn't get any additional data after move
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_after_move) == std::string::npos);
        }

        SUBCASE("Move assignment") {
            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            TextLogger other_txt_logger{};
            auto other_report = other_txt_logger.report();
            CHECK(other_report.empty());

            // moved-to has the data
            other_txt_logger = std::move(txt_logger);
            other_report = other_txt_logger.report();
            report_checker_helper(other_report);

            // moved-from is valid but empty
            report = txt_logger.report();
            CHECK(report.empty());
            CHECK_NOTHROW(txt_logger.log(unknown, msg_after_move));

            // moved-to didn't get any additional data after move
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_after_move) == std::string::npos);
        }
    }

    SUBCASE("With flush-handler") {
        auto flushed_report = std::string{};
        auto flush_handler = [&flushed_report](std::string_view buffer) { flushed_report = buffer; };
        auto throwing_flush_handler = [](std::string_view /*buffer*/) {
            throw RuntimeFlushException(msg_flush_failed);
        };

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
            CHECK_THROWS_AS(txt_logger.flush(), RuntimeFlushException);
            report = txt_logger.report();
            CHECK(report.empty()); // report should be cleared even if handler throws
        }

        SUBCASE("Destructor with handler that doesn't throw") {

            CHECK_NOTHROW({
                TextLogger scopped_txt_logger{flush_handler};
                CHECK(flushed_report.empty());
                logger_helper(scopped_txt_logger);
                report_checker_helper(scopped_txt_logger.report());
            });
            // after destruction, flushed report should have the contents of the original report
            report_checker_helper(flushed_report);
        }

        SUBCASE("Destructor with handler that throws") {
            // throwing destructor should be handled gracefully, report cleared and flushed report kept empty
            CHECK_NOTHROW({
                TextLogger scopped_txt_logger{throwing_flush_handler};
                CHECK(flushed_report.empty());
                logger_helper(scopped_txt_logger);
                report_checker_helper(scopped_txt_logger.report());
            });
            CHECK(flushed_report.empty());
        }

        SUBCASE("Move constructor with handler") {
            // we don't care if the handler throws or not for move semantics
            TextLogger txt_logger{flush_handler};
            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            // moved-to has the data and the handler
            TextLogger other_txt_logger{std::move(txt_logger)};
            auto other_report = other_txt_logger.report();
            report_checker_helper(other_report);
            CHECK_NOTHROW(other_txt_logger.flush());
            report_checker_helper(flushed_report);
            other_report = other_txt_logger.report();
            CHECK(other_report.empty()); // report should be cleared after flush

            // moved-from is valid but empty and has no handler
            report = txt_logger.report();
            CHECK(report.empty());
            CHECK_NOTHROW(txt_logger.log(unknown, msg_after_move));
            report = txt_logger.report();
            CHECK(report.find(msg_after_move) != std::string::npos);

            // moved-to didn't get any additional data after move
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_after_move) == std::string::npos);

            // flushing moved-from should still work and clear its report
            // but not affect moved-to
            other_report = other_txt_logger.report();
            CHECK(other_report.empty());
            other_txt_logger.log(unknown, msg_before_flush);
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_before_flush) != std::string::npos);
            CHECK_NOTHROW(txt_logger.flush());
            report = txt_logger.report();
            CHECK(report.empty());
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_after_move) == std::string::npos);
            CHECK(other_report.find(msg_before_flush) != std::string_view::npos);
        }

        SUBCASE("Move assignment with handler") {
            // we don't care if the handler throws or not for move semantics
            TextLogger txt_logger{flush_handler};
            logger_helper(txt_logger);
            auto report = txt_logger.report();
            report_checker_helper(report);

            TextLogger other_txt_logger{};
            auto other_report = other_txt_logger.report();
            CHECK(other_report.empty());

            // moved-to has the data and the handler
            other_txt_logger = std::move(txt_logger);
            other_report = other_txt_logger.report();
            report_checker_helper(other_report);
            CHECK_NOTHROW(other_txt_logger.flush());
            report_checker_helper(flushed_report);
            other_report = other_txt_logger.report();
            CHECK(other_report.empty()); // report should be cleared after flush

            // moved-from is valid but empty and has no handler
            report = txt_logger.report();
            CHECK(report.empty());
            CHECK_NOTHROW(txt_logger.log(unknown, msg_after_move));
            report = txt_logger.report();
            CHECK(report.find(msg_after_move) != std::string_view::npos);

            // moved-to didn't get any additional data after move
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_after_move) == std::string_view::npos);

            // flushing moved-from should still work and clear its report
            // but not affect moved-to
            other_report = other_txt_logger.report();
            CHECK(other_report.empty());
            other_txt_logger.log(unknown, msg_before_flush);
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_before_flush) != std::string_view::npos);
            CHECK_NOTHROW(txt_logger.flush());
            report = txt_logger.report();
            CHECK(report.empty());
            other_report = other_txt_logger.report();
            CHECK(other_report.find(msg_after_move) == std::string_view::npos);
            CHECK(other_report.find(msg_before_flush) != std::string_view::npos);
        }
    }
}
TEST_CASE("Test MultiThreadedTextLogger") {
    using enum LogEvent;
    auto single_thread_job = [](Idx n_threads, MultiThreadedTextLogger& multi_threaded_logger) {
        // MultiThreadedTextLogger.create_child() is tested here
        auto thread_logger_ptr = multi_threaded_logger.create_child();
        auto& thread_logger = dynamic_cast<TextLogger&>(*thread_logger_ptr);

        child_logger_helper(n_threads, thread_logger);
    }; // when the jthread ends, the ThreadLogger is destroyed and sync is called (tested)

    SUBCASE("General multi-threaded logging functionalities") {
        MultiThreadedTextLogger multi_threaded_logger{};

        SUBCASE("Log and report through child - single threaded") {
            run_parallel_jobs(one_thread, multi_threaded_logger, single_thread_job);
            multi_threaded_report_checker_helper(one_thread, multi_threaded_logger.report());
        }

        SUBCASE("Log and report through child - multi threaded") {
            run_parallel_jobs(arbitrary_n_threads, multi_threaded_logger, single_thread_job);
            multi_threaded_report_checker_helper(arbitrary_n_threads, multi_threaded_logger.report());
        }

        SUBCASE("Direct logging") {
            multi_threaded_logger.log(total, "");
            multi_threaded_logger.log(build_model, 1.0);
            multi_threaded_logger.log(total_single_calculation_in_thread, Idx{2});
            multi_threaded_logger.log(total_batch_calculation_in_thread, "4");

            // TODO(figueroa1395): is this behaviour intended? Or we want to allow direct logging with these overloads?
            auto& underlying_logger = multi_threaded_logger.get();
            underlying_logger.log("5");
            underlying_logger.log(copy_model, []() { return "6"; });
            underlying_logger.log([]() { return "7.0"; });

            auto report = multi_threaded_logger.report();
            report_checker_helper(report);
        }

        SUBCASE("Clear report") {
            auto report = multi_threaded_logger.report();
            CHECK(report.empty());

            run_parallel_jobs(arbitrary_n_threads, multi_threaded_logger, single_thread_job);
            multi_threaded_logger.clear();
            report = multi_threaded_logger.report();
            CHECK(report.empty());
        }
    }

    SUBCASE("Getters of underlying TextLogger") {
        MultiThreadedTextLogger multi_threaded_logger{};
        auto const n_threads = static_cast<Idx>(std::thread::hardware_concurrency());
        run_parallel_jobs(n_threads, multi_threaded_logger, single_thread_job);

        SUBCASE("Log and report - Non-const getter") {
            auto& underlying_logger = multi_threaded_logger.get();
            underlying_logger.log([]() { return msg_extra_log_one; });
            underlying_logger.log(unknown, msg_extra_log_two);

            auto report = underlying_logger.report();
            multi_threaded_report_checker_helper(n_threads, report);
            CHECK(report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_extra_log_one)) !=
                  std::string_view::npos);
            CHECK(report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_extra_log_two)) !=
                  std::string_view::npos);
        }

        SUBCASE("Report - Const getter") {
            auto const& underlying_logger = multi_threaded_logger.get();
            auto report = underlying_logger.report();
            multi_threaded_report_checker_helper(n_threads, report);
        }

        SUBCASE("Merge into another TextLogger") {
            auto const& underlying_logger_const = multi_threaded_logger.get();
            auto report = underlying_logger_const.report();
            multi_threaded_report_checker_helper(n_threads, report);

            TextLogger other_logger{};
            auto other_report = other_logger.report();
            CHECK(other_report.empty());

            underlying_logger_const.merge_into(other_logger);
            other_report = other_logger.report();
            multi_threaded_report_checker_helper(n_threads, other_report);

            // original logger should be kept the same
            report = underlying_logger_const.report();
            multi_threaded_report_checker_helper(n_threads, report);
        }
    }

    SUBCASE("Flush related tests") {
        auto flushed_report = std::string{};
        auto flush_handler = [&flushed_report](std::string_view buffer) { flushed_report = buffer; };
        auto throwing_flush_handler = [](std::string_view /*buffer*/) {
            throw RuntimeFlushException(msg_flush_failed);
        };

        SUBCASE("Flush report - No handler") {
            MultiThreadedTextLogger multi_threaded_logger{};
            auto report = multi_threaded_logger.report();
            CHECK(report.empty());

            // flushing empty reports without a handler is okay
            multi_threaded_logger.flush();
            CHECK(report.empty());

            run_parallel_jobs(arbitrary_n_threads, multi_threaded_logger, single_thread_job);
            report = multi_threaded_logger.report();
            multi_threaded_report_checker_helper(arbitrary_n_threads, report);

            // flushinig non-empty report without a handler should clear the report
            multi_threaded_logger.flush();
            report = multi_threaded_logger.report();
            CHECK(report.empty()); // report should be cleared after flush
        }

        SUBCASE("Flush report - With handler that doesn't throw") {
            MultiThreadedTextLogger multi_threaded_logger{flush_handler};
            auto report = multi_threaded_logger.report();
            CHECK(report.empty());

            run_parallel_jobs(arbitrary_n_threads, multi_threaded_logger, single_thread_job);
            report = multi_threaded_logger.report();
            multi_threaded_report_checker_helper(arbitrary_n_threads, report);

            multi_threaded_logger.flush();
            report = multi_threaded_logger.report();
            CHECK(report.empty()); // report should be cleared after flush
            multi_threaded_report_checker_helper(
                arbitrary_n_threads, flushed_report); // flushed report should have the contents of the original report
        }

        SUBCASE("Flush report - With handler that throws") {
            MultiThreadedTextLogger multi_threaded_logger{throwing_flush_handler};

            run_parallel_jobs(arbitrary_n_threads, multi_threaded_logger, single_thread_job);
            auto report = multi_threaded_logger.report();
            multi_threaded_report_checker_helper(arbitrary_n_threads, report);

            // flushing should throw and report should be cleared even if handler throws
            CHECK_THROWS_AS(multi_threaded_logger.flush(), RuntimeFlushException);
            report = multi_threaded_logger.report();
            CHECK(report.empty()); // report should be cleared even if handler throws
        }

        SUBCASE("Children can't flush with handler") {
            MultiThreadedTextLogger multi_threaded_logger{flush_handler};
            auto report = multi_threaded_logger.report();
            CHECK(report.empty());
            multi_threaded_logger.log(unknown, msg_extra_log_one);
            report = multi_threaded_logger.report();
            CHECK(report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_extra_log_one)) !=
                  std::string_view::npos);

            auto child_logger_ptr = multi_threaded_logger.create_child();
            auto& child_logger = dynamic_cast<TextLogger&>(*child_logger_ptr);
            auto child_report = child_logger.report();
            CHECK(child_report.empty());
            child_logger.log(msg_extra_log_two);
            child_report = child_logger.report();
            CHECK(child_report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_extra_log_two)) !=
                  std::string_view::npos);

            report = multi_threaded_logger.report();
            CHECK(report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_extra_log_one)) !=
                  std::string_view::npos); // parent report should not be affected by child logs

            // child loggers should not be able to flush with a flush handler, but instead just clear their report and
            // not affect the parent's report or the flushed report
            CHECK_NOTHROW(child_logger.flush());
            child_report = child_logger.report();
            CHECK(child_report.empty());
            CHECK(flushed_report.empty());
            report = multi_threaded_logger.report();
            CHECK(report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_extra_log_one)) !=
                  std::string_view::npos);
            CHECK(report.find(std::format("ns] Tag:{}: {}\n", std::to_underlying(unknown), msg_extra_log_two)) ==
                  std::string_view::npos);
        }
    }
}
} // namespace power_grid_model::common::logging
