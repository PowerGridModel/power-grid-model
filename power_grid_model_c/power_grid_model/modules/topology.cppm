// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <boost/container/small_vector.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/pending/property.hpp>
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
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.index_mapping;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/topology.hpp>

export module power_grid_model.topology;

export namespace power_grid_model::topology {
using power_grid_model::topology::Topology;
} // namespace power_grid_model::topology
