// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <cassert>
#include <concepts>
#include <exception>
#include <format>
#include <iterator>
#include <ranges>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

import power_grid_model.batch_parameter;
import power_grid_model.job_interface;
import power_grid_model.common.common;
import power_grid_model.common.logging;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.exception;
import power_grid_model.common.enums;
import power_grid_model.common.timer;
import power_grid_model.common.typing;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/job_dispatch.hpp>

export module power_grid_model.job_dispatch;

export namespace power_grid_model {
using power_grid_model::JobDispatch;
} // namespace power_grid_model
