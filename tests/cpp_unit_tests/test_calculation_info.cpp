// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/calculation_info.hpp>

#include <thread>
#include <utility>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace common::logging {
namespace {
void logger_helper(Logger& logger, Idx n_threads = Idx{1}) {
    logger.log(LogEvent::max_num_iter, 5.0); // max value if single thread
    logger.log(LogEvent::total, Idx{1});
    logger.log(LogEvent::total); // should be ignored
    logger.log(LogEvent::max_num_iter, Idx{2});
    logger.log(LogEvent::iterative_pf_solver_max_num_iter, Idx{4});
    logger.log(LogEvent::math_solver, 1.0);
    logger.log(LogEvent::total, 1.0);
    logger.log(LogEvent::max_num_iter, 3.0 * n_threads);            // max value if multiple threads
    logger.log(LogEvent::iterative_pf_solver_max_num_iter, Idx{7}); // max value
    logger.log(LogEvent::total, Idx{1});
    logger.log(LogEvent::build_model, "should be ignored"); // should be ignored
    logger.log(LogEvent::unknown, 1.0);                     // should be ignored
    logger.log(LogEvent::preprocess_measured_value, Idx{1});
}

void report_checker_helper(auto& report, Idx n_threads = Idx{1}) {
    CHECK(report.size() == 5);
    CHECK(report.at(LogEvent::total) == doctest::Approx(3.0 * n_threads));
    CHECK(report.at(LogEvent::math_solver) == doctest::Approx(1.0 * n_threads));
    CHECK(report.at(LogEvent::preprocess_measured_value) == doctest::Approx(1.0 * n_threads));
    CHECK(report.at(LogEvent::iterative_pf_solver_max_num_iter) == doctest::Approx(7.0));
    if (n_threads == 1) {
        CHECK(report.at(LogEvent::max_num_iter) == doctest::Approx(5.0));
    } else {
        CHECK(report.at(LogEvent::max_num_iter) == doctest::Approx(3.0 * n_threads));
    }
}

template <std::invocable<Idx> JobFn> void run_parallel_jobs(Idx n_threads, JobFn&& job) {
    std::vector<std::jthread> threads;
    threads.reserve(n_threads);
    for (Idx i = 0; i < n_threads; ++i) {
        threads.emplace_back(std::forward<JobFn>(job), n_threads);
    }
}
} // namespace
TEST_CASE("Test CalculationInfo") {
    CalculationInfo info{};

    SUBCASE("Log and report") {
        auto report = info.report();
        CHECK(report.empty());

        logger_helper(info);

        report = info.report();
        report_checker_helper(report);
    }

    SUBCASE("Clear report") {
        info.clear();
        auto clean_report = info.report();
        CHECK(clean_report.empty());

        logger_helper(info);
        info.clear();
        clean_report = info.report();
        CHECK(clean_report.empty());
    }

    SUBCASE("Merge reports") {
        logger_helper(info);
        info.merge_into(info);
        auto report = info.report();
        report_checker_helper(report);

        CalculationInfo other_info{};
        report = other_info.report();
        CHECK(report.empty());

        info.merge_into(other_info);
        report = other_info.report();
        report_checker_helper(report);
    }
}

TEST_CASE("Test MultiThreadedCalculationInfo") {
    // i need to test constructor, copy/move constructor and assignment, destructor for the child
    MultiThreadedCalculationInfo multi_threaded_info{};

    auto single_thread_job = [&multi_threaded_info](Idx n_threads) {
        // MultiThreadedCalculationInfo.create_child() is tested here
        auto thread_logger_ptr = multi_threaded_info.create_child();
        Logger& thread_logger = *thread_logger_ptr;

        logger_helper(thread_logger, n_threads);
    }; // when the jthread ends, the ThreadLogger is destroyed and sync is called (tested)

    SUBCASE("Log and report through child - single threaded") {
        Idx n_threads = 1;
        run_parallel_jobs(n_threads, single_thread_job);
        report_checker_helper(multi_threaded_info.report(), n_threads);
    }

    SUBCASE("Log and report through child - multi threaded") {
        Idx n_threads = 7; // arbitrary >1 value
        run_parallel_jobs(n_threads, single_thread_job);
        report_checker_helper(multi_threaded_info.report(), n_threads);
    }

    SUBCASE("Direct logging") {
        Idx n_threads = 9; // arbitrary >1 value
        run_parallel_jobs(n_threads, single_thread_job);

        // direct logging to the MultiThreadedCalculationInfo
        multi_threaded_info.log(LogEvent::total, Idx{1});
        multi_threaded_info.log(LogEvent::math_solver, "should be ignored");
        multi_threaded_info.log(LogEvent::preprocess_measured_value, 2.0);
        multi_threaded_info.log(LogEvent::iterative_pf_solver_max_num_iter, Idx{20});
        multi_threaded_info.log(LogEvent::max_num_iter);

        auto report = multi_threaded_info.report();
        CHECK(report.size() == 5);
        CHECK(report.at(LogEvent::total) == doctest::Approx((3.0 * (n_threads)) + 1.0));
        CHECK(report.at(LogEvent::math_solver) == doctest::Approx(1.0 * n_threads));
        CHECK(report.at(LogEvent::preprocess_measured_value) == doctest::Approx((1.0 * (n_threads)) + 2.0));
        CHECK(report.at(LogEvent::iterative_pf_solver_max_num_iter) == doctest::Approx(20.0));
        CHECK(report.at(LogEvent::max_num_iter) == doctest::Approx(3.0 * n_threads));
    }

    SUBCASE("Clear report") {
        Idx n_threads = 5; // arbitrary >1 value
        auto clean_report = multi_threaded_info.report();
        CHECK(clean_report.empty());

        run_parallel_jobs(n_threads, single_thread_job);
        multi_threaded_info.clear();
        clean_report = multi_threaded_info.report();
        CHECK(clean_report.empty());
    }

    SUBCASE("Getters of underlying CalculationInfo") {
        Idx n_threads = static_cast<Idx>(std::thread::hardware_concurrency());
        run_parallel_jobs(n_threads, single_thread_job);

        SUBCASE("Log and report - Non-const getter") {
            CalculationInfo& info = multi_threaded_info.get();
            logger_helper(info);
            auto report = info.report();
            CHECK(report.size() == 5);
            CHECK(report.at(LogEvent::total) == doctest::Approx(3.0 * (n_threads + 1)));
            CHECK(report.at(LogEvent::math_solver) == doctest::Approx(1.0 * (n_threads + 1)));
            CHECK(report.at(LogEvent::preprocess_measured_value) == doctest::Approx(1.0 * (n_threads + 1)));
            CHECK(report.at(LogEvent::iterative_pf_solver_max_num_iter) == doctest::Approx(7.0));
            CHECK(report.at(LogEvent::max_num_iter) ==
                  doctest::Approx(3.0 * n_threads)); // the + 1 from the info doesn't contribute
        }

        SUBCASE("Report - Const getter") {
            CalculationInfo const& info = multi_threaded_info.get();
            auto report = info.report();
            report_checker_helper(report, n_threads);
        }

        SUBCASE("Merge into another CalculationInfo") {
            CalculationInfo const& info_const = multi_threaded_info.get();
            CalculationInfo& info_non_const = multi_threaded_info.get();
            CalculationInfo info_new{};

            info_const.merge_into(info_new);
            info_new.merge_into(info_non_const);

            auto report = info_non_const.report();
            CHECK(report.size() == 5);
            CHECK(report.at(LogEvent::total) == doctest::Approx(3.0 * (n_threads * 2)));
            CHECK(report.at(LogEvent::math_solver) == doctest::Approx(1.0 * (n_threads * 2)));
            CHECK(report.at(LogEvent::preprocess_measured_value) == doctest::Approx(1.0 * (n_threads * 2)));
            CHECK(report.at(LogEvent::iterative_pf_solver_max_num_iter) == doctest::Approx(7.0));
            CHECK(report.at(LogEvent::max_num_iter) == doctest::Approx(3.0 * n_threads));
        }
    }
    SUBCASE("Child copy and move semantics") {
        auto thread_logger_ptr = multi_threaded_info.create_child();
        Logger& thread_logger = *thread_logger_ptr;

        logger_helper(thread_logger);
        auto report = multi_threaded_info.report();
        CHECK(report.empty());

        SUBCASE("Copy constructor") {
            auto thread_logger_copy{
                static_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger const&>(thread_logger)};
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_copy.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }

        SUBCASE("Copy assignment") {
            auto thread_logger_copy =
                static_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger const&>(thread_logger);
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_copy.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }

        SUBCASE("Move constructor") {
            auto thread_logger_moved{
                std::move(static_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger))};
            report = multi_threaded_info.report();
            CHECK(report.empty());

            auto& thread_logger_empty_typed =
                static_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger);
            thread_logger_empty_typed.sync();
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_moved.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }

        SUBCASE("Move assignment") {
            auto thread_logger_moved =
                std::move(static_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger));
            report = multi_threaded_info.report();
            CHECK(report.empty());

            auto& thread_logger_empty_typed =
                static_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger);
            thread_logger_empty_typed.sync();
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_moved.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }
    }
}
} // namespace common::logging
} // namespace power_grid_model
