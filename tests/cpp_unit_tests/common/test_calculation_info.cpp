// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.calculation_info;
import power_grid_model.common.common;
import power_grid_model.common.logging;
#else
#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/logging.hpp>
#endif

#include <doctest/doctest.h>

#include <concepts>

namespace power_grid_model {
static_assert(std::same_as<CalculationInfo, common::logging::CalculationInfo>);
static_assert(std::same_as<MultiThreadedCalculationInfo, common::logging::MultiThreadedCalculationInfo>);

TEST_CASE("CalculationInfo") {
    CalculationInfo info;
    info.log(LogEvent::total, 1.0);
    info.log(LogEvent::total, 2.0);
    info.log(LogEvent::max_num_iter, Idx{3});
    info.log(LogEvent::max_num_iter, Idx{2});

    CHECK(info.report().at(LogEvent::total) == doctest::Approx(3.0));
    CHECK(info.report().at(LogEvent::max_num_iter) == doctest::Approx(3.0));

    info.clear_content();
    CHECK(info.report().empty());
}

TEST_CASE("MultiThreadedCalculationInfo") {
    MultiThreadedCalculationInfo info;
    {
        auto child = info.create_child();
        child->log(LogEvent::total, 1.5);
    }
    CHECK(info.report().at(LogEvent::total) == doctest::Approx(1.5));
}
} // namespace power_grid_model
