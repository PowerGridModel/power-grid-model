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

struct RegulatorInfo {
    Idx regulated_branch{};
    bool status{};
    bool is_three_winding{};
};

struct SourceInfo {
    Idx node{};
    IntS status{};
};

struct NonTransformerBranchInfo {
    BranchIdx nodes{};
    BranchConnected status{};
};

struct TransformerBranchInfo {
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

template <main_core::main_model_state_c State>
inline auto build_transformer_graph(State const& state) -> TransformerGraph {
    auto const n_node = state.components.template citer<Node>().size();

    // retrieve attributes sources
    std::vector<SourceInfo> sources(state.components.template citer<Source>().size());
    std::transform(state.components.template citer<Source>().begin(), state.components.template citer<Source>().end(),
                   sources.begin(), [](Source const& source) {
                       return SourceInfo{source.node(), source.status()};
                   });

    // retrieve branch 3 info
    std::vector<ThreeWindingTransformerInfo> branch3s(
        state.components.template citer<ThreeWindingTransformer>().size());
    std::transform(state.components.template citer<ThreeWindingTransformer>().begin(),
                   state.components.template citer<ThreeWindingTransformer>().end(), branch3s.begin(),
                   [](ThreeWindingTransformer const& branch3) {
                       return ThreeWindingTransformerInfo{
                           Branch3Idx{branch3.node_1(), branch3.node_2(), branch3.node_3()},
                           Branch3Connected{static_cast<IntS>(branch3.status_1()),
                                            static_cast<IntS>(branch3.status_2()),
                                            static_cast<IntS>(branch3.status_3())},
                           branch3.tap_side(), false};
                   });

    // retrieve attributes transformers
    std::vector<TransformerBranchInfo> transformers(state.components.template citer<Transformer>().size());
    std::transform(state.components.template citer<Transformer>().begin(),
                   state.components.template citer<Transformer>().end(), transformers.begin(),
                   [](Transformer const& transformer) {
                       return TransformerBranchInfo{BranchIdx{transformer.from_node(), transformer.to_node()},
                                                    BranchConnected{transformer.from_status(), transformer.to_status()},
                                                    transformer.tap_side(), false};
                   });

    // retrieve attributes regulators
    std::vector<RegulatorInfo> regulators(state.components.template citer<TransformerTapRegulator>().size());
    std::transform(state.components.template citer<TransformerTapRegulator>().begin(),
                   state.components.template citer<TransformerTapRegulator>().end(), regulators.begin(),
                   [&state](TransformerTapRegulator const& regulator) {
                       using enum ControlSide;
                       if (regulator.control_side() == from || regulator.control_side() == to) {
                           return RegulatorInfo{
                               state.components.template get_seq<Transformer>(regulator.regulated_object()),
                               regulator.status(), false};
                       } else {
                           assert(regulator.control_side() == side_1 || regulator.control_side() == side_2 ||
                                  regulator.control_side() == side_3);
                           return RegulatorInfo{
                               state.components.template get_seq<ThreeWindingTransformer>(regulator.regulated_object()),
                               regulator.status(), true};
                       }
                   });

    // Mark regulated branches
    for (auto const& regulator : regulators) {
        if (regulator.status != 1) {
            continue;
        }
        if (!regulator.is_three_winding) {
            transformers[regulator.regulated_branch].regulator_present = true;
        } else {
            branch3s[regulator.regulated_branch].regulator_present = true;
        }
    }

    // retrieve attributes lines and links
    auto const n_lines = state.components.template citer<Line>().size();
    auto const n_links = state.components.template citer<Link>().size();
    std::vector<NonTransformerBranchInfo> other_branches(n_lines + n_links);
    std::transform(state.components.template citer<Line>().begin(), state.components.template citer<Line>().end(),
                   other_branches.begin(), [](Line const& branch) {
                       return NonTransformerBranchInfo{BranchIdx{branch.from_node(), branch.to_node()},
                                                       BranchConnected{static_cast<IntS>(branch.from_status()),
                                                                       static_cast<IntS>(branch.to_status())}};
                   });
    std::transform(state.components.template citer<Link>().begin(), state.components.template citer<Link>().end(),
                   other_branches.begin() + n_lines, [](Link const& branch) {
                       return NonTransformerBranchInfo{BranchIdx{branch.from_node(), branch.to_node()},
                                                       BranchConnected{static_cast<IntS>(branch.from_status()),
                                                                       static_cast<IntS>(branch.to_status())}};
                   });

    // Prepare edges / vertices
    std::vector<std::pair<TrafoGraphIdx, TrafoGraphIdx>> edges;
    std::vector<TrafoGraphEdge> edge_props;

    // add transformers
    for (auto const& transformer : transformers) {
        if (transformer.status[0] != 1 || transformer.status[1] != 1) {
            continue;
        }
        if (transformer.regulator_present) {
            Idx from_idx = 0 ? transformer.tap_side == BranchSide::from : 1;
            Idx to_idx = 1 ? transformer.tap_side == BranchSide::from : 0;
            edges.emplace_back(static_cast<TrafoGraphIdx>(transformer.nodes[from_idx]),
                               static_cast<TrafoGraphIdx>(transformer.nodes[to_idx]));
            edge_props.push_back(TrafoGraphEdge{1});
        } else {
            edges.emplace_back(static_cast<TrafoGraphIdx>(transformer.nodes[0]),
                               static_cast<TrafoGraphIdx>(transformer.nodes[1]));
            edge_props.push_back(TrafoGraphEdge{1});
            edges.emplace_back(static_cast<TrafoGraphIdx>(transformer.nodes[1]),
                               static_cast<TrafoGraphIdx>(transformer.nodes[0]));
            edge_props.push_back(TrafoGraphEdge{1});
        }
    }

    // k as branch number for 3-way branch
    std::array<Branch3Side, 3> branch3_side_map = {Branch3Side::side_1, Branch3Side::side_2, Branch3Side::side_3};
    std::array<std::tuple<IntS, IntS>, 3> branch3_combinations{{{0, 1}, {1, 2}, {0, 2}}};
    for (auto const& branch3 : branch3s) {
        for (auto const [from_pos, to_pos] : branch3_combinations) {
            if (branch3.status[from_pos] != 1 || branch3.status[to_pos] != 1) {
                continue;
            }
            if (branch3.regulator_present) {
                Idx single_from_pos = from_pos ? branch3.tap_side == branch3_side_map[from_pos] : to_pos;
                Idx single_to_pos = to_pos ? branch3.tap_side == branch3_side_map[from_pos] : from_pos;
                edges.emplace_back(static_cast<TrafoGraphIdx>(branch3.nodes[single_from_pos]),
                                   static_cast<TrafoGraphIdx>(branch3.nodes[single_to_pos]));
                edge_props.push_back(TrafoGraphEdge{1});
            } else {
                edges.emplace_back(static_cast<TrafoGraphIdx>(branch3.nodes[from_pos]),
                                   static_cast<TrafoGraphIdx>(branch3.nodes[to_pos]));
                edge_props.push_back(TrafoGraphEdge{1});
                edges.emplace_back(static_cast<TrafoGraphIdx>(branch3.nodes[to_pos]),
                                   static_cast<TrafoGraphIdx>(branch3.nodes[from_pos]));
                edge_props.push_back(TrafoGraphEdge{1});
            }
        }
    }

    for (auto const& branch : other_branches) {
        if (branch.status[0] != 1 || branch.status[1] != 1) {
            continue;
        }
        edges.emplace_back(static_cast<TrafoGraphIdx>(branch.nodes[0]), static_cast<TrafoGraphIdx>(branch.nodes[1]));
        edges.emplace_back(static_cast<TrafoGraphIdx>(branch.nodes[1]), static_cast<TrafoGraphIdx>(branch.nodes[0]));
        edge_props.push_back(TrafoGraphEdge{0});
        edge_props.push_back(TrafoGraphEdge{0});
    }

    // build graph
    TransformerGraph trafo_graph{boost::edges_are_unsorted_multi_pass, edges.cbegin(), edges.cend(),
                                 edge_props.cbegin(), static_cast<TrafoGraphIdx>(n_node)};
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
