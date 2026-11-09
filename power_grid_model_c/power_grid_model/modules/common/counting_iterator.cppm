// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <ranges>

import power_grid_model.common.common;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/common/counting_iterator.hpp>

export module power_grid_model.common.counting_iterator;

export namespace power_grid_model {
using power_grid_model::enumerate;
using power_grid_model::IdxCount;
using power_grid_model::IdxRange;
} // namespace power_grid_model
