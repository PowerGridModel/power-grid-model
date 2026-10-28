// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.logging;
import power_grid_model.common.text_logger;
#else
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/text_logger.hpp>
#endif

#include <doctest/doctest.h>

#include <concepts>
#include <string>

namespace power_grid_model {
static_assert(std::same_as<TextLogger, common::logging::TextLogger>);
static_assert(std::same_as<MultiThreadedTextLogger, common::logging::MultiThreadedTextLogger>);

TEST_CASE("TextLogger") {
    TextLogger logger;
    logger.log(LogEvent::info, "hello");
    CHECK(logger.report().find("Tag:3: hello\n") != std::string::npos);

    logger.clear_content();
    CHECK(logger.report_view().empty());
}

TEST_CASE("MultiThreadedTextLogger") {
    MultiThreadedTextLogger logger;
    {
        auto child = logger.create_child();
        child->log(LogEvent::warning, "from child");
    }
    CHECK(logger.report().find("Tag:2: from child\n") != std::string::npos);
}
} // namespace power_grid_model
