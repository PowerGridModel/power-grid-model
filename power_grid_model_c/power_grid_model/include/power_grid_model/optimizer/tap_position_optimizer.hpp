// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base_optimizer.hpp"

#include "../auxiliary/dataset.hpp"
#include "../auxiliary/meta_gen/update.hpp"
#include "../common/enum.hpp"
#include "../component/three_winding_transformer.hpp"
#include "../component/transformer.hpp"

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
const int infty = INT_MAX;

struct TrafoGraphVertex {
    bool is_source{}; // is_source = true if the vertex is a source
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
    // TODO(nbharambe): implement
    return {};
}

inline auto process_edges_dijkstra(Idx v, std::vector<EdgeWeight>& rank, std::vector<Idx2D>& sources,
                                   TransformerGraph const& graph) -> void {
    using TrafoGraphElement = std::pair<EdgeWeight, TrafoGraphIdx>;
    std::priority_queue<TrafoGraphElement, std::vector<TrafoGraphElement>, std::greater<>> pq;
    rank[v] = 0;
    sources[v] = {v, v};
    pq.push({0, v});

    while (!pq.empty()) {
        auto [dist, u] = pq.top();
        pq.pop();

        if (dist != rank[u]) {
            continue;
        }

        for (auto e : boost::make_iterator_range(boost::out_edges(u, graph))) {
            auto v = boost::target(e, graph);
            const EdgeWeight weight = graph[e].weight;

            if (rank[u] + weight < rank[v]) {
                rank[v] = rank[u] + weight;
                sources[v] = {sources[u].group, static_cast<Idx>(v)};
                pq.push({rank[v], v});
            }
        }
    }
}

// Step 2: Initialize the rank of all vertices (transformer nodes) as infinite (INT_MAX)
// Step 3: Loop all the connected sources (status == 1)
//      a. Perform Dijkstra shortest path algorithm from the vertex with that source.
//         This is to determine the shortest path of all vertices to this particular source.
inline auto get_edge_weights(TransformerGraph const& graph) -> WeightedTrafoList {
    std::vector<EdgeWeight> rank(boost::num_vertices(graph), infty);
    std::vector<Idx2D> sources(boost::num_vertices(graph));

    for (auto v : boost::make_iterator_range(boost::vertices(graph))) {
        if (graph[v].is_source) {
            process_edges_dijkstra(v, rank, sources, graph);
        }
    }

    WeightedTrafoList result;
    for (size_t i = 0; i < rank.size(); ++i) {
        result.emplace_back(sources[i], rank[i]);
    }

    return result;
}

// Step 4: Loop all transformers with automatic tap changers, including the transformers which are not
//         fully connected
//   a.Rank of the transformer <-
//       i. Infinity(INT_MAX), if tap side of the transformer is disconnected.
//          The transformer regulation should be ignored
//       ii.Rank of the vertex at the tap side of the transformer, if tap side of the transformer is connected
inline auto rank_transformers(WeightedTrafoList const& /*w_trafo_list*/) -> std::vector<Idx2D> { return {}; }

template <main_core::main_model_state_c State> inline auto rank_transformers(State const& state) -> std::vector<Idx2D> {
    return rank_transformers(get_edge_weights(build_transformer_graph(state)));
}

template <typename StateCalculator, typename StateUpdater_, typename State_>
    requires main_core::component_container_c<typename State_::ComponentContainer, Transformer> &&
             main_core::component_container_c<typename State_::ComponentContainer, ThreeWindingTransformer> &&
             detail::steady_state_calculator_c<StateCalculator, State_> &&
             std::invocable<std::remove_cvref_t<StateUpdater_>, ConstDataset const&>
class TapPositionOptimizer : public detail::BaseOptimizer<StateCalculator, State_> {
  public:
    using Base = detail::BaseOptimizer<StateCalculator, State_>;
    using typename Base::Calculator;
    using typename Base::ResultType;
    using typename Base::State;
    using StateUpdater = StateUpdater_;

  private:
    using ComponentContainer = typename State::ComponentContainer;

    struct UpdateBuffer {
        std::vector<TransformerUpdate> transformer_update;
        std::vector<ThreeWindingTransformerUpdate> three_winding_transformer_update;
        std::vector<Idx> transformer_rank;
        std::vector<Idx> three_winding_transformer_rank;
    };

  public:
    TapPositionOptimizer(Calculator calculator, StateUpdater updater, OptimizerStrategy strategy)
        : calculate_{std::move(calculator)}, update_{std::move(updater)}, strategy_{strategy} {}

    auto optimize(State const& state) -> ResultType final {
        auto const order = rank_transformers(state);

        auto const cache = this->cache_state(state, order);
        auto optimal_output = optimize(state, order);
        update_state(cache);
        return optimal_output;
    }

    constexpr auto get_strategy() { return strategy_; }

  private:
    // If regulation tactic is max voltage
    // * Call adjust_voltage(set_to_max)  // tap_pos = tap_min
    // If regulation tactic is min voltage
    // * Call adjust_voltage(set_to_min)  // tap_pos = tap_max
    // (If regulation tactic is none, do nothing)
    // math_result = try_power_flow_with_regulation(calculation_method=user_specified_method)
    // If regulation tactic is max voltage
    // * Call adjust_voltage(one_step_voltage_up)  // tap_pos one step towards tap_min
    // * math_result = try_power_flow_with_regulation(calculation_method=user_specified_method)
    // If regulation tactic is min voltage
    // * Call adjust_voltage(one_step_voltage_down)  // tap_pos  one step towards tap_max
    // * math_result = try_power_flow_with_regulation(calculation_method=user_specified_method)
    // Return math_result
    auto optimize(State const& state, std::vector<Idx2D> const& order) -> ResultType {

        // TODO(mgovers): implement outer loop tap changer
        auto output = calculate_(state);
    }

    void update_state(UpdateBuffer const& update_data) {
        ConstDataset const update_dataset{
            {"transformer", ConstDataPointer{update_data.transformer_update.data(),
                                             static_cast<Idx>(update_data.transformer_update.size())}},
            {"three_winding_transformer",
             ConstDataPointer{update_data.three_winding_transformer_update.data(),
                              static_cast<Idx>(update_data.three_winding_transformer_update.size())}}};

        update_(update_dataset);
    }

    auto adjust_voltage(State const& state) {}

    auto get_to_max() { return; }

    template <typename T> constexpr static T component_cache_update(State const& state, Idx2D const& index) {
        auto const& component = state.components.template get_item<T>(index);

        auto result = meta_data::get_component_nan<typename T::UpdateType>{}();
        result.id = component.id();
        result.tap_pos = component.tap_pos();

        return component.inverse(result);
    }

    static UpdateBuffer cache_state(State const& state, std::vector<Idx2D> const& order) {
        auto buffer = create_buffer(order);

        for (Idx rank : boost::counting_range(Idx{}, static_cast<Idx>(order.size()))) {
            auto const& index = order[rank];
            if (is_in_group<Transformer>(index)) {
                buffer.transformer_update = component_cache_update<Transformer>(state, index);
                buffer.transformer_rank = rank;
            }
            if (is_in_group<ThreeWindingTransformer>(index)) {
                buffer.three_winding_transformer_update = component_cache_update<ThreeWindingTransformer>(state, index);
                buffer.three_winding_transformer_rank = rank;
            }
        }

        return buffer;
    }

    template <typename T> static constexpr auto group_count(std::vector<Idx2D> const& indices) {
        return std::ranges::count_if(indices, is_in_group<T>);
    }

    template <typename T> static constexpr auto is_in_group(Idx2D const& index) {
        constexpr auto group_idx = ComponentContainer::template get_type_idx<T>();
        return index.group == group_idx;
    }

    Calculator calculate_;
    StateUpdater update_;
    OptimizerStrategy strategy_;
};

} // namespace tap_position_optimizer

template <typename StateCalculator, typename StateUpdater, typename State>
using TapPositionOptimizer = tap_position_optimizer::TapPositionOptimizer<StateCalculator, StateUpdater, State>;

} // namespace power_grid_model::optimizer
