// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/calculation_info.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace common::logging {
TEST_CASE("Test CalculationInfo") {
    CalculationInfo info{};

    SUBCASE("Basic logging and reporting") {
        // logging various events
        info.log(LogEvent::max_num_iter, 5.0); // max value
        info.log(LogEvent::total, Idx{1});
        info.log(LogEvent::total); // should be ignored
        info.log(LogEvent::max_num_iter, Idx{2});
        info.log(LogEvent::iterative_pf_solver_max_num_iter, Idx{4});
        info.log(LogEvent::math_solver, 1.0);
        info.log(LogEvent::total, 1.0);
        info.log(LogEvent::max_num_iter, 3.0);
        info.log(LogEvent::iterative_pf_solver_max_num_iter, Idx{7}); // max value
        info.log(LogEvent::total, Idx{1});
        info.log(LogEvent::build_model, "should be ignored"); // should be ignored
        info.log(LogEvent::unknown, 1.0);                     // should be ignored
        info.log(LogEvent::preprocess_measured_value, Idx{1});

        // check report
        auto report = info.report();

        auto report_checker = [](auto& report) {
            CHECK(report.size() == 5);
            CHECK(report.at(LogEvent::total) == doctest::Approx(3.0));
            CHECK(report.at(LogEvent::math_solver) == doctest::Approx(1.0));
            CHECK(report.at(LogEvent::preprocess_measured_value) == doctest::Approx(1.0));
            CHECK(report.at(LogEvent::max_num_iter) == doctest::Approx(5.0));
            CHECK(report.at(LogEvent::iterative_pf_solver_max_num_iter) == doctest::Approx(7.0));
        };
        report_checker(report);

        SUBCASE("Clear report") {
            info.clear();
            auto clean_report = info.report();
            CHECK(clean_report.empty());
        }

        SUBCASE("Merge reports") {
            info.merge_into(info); // merging into itself should do nothing
            auto self_merged_report = info.report();
            report_checker(self_merged_report);

            CalculationInfo other_info{};
            auto other_report = other_info.report();
            CHECK(other_report.empty());

            info.merge_into(other_info);
            auto other_merged_report = other_info.report();
            report_checker(other_merged_report);
        }
    }
}
} // namespace common::logging
} // namespace power_grid_model
