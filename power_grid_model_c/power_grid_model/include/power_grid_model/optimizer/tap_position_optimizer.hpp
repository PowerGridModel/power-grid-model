// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base_optimizer.hpp"

#include "../all_components.hpp"
#include "../auxiliary/dataset.hpp"
#include "../auxiliary/meta_gen/update.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../component/three_winding_transformer.hpp"
#include "../component/transformer.hpp"
#include "../component/transformer_tap_regulator.hpp"
#include "../main_core/output.hpp"
#include "../main_core/state_queries.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>

#include <algorithm>
#include <functional>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <variant>
#include <vector>

namespace power_grid_model::optimizer {
namespace tap_position_optimizer {

namespace detail = power_grid_model::optimizer::detail;

using container_impl::get_type_index;
using main_core::get_component;

using TrafoGraphIdx = Idx;
using EdgeWeight = int64_t;
using RankedTransformerGroups = std::vector<std::vector<Idx2D>>;

constexpr auto infty = std::numeric_limits<Idx>::max();
constexpr Idx2D unregulated_idx = {-1, -1};
struct TrafoGraphVertex {
    bool is_source{};
};

struct TrafoGraphEdge {
    Idx2D regulated_idx{};
    EdgeWeight weight{};

    constexpr TrafoGraphEdge() = default;
    constexpr TrafoGraphEdge(Idx2D regulated_idx_, EdgeWeight weight_)
        : regulated_idx{regulated_idx_}, weight{weight_} {}

    bool operator==(const TrafoGraphEdge& other) const {
        return regulated_idx == other.regulated_idx && weight == other.weight;
    } // thanks boost

    auto constexpr operator<=>(const TrafoGraphEdge& other) const {
        if (auto cmp = weight <=> other.weight; cmp != 0) { // NOLINT(modernize-use-nullptr)
            return cmp;
        }
        if (auto cmp = regulated_idx.group <=> other.regulated_idx.group; cmp != 0) { // NOLINT(modernize-use-nullptr)
            return cmp;
        }
        return regulated_idx.pos <=> other.regulated_idx.pos;
    }
};

constexpr auto unregulated_edge_prop = TrafoGraphEdge{unregulated_idx, 0};
using TrafoGraphEdges = std::vector<std::pair<TrafoGraphIdx, TrafoGraphIdx>>;
using TrafoGraphEdgeProperties = std::vector<TrafoGraphEdge>;

struct RegulatedTrafoProperties {
    Idx id{};
    ControlSide control_side{};

    auto operator<=>(RegulatedTrafoProperties const& other) const = default; // NOLINT(modernize-use-nullptr)
};

using RegulatedTrafos = std::set<RegulatedTrafoProperties>;

inline std::pair<bool, ControlSide> regulated_trafos_contain(RegulatedTrafos const& trafos_set, Idx const& id) {
    if (auto it =
            std::ranges::find_if(trafos_set, [&](RegulatedTrafoProperties const& trafo) { return trafo.id == id; });
        it != trafos_set.end()) {
        return {true, it->control_side};
    }
    return {false, ControlSide{}}; // no default invalid control side, won't be used by logic
}

struct RegulatedObjects {
    RegulatedTrafos trafos;
    RegulatedTrafos trafos3w;

    std::pair<bool, ControlSide> contains_trafo(Idx const& id) const { return regulated_trafos_contain(trafos, id); }
    std::pair<bool, ControlSide> contains_trafo3w(Idx const& id) const {
        return regulated_trafos_contain(trafos3w, id);
    }
};

using TransformerGraph = boost::compressed_sparse_row_graph<boost::directedS, TrafoGraphVertex, TrafoGraphEdge,
                                                            boost::no_property, TrafoGraphIdx, TrafoGraphIdx>;

template <class ComponentContainer>
    requires main_core::model_component_state_c<main_core::MainModelState, ComponentContainer, Node>
inline void add_to_edge(main_core::MainModelState<ComponentContainer> const& state, TrafoGraphEdges& edges,
                        TrafoGraphEdgeProperties& edge_props, ID const& start, ID const& end,
                        TrafoGraphEdge const& edge_prop) {
    Idx const start_idx = main_core::get_component_sequence_idx<Node>(state, start);
    Idx const end_idx = main_core::get_component_sequence_idx<Node>(state, end);
    edges.emplace_back(start_idx, end_idx);
    edge_props.emplace_back(edge_prop);
}

inline void process_trafo3w_edge(main_core::main_model_state_c auto const& state,
                                 ThreeWindingTransformer const& transformer3w, bool const& trafo3w_is_regulated,
                                 ControlSide const& control_side, Idx2D const& trafo3w_idx, TrafoGraphEdges& edges,
                                 TrafoGraphEdgeProperties& edge_props) {
    using enum Branch3Side;

    constexpr std::array<std::tuple<Branch3Side, Branch3Side>, 3> const branch3_combinations{
        {{side_1, side_2}, {side_1, side_3}, {side_2, side_3}}};

    for (auto const& [first_side, second_side] : branch3_combinations) {
        if (!transformer3w.status(first_side) || !transformer3w.status(second_side)) {
            continue;
        }
        auto const& from_node = transformer3w.node(first_side);
        auto const& to_node = transformer3w.node(second_side);

        auto const tap_at_first_side = transformer3w.tap_side() == first_side;
        auto const connected_to_primary_side_regulated =
            trafo3w_is_regulated && (tap_at_first_side || (transformer3w.tap_side() == second_side));

        auto const tap_at_control = static_cast<IntS>(control_side) == static_cast<IntS>(transformer3w.tap_side());

        // only add weighted edge if the trafo3w meets the condition
        if (connected_to_primary_side_regulated) {
            auto const& tap_side_node = tap_at_first_side ? from_node : to_node;
            auto const& non_tap_side_node = tap_at_first_side ? to_node : from_node;
            auto const edge_from_node = tap_at_control ? non_tap_side_node : tap_side_node;
            auto const edge_to_node = tap_at_control ? tap_side_node : non_tap_side_node;
            // add regulated idx only when the first side node is tap side node.
            // This is done to add only one directional edge with regulated idx.
            auto const edge_value = TrafoGraphEdge{trafo3w_idx, 1};
            add_to_edge(state, edges, edge_props, edge_from_node, edge_to_node, edge_value);
        } else {
            add_to_edge(state, edges, edge_props, from_node, to_node, unregulated_edge_prop);
            add_to_edge(state, edges, edge_props, to_node, from_node, unregulated_edge_prop);
        }
    }
}

template <std::derived_from<ThreeWindingTransformer> Component, class ComponentContainer>
    requires main_core::model_component_state_c<main_core::MainModelState, ComponentContainer, Component>
constexpr void add_edge(main_core::MainModelState<ComponentContainer> const& state,
                        RegulatedObjects const& regulated_objects, TrafoGraphEdges& edges,
                        TrafoGraphEdgeProperties& edge_props) {

    for (auto const& transformer3w : state.components.template citer<ThreeWindingTransformer>()) {
        auto const trafo3w_is_regulated = regulated_objects.contains_trafo3w(transformer3w.id());
        Idx2D const trafo3w_idx = main_core::get_component_idx_by_id(state, transformer3w.id());
        process_trafo3w_edge(state, transformer3w, trafo3w_is_regulated.first, trafo3w_is_regulated.second, trafo3w_idx,
                             edges, edge_props);
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
        auto const trafo_regulated = regulated_objects.contains_trafo(transformer.id());
        if (trafo_regulated.first) {
            auto const control_side = trafo_regulated.second;
            auto const control_side_node = control_side == ControlSide::from ? from_node : to_node;
            auto const non_control_side_node = control_side == ControlSide::from ? to_node : from_node;
            auto const trafo_idx = main_core::get_component_idx_by_id(state, transformer.id());

            add_to_edge(state, edges, edge_props, non_control_side_node, control_side_node, {trafo_idx, 1});
        } else {
            add_to_edge(state, edges, edge_props, from_node, to_node, unregulated_edge_prop);
            add_to_edge(state, edges, edge_props, to_node, from_node, unregulated_edge_prop);
        }
    }
}

template <std::derived_from<Branch> Component, class ComponentContainer>
    requires main_core::model_component_state_c<main_core::MainModelState, ComponentContainer, Component> &&
             (!transformer_c<Component>)
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
        add_to_edge(state, edges, edge_props, branch.from_node(), branch.to_node(), unregulated_edge_prop);
        add_to_edge(state, edges, edge_props, branch.to_node(), branch.from_node(), unregulated_edge_prop);
    }
}

template <typename... ComponentTypes, main_core::main_model_state_c State>
inline auto add_edges(State const& state, RegulatedObjects const& regulated_objects, TrafoGraphEdges& edges,
                      TrafoGraphEdgeProperties& edge_props) {
    (add_edge<ComponentTypes>(state, regulated_objects, edges, edge_props), ...);
}

template <main_core::main_model_state_c State>
inline auto retrieve_regulator_info(State const& state) -> RegulatedObjects {
    RegulatedObjects regulated_objects;
    for (auto const& regulator : state.components.template citer<TransformerTapRegulator>()) {
        if (!regulator.status()) {
            continue;
        }
        auto const control_side = regulator.control_side();
        if (regulator.regulated_object_type() == ComponentType::branch) {
            regulated_objects.trafos.emplace(RegulatedTrafoProperties{regulator.regulated_object(), control_side});
        } else {
            regulated_objects.trafos3w.emplace(RegulatedTrafoProperties{regulator.regulated_object(), control_side});
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

    // Mark sources
    for (auto const& source : state.components.template citer<Source>()) {
        // ignore disabled sources
        trafo_graph[main_core::get_component_sequence_idx<Node>(state, source.node())].is_source = source.status();
    }

    return trafo_graph;
}

inline void process_edges_dijkstra(Idx v, std::vector<EdgeWeight>& vertex_distances, TransformerGraph const& graph) {
    using TrafoGraphElement = std::pair<EdgeWeight, TrafoGraphIdx>;
    std::priority_queue<TrafoGraphElement, std::vector<TrafoGraphElement>, std::greater<>> pq;
    vertex_distances[v] = 0;
    pq.emplace(0, v);

    while (!pq.empty()) {
        auto [dist, u] = pq.top();
        pq.pop();

        if (dist != vertex_distances[u]) {
            continue;
        }

        BGL_FORALL_EDGES(e, graph, TransformerGraph) {
            auto s = boost::source(e, graph);
            auto t = boost::target(e, graph);
            const EdgeWeight weight = graph[e].weight;

            // We can not use BGL_FORALL_OUTEDGES here because we need information
            // regardless of edge direction
            if (u == s && vertex_distances[s] + weight < vertex_distances[t]) {
                vertex_distances[t] = vertex_distances[s] + weight;
                pq.emplace(vertex_distances[t], t);
            } else if (u == t && vertex_distances[t] + weight < vertex_distances[s]) {
                vertex_distances[s] = vertex_distances[t] + weight;
                pq.emplace(vertex_distances[s], s);
            }
        }
    }
}

inline bool is_unreachable(EdgeWeight edge_res) { return edge_res == infty; }

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
        auto const edge_src_rank = vertex_distances[boost::source(e, graph)];
        auto const edge_tgt_rank = vertex_distances[boost::target(e, graph)];
        auto const edge_res = std::min(edge_src_rank, edge_tgt_rank);

        // New edge logic for ranking
        // |  Tap  | Control |         All edges       |
        // ---------------------------------------------
        // |   A   |    A    | [B->A], [C->A], [B<->C] |
        // |   A   |    B    | [A->B], [A->C], [B<->C] |
        // |   A   |    C    | [A->B], [A->C], [B<->C] |
        // |   B   |    A    | [B->A], [C<->A], [B->C] |
        // |   B   |    B    | [A->B], [C<->A], [C->B] |
        // |   B   |    C    | [B->A], [C<->A], [B->C] |
        // |   C   |    A    | [A<->B], [C->A], [C->B] |
        // |   C   |    B    | [A<->B], [C->A], [C->B] |
        // |   C   |    C    | [A<->B], [A->C], [A->B] |
        // In two winding trafo, the edge is always pointing to the control side; in three winding trafo edges, the
        // unidirectional edges are always pointing towards the control side and the node connected to the control
        // side via the bidirectional edge (if it exists). For delta configuration ABC, the above
        // situations can happen.
        // The logic still holds in meshed grids, albeit operating a more complex graph.
        if (edge_src_rank != edge_tgt_rank - 1) {
            throw AutomaticTapInputError("The control side of a transformer regulator should be relatively further "
                                         "away from the source than the tap side.\n");
        }
        if (!is_unreachable(edge_res)) {
            result.emplace_back(graph[e].regulated_idx, edge_tgt_rank);
        }
    }

    return result;
}

inline auto rank_transformers(TrafoGraphEdgeProperties const& w_trafo_list) -> RankedTransformerGroups {
    auto sorted_trafos = w_trafo_list;

    std::sort(sorted_trafos.begin(), sorted_trafos.end(),
              [](const TrafoGraphEdge& a, const TrafoGraphEdge& b) { return a.weight < b.weight; });

    RankedTransformerGroups groups;
    auto previous_weight = std::numeric_limits<EdgeWeight>::lowest();
    for (auto const& trafo : sorted_trafos) {
        if (trafo.weight > previous_weight) {
            groups.emplace_back();
            previous_weight = trafo.weight;
        }
        auto& current_group = groups.back(); // avoid duplicates
        if (std::ranges::find(current_group, trafo.regulated_idx) == current_group.end()) {
            current_group.emplace_back(trafo.regulated_idx);
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

constexpr IntS one_step_tap_up(transformer_c auto const& transformer) {
    IntS const tap_pos = transformer.tap_pos();
    IntS const tap_max = transformer.tap_max();
    IntS const tap_min = transformer.tap_min();

    if (tap_pos == tap_max) {
        return tap_max;
    }

    assert((tap_min <=> tap_max) == (tap_pos <=> tap_max));

    return tap_min < tap_max ? tap_pos + IntS{1} : tap_pos - IntS{1};
}
constexpr IntS one_step_tap_down(transformer_c auto const& transformer) {
    IntS const tap_pos = transformer.tap_pos();
    IntS const tap_max = transformer.tap_max();
    IntS const tap_min = transformer.tap_min();

    if (tap_pos == tap_min) {
        return tap_min;
    }

    assert((tap_max <=> tap_min) == (tap_pos <=> tap_min));

    return tap_min < tap_max ? tap_pos - IntS{1} : tap_pos + IntS{1};
}
// higher voltage at control side => lower voltage at tap side => lower tap pos
constexpr IntS one_step_control_voltage_up(transformer_c auto const& transformer, bool control_at_tap_side) {
    if (control_at_tap_side) {
        // control side is tap side, voltage up requires tap up
        return one_step_tap_up(transformer);
    }
    return one_step_tap_down(transformer);
}
// lower voltage at control side => higher voltage at tap side => higher tap pos
constexpr IntS one_step_control_voltage_down(transformer_c auto const& transformer, bool control_at_tap_side) {
    if (control_at_tap_side) {
        // control side is tap side, voltage down requires tap down
        return one_step_tap_down(transformer);
    }
    return one_step_tap_up(transformer);
}

template <transformer_c... TransformerTypes> class TransformerWrapper {
  public:
    template <transformer_c TransformerType>
    TransformerWrapper(std::reference_wrapper<const TransformerType> transformer, Idx2D const& index,
                       Idx topology_index)
        : transformer_{std::move(transformer)}, index_{index}, topology_index_{topology_index} {}

    constexpr auto index() const { return index_; }
    constexpr auto topology_index() const { return topology_index_; }

    ID id() const {
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
    IntS tap_side() const {
        return apply([](auto const& t) { return static_cast<IntS>(t.tap_side()); });
    }
    int64_t tap_range() const {
        return apply([](auto const& t) {
            return std::abs(static_cast<int64_t>(t.tap_max()) - static_cast<int64_t>(t.tap_min()));
        });
    }
    template <typename Func>
        requires(std::invocable<Func, TransformerTypes const&> && ...)
    auto apply(Func const& func) const {
        return std::visit([&func](auto const& t) { return func(t.get()); }, transformer_);
    }

  private:
    std::variant<std::reference_wrapper<const TransformerTypes>...> transformer_;

    Idx2D index_;
    Idx topology_index_;
};

template <transformer_c... TransformerTypes> struct TapRegulatorRef {
    std::reference_wrapper<const TransformerTapRegulator> regulator;
    TransformerWrapper<TransformerTypes...> transformer;

    bool control_at_tap_side() const {
        return static_cast<IntS>(regulator.get().control_side()) == transformer.tap_side();
    }
};

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

template <transformer_c... TransformerTypes, typename State>
    requires(main_core::component_container_c<typename State::ComponentContainer, TransformerTypes> && ...)
inline TapRegulatorRef<TransformerTypes...> regulator_mapping(State const& state, Idx2D const& transformer_index) {
    using ResultType = TapRegulatorRef<TransformerTypes...>;
    using IsType = bool (*)(Idx2D const&);
    using TransformerMapping = ResultType (*)(State const&, Idx2D const&);

    constexpr auto n_types = sizeof...(TransformerTypes);
    constexpr auto is_type = std::array<IsType, n_types>{[](Idx2D const& index) {
        constexpr auto group_idx = State::ComponentContainer::template get_type_idx<TransformerTypes>();
        return index.group == group_idx;
    }...};
    constexpr auto transformer_mappings =
        std::array<TransformerMapping, n_types>{[](State const& state_, Idx2D const& transformer_index_) {
            auto const& transformer = get_component<TransformerTypes>(state_, transformer_index_);
            auto const& regulator = find_regulator(state_, transformer.id());

            assert(transformer.status(transformer.tap_side()));
            assert(transformer.status(static_cast<typename TransformerTypes::SideType>(regulator.control_side())));

            auto const topology_index = get_topology_index<TransformerTypes>(state_, transformer_index_);
            return ResultType{.regulator = std::cref(regulator),
                              .transformer = {std::cref(transformer), transformer_index_, topology_index}};
        }...};

    for (Idx idx = 0; idx < static_cast<Idx>(n_types); ++idx) {
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
    result.reserve(order.size());

    for (auto const& index : order) {
        result.push_back(regulator_mapping<TransformerTypes...>(state, index));
    }

    return result;
}

template <transformer_c... TransformerTypes, typename State>
    requires(main_core::component_container_c<typename State::ComponentContainer, TransformerTypes> && ...)
inline auto regulator_mapping(State const& state, RankedTransformerGroups const& order) {
    std::vector<std::vector<TapRegulatorRef<TransformerTypes...>>> result;
    result.reserve(order.size());

    for (auto const& sub_order : order) {
        result.push_back(regulator_mapping<TransformerTypes...>(state, sub_order));
    }

    return result;
}

template <std::derived_from<Branch> ComponentType, steady_state_solver_output_type SolverOutputType>
inline auto i_pu(std::vector<SolverOutputType> const& solver_output, Idx2D const& math_id, ControlSide control_side) {
    using enum ControlSide;

    auto const& branch_output = solver_output[math_id.group].branch[math_id.pos];

    switch (control_side) {
    case from:
        return branch_output.i_f;
    case to:
        return branch_output.i_t;
    default:
        throw MissingCaseForEnumError{"adjust_transformer<Branch>", control_side};
    }
}

template <std::derived_from<Branch3> ComponentType, steady_state_solver_output_type SolverOutputType>
inline auto i_pu(std::vector<SolverOutputType> const& solver_output, Idx2DBranch3 const& math_id,
                 ControlSide control_side) {
    using enum ControlSide;

    auto const& branch_outputs = solver_output[math_id.group].branch;

    switch (control_side) {
    case side_1:
        return branch_outputs[math_id.pos[0]].i_f;
    case side_2:
        return branch_outputs[math_id.pos[1]].i_f;
    case side_3:
        return branch_outputs[math_id.pos[2]].i_f;
    default:
        throw MissingCaseForEnumError{"adjust_transformer<Branch3>", control_side};
    }
}

template <component_c ComponentType, typename... RegulatedTypes, typename State,
          steady_state_solver_output_type SolverOutputType>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
inline auto i_pu_controlled_node(TapRegulatorRef<RegulatedTypes...> const& regulator, State const& state,
                                 std::vector<SolverOutputType> const& solver_output) {
    auto const& branch_math_id = get_math_id<ComponentType>(state, regulator.transformer.topology_index());
    return i_pu<ComponentType>(solver_output, branch_math_id, regulator.regulator.get().control_side());
}

template <transformer_c ComponentType, typename State, steady_state_solver_output_type SolverOutputType>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType> &&
             requires(State const& state, Idx const i) {
                 { get_branch_nodes<ComponentType>(state, i)[i] } -> std::convertible_to<Idx>;
             }
inline auto u_pu(State const& state, std::vector<SolverOutputType> const& solver_output, Idx topology_index,
                 ControlSide control_side) {
    auto const controlled_node_idx = get_topo_node<ComponentType>(state, topology_index, control_side);
    auto const node_math_id = get_math_id<Node>(state, controlled_node_idx);
    return solver_output[node_math_id.group].u[node_math_id.pos];
}

template <component_c ComponentType, typename... RegulatedTypes, typename State,
          steady_state_solver_output_type SolverOutputType>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
inline auto u_pu_controlled_node(TapRegulatorRef<RegulatedTypes...> const& regulator, State const& state,
                                 std::vector<SolverOutputType> const& solver_output) {
    return u_pu<ComponentType>(state, solver_output, regulator.transformer.topology_index(),
                               regulator.regulator.get().control_side());
}

template <component_c ComponentType, typename... RegulatedTypes, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
inline bool is_regulated_transformer_connected(TapRegulatorRef<RegulatedTypes...> const& regulator,
                                               State const& state) {
    auto const controlled_node_idx = get_topo_node<ComponentType>(state, regulator.transformer.topology_index(),
                                                                  regulator.regulator.get().control_side());
    return get_math_id<Node>(state, controlled_node_idx) != Idx2D{-1, -1};
}

struct VoltageBand {
    double u_set{};
    double u_band{};

    friend constexpr auto operator<=>(double voltage, VoltageBand const& band) {
        assert(band.u_band >= 0.0);

        auto const lower = band.u_set - 0.5 * band.u_band;
        auto const upper = band.u_set + 0.5 * band.u_band;

        auto const lower_cmp = voltage <=> lower;
        if (auto const upper_cmp = voltage <=> upper; lower_cmp == upper_cmp) {
            return lower_cmp;
        }
        return std::partial_ordering::equivalent;
    }
};

template <symmetry_tag sym> struct NodeState {
    ComplexValue<sym> u;
    ComplexValue<sym> i;

    friend auto operator<=>(NodeState<sym> state, TransformerTapRegulatorCalcParam const& param) {
        auto const u_compensated = state.u + param.z_compensation * state.i;
        auto const v_compensated = mean_val(cabs(u_compensated)); // TODO(mgovers): handle asym correctly
        return v_compensated <=> VoltageBand{.u_set = param.u_set, .u_band = param.u_band};
    }
};

class RankIteration {
  public:
    RankIteration(std::vector<IntS> iterations_per_rank, Idx rank_index)
        : iterations_per_rank_{std::move(iterations_per_rank)}, rank_index_{rank_index} {}

    // Getters
    constexpr auto const& iterations_per_rank() const { return iterations_per_rank_; }
    constexpr auto rank_index() const { return rank_index_; }

    // Setters
    void set_rank_index(Idx rank_index) { rank_index_ = rank_index; }

    bool iterate_ranks(auto const& ranked_order, auto apply_single_object_in_rank, bool adjusted) {
        for (Idx i = 0; i < static_cast<Idx>(ranked_order.size()); ++i) {
            auto const& same_rank_regulators = ranked_order[i];
            for (Idx j = 0; j < static_cast<Idx>(same_rank_regulators.size()); ++j) {
                adjusted = apply_single_object_in_rank(i, j, same_rank_regulators) || adjusted;
            }
            if (adjusted) {
                if (rank_index_ < static_cast<Idx>(iterations_per_rank_.size()) - 1) {
                    std::fill(iterations_per_rank_.begin() + rank_index_ + 1, iterations_per_rank_.end(), 0);
                }
                ++iterations_per_rank_[rank_index_];
                return adjusted;
            }
            ++rank_index_;
        }
        return adjusted;
    };

  private:
    std::vector<IntS> iterations_per_rank_;
    Idx rank_index_{};
};

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
    std::vector<uint64_t> max_tap_ranges_per_rank;
    using ComponentContainer = typename State::ComponentContainer;
    using RegulatedTransformer = TapRegulatorRef<TransformerTypes...>;
    using UpdateBuffer = std::tuple<std::vector<typename TransformerTypes::UpdateType>...>;

    template <transformer_c T>
    static constexpr auto transformer_index_of = container_impl::get_cls_pos_v<T, TransformerTypes...>;
    static_assert(((transformer_index_of<TransformerTypes> < sizeof...(TransformerTypes)) && ...));

    class BinarySearch {
      public:
        BinarySearch() = default;
        BinarySearch(IntS tap_pos, IntS tap_min, IntS tap_max, bool control_at_tap_side) {
            reset(tap_pos, tap_min, tap_max, control_at_tap_side);
        }

        constexpr IntS get_current_tap() const { return current_; }
        constexpr bool get_last_down() const { return last_down_; }
        constexpr bool get_inevitable_run() const { return inevitable_run_; }
        constexpr bool get_end_of_bs() const { return lower_bound_ >= upper_bound_; }

        constexpr void set_current_tap(IntS current_tap) { current_ = current_tap; }
        constexpr void set_last_check(bool last_check) { last_check_ = last_check; }
        constexpr void set_inevitable_run(bool inevitable_run) { inevitable_run_ = inevitable_run; }

        void recalibrate(bool strategy_max) {
            // This if statement checks both conditions in the corresponding transformer
            // whether the tap_max and tap_min are reversed, as well as whether the optimization
            // has a max strategy.
            // Lower bound should be updated to the current tap position if following is the case:
            //  - tap_max > tap_min && strategy_max == true
            //  - tap_max < tap_min && strategy_max == false
            bool const invert_strategy = control_at_tap_side_ != strategy_max;
            if (tap_reverse_ == invert_strategy) {
                lower_bound_ = current_;
                last_down_ = false;
            } else {
                upper_bound_ = current_;
                last_down_ = true;
            }
        }

        void propose_new_pos(bool strategy_max, bool above_range) {
            bool const is_down = (above_range == tap_reverse_) != control_at_tap_side_;
            if (last_check_) {
                current_ = is_down ? lower_bound_ : upper_bound_;
                inevitable_run_ = true;
            } else {
                last_down_ = is_down;
                adjust(strategy_max);
            }
        }

        IntS repropose_tap(bool strategy_max, bool previous_down, bool& tap_changed) {
            // __prefer_higher__ indicates a preference towards higher voltage
            // that is a result of both the strategy as well as whether the current
            // transformer has a reversed tap_max and tap_min
            bool const prefer_higher = (strategy_max != tap_reverse_) != control_at_tap_side_;
            auto const tap_pos = search(prefer_higher);
            auto const tap_diff = tap_pos - get_current_tap();
            if (tap_diff == 0) {
                if (!inevitable_run_) {
                    inevitable_run_ = true;
                    tap_changed = true;
                } else {
                    tap_changed = false;
                }
                return tap_pos;
            }
            if ((tap_diff == 1 && previous_down) || (tap_diff == -1 && !previous_down)) {
                last_check_ = true;
            }
            tap_changed = true;
            current_ = tap_pos;
            return tap_pos;
        }

      private:
        void reset(IntS tap_pos, IntS tap_min, IntS tap_max, bool control_at_tap_side) {
            last_down_ = false;
            last_check_ = false;
            current_ = tap_pos;
            inevitable_run_ = false;
            lower_bound_ = std::min(tap_min, tap_max);
            upper_bound_ = std::max(tap_min, tap_max);
            tap_reverse_ = tap_max < tap_min;
            control_at_tap_side_ = control_at_tap_side;
        }

        void adjust(bool strategy_max = true) {
            if (get_last_down()) {
                upper_bound_ = current_;
            } else {
                lower_bound_ = current_;
            }
            if (lower_bound_ < upper_bound_) {
                bool const prefer_higher = strategy_max != tap_reverse_;
                IntS const tap_pos = search(prefer_higher);
                current_ = tap_pos;
            }
        }

        IntS search(bool prefer_higher_) const {
            // This logic is used to determine which of the middle points could be of interest
            // given strategy used in optimization:
            // Since in BinarySearch we insist on absolute upper and lower bounds, we only need to
            // find the corresponding mid point. std::midpoint returns always the lower mid point
            // if the range is of even length. This is why we need to adjust bounds accordingly.
            // Not because upper bound and lower bound might be reversed, which is not possible.
            bool const prefer_higher = control_at_tap_side_ != prefer_higher_;
            auto const primary_bound = prefer_higher ? upper_bound_ : lower_bound_;
            auto const secondary_bound = prefer_higher ? lower_bound_ : upper_bound_;
            return std::midpoint(primary_bound, secondary_bound);
        }

        IntS lower_bound_{};              // tap position lower bound
        IntS upper_bound_{};              // tap position upper bound
        IntS current_{0};                 // current tap position
        bool last_down_{false};           // last direction
        bool last_check_{false};          // last run checked
        bool tap_reverse_{false};         // tap range normal or reversed
        bool inevitable_run_{false};      // inevitable run
        bool control_at_tap_side_{false}; // regulator control side is at tap side
    };
    std::vector<std::vector<BinarySearch>> binary_search_;
    struct BinarySearchOptions {
        bool strategy_max{false};
        Idx2D idx_bs{0, 0};
    };
    Idx total_iterations{0}; // metric purpose only

  public:
    TapPositionOptimizerImpl(Calculator calculator, StateUpdater updater, OptimizerStrategy strategy,
                             meta_data::MetaData const& meta_data,
                             std::optional<SearchMethod> tap_search = std::nullopt)
        : meta_data_{&meta_data}, calculate_{std::move(calculator)}, update_{std::move(updater)}, strategy_{strategy} {
        auto const is_supported = [&strategy](std::optional<SearchMethod> const& search) {
            if (!search) {
                return true;
            }
            switch (strategy) {
            case OptimizerStrategy::any:
                return search == SearchMethod::linear_search;
            case OptimizerStrategy::fast_any:
                return search == SearchMethod::binary_search;
            default:
                return true;
            }
        };

        if (tap_search && !is_supported(tap_search)) {
            throw TapSearchStrategyIncompatibleError{
                "Search method is incompatible with optimization strategy: ", strategy, tap_search.value()};
        }

        if (!tap_search) {
            switch (strategy) {
            case OptimizerStrategy::any:
                tap_search_ = SearchMethod::linear_search;
                break;
            case OptimizerStrategy::fast_any:
            case OptimizerStrategy::local_maximum:
            case OptimizerStrategy::global_maximum:
            case OptimizerStrategy::local_minimum:
            case OptimizerStrategy::global_minimum:
                tap_search_ = SearchMethod::binary_search;
                break;
            default:
                throw MissingCaseForEnumError{"TapPositionOptimizer::TapPositionOptimizerImpl", strategy};
            }
        } else {
            tap_search_ = tap_search.value();
        }
    }

    auto optimize(State const& state, CalculationMethod method) -> MathOutput<ResultType> final {
        auto const order = regulator_mapping<TransformerTypes...>(state, TransformerRanker{}(state));
        auto const cache = this->cache_states(order);
        try {
            opt_prep(order);
            auto result = optimize(state, order, method);
            update_state(cache);
            return result;
        } catch (...) {
            update_state(cache);
            throw;
        }
    }

    constexpr auto get_strategy() const { return strategy_; }
    Idx get_total_iterations() const { return total_iterations; }

  private:
    void opt_prep(std::vector<std::vector<RegulatedTransformer>> const& regulator_order) {
        constexpr auto tap_pos_range_cmp = [](RegulatedTransformer const& a, RegulatedTransformer const& b) {
            return a.transformer.tap_range() < b.transformer.tap_range();
        };

        bs_prep(regulator_order);

        if (max_tap_ranges_per_rank.empty()) {
            max_tap_ranges_per_rank.reserve(regulator_order.size());
            for (auto const& same_rank_regulators : regulator_order) {
                max_tap_ranges_per_rank.push_back(std::ranges::max_element(same_rank_regulators.begin(),
                                                                           same_rank_regulators.end(),
                                                                           tap_pos_range_cmp)
                                                      ->transformer.tap_range());
            }
        }

        total_iterations = 0;
    }

    void bs_prep(std::vector<std::vector<RegulatedTransformer>> const& regulator_order) {
        if (tap_search_ == SearchMethod::linear_search) {
            return;
        }

        binary_search_.reserve(regulator_order.size());
        for (auto const& same_rank_regulators : regulator_order) {
            std::vector<BinarySearch> binary_search_group(same_rank_regulators.size());
            std::ranges::transform(same_rank_regulators, binary_search_group.begin(), [](auto const& regulator) {
                return BinarySearch{regulator.transformer.tap_pos(), regulator.transformer.tap_min(),
                                    regulator.transformer.tap_max(), regulator.control_at_tap_side()};
            });
            binary_search_.push_back(std::move(binary_search_group));
        }
    }

    auto optimize(State const& state, std::vector<std::vector<RegulatedTransformer>> const& regulator_order,
                  CalculationMethod method) -> MathOutput<ResultType> {
        pilot_run(regulator_order);

        if (auto result = iterate_with_fallback(state, regulator_order, method, tap_search_);
            strategy_ == OptimizerStrategy::any || strategy_ == OptimizerStrategy::fast_any) {
            return produce_output(regulator_order, std::move(result));
        }

        exploit_neighborhood(regulator_order);
        return produce_output(regulator_order,
                              iterate_with_fallback(state, regulator_order, method, SearchMethod::linear_search));
    }

    auto produce_output(std::vector<std::vector<RegulatedTransformer>> const& regulator_order,
                        ResultType solver_output) const -> MathOutput<ResultType> {
        TransformerTapPositionOutput transformer_tap_positions;

        for (auto const& sub_order : regulator_order) {
            for (auto const& regulator : sub_order) {
                auto const& transformer = regulator.transformer;
                transformer_tap_positions.push_back(
                    {.transformer_id = transformer.id(), .tap_position = transformer.tap_pos()});
            }
        }

        return {.solver_output = {std::move(solver_output)},
                .optimizer_output = {std::move(transformer_tap_positions)}};
    }

    auto iterate_with_fallback(State const& state,
                               std::vector<std::vector<RegulatedTransformer>> const& regulator_order,
                               CalculationMethod method, SearchMethod search) -> ResultType {
        auto fallback = [this, &state, &regulator_order, &method, &search] {
            std::ignore = iterate(state, regulator_order, CalculationMethod::linear, search);
            return iterate(state, regulator_order, method, search);
        };

        try {
            return iterate(state, regulator_order, method, search);
        } catch (IterationDiverge const& /* ex */) {
            return fallback();
        } catch (SparseMatrixError const& /* ex */) {
            return fallback();
        }
    }

    auto iterate(State const& state, std::vector<std::vector<RegulatedTransformer>> const& regulator_order,
                 CalculationMethod method, SearchMethod search) -> ResultType {
        auto result = calculate_(state, method);
        ++total_iterations;

        bool const strategy_max =
            strategy_ == OptimizerStrategy::global_maximum || strategy_ == OptimizerStrategy::local_maximum;
        bool tap_changed = true;
        Idx rank_index = 0;
        RankIteration rank_iterator(std::vector<IntS>(regulator_order.size()), rank_index);

        while (tap_changed) {
            tap_changed = false;
            UpdateBuffer update_data;
            rank_iterator.set_rank_index(0);

            auto const adjust_transformer_in_rank = [&](Idx const& rank_idx, Idx const& transformer_idx,
                                                        std::vector<RegulatedTransformer> const& same_rank_regulators) {
                auto const& regulator = same_rank_regulators[transformer_idx];
                BinarySearchOptions const options{strategy_max, Idx2D{rank_idx, transformer_idx}};
                tap_changed = adjust_transformer(regulator, state, result, update_data, search, options) || tap_changed;
                return tap_changed;
            };

            tap_changed = rank_iterator.iterate_ranks(regulator_order, adjust_transformer_in_rank, tap_changed);
            auto const& iterations_per_rank = rank_iterator.iterations_per_rank();
            rank_index = rank_iterator.rank_index();

            if (tap_changed) {
                if (static_cast<uint64_t>(iterations_per_rank[rank_index]) > 2 * max_tap_ranges_per_rank[rank_index]) {
                    throw MaxIterationReached{"TapPositionOptimizer::iterate"};
                }
                update_state(update_data);
                result = calculate_(state, method);
                ++total_iterations;
            }
        }
        return result;
    }

    bool adjust_transformer(RegulatedTransformer const& regulator, State const& state, ResultType const& solver_output,
                            UpdateBuffer& update_data, SearchMethod search, BinarySearchOptions const& options) {
        switch (search) {
        case SearchMethod::binary_search:
            return adjust_transformer_bs(regulator, state, solver_output, update_data, options);
        case SearchMethod::linear_search:
            return adjust_transformer_scan(regulator, state, solver_output, update_data);
        default:
            throw MissingCaseForEnumError{"TapPositionOptimizer::adjust_transformer", search};
        }
    }

    template <typename TransformerType, typename Regulator, typename State, typename ResultType>
    auto compute_node_state_and_param(Regulator const& regulator, State const& state, ResultType const& solver_output) {
        using sym = typename ResultType::value_type::sym;

        auto const param = regulator.regulator.get().template calc_param<sym>();
        auto const node_state =
            NodeState<sym>{.u = u_pu_controlled_node<TransformerType>(regulator, state, solver_output),
                           .i = i_pu_controlled_node<TransformerType>(regulator, state, solver_output)};

        return std::make_pair(node_state, param);
    }

    bool adjust_transformer_scan(RegulatedTransformer const& regulator, State const& state,
                                 ResultType const& solver_output, UpdateBuffer& update_data) {
        bool tap_changed = false;

        regulator.transformer.apply([&](transformer_c auto const& transformer) {
            using TransformerType = std::remove_cvref_t<decltype(transformer)>;
            if (!is_regulated_transformer_connected<TransformerType>(regulator, state)) {
                return;
            }

            auto [node_state, param] = compute_node_state_and_param<TransformerType>(regulator, state, solver_output);

            bool control_at_tap_side = regulator.control_at_tap_side();

            auto const cmp = node_state <=> param;
            auto new_tap_pos = [&transformer, &cmp, &control_at_tap_side] {
                if (cmp > 0) { // NOLINT(modernize-use-nullptr)
                    return one_step_control_voltage_down(transformer, control_at_tap_side);
                }
                if (cmp < 0) { // NOLINT(modernize-use-nullptr)
                    return one_step_control_voltage_up(transformer, control_at_tap_side);
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

    bool adjust_transformer_bs(RegulatedTransformer const& regulator, State const& state,
                               ResultType const& solver_output, UpdateBuffer& update_data,
                               BinarySearchOptions const& options) {
        auto const strategy_max = options.strategy_max;
        bool tap_changed = false;
        auto& current_bs = binary_search_[options.idx_bs.group][options.idx_bs.pos];

        auto const adjust_transformer_ = [&](transformer_c auto const& transformer) { // NOSONAR
            using TransformerType = std::remove_cvref_t<decltype(transformer)>;

            if (!is_regulated_transformer_connected<TransformerType>(regulator, state)) {
                return;
            }

            if (current_bs.get_end_of_bs() || current_bs.get_inevitable_run()) {
                return;
            }

            auto [node_state, param] = compute_node_state_and_param<TransformerType>(regulator, state, solver_output);

            auto const cmp = node_state <=> param;
            if (auto new_tap_pos =
                    [&cmp, strategy_max, &current_bs] {
                        if (cmp != 0) {                                        // NOLINT(modernize-use-nullptr)
                            current_bs.propose_new_pos(strategy_max, cmp > 0); // NOLINT(modernize-use-nullptr)
                        }
                        return current_bs.get_current_tap();
                    }();
                new_tap_pos != transformer.tap_pos()) {
                current_bs.set_current_tap(new_tap_pos);
                add_tap_pos_update(new_tap_pos, transformer, update_data);
                tap_changed = true;
                return;
            }

            if (strategy_ == OptimizerStrategy::fast_any) {
                tap_changed = false;
                return;
            }

            bool const previous_down = current_bs.get_last_down();
            current_bs.recalibrate(strategy_max);

            IntS const tap_pos = current_bs.repropose_tap(strategy_max, previous_down, tap_changed);
            add_tap_pos_update(tap_pos, transformer, update_data);
        };
        regulator.transformer.apply(adjust_transformer_);

        return tap_changed;
    }

    void update_state(UpdateBuffer const& update_data) const {
        static_assert(sizeof...(TransformerTypes) == std::tuple_size_v<UpdateBuffer>);

        ConstDataset update_dataset{false, 1, "update", *meta_data_};
        auto const update_component = [&update_data, &update_dataset]<transformer_c TransformerType>() {
            auto const& component_update = get<TransformerType>(update_data);
            if (!component_update.empty()) {
                add_buffer_to_update_dataset<TransformerType>(update_data, TransformerType::name, update_dataset);
            }
        };
        (update_component.template operator()<TransformerTypes>(), ...);

        if (!update_dataset.empty()) {
            update_(update_dataset);
        }
    }

    void update_binary_search(std::vector<std::vector<RegulatedTransformer>> const& regulator_order) {
        for (Idx i = 0; i < static_cast<Idx>(regulator_order.size()); ++i) {
            auto const& sub_order = regulator_order[i];
            for (Idx j = 0; j < static_cast<Idx>(sub_order.size()); ++j) {
                auto const& regulator = sub_order[j];
                if (i < static_cast<Idx>(binary_search_.size()) && j < static_cast<Idx>(binary_search_[i].size())) {
                    binary_search_[i][j].set_current_tap(regulator.transformer.tap_pos());
                    binary_search_[i][j].set_last_check(false);
                    binary_search_[i][j].set_inevitable_run(false);
                }
            }
        }
    }

    auto pilot_run(std::vector<std::vector<RegulatedTransformer>> const& regulator_order) {
        using namespace std::string_literals;

        constexpr auto max_voltage_pos = [](transformer_c auto const& transformer, bool control_at_tap_side) -> IntS {
            if (control_at_tap_side) {
                // max voltage at tap side <=> max tap pos
                return transformer.tap_max();
            }
            // max voltage at control side => min voltage at tap side => min tap pos
            return transformer.tap_min();
        };
        constexpr auto min_voltage_pos = [](transformer_c auto const& transformer, bool control_at_tap_side) -> IntS {
            if (control_at_tap_side) {
                // min voltage at tap side <=> min tap pos
                return transformer.tap_min();
            }
            // min voltage at control side => max voltage at tap side => max tap pos
            return transformer.tap_max();
        };

        switch (strategy_) {
        case OptimizerStrategy::fast_any:
            [[fallthrough]];
        case OptimizerStrategy::any:
            break;
        case OptimizerStrategy::global_maximum:
            [[fallthrough]];
        case OptimizerStrategy::local_maximum:
            regulate_transformers(max_voltage_pos, regulator_order);
            break;
        case OptimizerStrategy::global_minimum:
            [[fallthrough]];
        case OptimizerStrategy::local_minimum:
            regulate_transformers(min_voltage_pos, regulator_order);
            break;
        default:
            throw MissingCaseForEnumError{"TapPositionOptimizer::pilot_run"s, strategy_};
        }
        if (tap_search_ == SearchMethod::binary_search) {
            update_binary_search(regulator_order);
        }
    }

    void exploit_neighborhood(std::vector<std::vector<RegulatedTransformer>> const& regulator_order) {
        using namespace std::string_literals;

        constexpr auto increment_voltage = [](transformer_c auto const& transformer, bool control_at_tap_side) -> IntS {
            return one_step_control_voltage_up(transformer, control_at_tap_side);
        };
        constexpr auto decrement_voltage = [](transformer_c auto const& transformer, bool control_at_tap_side) -> IntS {
            return one_step_control_voltage_down(transformer, control_at_tap_side);
        };

        switch (strategy_) {
        case OptimizerStrategy::fast_any:
            [[fallthrough]];
        case OptimizerStrategy::any:
            break;
        case OptimizerStrategy::global_maximum:
            [[fallthrough]];
        case OptimizerStrategy::local_maximum:
            regulate_transformers(increment_voltage, regulator_order);
            break;
        case OptimizerStrategy::global_minimum:
            [[fallthrough]];
        case OptimizerStrategy::local_minimum:
            regulate_transformers(decrement_voltage, regulator_order);
            break;
        default:
            throw MissingCaseForEnumError{"TapPositionOptimizer::exploit_neighborhood"s, strategy_};
        }
    }

    static auto add_tap_pos_update(IntS new_tap_pos, transformer_c auto const& transformer, UpdateBuffer& update_data) {
        auto result = get_nan_update(transformer);
        result.id = transformer.id();
        result.tap_pos = new_tap_pos;
        get<std::remove_cvref_t<decltype(transformer)>>(update_data).push_back(result);
    }

    template <typename Func>
        requires((std::invocable<Func, TransformerTypes const&, bool> &&
                  std::same_as<std::invoke_result_t<Func, TransformerTypes const&, bool>, IntS>) &&
                 ...)
    auto regulate_transformers(Func to_new_tap_pos,
                               std::vector<std::vector<RegulatedTransformer>> const& regulator_order) const {
        UpdateBuffer update_data;

        auto const get_update = [to_new_tap_pos_func = std::move(to_new_tap_pos),
                                 &update_data](transformer_c auto const& transformer, bool control_at_tap_side) {
            add_tap_pos_update(to_new_tap_pos_func(transformer, control_at_tap_side), transformer, update_data);
        };

        for (auto const& sub_order : regulator_order) {
            for (auto const& regulator : sub_order) {
                bool const control_at_tap_side = regulator.control_at_tap_side();
                regulator.transformer.apply([&get_update, &control_at_tap_side](auto const& transformer) {
                    get_update(transformer, control_at_tap_side);
                });
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

    static UpdateBuffer cache_states(std::vector<std::vector<RegulatedTransformer>> const& regulator_order) {
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
        return std::get<transformer_index_of<T>>(update_data);
    }

    template <transformer_c T> static std::vector<typename T::UpdateType> const& get(UpdateBuffer const& update_data) {
        return std::get<transformer_index_of<T>>(update_data);
    }

    template <transformer_c T>
        requires requires(UpdateBuffer const& u) {
            { get<T>(u).data() } -> std::convertible_to<void const*>;
            { get<T>(u).size() } -> std::convertible_to<Idx>;
        }
    static auto add_buffer_to_update_dataset(UpdateBuffer const& update_buffer, std::string_view component_name,
                                             ConstDataset& update_data) {
        auto const& data = get<T>(update_buffer);
        update_data.add_buffer(component_name, static_cast<Idx>(data.size()), static_cast<Idx>(data.size()), nullptr,
                               data.data());
    }

    static constexpr auto get_nan_update(auto const& component) {
        using UpdateType = typename std::remove_cvref_t<decltype(component)>::UpdateType;
        return UpdateType{};
    }

    meta_data::MetaData const* meta_data_;
    Calculator calculate_;
    StateUpdater update_;
    OptimizerStrategy strategy_;
    SearchMethod tap_search_;
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
