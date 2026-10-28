// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.composite_logging;
import power_grid_model.common.logging;
import power_grid_model.common.multi_threaded_logging;
#else
#include <power_grid_model/common/composite_logging.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#endif

#include <doctest/doctest.h>

#include <memory>

namespace power_grid_model::common::logging {
TEST_CASE("MultiThreadedCompositeLogger") {
    MultiThreadedCompositeLogger composite;
    CHECK(composite.empty());
    CHECK_FALSE(composite.should_log(LogEvent::info));

    auto const logger = std::make_shared<NoMultiThreadedLogger>();
    composite.add(logger);
    composite.add(logger); // deduplicated
    composite.add(nullptr);
    CHECK_FALSE(composite.empty());

    auto child = composite.create_child();
    REQUIRE(child != nullptr);
    CHECK(dynamic_cast<CompositeChildLogger*>(child.get()) != nullptr);
    CHECK_FALSE(child->should_log(LogEvent::info));
    child->log(LogEvent::info, "message");
    child.reset();

    composite.remove(logger.get());
    CHECK(composite.empty());
}
} // namespace power_grid_model::common::logging
