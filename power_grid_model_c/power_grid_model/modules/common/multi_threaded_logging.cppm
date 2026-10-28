// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/multi_threaded_logging.hpp>

export module power_grid_model.common.multi_threaded_logging;

export namespace power_grid_model::common::logging {
using power_grid_model::common::logging::MultiThreadedLoggerImpl;
using power_grid_model::common::logging::NoMultiThreadedLogger;
} // namespace power_grid_model::common::logging
