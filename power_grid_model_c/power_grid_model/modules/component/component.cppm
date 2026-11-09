// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <concepts>
#include <string_view>

import power_grid_model.common.common;
import power_grid_model.common.enums;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/component/component.hpp>

export module power_grid_model.component.component;

export namespace power_grid_model {
using power_grid_model::appliance_type_tag;
using power_grid_model::component_c;
using power_grid_model::gen_appliance_t;
using power_grid_model::is_generator_v;
using power_grid_model::load_appliance_t;
using power_grid_model::UpdateChange;
} // namespace power_grid_model
