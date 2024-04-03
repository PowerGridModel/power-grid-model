// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base_optimizer.hpp"

#include "../all_components.hpp"
#include "../auxiliary/dataset.hpp"
#include "../common/enum.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <functional>
#include <queue>
#include <vector>

namespace power_grid_model::optimizer {
namespace tap_position_optimizer {

namespace detail = power_grid_model::optimizer::detail;

using TrafoGraphIdx = Idx;
using EdgeWeight = int64_t;
using WeightedTrafo = std::pair<Idx2D, EdgeWeight>;
using WeightedTrafoList = std::vector<WeightedTrafo>;
using RankedTransformerGroups = std::vector<std::vector<Idx2D>>;
constexpr auto infty = std::numeric_limits<Idx>::max();

struct TrafoGraphVertex {
    bool is_source{}; // is_source = true if the vertex is a source
};

struct TrafoGraphEdge {
    Idx2D pos{};
    EdgeWeight weight{};
};

using TrafoGraphEdges = std::vector<std::pair<TrafoGraphIdx, TrafoGraphIdx>>;
using TrafoGraphEdgeProperties = std::vector<TrafoGraphEdge>;

struct SourceInfo {
    Idx node{};
    IntS status{};
};

struct NonTransformerBranchInfo {
    BranchIdx nodes{};
    BranchConnected status{};
};

struct TransformerInfo {
    BranchIdx nodes{};
    BranchConnected status{};
    BranchSide tap_side{};
    bool regulator_present{};
};

struct ThreeWindingTransformerInfo {
    Branch3Idx nodes{};
    Branch3Connected status{};
    Branch3Side tap_side{};
    bool regulator_present{};
};

// TODO(mgovers): investigate whether this really is the correct graph structure
using TransformerGraph = boost::compressed_sparse_row_graph<boost::directedS, TrafoGraphVertex, TrafoGraphEdge,
                                                            boost::no_property, TrafoGraphIdx, TrafoGraphIdx>;

template <main_core::main_model_state_c State> inline std::vector<SourceInfo> retrieve_source_info(State const& state) {
    auto const& iter = state.components.template citer<Source>();
    std::vector<SourceInfo> sources(iter.size());
    std::transform(iter.begin(), iter.end(), sources.begin(), [](Source const& source) {
        return SourceInfo{source.node(), source.status()};
    });
    return sources;
}

template <main_core::main_model_state_c State>
inline std::vector<ThreeWindingTransformerInfo> retrieve_transformers3w_info(State const& state) {
    auto const& iter = state.components.template citer<ThreeWindingTransformer>();
    std::vector<ThreeWindingTransformerInfo> transformers3w(iter.size());
    std::transform(iter.begin(), iter.end(), transformers3w.begin(), [](ThreeWindingTransformer const& branch3) {
        return ThreeWindingTransformerInfo{Branch3Idx{branch3.node_1(), branch3.node_2(), branch3.node_3()},
                                           Branch3Connected{static_cast<IntS>(branch3.status_1()),
                                                            static_cast<IntS>(branch3.status_2()),
                                                            static_cast<IntS>(branch3.status_3())},
                                           branch3.tap_side(), false};
    });
    return transformers3w;
}

template <main_core::main_model_state_c State>
inline std::vector<TransformerInfo> retrieve_transformer_info(State const& state) {
    auto const& iter = state.components.template citer<Transformer>();
    std::vector<TransformerInfo> transformers(iter.size());
    std::transform(iter.begin(), iter.end(), transformers.begin(), [](Transformer const& transformer) {
        return TransformerInfo{BranchIdx{transformer.from_node(), transformer.to_node()},
                               BranchConnected{transformer.from_status(), transformer.to_status()},
                               transformer.tap_side(), false};
    });
    return transformers;
}

template <main_core::main_model_state_c State>
inline std::vector<NonTransformerBranchInfo> retrieve_other_branches_info(State const& state) {
    auto const& line_iter = state.components.template citer<Line>();
    auto const& link_iter = state.components.template citer<Link>();
    std::vector<NonTransformerBranchInfo> other_branches(line_iter.size() + link_iter.size());
    std::transform(line_iter.begin(), line_iter.end(), other_branches.begin(), [](Line const& branch) {
        return NonTransformerBranchInfo{
            BranchIdx{branch.from_node(), branch.to_node()},
            BranchConnected{static_cast<IntS>(branch.from_status()), static_cast<IntS>(branch.to_status())}};
    });
    std::transform(link_iter.begin(), link_iter.end(), other_branches.begin() + line_iter.size(),
                   [](Link const& branch) {
                       return NonTransformerBranchInfo{BranchIdx{branch.from_node(), branch.to_node()},
                                                       BranchConnected{static_cast<IntS>(branch.from_status()),
                                                                       static_cast<IntS>(branch.to_status())}};
                   });
    return other_branches;
}

template <main_core::main_model_state_c State>
inline void set_regulators_info(State const& state, auto& transformers, auto& transformers3w) {
    for (auto const& regulator : state.components.template citer<TransformerTapRegulator>()) {
        if (!regulator.status()) {
            continue;
        }
        if (regulator.control_side() == ControlSide::from || regulator.control_side() == ControlSide::to) {
            transformers[state.components.template get_seq<Transformer>(regulator.regulated_object())]
                .regulator_present = true;
        } else {
            transformers3w[state.components.template get_seq<ThreeWindingTransformer>(regulator.regulated_object())]
                .regulator_present = true;
        }
    }
}

inline void create_edge(TrafoGraphEdges& edges, TrafoGraphEdgeProperties& edge_props, Idx from_idx, Idx to_idx,
                        EdgeWeight weight) {
    edges.emplace_back(static_cast<TrafoGraphIdx>(from_idx), static_cast<TrafoGraphIdx>(to_idx));
    edge_props.push_back(TrafoGraphEdge{{}, weight});
}

inline void add_transformers_to_edges(auto const& transformers, TrafoGraphEdges& edges,
                                      TrafoGraphEdgeProperties& edge_props) {
    for (auto const& transformer : transformers) {
        if (transformer.status[0] != 1 || transformer.status[1] != 1) {
            continue;
        }
        if (transformer.regulator_present) {
            Idx const from_pos = 0 ? transformer.tap_side == BranchSide::from : 1;
            Idx const to_pos = 1 ? transformer.tap_side == BranchSide::from : 0;
            create_edge(edges, edge_props, transformer.nodes[from_pos], transformer.nodes[to_pos], 1);
        } else {
            create_edge(edges, edge_props, transformer.nodes[0], transformer.nodes[1], 1);
            create_edge(edges, edge_props, transformer.nodes[1], transformer.nodes[0], 1);
        }
    }
}

inline void add_transformers3w_to_edges(auto const& transformers3w, TrafoGraphEdges& edges,
                                        TrafoGraphEdgeProperties& edge_props) {
    std::array<Branch3Side, 3> branch3_side_map = {Branch3Side::side_1, Branch3Side::side_2, Branch3Side::side_3};
    std::array<std::tuple<IntS, IntS>, 3> branch3_combinations{{{0, 1}, {1, 2}, {0, 2}}};
    for (auto const& branch3 : transformers3w) {
        for (auto const [from_pos, to_pos] : branch3_combinations) {
            if (branch3.status[from_pos] != 1 || branch3.status[to_pos] != 1) {
                continue;
            }
            if (branch3.regulator_present) {
                Idx const single_from_pos = from_pos ? branch3.tap_side == branch3_side_map[from_pos] : to_pos;
                Idx const single_to_pos = to_pos ? branch3.tap_side == branch3_side_map[from_pos] : from_pos;
                create_edge(edges, edge_props, branch3.nodes[single_from_pos], branch3.nodes[single_to_pos], 1);
            } else {
                create_edge(edges, edge_props, branch3.nodes[from_pos], branch3.nodes[to_pos], 1);
                create_edge(edges, edge_props, branch3.nodes[to_pos], branch3.nodes[from_pos], 1);
            }
        }
    }
}

inline void add_other_branches_to_edges(auto const& other_branches, TrafoGraphEdges& edges,
                                        TrafoGraphEdgeProperties& edge_props) {
    for (auto const& branch : other_branches) {
        if (branch.status[0] != 1 || branch.status[1] != 1) {
            continue;
        }
        create_edge(edges, edge_props, branch.nodes[0], branch.nodes[1], 0);
        create_edge(edges, edge_props, branch.nodes[1], branch.nodes[0], 1);
    }
}

template <main_core::main_model_state_c State>
inline auto build_transformer_graph(State const& state) -> TransformerGraph {
    // Retrieve attributes needed for graph
    auto const n_node = state.components.template citer<Node>().size();
    auto const& sources = retrieve_source_info(state);
    auto transformers = retrieve_transformer_info(state);
    auto transformers3w = retrieve_transformers3w_info(state);
    auto const& other_branches = retrieve_other_branches_info(state);
    set_regulators_info(state, transformers, transformers3w);

    // Prepare edges
    TrafoGraphEdges edges;
    TrafoGraphEdgeProperties edge_props;
    add_transformers_to_edges(transformers, edges, edge_props);
    add_transformers3w_to_edges(transformers3w, edges, edge_props);
    add_other_branches_to_edges(other_branches, edges, edge_props);

    // build graph
    TransformerGraph trafo_graph{boost::edges_are_unsorted_multi_pass, edges.cbegin(), edges.cend(),
                                 edge_props.cbegin(), static_cast<TrafoGraphIdx>(n_node)};

    // Mark sources
    BGL_FORALL_VERTICES(v, trafo_graph, TransformerGraph) { trafo_graph[v].is_source = false; }
    for (auto const& source : sources) {
        trafo_graph[source.node].is_source = source.status;
    }

    return trafo_graph;
}

inline auto process_edges_dijkstra(Idx v, std::vector<EdgeWeight>& edge_weight, std::vector<Idx2D>& edge_pos,
                                   TransformerGraph const& graph) -> void {
    using TrafoGraphElement = std::pair<EdgeWeight, TrafoGraphIdx>;
    std::priority_queue<TrafoGraphElement, std::vector<TrafoGraphElement>, std::greater<>> pq;
    edge_weight[v] = 0;
    edge_pos[v] = {v, v};
    pq.push({0, v});

    while (!pq.empty()) {
        auto [dist, u] = pq.top();
        pq.pop();

        if (dist != edge_weight[u]) {
            continue;
        }

        for (auto e : boost::make_iterator_range(boost::out_edges(u, graph))) {
            auto v = boost::target(e, graph);
            const EdgeWeight weight = graph[e].weight;

            if (edge_weight[u] + weight < edge_weight[v]) {
                edge_weight[v] = edge_weight[u] + weight;
                edge_pos[v] = graph[e].pos;
                pq.push({edge_weight[v], v});
            }
        }
    }
}

// Step 2: Initialize the rank of all vertices (transformer nodes) as infinite (INT_MAX)
// Step 3: Loop all the connected sources (status == 1)
//      a. Perform Dijkstra shortest path algorithm from the vertex with that source.
//         This is to determine the shortest path of all vertices to this particular source.
inline auto get_edge_weights(TransformerGraph const& graph) -> WeightedTrafoList {
    std::vector<EdgeWeight> edge_weight(boost::num_vertices(graph), infty);
    std::vector<Idx2D> edge_pos(boost::num_vertices(graph));

    for (auto v : boost::make_iterator_range(boost::vertices(graph))) {
        if (graph[v].is_source) {
            process_edges_dijkstra(v, edge_weight, edge_pos, graph);
        }
    }

    WeightedTrafoList result;
    for (size_t i = 0; i < edge_weight.size(); ++i) {
        result.emplace_back(edge_pos[i], edge_weight[i]);
    }

    return result;
}

// Step 4: Loop all transformers with automatic tap changers, including the transformers which are not
//         fully connected
//   a.Rank of the transformer <-
//       i. Infinity(INT_MAX), if tap side of the transformer is disconnected.
//          The transformer regulation should be ignored
//       ii.Rank of the vertex at the tap side of the transformer, if tap side of the transformer is connected
inline auto transformer_disconnected(Idx2D const& /*pos*/) -> bool {
    // <TODO: jguo> waiting for the functionalities in step 1 to be implemented
    return false;
}

inline auto rank_transformers(WeightedTrafoList const& w_trafo_list) -> RankedTransformerGroups {
    auto sorted_trafos = w_trafo_list;

    for (auto& trafo : sorted_trafos) {
        if (transformer_disconnected(trafo.first)) {
            trafo.second = infty;
        }
    }

    std::sort(sorted_trafos.begin(), sorted_trafos.end(),
              [](const WeightedTrafo& a, const WeightedTrafo& b) { return a.second < b.second; });

    RankedTransformerGroups groups;
    for (const auto& trafo : sorted_trafos) {
        if (groups.empty() || groups.back().back().pos != trafo.second) {
            groups.push_back(std::vector<Idx2D>{trafo.first});
        } else {
            groups.back().push_back(trafo.first);
        }
    }
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
