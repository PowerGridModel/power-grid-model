// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <chrono>
#include <concepts>
#include <format>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

import power_grid_model.common.common;
import power_grid_model.common.logging;
import power_grid_model.common.multi_threaded_logging;
import power_grid_model.common.dummy_logging;

#define PGM_CXX_MODULE_PURVIEW
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
