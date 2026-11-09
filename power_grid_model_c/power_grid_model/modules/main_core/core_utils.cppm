// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

import power_grid_model.common.common;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/main_core/core_utils.hpp>

export module power_grid_model.main_core.core_utils;

export namespace power_grid_model::main_core::utils {
using power_grid_model::main_core::utils::invalid_index;
using power_grid_model::main_core::utils::run_functor_with_tuple_return_array;
using power_grid_model::main_core::utils::run_functor_with_tuple_return_void;
using power_grid_model::main_core::utils::sequential;
} // namespace power_grid_model::main_core::utils
