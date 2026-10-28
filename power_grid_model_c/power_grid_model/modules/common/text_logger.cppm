// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/text_logger.hpp>

export module power_grid_model.common.text_logger;

export namespace power_grid_model {
namespace common::logging {
using power_grid_model::common::logging::MultiThreadedTextLogger;
using power_grid_model::common::logging::TextLogger;
} // namespace common::logging

using power_grid_model::MultiThreadedTextLogger;
using power_grid_model::TextLogger;
} // namespace power_grid_model
