// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <concepts>
#include <cstddef>
#include <iterator>

import power_grid_model.common.common;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/container_fwd.hpp>

export module power_grid_model.container_fwd;

export namespace power_grid_model::common {
using power_grid_model::common::component_container_c;
using power_grid_model::common::storagable_component_container_c;
} // namespace power_grid_model::common
