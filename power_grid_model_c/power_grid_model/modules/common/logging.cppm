// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/logging.hpp>

export module power_grid_model.common.logging;

export namespace power_grid_model {
namespace common::logging {
using power_grid_model::common::logging::LazyLoggingFn;
using power_grid_model::common::logging::LogEvent;
using power_grid_model::common::logging::Logger;
using power_grid_model::common::logging::MultiThreadedLogger;
} // namespace common::logging

using power_grid_model::LogEvent;
using power_grid_model::Logger;
} // namespace power_grid_model
