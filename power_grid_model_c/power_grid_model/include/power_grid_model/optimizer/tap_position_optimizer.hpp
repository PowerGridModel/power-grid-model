// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base_optimizer.hpp"

#include "../all_components.hpp"
#include "../auxiliary/dataset.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../main_core/state_queries.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <functional>
#include <queue>
#include <vector>

namespace power_grid_model::optimizer {
namespace tap_position_optimizer {

namespace detail = power_grid_model::optimizer::detail;

using TrafoGraphIdx = Idx;
using EdgeWeight = int64_t;
constexpr auto infty = std::numeric_limits<Idx>::max();
constexpr Idx2D unregulated_idx = {-1, -1};

struct TrafoGraphVertex {
    bool is_source{};
};

struct TrafoGraphEdge {
    Idx2D regulated_idx{};
    EdgeWeight weight{};

    bool operator==(const TrafoGraphEdge& other) const {
        return regulated_idx == other.regulated_idx && weight == other.weight;
    } // thanks boost

    auto operator<=>(const TrafoGraphEdge& other) const {
        if (auto cmp = weight <=> other.weight; cmp != 0) { // NOLINT(modernize-use-nullptr)
            return cmp;
        }
        if (auto cmp = regulated_idx.group <=> other.regulated_idx.group; cmp != 0) { // NOLINT(modernize-use-nullptr)
            return cmp;
        }
        return regulated_idx.pos <=> other.regulated_idx.pos;
    }
};

using TrafoGraphEdges = std::vector<std::pair<TrafoGraphIdx, TrafoGraphIdx>>;
using TrafoGraphEdgeProperties = std::vector<TrafoGraphEdge>;
using RankedTransformerGroups = std::vector<Idx2D>;

struct RegulatedObjects {
    std::set<Idx> transformers{};
    std::set<Idx> transformers3w{};
};

using TransformerGraph = boost::compressed_sparse_row_graph<boost::directedS, TrafoGraphVertex, TrafoGraphEdge,
                                                            boost::no_property, TrafoGraphIdx, TrafoGraphIdx>;

template <std::derived_from<ThreeWindingTransformer> Component, class ComponentContainer>
    requires main_core::model_component_state_c<main_core::MainModelState, ComponentContainer, Component>
constexpr void add_edge(main_core::MainModelState<ComponentContainer> const& state,
                        RegulatedObjects const& regulated_objects, TrafoGraphEdges& edges,
                        TrafoGraphEdgeProperties& edge_props) {
    // Only add idx2d to one of the 2 directional edges of 3 winding transformer which ensures a single ranking
    bool assigned_idx{};
    using enum Branch3Side;
    std::array<std::tuple<Branch3Side, Branch3Side>, 3> const branch3_combinations{
        {{side_1, side_2}, {side_2, side_3}, {side_1, side_3}}};
    for (auto const& transformer3w : state.components.template citer<ThreeWindingTransformer>()) {
        for (auto const& [first_side, second_side] : branch3_combinations) {
            if (!transformer3w.status(first_side) || !transformer3w.status(second_side)) {
                continue;
            }
            auto const& from_node = transformer3w.node(first_side);
            auto const& to_node = transformer3w.node(second_side);

            auto const tap_at_first_side = transformer3w.tap_side() == first_side;
            auto const single_direction_condition = regulated_objects.transformers3w.contains(transformer3w.id()) &&
                                                    (tap_at_first_side || transformer3w.tap_side() == second_side);
            if (single_direction_condition) {
                auto const& tap_side_node = tap_at_first_side ? from_node : to_node;
                auto const& non_tap_side_node = tap_at_first_side ? to_node : from_node;
                Idx2D const regulated_idx =
                    assigned_idx ? unregulated_idx : main_core::get_component_idx_by_id(state, transformer3w.id());
                assigned_idx = true;
                add_to_edge(edges, edge_props, tap_side_node, non_tap_side_node, {regulated_idx, 1});
            } else {
                add_to_edge(edges, edge_props, from_node, to_node, {unregulated_idx, 1});
                add_to_edge(edges, edge_props, to_node, from_node, {unregulated_idx, 1});
            }
        }
    }
}

template <std::derived_from<Transformer> Component, class ComponentContainer>
    requires main_core::model_component_state_c<main_core::MainModelState, ComponentContainer, Component>
constexpr void add_edge(main_core::MainModelState<ComponentContainer> const& state,
                        RegulatedObjects const& regulated_objects, TrafoGraphEdges& edges,
                        TrafoGraphEdgeProperties& edge_props) {
    for (auto const& transformer : state.components.template citer<Transformer>()) {
        if (!transformer.from_status() || !transformer.to_status()) {
            continue;
        }
        auto const& from_node = transformer.from_node();
        auto const& to_node = transformer.to_node();

        if (regulated_objects.transformers.contains(transformer.id())) {
            auto const tap_at_from_side = transformer.tap_side() == BranchSide::from;
            auto const& tap_side_node = tap_at_from_side ? from_node : to_node;
            auto const& non_tap_side_node = tap_at_from_side ? to_node : from_node;
            if (get_component<Node>(state, tap_side_node).u_rated() <
                get_component<Node>(state, non_tap_side_node).u_rated()) {
                throw AutomaticTapCalculationError(transformer.id());
            }
            add_to_edge(edges, edge_props, tap_side_node, non_tap_side_node,
                        {main_core::get_component_idx_by_id(state, transformer.id()), 1});
        } else {
            add_to_edge(edges, edge_props, from_node, to_node, {unregulated_idx, 1});
            add_to_edge(edges, edge_props, to_node, from_node, {unregulated_idx, 1});
        }
    }
}

template <typename Component>
concept non_regulating_branch_c = std::derived_from<Link, Component> || std::derived_from<Line, Component>;

template <non_regulating_branch_c Component, class ComponentContainer>
    requires main_core::model_component_state_c<main_core::MainModelState, ComponentContainer, Component>
constexpr void add_edge(main_core::MainModelState<ComponentContainer> const& state,
                        RegulatedObjects const& /* regulated_objects */, TrafoGraphEdges& edges,
                        TrafoGraphEdgeProperties& edge_props) {
    auto const& iter = state.components.template citer<Component>();
    edges.reserve(std::distance(iter.begin(), iter.end()) * 2);
    edge_props.reserve(std::distance(iter.begin(), iter.end()) * 2);
    for (auto const& branch : iter) {
        if (!branch.from_status() || !branch.to_status()) {
            continue;
        }
        add_to_edge(edges, edge_props, branch.from_node(), branch.to_node(), {unregulated_idx, 0});
        add_to_edge(edges, edge_props, branch.to_node(), branch.from_node(), {unregulated_idx, 0});
    }
}

template <typename... ComponentTypes, main_core::main_model_state_c State>
inline auto add_edges(State const& state, RegulatedObjects const& regulated_objects, TrafoGraphEdges& edges,
                      TrafoGraphEdgeProperties& edge_props) {
    (add_edge<ComponentTypes>(state, regulated_objects, edges, edge_props), ...);
}

inline void add_to_edge(TrafoGraphEdges& edges, TrafoGraphEdgeProperties& edge_props, Idx const& start, Idx const& end,
                        TrafoGraphEdge const& edge_prop) {
    edges.emplace_back(start, end);
    edge_props.emplace_back(edge_prop);
}

template <main_core::main_model_state_c State>
inline auto retrieve_regulator_info(State const& state) -> RegulatedObjects {
    RegulatedObjects regulated_objects;
    for (auto const& regulator : state.components.template citer<TransformerTapRegulator>()) {
        if (!regulator.status()) {
            continue;
        }
        if (regulator.regulated_object_type() == ComponentType::branch) {
            regulated_objects.transformers.emplace(regulator.regulated_object());
        } else {
            regulated_objects.transformers3w.emplace(regulator.regulated_object());
        }
    }
    return regulated_objects;
}

template <main_core::main_model_state_c State>
inline auto build_transformer_graph(State const& state) -> TransformerGraph {
    TrafoGraphEdges edges;
    TrafoGraphEdgeProperties edge_props;

    const RegulatedObjects regulated_objects = retrieve_regulator_info(state);

    add_edges<Transformer, ThreeWindingTransformer, Line, Link>(state, regulated_objects, edges, edge_props);

    // build graph
    TransformerGraph trafo_graph{boost::edges_are_unsorted_multi_pass, edges.cbegin(), edges.cend(),
                                 edge_props.cbegin(),
                                 static_cast<TrafoGraphIdx>(state.components.template size<Node>())};

    BGL_FORALL_VERTICES(v, trafo_graph, TransformerGraph) { trafo_graph[v].is_source = false; }

    if (state.components.template size<Source>() == 0) {
        return trafo_graph;
    }

    // Mark sources
    for (auto const& source : state.components.template citer<Source>()) {
        // ignore disabled sources
        trafo_graph[source.node()].is_source = source.status();
    }

    return trafo_graph;
}

inline auto process_edges_dijkstra(Idx v, std::vector<EdgeWeight>& vertex_distances, TransformerGraph const& graph)
    -> void {
    using TrafoGraphElement = std::pair<EdgeWeight, TrafoGraphIdx>;
    std::priority_queue<TrafoGraphElement, std::vector<TrafoGraphElement>, std::greater<>> pq;
    vertex_distances[v] = 0;
    pq.push({0, v});

    while (!pq.empty()) {
        auto [dist, u] = pq.top();
        pq.pop();

        if (dist != vertex_distances[u]) {
            continue;
        }

        BGL_FORALL_OUTEDGES(u, e, graph, TransformerGraph) {
            auto v = boost::target(e, graph);
            const EdgeWeight weight = graph[e].weight;

            if (vertex_distances[u] + weight < vertex_distances[v]) {
                vertex_distances[v] = vertex_distances[u] + weight;
                pq.push({vertex_distances[v], v});
            }
        }
    }
}

inline auto get_edge_weights(TransformerGraph const& graph) -> TrafoGraphEdgeProperties {
    std::vector<EdgeWeight> vertex_distances(boost::num_vertices(graph), infty);
    BGL_FORALL_VERTICES(v, graph, TransformerGraph) {
        if (graph[v].is_source) {
            process_edges_dijkstra(v, vertex_distances, graph);
        }
    }

    TrafoGraphEdgeProperties result;
    BGL_FORALL_EDGES(e, graph, TransformerGraph) {
        if (graph[e].regulated_idx == unregulated_idx) {
            continue;
        }
        // result.emplace_back(graph[e].regulated_idx, vertex_distances[boost::source(e, graph)]);
        result.push_back({graph[e].regulated_idx, vertex_distances[boost::source(e, graph)]});
    }

    return result;
}

inline auto rank_transformers(TrafoGraphEdgeProperties const& w_trafo_list) -> RankedTransformerGroups {
    auto sorted_trafos = w_trafo_list;

    std::sort(sorted_trafos.begin(), sorted_trafos.end(),
              [](const TrafoGraphEdge& a, const TrafoGraphEdge& b) { return a.weight < b.weight; });

    RankedTransformerGroups groups(sorted_trafos.size());
    std::ranges::transform(sorted_trafos.begin(), sorted_trafos.end(), groups.begin(),
                           [](const TrafoGraphEdge& x) { return x.regulated_idx; });
    return groups;
}

template <main_core::main_model_state_c State>
inline auto rank_transformers(State const& state) -> RankedTransformerGroups {
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
    auto optimize(State const& /*state*/, RankedTransformerGroups const& /*order*/) -> ResultType {
        // TODO(mgovers): implement outter loop tap changer
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
