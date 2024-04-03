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
#include "../component/transformer_tap_regulator.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <functional>
#include <queue>
#include <vector>

namespace power_grid_model::optimizer {
namespace tap_position_optimizer {

namespace detail = power_grid_model::optimizer::detail;

template <typename T>
concept transformer_c = std::derived_from<std::remove_cvref_t<T>, Transformer> ||
                        std::derived_from<std::remove_cvref_t<T>, ThreeWindingTransformer>;

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
inline auto rank_transformers(WeightedTrafoList const& /*w_trafo_list*/) -> std::vector<std::vector<Idx2D>> {
    return {};
}

template <main_core::main_model_state_c State>
inline auto rank_transformers(State const& state) -> std::vector<std::vector<Idx2D>> {
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

    class TransformerRef {
      public:
        TransformerRef() = default;

        TransformerRef(Idx2D const& index, std::reference_wrapper<const Transformer> transformer)
            : index_{index}, transformer_{std::move(transformer)} {}

        TransformerRef(Idx2D const& index,
                       std::reference_wrapper<const ThreeWindingTransformer> three_winding_transformer)
            : index_{index}, three_winding_transformer_{std::move(three_winding_transformer)} {}

        IntS tap_pos() const {
            return apply([](auto const& t) { return t.tap_pos(); });
        }
        IntS tap_min() const {
            return apply([](auto const& t) { return t.tap_min(); });
        }
        IntS tap_max() const {
            return apply([](auto const& t) { return t.tap_max(); });
        }
        template <typename Func>
            requires std::invocable<Func, Transformer const&> && std::invocable<Func, ThreeWindingTransformer const&>
        auto apply(Func const& func) const {
            assert(transformer_ || three_winding_transformer_);

            if (transformer_) {
                return func(transformer_->get());
            }
            if (three_winding_transformer_) {
                return func(three_winding_transformer_->get());
            }
        }

      private:
        Idx2D index_;

        std::optional<std::reference_wrapper<const Transformer>> transformer_;
        std::optional<std::reference_wrapper<const ThreeWindingTransformer>> three_winding_transformer_;
    };

    struct TapRegulatorRef {
        std::reference_wrapper<const TransformerTapRegulator> tap_regulator;
        TransformerRef transformer;
    };

    struct UpdateBuffer {
        std::vector<TransformerUpdate> transformer_update;
        std::vector<ThreeWindingTransformerUpdate> three_winding_transformer_update;
    };

  public:
    TapPositionOptimizer(Calculator calculator, StateUpdater updater, OptimizerStrategy strategy)
        : calculate_{std::move(calculator)}, update_{std::move(updater)}, strategy_{strategy} {}

    auto optimize(State const& state) -> ResultType final {
        auto const order = this->regulator_mapping(state, rank_transformers(state));

        auto const cache = this->cache_state(state, order);
        auto const optimal_output = optimize(state, order);
        update_state(cache);

        return optimal_output;
    }

    constexpr auto get_strategy() { return strategy_; }

  private:
    auto optimize(State const& state, std::vector<std::vector<TapRegulatorRef>> const& regulator_order) -> ResultType {
        initialize(state, regulator_order);

        if (auto const result = try_calculation_with_regulation(state, regulator_order);
            strategy_ == OptimizerStrategy::any) {
            return result;
        }

        // refine solution
        step_all(state, regulator_order);
        return try_calculation_with_regulation(state, regulator_order);
    }

    auto try_calculation_with_regulation(State const& state,
                                         std::vector<std::vector<TapRegulatorRef>> const& /*regulator_order*/)
        -> ResultType {
        // TODO(mgovers): implement outer loop tap changer
        class InvalidPFResult : public PowerGridError {};

        ResultType result;
        // try {
        //     auto const result = calculate_(state);

        // }
        return calculate_(state);
    }

    void update_state(UpdateBuffer const& update_data) {
        ConstDataset const update_dataset{
            {"transformer", this->get_data_ptr<Transformer>(update_data)},
            {"three_winding_transformer", this->get_data_ptr<ThreeWindingTransformer>(update_data)}};

        update_(update_dataset);
    }

    auto initialize(State const& state, std::vector<std::vector<TapRegulatorRef>> const& regulator_order) {
        using namespace std::string_literals;

        constexpr auto get_max = [](transformer_c auto const& transformer) -> IntS { return transformer.tap_max(); };
        constexpr auto get_min = [](transformer_c auto const& transformer) -> IntS { return transformer.tap_min(); };

        switch (strategy_) {
        case OptimizerStrategy::any:
            break;
        case OptimizerStrategy::global_minimum:
            [[fallthrough]];
        case OptimizerStrategy::local_minimum:
            adjust_voltage(get_min, state, regulator_order);
            break;
        case OptimizerStrategy::global_maximum:
            [[fallthrough]];
        case OptimizerStrategy::local_maximum:
            adjust_voltage(get_max, state, regulator_order);
            break;
        default:
            throw MissingCaseForEnumError{"TapPositionOptimizer::initialize"s, strategy_};
        }
    }

    void step_all(State const& state, std::vector<std::vector<TapRegulatorRef>> const& regulator_order) {
        using namespace std::string_literals;

        constexpr auto one_step_up = [](transformer_c auto const& transformer) -> IntS {
            auto const tap_pos = transformer.tap_pos();
            auto const tap_max = transformer.tap_max();
            return tap_pos < tap_max ? tap_pos + 1 : tap_max;
        };
        constexpr auto one_step_down = [](transformer_c auto const& transformer) -> IntS {
            auto const tap_pos = transformer.tap_pos();
            auto const tap_min = transformer.tap_min();
            return tap_pos > tap_min ? tap_pos - 1 : tap_min;
        };

        switch (strategy_) {
        case OptimizerStrategy::any:
            break;
        case OptimizerStrategy::global_minimum:
            [[fallthrough]];
        case OptimizerStrategy::local_minimum:
            adjust_voltage(one_step_down, state, regulator_order);
            break;
        case OptimizerStrategy::global_maximum:
            [[fallthrough]];
        case OptimizerStrategy::local_maximum:
            adjust_voltage(one_step_up, state, regulator_order);
            break;
        default:
            throw MissingCaseForEnumError{"TapPositionOptimizer::initialize"s, strategy_};
        }
    }

    template <typename Func>
        requires std::invocable<Func, Transformer const&> && std::invocable<Func, ThreeWindingTransformer const&> &&
                 std::same_as<std::invoke_result_t<Func, Transformer const&>, IntS> &&
                 std::same_as<std::invoke_result_t<Func, ThreeWindingTransformer const&>, IntS>
    auto adjust_voltage(Func new_tap_pos, State const& state,
                        std::vector<std::vector<TapRegulatorRef>> const& regulator_order) {
        UpdateBuffer update_data;

        auto const cache_transformer = [&new_tap_pos, &update_data](transformer_c auto const& transformer) {
            auto result = get_nan_update(transformer);
            result.id = transformer.id();
            result.tap_pos = new_tap_pos(transformer);
            get<std::remove_cvref_t<decltype(transformer)>>(update_data).push_back(result);
        };

        for (auto const& sub_order : regulator_order) {
            for (auto const& regulator : sub_order) {
                regulator.transformer.apply(cache_transformer);
            }
        }

        update_state(update_data);
    }

    template <typename T> constexpr static auto& get_component(State const& state, auto const& id_or_index) {
        return state.components.template get_item<T>(id_or_index);
    }

    static TapRegulatorRef regulator_mapping(State const& state, Idx2D const& regulator_index) {
        auto const& regulator = get_component<TransformerTapRegulator>(state, regulator_index);
        auto const transformer_id = regulator.regulated_object();
        auto const transformer_index = state.components.get_idx_by_id(transformer_id);

        auto map_regulator = [&regulator, &state, &transformer_index]<typename T>() {
            return TapRegulatorRef{
                std::cref(regulator),
                {transformer_index, std::cref(get_component<Transformer>(state, transformer_index))}};
        };
        if (is_in_group<Transformer>(transformer_index)) {
            return map_regulator.template operator()<Transformer>();
        }
        if (is_in_group<ThreeWindingTransformer>(transformer_index)) {
            return map_regulator.template operator()<ThreeWindingTransformer>();
        }
        throw InvalidRegulatedObject{transformer_id, "TransformerTapRegulator"};
    }

    static auto regulator_mapping(State const& state, std::vector<Idx2D> const& order) {
        std::vector<TapRegulatorRef> result;

        for (auto const& index : order) {
            result.push_back(regulator_mapping(state, index));
        }

        return result;
    }

    static auto regulator_mapping(State const& state, std::vector<std::vector<Idx2D>> const& order) {
        std::vector<std::vector<TapRegulatorRef>> result;

        for (auto const& sub_order : order) {
            result.push_back(regulator_mapping(state, sub_order));
        }

        return result;
    }

    constexpr static auto component_cache_update(transformer_c auto const& transformer) {
        auto result = get_nan_update(transformer);

        result.id = transformer.id();
        result.tap_pos = transformer.tap_pos();

        return transformer.inverse(result);
    }

    static UpdateBuffer cache_state(State const& state,
                                    std::vector<std::vector<TapRegulatorRef>> const& regulator_order) {
        UpdateBuffer result;

        auto const cache_transformer = [&result](transformer_c auto const& transformer) {
            get<std::remove_cvref_t<decltype(transformer)>>(result).push_back(component_cache_update(transformer));
        };

        for (auto const& same_rank_regulators : regulator_order) {
            for (auto const& regulator_index : same_rank_regulators) {
                regulator_index.transformer.apply(cache_transformer);
            }
        }

        return result;
    }

    template <typename T> static constexpr auto group_count(std::vector<Idx2D> const& indices) {
        return std::ranges::count_if(indices, is_in_group<T>);
    }

    template <typename T> static constexpr auto is_in_group(Idx2D const& index) {
        constexpr auto group_idx = ComponentContainer::template get_type_idx<T>();
        return index.group == group_idx;
    }

    template <transformer_c T> static auto get(UpdateBuffer& update_data) {
        if constexpr (std::derived_from<std::remove_cvref_t<T>, Transformer>) {
            return update_data.transformer_update;
        }
        if constexpr (std::derived_from<std::remove_cvref_t<T>, ThreeWindingTransformer>) {
            return update_data.three_winding_transformer_update;
        }
    }

    template <transformer_c T> static auto const& get(UpdateBuffer const& update_data) {
        if constexpr (std::derived_from<std::remove_cvref_t<T>, Transformer>) {
            return update_data.transformer_update;
        }
        if constexpr (std::derived_from<std::remove_cvref_t<T>, ThreeWindingTransformer>) {
            return update_data.three_winding_transformer_update;
        }
    }

    template <typename T>
        requires requires(UpdateBuffer const& u) {
                     { get<T>(u).data() } -> std::convertible_to<void const*>;
                     { get<T>(u).size() } -> std::convertible_to<Idx>;
                 }
    static auto const get_data_ptr(UpdateBuffer const& update_buffer) {
        auto const& data = get<T>(update_buffer);
        return ConstDataPointer{data.data(), static_cast<Idx>(data.size())};
    };

    static auto get_nan_update(auto const& component) {
        return meta_data::get_component_nan<typename std::remove_cvref_t<decltype(component)>::UpdateType>{}();
    }

    Calculator calculate_;
    StateUpdater update_;
    OptimizerStrategy strategy_;
};

} // namespace tap_position_optimizer

template <typename StateCalculator, typename StateUpdater, typename State>
using TapPositionOptimizer = tap_position_optimizer::TapPositionOptimizer<StateCalculator, StateUpdater, State>;

} // namespace power_grid_model::optimizer
