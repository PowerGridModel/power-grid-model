// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/calculation_info.hpp>

#include <thread>
#include <utility>

#include <doctest/doctest.h>

namespace power_grid_model::common::logging {
namespace {
constexpr Idx arbitrary_n_threads = Idx{7};

Idx some_func(Idx n_threads, Idx multiplier) { return n_threads * n_threads * multiplier; }

void logger_helper(Logger& logger, Idx n_threads = Idx{1}) {
    using enum LogEvent;
    logger.log(max_num_iter, 5.0); // max value if single thread
    logger.log(total, Idx{1});
    logger.log(total); // should be ignored
    logger.log(max_num_iter, Idx{2});
    logger.log(iterative_pf_solver_max_num_iter, Idx{4});
    logger.log(math_solver, 1.0);
    logger.log(total, 1.0);
    logger.log(max_num_iter, 3.0 * static_cast<double>(n_threads));             // max value if multiple threads
    logger.log(iterative_pf_solver_max_num_iter, some_func(n_threads, Idx{7})); // max value
    logger.log(total, Idx{1});
    logger.log(build_model, "should be ignored"); // should be ignored
    logger.log(unknown, 1.0);                     // should be ignored
    logger.log(preprocess_measured_value, Idx{1});
}

void report_checker_helper(auto& report, Idx n_threads = Idx{1}) {
    using enum LogEvent;
    CHECK(report.size() == 5);
    CHECK(report.at(total) == doctest::Approx(3.0 * static_cast<double>(n_threads)));
    CHECK(report.at(math_solver) == doctest::Approx(1.0 * static_cast<double>(n_threads)));
    CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0 * static_cast<double>(n_threads)));
    CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(some_func(n_threads, Idx{7})));
    if (n_threads == 1) {
        CHECK(report.at(max_num_iter) == doctest::Approx(5.0));
    } else {
        CHECK(report.at(max_num_iter) == doctest::Approx(3.0 * static_cast<double>(n_threads)));
    }
}

template <typename JobFn>
    requires std::invocable<JobFn, Idx>
void run_parallel_jobs(Idx n_threads, JobFn&& job) {
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

    SUBCASE("Merge into itself") {
        logger_helper(info);
        auto report = info.report();
        report_checker_helper(report);

        // merging into itself should not change the report
        info.merge_into(info);
        report = info.report();
        report_checker_helper(report);
    }

    SUBCASE("Merge into empty CalculationInfo") {
        logger_helper(info);

        CalculationInfo other_info{};
        auto report = other_info.report();
        CHECK(report.empty());

        info.merge_into(other_info);
        report = other_info.report();
        report_checker_helper(report);
    }

    SUBCASE("Merge into non-empty-different CalculationInfo") {
        logger_helper(info);

        CalculationInfo other_info{};
        using enum LogEvent;
        other_info.log(total, 2.0);
        other_info.log(scenario_exception, 13.0);
        other_info.log(iterative_pf_solver_max_num_iter, Idx{10});

        info.merge_into(other_info);

        auto report = other_info.report();
        CHECK(report.size() == 6);
        CHECK(report.at(total) == doctest::Approx(3.0 + 2.0));
        CHECK(report.at(scenario_exception) == doctest::Approx(13.0));
        CHECK(report.at(math_solver) == doctest::Approx(1.0));
        CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0));
        CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(10.0));
        CHECK(report.at(max_num_iter) == doctest::Approx(5.0));
    }
}

TEST_CASE("Test MultiThreadedCalculationInfo") {
    MultiThreadedCalculationInfo multi_threaded_info{};

    auto single_thread_job = [&multi_threaded_info](Idx n_threads) {
        // MultiThreadedCalculationInfo.create_child() is tested here
        auto thread_logger_ptr = multi_threaded_info.create_child();
        Logger& thread_logger = *thread_logger_ptr;

        logger_helper(thread_logger, n_threads);
    }; // when the jthread ends, the ThreadLogger is destroyed and sync is called (tested)

    SUBCASE("Log and report through child - single threaded") {
        constexpr Idx const n_threads = 1;
        run_parallel_jobs(n_threads, single_thread_job);
        report_checker_helper(multi_threaded_info.report(), n_threads);
    }

    SUBCASE("Log and report through child - multi threaded") {
        run_parallel_jobs(arbitrary_n_threads, single_thread_job);
        report_checker_helper(multi_threaded_info.report(), arbitrary_n_threads);
    }

    SUBCASE("Direct logging") {
        run_parallel_jobs(arbitrary_n_threads, single_thread_job);

        // direct logging to the MultiThreadedCalculationInfo
        using enum LogEvent;
        multi_threaded_info.log(total, Idx{1});
        multi_threaded_info.log(math_solver, "should be ignored");
        multi_threaded_info.log(preprocess_measured_value, 2.0);
        multi_threaded_info.log(iterative_pf_solver_max_num_iter, some_func(arbitrary_n_threads + Idx{2}, Idx{5}));
        multi_threaded_info.log(max_num_iter);

        auto report = multi_threaded_info.report();
        CHECK(report.size() == 5);
        CHECK(report.at(total) == doctest::Approx((3.0 * static_cast<double>(arbitrary_n_threads)) + 1.0));
        CHECK(report.at(math_solver) == doctest::Approx(1.0 * static_cast<double>(arbitrary_n_threads)));
        CHECK(report.at(preprocess_measured_value) ==
              doctest::Approx((1.0 * static_cast<double>(arbitrary_n_threads)) + 2.0));
        CHECK(report.at(iterative_pf_solver_max_num_iter) ==
              doctest::Approx(some_func(arbitrary_n_threads + Idx{2}, Idx{5})));
        CHECK(report.at(max_num_iter) == doctest::Approx(3.0 * static_cast<double>(arbitrary_n_threads)));
    }

    SUBCASE("Clear report") {
        auto clean_report = multi_threaded_info.report();
        CHECK(clean_report.empty());

        run_parallel_jobs(arbitrary_n_threads, single_thread_job);
        multi_threaded_info.clear();
        clean_report = multi_threaded_info.report();
        CHECK(clean_report.empty());
    }

    SUBCASE("Getters of underlying CalculationInfo") {
        auto const n_threads = static_cast<Idx>(std::thread::hardware_concurrency());
        run_parallel_jobs(n_threads, single_thread_job);

        SUBCASE("Log and report - Non-const getter") {
            CalculationInfo& info = multi_threaded_info.get();
            logger_helper(info);
            auto report = info.report();
            using enum LogEvent;
            CHECK(report.size() == 5);
            CHECK(report.at(total) == doctest::Approx(3.0 * static_cast<double>(n_threads + 1)));
            CHECK(report.at(math_solver) == doctest::Approx(1.0 * static_cast<double>(n_threads + 1)));
            CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0 * static_cast<double>(n_threads + 1)));
            CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(some_func(n_threads, Idx{7})));
            CHECK(report.at(max_num_iter) ==
                  doctest::Approx(3.0 * static_cast<double>(n_threads))); // the + 1 from the info doesn't contribute
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
            using enum LogEvent;
            CHECK(report.at(total) == doctest::Approx(3.0 * static_cast<double>(n_threads * 2)));
            CHECK(report.at(math_solver) == doctest::Approx(1.0 * static_cast<double>(n_threads * 2)));
            CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0 * static_cast<double>(n_threads * 2)));
            CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(some_func(n_threads, Idx{7})));
            CHECK(report.at(max_num_iter) == doctest::Approx(3.0 * static_cast<double>(n_threads)));
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
                dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger const&>(thread_logger)};
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_copy.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }

        SUBCASE("Copy assignment") {
            auto thread_logger_copy =
                dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger const&>(thread_logger);
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_copy.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }

        SUBCASE("Move constructor") {
            auto thread_logger_moved{
                std::move(dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger))};
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_moved.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }

        SUBCASE("Move assignment") {
            auto thread_logger_moved =
                std::move(dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger));
            report = multi_threaded_info.report();
            CHECK(report.empty());

            thread_logger_moved.sync();
            report = multi_threaded_info.report();
            report_checker_helper(report);
        }
    }
}
} // namespace power_grid_model::common::logging
