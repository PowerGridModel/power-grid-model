// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.dummy_logging;
import power_grid_model.common.logging;
#else
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/logging.hpp>
#endif

#include <doctest/doctest.h>

#include <string>

namespace power_grid_model::common::logging {
TEST_CASE("NoLogger") {
    NoLogger logger;
    CHECK_FALSE(logger.should_log(LogEvent::info));

    bool called{false};
    logger.log(LogEvent::info, [&called] {
        called = true;
        return std::string{"lazy"};
    });
    CHECK_FALSE(called);

    NoLogger destination;
    CHECK(&logger.merge_into(destination) == &destination);
}
} // namespace power_grid_model::common::logging
