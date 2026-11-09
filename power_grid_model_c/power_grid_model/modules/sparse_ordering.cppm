// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

import power_grid_model.common.common;
import power_grid_model.common.counting_iterator;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/sparse_ordering.hpp>

export module power_grid_model.sparse_ordering;

export namespace power_grid_model {
using power_grid_model::minimum_degree_ordering;
} // namespace power_grid_model

export namespace power_grid_model::detail {
using power_grid_model::detail::in_graph;
}
