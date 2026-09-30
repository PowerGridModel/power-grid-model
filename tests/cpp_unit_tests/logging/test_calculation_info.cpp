// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/calculation_info.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>

#include <doctest/doctest.h>

#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace power_grid_model::common::logging {
namespace {
constexpr Idx arbitrary_n_threads = Idx{7};

double max_thread_value(Idx n_threads, Idx multiplier) {
    return static_cast<double>(n_threads * n_threads * multiplier);
}

void logger_helper(Logger& logger, Idx n_threads = Idx{1}) {
    using enum LogEvent;
    logger.log(max_num_iter, 5.0); // max value if single thread
    logger.log(total, Idx{1});
    logger.log(total); // should be ignored
    logger.log(max_num_iter, Idx{2});
    logger.log(iterative_pf_solver_max_num_iter, Idx{4});
    logger.log(math_solver, 1.0);
    logger.log(total, 1.0);
    logger.log(max_num_iter, 3.0 * static_cast<double>(n_threads));                    // max value if multiple threads
    logger.log(iterative_pf_solver_max_num_iter, max_thread_value(n_threads, Idx{7})); // max value
    logger.log(total, Idx{1});
    logger.log(build_model, "should be ignored"); // should be ignored
    logger.log(unknown, 1.0);                     // should be ignored
    logger.log(preprocess_measured_value, Idx{1});
}

void report_checker_helper(auto& report, Idx n_threads = Idx{1}) {
    using enum LogEvent;
    auto const some_value = static_cast<double>(n_threads); // arbitrary choice that also keeps track of input args
    CHECK(report.size() == 5);
    CHECK(report.at(total) == doctest::Approx(3.0 * some_value));
    CHECK(report.at(math_solver) == doctest::Approx(1.0 * some_value));
    CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0 * some_value));
    CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(max_thread_value(n_threads, Idx{7})));
    if (n_threads == 1) {
        CHECK(report.at(max_num_iter) == doctest::Approx(5.0));
    } else {
        CHECK(report.at(max_num_iter) == doctest::Approx(3.0 * some_value));
    }
}

void run_parallel_jobs(Idx n_threads, functor_c auto job) {
    std::vector<std::jthread> threads;
    threads.reserve(n_threads);
    for ([[maybe_unused]] Idx const i : IdxRange{n_threads}) {
        threads.emplace_back(job, n_threads);
    }
    capturing::into_the_void(job);
}
} // namespace

TEST_CASE("Test CalculationInfo") {
    CalculationInfo log{};

    SUBCASE("Log and report") {
        auto report = log.report();
        CHECK(report.empty());

        logger_helper(log);

        report = log.report();
        report_checker_helper(report);
    }

    SUBCASE("Clear report") {
        log.clear_content();
        auto clean_report = log.report();
        CHECK(clean_report.empty());

        logger_helper(log);
        log.clear_content();
        clean_report = log.report();
        CHECK(clean_report.empty());
    }

    SUBCASE("Merge into itself") {
        logger_helper(log);
        auto report = log.report();
        report_checker_helper(report);

        // merging into itself should not change the report
        log.merge_into(log);
        report = log.report();
        report_checker_helper(report);
    }

    SUBCASE("Merge into empty CalculationInfo") {
        logger_helper(log);

        CalculationInfo other_log{};
        auto report = other_log.report();
        CHECK(report.empty());

        log.merge_into(other_log);
        report = other_log.report();
        report_checker_helper(report);
    }

    SUBCASE("Merge into non-empty-different CalculationInfo") {
        logger_helper(log);

        CalculationInfo other_log{};
        using enum LogEvent;
        other_log.log(total, 2.0);
        other_log.log(scenario_exception, 13.0);
        other_log.log(iterative_pf_solver_max_num_iter, Idx{10});

        log.merge_into(other_log);

        auto const& report = other_log.report();
        CHECK(report.size() == 6);
        CHECK(report.at(total) == doctest::Approx(3.0 + 2.0));
        CHECK(report.at(scenario_exception) == doctest::Approx(13.0));
        CHECK(report.at(math_solver) == doctest::Approx(1.0));
        CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0));
        CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(10.0));
        CHECK(report.at(max_num_iter) == doctest::Approx(5.0));
    }

    SUBCASE("Lazy-logging is ignored") {
        bool called = false;
        auto const lazy_log = [&called] {
            called = true;
            return "called";
        };
        SUBCASE("Without event") {
            log.log(lazy_log);
            CHECK_FALSE(called);
            CHECK(log.report().empty());
        }
        SUBCASE("With event") {
            log.log(LogEvent::total, lazy_log);
            CHECK_FALSE(called);
            CHECK(log.report().empty());
        }
    }
}

TEST_CASE("Test MultiThreadedCalculationInfo") {
    MultiThreadedCalculationInfo multi_threaded_log{};

    auto single_thread_job = [&multi_threaded_log](Idx n_threads) {
        // MultiThreadedCalculationInfo.create_child() is tested here
        auto thread_logger_ptr = multi_threaded_log.create_child();
        Logger& thread_logger = *thread_logger_ptr;

        logger_helper(thread_logger, n_threads);
    }; // when the jthread ends, the ThreadLogger is destroyed and sync is called (tested)

    SUBCASE("Log and report through child - single threaded") {
        constexpr Idx n_threads = 1;
        run_parallel_jobs(n_threads, single_thread_job);
        report_checker_helper(multi_threaded_log.report(), n_threads);
    }

    SUBCASE("Log and report through child - multi threaded") {
        run_parallel_jobs(arbitrary_n_threads, single_thread_job);
        report_checker_helper(multi_threaded_log.report(), arbitrary_n_threads);
    }

    SUBCASE("Direct logging") {
        run_parallel_jobs(arbitrary_n_threads, single_thread_job);

        // direct logging to the MultiThreadedCalculationInfo
        using enum LogEvent;
        multi_threaded_log.log(total, Idx{1});
        multi_threaded_log.log(math_solver, "should be ignored");
        multi_threaded_log.log(preprocess_measured_value, 2.0);
        multi_threaded_log.log(iterative_pf_solver_max_num_iter,
                               max_thread_value(arbitrary_n_threads + Idx{2}, Idx{5}));
        multi_threaded_log.log(max_num_iter);

        auto const& report = multi_threaded_log.report();
        CHECK(report.size() == 5);
        CHECK(report.at(total) == doctest::Approx((3.0 * static_cast<double>(arbitrary_n_threads)) + 1.0));
        CHECK(report.at(math_solver) == doctest::Approx(1.0 * static_cast<double>(arbitrary_n_threads)));
        CHECK(report.at(preprocess_measured_value) ==
              doctest::Approx((1.0 * static_cast<double>(arbitrary_n_threads)) + 2.0));
        CHECK(report.at(iterative_pf_solver_max_num_iter) ==
              doctest::Approx(max_thread_value(arbitrary_n_threads + Idx{2}, Idx{5})));
        CHECK(report.at(max_num_iter) == doctest::Approx(3.0 * static_cast<double>(arbitrary_n_threads)));
    }
    SUBCASE("Direct logging: Lazy-logging is ignored") {
        bool called = false;
        auto const lazy_log = [&called] {
            called = true;
            return "called";
        };
        SUBCASE("Without event") {
            multi_threaded_log.log(lazy_log);
            CHECK_FALSE(called);
            CHECK(multi_threaded_log.report().empty());
        }
        SUBCASE("With event") {
            multi_threaded_log.log(LogEvent::total, lazy_log);
            CHECK_FALSE(called);
            CHECK(multi_threaded_log.report().empty());
        }
    }

    SUBCASE("Clear report") {
        auto clean_report = multi_threaded_log.report();
        CHECK(clean_report.empty());

        run_parallel_jobs(arbitrary_n_threads, single_thread_job);
        multi_threaded_log.clear_content();
        clean_report = multi_threaded_log.report();
        CHECK(clean_report.empty());
    }

    SUBCASE("Get output snapshot") {
        logger_helper(multi_threaded_log);
        auto const expected_output = multi_threaded_log.string_report();
        std::string output;

        // Re-enter from the callback to verify get_output releases its mutex before
        // invoking user code and that the callback receives a pre-clear snapshot.
        multi_threaded_log.get_output([&output, &multi_threaded_log](std::string_view snapshot) {
            output = snapshot;
            multi_threaded_log.clear_content();
        });

        CHECK(output == expected_output);
        CHECK(multi_threaded_log.report().empty());
    }

    SUBCASE("Get output snapshot - multi threaded") {
        run_parallel_jobs(arbitrary_n_threads, single_thread_job);
        auto const expected_output = multi_threaded_log.string_report();
        std::string output;

        // Re-enter from the callback to verify get_output releases its mutex before
        // invoking user code and that the callback receives a pre-clear snapshot.
        multi_threaded_log.get_output([&output, &multi_threaded_log](std::string_view snapshot) {
            output = snapshot;
            multi_threaded_log.clear_content();
        });

        CHECK(output == expected_output);
        CHECK(multi_threaded_log.report().empty());
    }

    SUBCASE("Getters of underlying CalculationInfo") {
        auto const n_threads = static_cast<Idx>(std::jthread::hardware_concurrency());
        run_parallel_jobs(n_threads, single_thread_job);

        SUBCASE("Log and report - Non-const getter") {
            using enum LogEvent;

            CalculationInfo& log = multi_threaded_log.get();
            logger_helper(log);
            auto report = log.report();

            // arbitrary choice that also keeps track of input args
            auto const some_value_a = static_cast<double>(n_threads);
            auto const some_value_b = static_cast<double>(n_threads + 1);

            CHECK(report.size() == 5);
            CHECK(report.at(total) == doctest::Approx(3.0 * some_value_b));
            CHECK(report.at(math_solver) == doctest::Approx(1.0 * some_value_b));
            CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0 * some_value_b));
            CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(max_thread_value(n_threads, Idx{7})));
            CHECK(report.at(max_num_iter) ==
                  doctest::Approx(3.0 * some_value_a)); // the + 1 from the log doesn't contribute
        }

        SUBCASE("Report - Const getter") {
            CalculationInfo const& log = multi_threaded_log.get();
            auto report = log.report();
            report_checker_helper(report, n_threads);
        }

        SUBCASE("Merge into another CalculationInfo") {
            using enum LogEvent;

            CalculationInfo const& log_const = multi_threaded_log.get();
            CalculationInfo& log_non_const = multi_threaded_log.get();
            CalculationInfo log_new{};

            log_const.merge_into(log_new);
            log_new.merge_into(log_non_const);

            auto report = log_non_const.report();

            // arbitrary choice that also keeps track of input args
            auto const some_value_a = static_cast<double>(n_threads);
            auto const some_value_b = static_cast<double>(n_threads * 2);

            CHECK(report.size() == 5);
            CHECK(report.at(total) == doctest::Approx(3.0 * some_value_b));
            CHECK(report.at(math_solver) == doctest::Approx(1.0 * some_value_b));
            CHECK(report.at(preprocess_measured_value) == doctest::Approx(1.0 * some_value_b));
            CHECK(report.at(iterative_pf_solver_max_num_iter) == doctest::Approx(max_thread_value(n_threads, Idx{7})));
            CHECK(report.at(max_num_iter) == doctest::Approx(3.0 * some_value_a));
        }
    }

    SUBCASE("Child copy and move semantics") {
        auto thread_logger_ptr = multi_threaded_log.create_child();
        Logger& thread_logger = *thread_logger_ptr;

        logger_helper(thread_logger);
        auto report = multi_threaded_log.report();
        CHECK(report.empty());

        SUBCASE("Copy constructor") {
            auto thread_logger_copy{
                dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger const&>(thread_logger)};
            report = multi_threaded_log.report();
            CHECK(report.empty());

            thread_logger_copy.sync();
            report = multi_threaded_log.report();
            report_checker_helper(report);
        }

        SUBCASE("Copy assignment") {
            auto thread_logger_copy =
                dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger const&>(thread_logger);
            report = multi_threaded_log.report();
            CHECK(report.empty());

            thread_logger_copy.sync();
            report = multi_threaded_log.report();
            report_checker_helper(report);
        }

        SUBCASE("Move constructor") {
            auto thread_logger_moved{
                std::move(dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger))};
            report = multi_threaded_log.report();
            CHECK(report.empty());

            thread_logger_moved.sync();
            report = multi_threaded_log.report();
            report_checker_helper(report);
        }

        SUBCASE("Move assignment") {
            auto thread_logger_moved =
                std::move(dynamic_cast<MultiThreadedLoggerImpl<CalculationInfo>::ThreadLogger&>(thread_logger));
            report = multi_threaded_log.report();
            CHECK(report.empty());

            thread_logger_moved.sync();
            report = multi_threaded_log.report();
            report_checker_helper(report);
        }
    }
}
} // namespace power_grid_model::common::logging
