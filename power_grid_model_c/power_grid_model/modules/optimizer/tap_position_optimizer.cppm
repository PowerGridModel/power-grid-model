// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

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
