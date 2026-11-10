// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

import power_grid_model.optimizer.base_optimizer;
import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.main_core.state;
import power_grid_model.container_fwd;
import power_grid_model.auxiliary.dataset;
import power_grid_model.auxiliary.meta_data;
import power_grid_model.common.exception;
import power_grid_model.component.edge;
import power_grid_model.component.component;
import power_grid_model.component.branch3;
import power_grid_model.component.line;
import power_grid_model.component.link;
import power_grid_model.component.node;
import power_grid_model.component.source;
import power_grid_model.component.three_winding_transformer;
import power_grid_model.component.transformer;
import power_grid_model.component.transformer_utils;
import power_grid_model.component.transformer_tap_regulator;
import power_grid_model.container;
import power_grid_model.main_core.container_queries;
import power_grid_model.all_components;
import power_grid_model.component.branch;
import power_grid_model.main_core.state_queries;
import power_grid_model.auxiliary.dataset_fwd;
import power_grid_model.auxiliary.input;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.common.component_list;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.matrix_utils;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.typing;
import power_grid_model.component.appliance;
import power_grid_model.component.asym_line;
import power_grid_model.component.base;
import power_grid_model.component.current_sensor;
import power_grid_model.component.fault;
import power_grid_model.component.generic_branch;
import power_grid_model.component.line_utils;
import power_grid_model.component.load_gen;
import power_grid_model.component.power_sensor;
import power_grid_model.component.regulator;
import power_grid_model.component.sensor;
import power_grid_model.component.shunt;
import power_grid_model.component.voltage_regulator;
import power_grid_model.component.voltage_sensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/optimizer/tap_position_optimizer.hpp>

export module power_grid_model.optimizer.tap_position_optimizer;

export namespace power_grid_model::optimizer {
namespace tap_position_optimizer {
using power_grid_model::optimizer::tap_position_optimizer::EdgeWeight;
using power_grid_model::optimizer::tap_position_optimizer::RankedTransformerGroups;
using power_grid_model::optimizer::tap_position_optimizer::TrafoGraphIdx;

using power_grid_model::optimizer::tap_position_optimizer::build_transformer_graph;
using power_grid_model::optimizer::tap_position_optimizer::get_edge_weights;
using power_grid_model::optimizer::tap_position_optimizer::i_pu;
using power_grid_model::optimizer::tap_position_optimizer::i_pu_controlled_node;
using power_grid_model::optimizer::tap_position_optimizer::infty;
using power_grid_model::optimizer::tap_position_optimizer::rank_transformers;
using power_grid_model::optimizer::tap_position_optimizer::RankIteration;
using power_grid_model::optimizer::tap_position_optimizer::TapPositionOptimizer;
using power_grid_model::optimizer::tap_position_optimizer::TrafoGraphEdge;
using power_grid_model::optimizer::tap_position_optimizer::TrafoGraphEdgeProperties;
using power_grid_model::optimizer::tap_position_optimizer::TrafoGraphEdges;
using power_grid_model::optimizer::tap_position_optimizer::TrafoGraphVertex;
using power_grid_model::optimizer::tap_position_optimizer::transformer_types_t;
using power_grid_model::optimizer::tap_position_optimizer::TransformerGraph;
using power_grid_model::optimizer::tap_position_optimizer::TransformerRanker;
using power_grid_model::optimizer::tap_position_optimizer::u_pu;
using power_grid_model::optimizer::tap_position_optimizer::u_pu_controlled_node;
using power_grid_model::optimizer::tap_position_optimizer::unregulated_idx;
} // namespace tap_position_optimizer

using power_grid_model::optimizer::TapPositionOptimizer;
} // namespace power_grid_model::optimizer
