// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.calculation_info;
import power_grid_model.common.logging;
import power_grid_model.common.timer;
#else
#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/timer.hpp>
#endif

#include <doctest/doctest.h>

#include <chrono>
#include <concepts>
#include <utility>

namespace power_grid_model {
static_assert(std::same_as<Stopwatch, std::chrono::steady_clock>);
static_assert(std::same_as<Duration, std::chrono::duration<double>>);

TEST_CASE("Timer") {
    CalculationInfo info;
    {
        Timer const timer{info, LogEvent::total};
    }
    CHECK(info.report().contains(LogEvent::total));
    CHECK(info.report().at(LogEvent::total) >= 0.0);

    SUBCASE("Default-constructed and moved-from timers do not log") {
        info.clear_content();
        Timer timer;
        Timer moved_to{info, LogEvent::prepare};
        Timer moved_from = std::move(moved_to);
        moved_from.stop();
        CHECK(info.report().size() == 1);
    }
}
} // namespace power_grid_model
