// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <concepts>

import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.component.component;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/component/transformer_utils.hpp>

export module power_grid_model.component.transformer_utils;

export namespace power_grid_model {
using power_grid_model::transformer_c;
} // namespace power_grid_model

export namespace power_grid_model {
using power_grid_model::is_valid_clock;
using power_grid_model::tap_adjust_impedance;
} // namespace power_grid_model
