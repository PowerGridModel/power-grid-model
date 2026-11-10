// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

import power_grid_model.common.common;
import power_grid_model.common.component_list;
import power_grid_model.common.exception;
import power_grid_model.common.iterator_facade;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/container.hpp>

export module power_grid_model.container;

export namespace power_grid_model {
using power_grid_model::Container;
} // namespace power_grid_model

export namespace power_grid_model::container_impl {
using power_grid_model::container_impl::get_cls_pos_v;
using power_grid_model::container_impl::get_sub_cls_pos_v;
using power_grid_model::container_impl::get_type_index;
} // namespace power_grid_model::container_impl
