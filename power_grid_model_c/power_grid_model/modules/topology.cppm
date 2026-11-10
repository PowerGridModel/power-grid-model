// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <array>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

import power_grid_model.common.typing;
import power_grid_model.sparse_ordering;
import power_grid_model.common.common;
import power_grid_model.common.counting_iterator;
import power_grid_model.calculation_parameters;
import power_grid_model.common.enums;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.index_mapping;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/topology.hpp>

export module power_grid_model.topology;

export namespace power_grid_model::topology {
using power_grid_model::topology::Topology;
} // namespace power_grid_model::topology
