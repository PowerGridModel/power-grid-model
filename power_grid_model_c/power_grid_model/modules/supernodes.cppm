// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/supernodes.hpp>

export module power_grid_model.supernodes;

export namespace power_grid_model::supernodes {
using power_grid_model::supernodes::reduce_topology;
using power_grid_model::supernodes::TopologicalNodeMapping;

namespace detail {
using power_grid_model::supernodes::detail::construct_reduced_topology;
using power_grid_model::supernodes::detail::create_map;
using power_grid_model::supernodes::detail::create_topological_nodes;
} // namespace detail
} // namespace power_grid_model::supernodes
