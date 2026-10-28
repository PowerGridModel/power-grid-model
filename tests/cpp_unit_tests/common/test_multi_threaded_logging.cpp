// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.dummy_logging;
import power_grid_model.common.logging;
import power_grid_model.common.multi_threaded_logging;
#else
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#endif

#include <doctest/doctest.h>

#include <concepts>
#include <memory>
#include <string>
#include <string_view>

namespace power_grid_model::common::logging {
static_assert(std::same_as<NoMultiThreadedLogger, MultiThreadedLoggerImpl<NoLogger>>);

TEST_CASE("NoMultiThreadedLogger") {
    NoMultiThreadedLogger logger;
    CHECK_FALSE(logger.should_log(LogEvent::info));

    auto child = logger.create_child();
    REQUIRE(child != nullptr);
    child->log(LogEvent::info, "message");
    child.reset(); // syncs into the parent

    std::string output{"not empty"};
    logger.get_output([&output](std::string_view view) { output = view; });
    CHECK(output.empty());
    logger.clear_content();
}
} // namespace power_grid_model::common::logging
