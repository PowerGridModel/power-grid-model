// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <concepts>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

import power_grid_model.common.common;
import power_grid_model.common.logging;
import power_grid_model.common.multi_threaded_logging;
import power_grid_model.common.dummy_logging;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/common/calculation_info.hpp>

export module power_grid_model.common.calculation_info;

export namespace power_grid_model {
namespace common::logging {
using power_grid_model::common::logging::CalculationInfo;
using power_grid_model::common::logging::MultiThreadedCalculationInfo;
} // namespace common::logging

using power_grid_model::CalculationInfo;
using power_grid_model::MultiThreadedCalculationInfo;
} // namespace power_grid_model
