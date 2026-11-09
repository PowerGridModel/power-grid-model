// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <concepts>
#include <utility>

import power_grid_model.common.common;
import power_grid_model.common.logging;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/job_interface.hpp>

export module power_grid_model.job_interface;

export namespace power_grid_model {
using power_grid_model::JobInterface;
} // namespace power_grid_model
