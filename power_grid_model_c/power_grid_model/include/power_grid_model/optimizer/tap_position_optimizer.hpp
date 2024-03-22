// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base_optimizer.hpp"

#include "../auxiliary/dataset.hpp"
#include "../common/enum.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <functional>
#include <queue>
#include <vector>

namespace power_grid_model::optimizer {
namespace tap_position_optimizer {

namespace detail = power_grid_model::optimizer::detail;

using TrafoGraphIdx = size_t; // Questionable: should we simply use Idx?
using EdgeWeight = int64_t;
const int infty = INT_MAX;

struct TrafoGraphVertex {
    TrafoGraphIdx status{}; // status = 1 if the vertex is a source
};

struct TrafoGraphEdge {
    Idx2D pos{};
    EdgeWeight weight{};
};

// TODO(mgovers): investigate whether this really is the correct graph structure
using TransformerGraph = boost::compressed_sparse_row_graph<boost::directedS, TrafoGraphVertex, TrafoGraphEdge,
                                                            boost::no_property, TrafoGraphIdx, TrafoGraphIdx>;

template <main_core::main_model_state_c State>
inline auto build_transformer_graph(State const& /*state*/) -> TransformerGraph {
    // TODO(mgovers): implement
    return {};
}

inline auto get_edge_weights(TransformerGraph const& graph) -> std::vector<std::pair<Idx2D, EdgeWeight>> {
    // Step 2: Initialize the rank of all vertices (transformer nodes) as infinite (INT_MAX)
    std::vector<EdgeWeight> rank(boost::num_vertices(graph), infty);
    std::vector<Idx2D> sources(boost::num_vertices(graph));

    // Step 3: Loop all the connected sources (status == 1)
    for (auto v : boost::make_iterator_range(boost::vertices(graph))) {
        if (graph[v].status == 1) {
            // a. Perform Dijkstra shortest path algorithm from the vertex with that source.
            // This is to determine the shortest path of all vertices to this particular source.
            std::priority_queue<std::pair<EdgeWeight, TrafoGraphIdx>, std::vector<std::pair<EdgeWeight, TrafoGraphIdx>>,
                                std::greater<>>
                pq;
            rank[v] = 0;
            sources[v] = {static_cast<Idx>(v), static_cast<Idx>(v)};
            pq.push({0, v});

            while (!pq.empty()) {
                auto [dist, u] = pq.top();
                pq.pop();

                if (dist != rank[u])
                    continue;

                for (auto e : boost::make_iterator_range(boost::out_edges(u, graph))) {
                    auto v = boost::target(e, graph);
                    EdgeWeight weight = graph[e].weight;

                    if (rank[u] + weight < rank[v]) {
                        rank[v] = rank[u] + weight;
                        sources[v] = {sources[u].group, static_cast<Idx>(v)};
                        pq.push({rank[v], v});
                    }
                }
            }
        }
    }

    // Convert rank vector to vector of pairs
    std::vector<std::pair<Idx2D, EdgeWeight>> result;
    for (size_t i = 0; i < rank.size(); ++i) {
        result.push_back({sources[i], rank[i]});
    }

    return result;
}

inline auto rank_transformers(std::vector<std::pair<Idx2D, EdgeWeight>> const& /*distances*/) -> std::vector<Idx2D> {
    // TODO(mgovers): rank Idx2D of transformers as listed in the container

    // Step 4: Loop all transformers with automatic tap changers, including the transformers which are not
    //         fully connected
    //   a.Rank of the transformer <-
    //       i. Infinity(INT_MAX), if tap side of the transformer is disconnected.
    //          The transformer regulation should be ignored
    //       ii.Rank of the vertex at the tap side of the transformer, if tap side of the transformer is connected
    return {};
}

template <main_core::main_model_state_c State> inline auto rank_transformers(State const& state) -> std::vector<Idx2D> {
    return rank_transformers(get_edge_weights(build_transformer_graph(state)));
}

template <typename StateCalculator, typename StateUpdater_, typename State_>
    requires detail::steady_state_calculator_c<StateCalculator, State_> &&
             std::invocable<std::remove_cvref_t<StateUpdater_>, ConstDataset const&>
class TapPositionOptimizer : public detail::BaseOptimizer<StateCalculator, State_> {
  public:
    using Base = detail::BaseOptimizer<StateCalculator, State_>;
    using typename Base::Calculator;
    using typename Base::ResultType;
    using typename Base::State;
    using StateUpdater = StateUpdater_;

    TapPositionOptimizer(Calculator calculator, StateUpdater updater, OptimizerStrategy strategy)
        : calculate_{std::move(calculator)}, update_{std::move(updater)}, strategy_{strategy} {}

    auto optimize(State const& state) -> ResultType final {
        auto const order = rank_transformers(state);
        return optimize(state, order);
    }

    constexpr auto get_strategy() { return strategy_; }

  private:
    auto optimize(State const& /*state*/, std::vector<Idx2D> const& /*order*/) -> ResultType {
        // TODO(jguo): implement outter loop tap changer
        throw PowerGridError{};
    }

    Calculator calculate_;
    StateUpdater update_;
    OptimizerStrategy strategy_;
};

} // namespace tap_position_optimizer

template <typename StateCalculator, typename StateUpdater, typename State>
using TapPositionOptimizer = tap_position_optimizer::TapPositionOptimizer<StateCalculator, StateUpdater, State>;

} // namespace power_grid_model::optimizer
