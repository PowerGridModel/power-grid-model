// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <type_traits>

import power_grid_model.common.common;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/common/component_list.hpp>

export module power_grid_model.common.component_list;

export namespace power_grid_model {
using power_grid_model::before_in_list_c;
using power_grid_model::ComponentList;
using power_grid_model::ExtraRetrievableTypes;
using power_grid_model::IsInList;
} // namespace power_grid_model
