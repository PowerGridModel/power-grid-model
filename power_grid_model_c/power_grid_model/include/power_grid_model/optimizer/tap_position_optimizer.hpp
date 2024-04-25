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
#include "../main_core/output.hpp"
#include "../main_core/state_queries.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <functional>
#include <queue>
#include <variant>
#include <vector>

namespace power_grid_model::optimizer {
namespace tap_position_optimizer {

namespace detail = power_grid_model::optimizer::detail;

using container_impl::get_type_index;
using main_core::get_component;

using TrafoGraphIdx = Idx;
using EdgeWeight = int64_t;
using WeightedTrafo = std::pair<Idx2D, EdgeWeight>;
using WeightedTrafoList = std::vector<WeightedTrafo>;
using RankedTransformerGroups = std::vector<std::vector<Idx2D>>;

constexpr auto infty = std::numeric_limits<Idx>::max();

template <typename ComponentType, typename ComponentContainer>
    requires main_core::component_container_c<ComponentContainer, ComponentType>
constexpr auto is_in_group(Idx2D const& index) {
    constexpr auto group_idx = ComponentContainer::template get_type_idx<ComponentType>();
    return index.group == group_idx;
}

template <typename ComponentType, typename ComponentContainer>
    requires main_core::component_container_c<ComponentContainer, ComponentType>
constexpr auto group_count(std::vector<Idx2D> const& indices) {
    return std::ranges::count_if(indices, is_in_group<ComponentType>);
}

template <std::derived_from<Branch> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topology_index(State const& state, auto const& id_or_index) {
    return main_core::get_component_sequence<Branch>(state, id_or_index);
}

template <std::derived_from<Branch3> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topology_index(State const& state, auto const& id_or_index) {
    return main_core::get_component_sequence<Branch3>(state, id_or_index);
}

template <std::derived_from<Regulator> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topology_index(State const& state, auto const& id_or_index) {
    return main_core::get_component_sequence<Regulator>(state, id_or_index);
}

template <std::derived_from<Branch> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_branch_nodes(State const& state, Idx topology_sequence_idx) {
    return state.comp_topo->branch_node_idx[topology_sequence_idx];
}

template <std::derived_from<Branch3> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_branch_nodes(State const& state, Idx topology_sequence_idx) {
    return state.comp_topo->branch3_node_idx[topology_sequence_idx];
}

template <transformer_c ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType> &&
             requires(State const& state, Idx const i) {
                 { get_branch_nodes<ComponentType>(state, i)[i] } -> std::convertible_to<Idx>;
             }
inline auto get_topo_node(State const& state, Idx topology_index, ControlSide control_side) {
    auto const& nodes = get_branch_nodes<ComponentType>(state, topology_index);

    auto const control_side_idx = static_cast<Idx>(control_side);
    assert(control_side_idx < nodes.size());

    return nodes[control_side_idx];
}

template <std::derived_from<Node> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->node[topology_sequence_idx];
}

template <std::derived_from<Branch> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->branch[topology_sequence_idx];
}

template <std::derived_from<Branch3> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->branch3[topology_sequence_idx];
}

template <std::derived_from<Regulator> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& state, Idx topology_sequence_idx) {
    return state.topo_comp_coup->regulator[topology_sequence_idx];
}

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
    Idx last_weight = -1;
    for (const auto& trafo : sorted_trafos) {
        if (groups.empty() || last_weight != trafo.second) {
            groups.push_back(std::vector<Idx2D>{trafo.first});
            last_weight = trafo.second;
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

struct TransformerRanker {
    template <main_core::main_model_state_c State>
    auto operator()(State const& state) const -> RankedTransformerGroups {
        return rank_transformers(state);
    }
};

constexpr IntS tap_one_step_up(transformer_c auto const& transformer) {
    auto const tap_pos = transformer.tap_pos();
    auto const tap_max = transformer.tap_max();
    return tap_pos < tap_max ? tap_pos + 1 : tap_max;
}

constexpr IntS tap_one_step_down(transformer_c auto const& transformer) {
    auto const tap_pos = transformer.tap_pos();
    auto const tap_min = transformer.tap_min();
    return tap_pos > tap_min ? tap_pos - 1 : tap_min;
}

template <transformer_c... TransformerTypes> class TransformerWrapper {
  public:
    template <transformer_c TransformerType>
    TransformerWrapper(std::reference_wrapper<const TransformerType> transformer, Idx2D const& index,
                       Idx topology_index)
        : transformer_{std::move(transformer)}, index_{index}, topology_index_{topology_index} {}

    constexpr auto index() const { return index_; }
    constexpr auto topology_index() const { return topology_index_; }

    IntS id() const {
        return apply([](auto const& t) { return t.id(); });
    }
    IntS tap_pos() const {
        return apply([](auto const& t) { return t.tap_pos(); });
    }
    IntS tap_min() const {
        return apply([](auto const& t) { return t.tap_min(); });
    }
    IntS tap_max() const {
        return apply([](auto const& t) { return t.tap_max(); });
    }
    bool connected_at_tap_side() const {
        return apply([](auto const& t) { return t.status(t.tap_side()); });
    }
    bool connected_at_control_side(TransformerTapRegulator const& regulator) const {
        return apply([side = regulator.control_side()](auto const& t) {
            using SideType = typename std::remove_cvref_t<decltype(t)>::SideType;
            return t.status(static_cast<SideType>(side));
        });
    }

    template <typename Func>
        requires(std::invocable<Func, TransformerTypes const&> && ...)
    auto apply(Func const& func) const {
        return std::visit([&func](auto const& t) { return func(t.get()); }, transformer_);
    }

  private:
    Idx2D index_;
    Idx topology_index_;

    std::variant<std::reference_wrapper<const TransformerTypes>...> transformer_;
};

template <transformer_c... TransformerTypes> struct TapRegulatorRef {
    using TransformerWrapper = TransformerWrapper<TransformerTypes...>;

    std::reference_wrapper<const TransformerTapRegulator> regulator;
    TransformerWrapper transformer;
};

template <typename... Ts> struct transformer_types_s;
template <> struct transformer_types_s<std::tuple<>> {
    using type = std::tuple<>;
};
template <typename T, typename... Ts> struct transformer_types_s<T, std::tuple<Ts...>> {
    using type = std::tuple<T, Ts...>;
};
template <typename T, typename... Ts> struct transformer_types_s<std::tuple<T, Ts...>> {
    using type =
        std::conditional_t<transformer_c<T>,
                           typename transformer_types_s<T, typename transformer_types_s<std::tuple<Ts...>>::type>::type,
                           typename transformer_types_s<std::tuple<Ts...>>::type>;
};
template <typename... Ts> struct transformer_types_s<std::tuple<std::tuple<Ts...>>> {
    using type = typename transformer_types_s<std::tuple<Ts...>>::type;
};

template <typename... Ts> using transformer_types_t = typename transformer_types_s<std::tuple<Ts...>>::type;

template <typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, TransformerTapRegulator>
TransformerTapRegulator const& find_regulator(State const& state, ID regulated_object) {
    auto const regulators = get_component_citer<TransformerTapRegulator>(state);

    auto result_it = std::ranges::find_if(regulators, [regulated_object](auto const& regulator) {
        return regulator.regulated_object() == regulated_object;
    });
    assert(result_it != regulators.end());

    return *result_it;
}

template <transformer_c... TransformerTypes, typename State>
    requires(main_core::component_container_c<typename State::ComponentContainer, TransformerTypes> && ...)
inline TapRegulatorRef<TransformerTypes...> regulator_mapping(State const& state, Idx2D const& transformer_index) {
    using ComponentContainer = typename State::ComponentContainer;
    using ResultType = TapRegulatorRef<TransformerTypes...>;
    using IsType = bool (*)(Idx2D const&);
    using TransformerMapping = ResultType (*)(State const&, Idx2D const&);

    constexpr auto n_types = sizeof...(TransformerTypes);

    constexpr auto is_type = std::array<IsType, n_types>{is_in_group<TransformerTypes, ComponentContainer>...};
    constexpr auto transformer_mappings =
        std::array<TransformerMapping, n_types>{[](State const& state_, Idx2D const& transformer_index_) {
            auto const& transformer = get_component<TransformerTypes>(state_, transformer_index_);
            auto const topology_index = get_topology_index<TransformerTypes>(state_, transformer_index_);
            return ResultType{.regulator = std::cref(find_regulator(state_, transformer.id())),
                              .transformer = {std::cref(transformer), transformer_index_, topology_index}};
        }...};

    for (Idx idx = 0; idx < n_types; ++idx) {
        if (is_type[idx](transformer_index)) {
            return transformer_mappings[idx](state, transformer_index);
        }
    }
    throw UnreachableHit{"TapPositionOptimizer::regulator_mapping", "Transformer must be regulated"};
}

template <transformer_c... TransformerTypes, typename State>
    requires(main_core::component_container_c<typename State::ComponentContainer, TransformerTypes> && ...)
inline auto regulator_mapping(State const& state, std::vector<Idx2D> const& order) {
    std::vector<TapRegulatorRef<TransformerTypes...>> result;

    for (auto const& index : order) {
        result.push_back(regulator_mapping<TransformerTypes...>(state, index));
    }

    return result;
}

template <transformer_c... TransformerTypes, typename State>
    requires(main_core::component_container_c<typename State::ComponentContainer, TransformerTypes> && ...)
inline auto regulator_mapping(State const& state, RankedTransformerGroups const& order) {
    std::vector<std::vector<TapRegulatorRef<TransformerTypes...>>> result;

    for (auto const& sub_order : order) {
        result.push_back(regulator_mapping<TransformerTypes...>(state, sub_order));
    }

    return result;
}

template <std::derived_from<Branch> ComponentType, steady_state_math_output_type MathOutputType>
inline auto i_pu(std::vector<MathOutputType> const& math_output, Idx2D const& math_id, ControlSide control_side) {
    using enum ControlSide;

    auto const& branch_output = math_output[math_id.group].branch[math_id.pos];

    switch (control_side) {
    case from:
        return branch_output.i_f;
    case to:
        return branch_output.i_t;
    default:
        throw MissingCaseForEnumError("control_transformer<Branch>", control_side);
    }
}

template <std::derived_from<Branch3> ComponentType, steady_state_math_output_type MathOutputType>
inline auto i_pu(std::vector<MathOutputType> const& math_output, Idx2DBranch3 const& math_id,
                 ControlSide control_side) {
    using enum ControlSide;

    auto const& branch_outputs = math_output[math_id.group].branch;

    switch (control_side) {
    case side_1:
        return branch_outputs[math_id.pos[0]].i_f;
    case side_2:
        return branch_outputs[math_id.pos[1]].i_f;
    case side_3:
        return branch_outputs[math_id.pos[2]].i_f;
    default:
        throw MissingCaseForEnumError("control_transformer<Branch3>", control_side);
    }
}

template <component_c ComponentType, typename... RegulatedTypes, typename State,
          steady_state_math_output_type MathOutputType>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
inline auto i_pu_controlled_node(TapRegulatorRef<RegulatedTypes...> const& regulator, State const& state,
                                 std::vector<MathOutputType> const& math_output) {
    auto const& branch_math_id = get_math_id<ComponentType>(state, regulator.transformer.topology_index());
    return i_pu<ComponentType>(math_output, branch_math_id, regulator.regulator.get().control_side());
}

template <transformer_c ComponentType, typename State, steady_state_math_output_type MathOutputType>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType> &&
             requires(State const& state, Idx const i) {
                 { get_branch_nodes<ComponentType>(state, i)[i] } -> std::convertible_to<Idx>;
             }
inline auto u_pu(State const& state, std::vector<MathOutputType> const& math_output, Idx topology_index,
                 ControlSide control_side) {
    auto const controlled_node_idx = get_topo_node<ComponentType>(state, topology_index, control_side);
    auto const node_math_id = get_math_id<Node>(state, controlled_node_idx);
    return math_output[node_math_id.group].u[node_math_id.pos];
}

template <component_c ComponentType, typename... RegulatedTypes, typename State,
          steady_state_math_output_type MathOutputType>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
inline auto u_pu_controlled_node(TapRegulatorRef<RegulatedTypes...> const& regulator, State const& state,
                                 std::vector<MathOutputType> const& math_output) {
    return u_pu<ComponentType>(state, math_output, regulator.transformer.topology_index(),
                               regulator.regulator.get().control_side());
}

template <main_core::main_model_state_c State, steady_state_math_output_type MathOutputType>
inline void create_tap_regulator_output(State const& state, std::vector<MathOutputType>& math_output) {
    for (Idx group = 0; group < math_output.size(); ++group) {
        math_output[group].transformer_tap_regulator.resize(state.math_topology[group]->n_transformer_tap_regulator(),
                                                            {.tap_pos = na_IntS});
    }
}

template <main_core::main_model_state_c State, steady_state_math_output_type MathOutputType>
inline void add_tap_regulator_output(State const& state, TransformerTapRegulator const& regulator, IntS tap_pos,
                                     std::vector<MathOutputType>& math_output) {
    Idx2D const& math_id =
        get_math_id<TransformerTapRegulator>(state, get_topology_index<TransformerTapRegulator>(state, regulator.id()));
    math_output[math_id.group].transformer_tap_regulator[math_id.pos] = {tap_pos};
}

template <typename... RegulatedTypes, main_core::main_model_state_c State, steady_state_math_output_type MathOutputType>
    requires(main_core::component_container_c<typename State::ComponentContainer, RegulatedTypes> && ...)
void add_tap_regulator_output(State const& state,
                              std::vector<std::vector<TapRegulatorRef<RegulatedTypes...>>> const& regulator_order,
                              std::vector<MathOutputType>& math_output) {
    create_tap_regulator_output(state, math_output);

    for (auto const& same_rank_regulators : regulator_order) {
        for (auto const& regulator : same_rank_regulators) {
            add_tap_regulator_output(state, regulator.regulator.get(), regulator.transformer.tap_pos(), math_output);
        }
    }
}

template <typename... T> class TapPositionOptimizerImpl;
template <transformer_c... TransformerTypes, typename StateCalculator, typename StateUpdater_, typename State_,
          typename TransformerRanker_>
    requires(main_core::component_container_c<typename State_::ComponentContainer, TransformerTypes> && ...) &&
            detail::steady_state_calculator_c<StateCalculator, State_> &&
            std::invocable<std::remove_cvref_t<StateUpdater_>, ConstDataset const&> &&
            requires(TransformerRanker_ const& ranker, State_ const& state) {
                { ranker(state) } -> std::convertible_to<RankedTransformerGroups>;
            }
class TapPositionOptimizerImpl<std::tuple<TransformerTypes...>, StateCalculator, StateUpdater_, State_,
                               TransformerRanker_> : public detail::BaseOptimizer<StateCalculator, State_> {
  public:
    using Base = detail::BaseOptimizer<StateCalculator, State_>;
    using typename Base::Calculator;
    using typename Base::ResultType;
    using typename Base::State;
    using StateUpdater = StateUpdater_;
    using TransformerRanker = TransformerRanker_;

  private:
    using ComponentContainer = typename State::ComponentContainer;
    using TapRegulatorRef = TapRegulatorRef<TransformerTypes...>;
    using UpdateBuffer = std::tuple<std::vector<typename TransformerTypes::UpdateType>...>;

    template <transformer_c T>
    static constexpr auto transformer_index_of = [] {
        using TransformerTypesTuple = std::tuple<TransformerTypes...>;
        constexpr auto n_transformers = std::tuple_size_v<TransformerTypesTuple>;

        constexpr auto is_same_type = []<size_t... I>(std::index_sequence<I...> /* indices */) {
            return std::array{std::same_as<T, std::tuple_element_t<I, TransformerTypesTuple>>...};
        }
        (std::make_index_sequence<n_transformers>{});

        static_assert(std::ranges::any_of(is_same_type, [](bool a) { return a; }));
        for (size_t k = 0; k < is_same_type.size(); ++k) {
            if (is_same_type[k]) {
                return k;
            }
        }
        assert(false); // unreachable
        return n_transformers;
    }();
    static_assert(((transformer_index_of<TransformerTypes> < sizeof...(TransformerTypes)) && ...));

  public:
    TapPositionOptimizerImpl(Calculator calculator, StateUpdater updater, OptimizerStrategy strategy)
        : calculate_{std::move(calculator)}, update_{std::move(updater)}, strategy_{strategy} {}

    auto optimize(State const& state, CalculationMethod method) -> ResultType final {
        auto const order = regulator_mapping<TransformerTypes...>(state, TransformerRanker{}(state));

        auto const cache = this->cache_state(state, order);
        auto const optimal_output = optimize(state, order, method);
        update_state(cache);

        return optimal_output;
    }

    constexpr auto get_strategy() const { return strategy_; }

  private:
    auto optimize(State const& state, std::vector<std::vector<TapRegulatorRef>> const& regulator_order,
                  CalculationMethod method) const -> ResultType {
        initialize(state, regulator_order);

        if (auto const result = try_calculation_with_regulation(state, regulator_order, method);
            strategy_ == OptimizerStrategy::any) {
            return result;
        }

        // refine solution
        step_all(state, regulator_order);
        return try_calculation_with_regulation(state, regulator_order, method);
    }

    auto try_calculation_with_regulation(State const& state,
                                         std::vector<std::vector<TapRegulatorRef>> const& regulator_order,
                                         CalculationMethod method) const -> ResultType {
        auto result = calculate_with_retry(state, method);

        bool tap_changed = true;
        while (tap_changed) {
            tap_changed = false;
            UpdateBuffer update_data;

            for (auto const& same_rank_regulators : regulator_order) {
                for (auto const& regulator : same_rank_regulators) {
                    if (regulator.transformer.connected_at_tap_side() &&
                        regulator.transformer.connected_at_control_side(regulator.regulator)) {
                        tap_changed = tap_changed || control_transformer(regulator, state, result, update_data);
                    }
                }
                if (tap_changed) {
                    break;
                }
            }
            if (tap_changed) {
                update_state(update_data);
                result = calculate_(state, method);
            }
        }

        add_tap_regulator_output(state, regulator_order, result);

        return result;
    }

    auto calculate_with_retry(State const& state, CalculationMethod method) const {
        try {
            return calculate_(state, method);
        } catch (SparseMatrixError const&) {
            return calculate_(state, CalculationMethod::linear);
        } catch (IterationDiverge const&) {
            return calculate_(state, CalculationMethod::linear);
        }
    }

    bool control_transformer(TapRegulatorRef const& regulator, State const& state, ResultType const& math_output,
                             UpdateBuffer& update_data) const {
        bool tap_changed = false;

        regulator.transformer.apply([&](transformer_c auto const& transformer) {
            using TransformerType = std::remove_cvref_t<decltype(transformer)>;

            auto const param = regulator.regulator.get().template calc_param<typename ResultType::value_type::sym>();

            auto const& u_node = u_pu_controlled_node<TransformerType>(regulator, state, math_output);
            auto const& i_node = i_pu_controlled_node<TransformerType>(regulator, state, math_output);
            auto const u_measured = u_node + param.z_compensation * i_node;

            auto const v_measured = mean_val(cabs(u_measured)); // TODO(mgovers): handle asym correctly

            auto new_tap_pos = [&transformer, &v_measured, &param] {
                if (v_measured > param.u_set + 0.5 * param.u_band) {
                    return tap_one_step_up(transformer);
                }
                if (v_measured < param.u_set - 0.5 * param.u_band) {
                    return tap_one_step_down(transformer);
                }
                return transformer.tap_pos();
            }();
            if (new_tap_pos != transformer.tap_pos()) {
                add_tap_pos_update(new_tap_pos, transformer, update_data);
                tap_changed = true;
            }
        });

        return tap_changed;
    }

    void update_state(UpdateBuffer const& update_data) const {
        static_assert(sizeof...(TransformerTypes) == std::tuple_size_v<UpdateBuffer>);

        ConstDataset update_dataset;
        auto const update_component = [this, &update_data, &update_dataset]<transformer_c TransformerType>() {
            auto const& component_update = get<TransformerType>(update_data);
            if (!component_update.empty()) {
                update_dataset.emplace(TransformerType::name, this->get_data_ptr<TransformerType>(update_data));
            }
        };
        (update_component.template operator()<TransformerTypes>(), ...);

        if (!update_dataset.empty()) {
            update_(update_dataset);
        }
    }

    auto initialize(State const& state, std::vector<std::vector<TapRegulatorRef>> const& regulator_order) const {
        using namespace std::string_literals;

        constexpr auto get_max = [](transformer_c auto const& transformer) -> IntS { return transformer.tap_max(); };
        constexpr auto get_min = [](transformer_c auto const& transformer) -> IntS { return transformer.tap_min(); };
        constexpr auto get_clamped = [](transformer_c auto const& transformer) -> IntS {
            return std::clamp(transformer.tap_pos(), transformer.tap_min(), transformer.tap_max());
        };

        switch (strategy_) {
        case OptimizerStrategy::any:
            adjust_voltage(get_clamped, state, regulator_order);
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

    void step_all(State const& state, std::vector<std::vector<TapRegulatorRef>> const& regulator_order) const {
        using namespace std::string_literals;

        constexpr auto one_step_down = [](transformer_c auto const& transformer) -> IntS {
            return tap_one_step_up(transformer);
        };
        constexpr auto one_step_up = [](transformer_c auto const& transformer) -> IntS {
            return tap_one_step_down(transformer);
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

    static auto add_tap_pos_update(IntS new_tap_pos, transformer_c auto const& transformer, UpdateBuffer& update_data) {
        auto result = get_nan_update(transformer);
        result.id = transformer.id();
        result.tap_pos = new_tap_pos;
        get<std::remove_cvref_t<decltype(transformer)>>(update_data).push_back(result);
    }

    template <typename Func>
        requires std::invocable<Func, Transformer const&> && std::invocable<Func, ThreeWindingTransformer const&> &&
                 std::same_as<std::invoke_result_t<Func, Transformer const&>, IntS> &&
                 std::same_as<std::invoke_result_t<Func, ThreeWindingTransformer const&>, IntS>
    auto adjust_voltage(Func new_tap_pos, State const& state,
                        std::vector<std::vector<TapRegulatorRef>> const& regulator_order) const {
        UpdateBuffer update_data;

        auto const get_update = [new_tap_pos = std::move(new_tap_pos),
                                 &update_data](transformer_c auto const& transformer) {
            add_tap_pos_update(new_tap_pos(transformer), transformer, update_data);
        };

        for (auto const& sub_order : regulator_order) {
            for (auto const& regulator : sub_order) {
                regulator.transformer.apply(get_update);
            }
        }

        update_state(update_data);
    }

    static constexpr auto component_cache_update(transformer_c auto const& transformer) {
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

    template <transformer_c T> static std::vector<typename T::UpdateType>& get(UpdateBuffer& update_data) {
        using ResultType = std::vector<typename T::UpdateType>;
        return std::get<transformer_index_of<T>>(update_data);
    }

    template <transformer_c T> static std::vector<typename T::UpdateType> const& get(UpdateBuffer const& update_data) {
        using ResultType = std::vector<typename T::UpdateType>;
        return std::get<transformer_index_of<T>>(update_data);
    }

    template <transformer_c T>
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

template <typename StateCalculator, typename StateUpdater, main_core::main_model_state_c State,
          typename TransformerRanker_>
    requires detail::steady_state_calculator_c<StateCalculator, State> &&
                 std::invocable<std::remove_cvref_t<StateUpdater>, ConstDataset const&>
using TapPositionOptimizer =
    TapPositionOptimizerImpl<transformer_types_t<typename State::ComponentContainer::gettable_types>, StateCalculator,
                             StateUpdater, State, TransformerRanker_>;

} // namespace tap_position_optimizer

template <typename StateCalculator, typename StateUpdater, typename State>
using TapPositionOptimizer = tap_position_optimizer::TapPositionOptimizer<StateCalculator, StateUpdater, State,
                                                                          tap_position_optimizer::TransformerRanker>;

} // namespace power_grid_model::optimizer
