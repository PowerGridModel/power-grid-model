// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_optimizer.hpp"

#include <power_grid_model/main_core/input.hpp>
#include <power_grid_model/main_core/update.hpp>
#include <power_grid_model/optimizer/tap_position_optimizer.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <map>
#include <ranges>

TEST_SUITE_BEGIN("Automatic Tap Changer");

namespace power_grid_model {
namespace optimizer::tap_position_optimizer::test {
namespace {
using TestComponentContainer =
    Container<ExtraRetrievableTypes<Base, Node, Branch, Branch3, Appliance, Regulator>, Line, Link, Node, Transformer,
              ThreeWindingTransformer, TransformerTapRegulator, Source>;
using TestState = main_core::MainModelState<TestComponentContainer>;

TransformerInput get_transformer(ID id, ID from, ID to, BranchSide tap_side, IntS tap_pos = na_IntS) {
    return TransformerInput{.id = id,
                            .from_node = from,
                            .to_node = to,
                            .from_status = 1,
                            .to_status = 1,
                            .u1 = nan,
                            .u2 = nan,
                            .sn = nan,
                            .uk = nan,
                            .pk = nan,
                            .i0 = nan,
                            .p0 = nan,
                            .winding_from = WindingType::wye_n,
                            .winding_to = WindingType::wye_n,
                            .clock = 0,
                            .tap_side = tap_side,
                            .tap_pos = tap_pos,
                            .tap_min = std::numeric_limits<IntS>::min(),
                            .tap_max = std::numeric_limits<IntS>::max(),
                            .tap_nom = 0,
                            .tap_size = nan,
                            .uk_min = nan,
                            .uk_max = nan,
                            .pk_min = nan,
                            .pk_max = nan,
                            .r_grounding_from = nan,
                            .x_grounding_from = nan,
                            .r_grounding_to = nan,
                            .x_grounding_to = nan};
}

ThreeWindingTransformerInput get_transformer3w(ID id, ID node_1, ID node_2, ID node_3, IntS tap_pos = 0) {
    return ThreeWindingTransformerInput{
        .id = id,
        .node_1 = node_1,
        .node_2 = node_2,
        .node_3 = node_3,
        .status_1 = 1,
        .status_2 = 1,
        .status_3 = 1,
        .u1 = nan,
        .u2 = nan,
        .u3 = nan,
        .sn_1 = nan,
        .sn_2 = nan,
        .sn_3 = nan,
        .uk_12 = nan,
        .uk_13 = nan,
        .uk_23 = nan,
        .pk_12 = nan,
        .pk_13 = nan,
        .pk_23 = nan,
        .i0 = nan,
        .p0 = nan,
        .winding_1 = WindingType::wye_n,
        .winding_2 = WindingType::wye_n,
        .winding_3 = WindingType::wye_n,
        .clock_12 = 0,
        .clock_13 = 0,
        .tap_side = Branch3Side::side_1,
        .tap_pos = tap_pos,
        .tap_min = std::numeric_limits<IntS>::min(),
        .tap_max = std::numeric_limits<IntS>::max(),
        .tap_nom = 0,
        .tap_size = nan,
        .uk_12_min = nan,
        .uk_12_max = nan,
        .uk_13_min = nan,
        .uk_13_max = nan,
        .uk_23_min = nan,
        .uk_23_max = nan,
        .pk_12_min = nan,
        .pk_12_max = nan,
        .pk_13_min = nan,
        .pk_13_max = nan,
        .pk_23_min = nan,
        .pk_23_max = nan,
        .r_grounding_1 = nan,
        .x_grounding_1 = nan,
        .r_grounding_2 = nan,
        .x_grounding_2 = nan,
        .r_grounding_3 = nan,
        .x_grounding_3 = nan,
    };
}

LineInput get_line_input(ID id, ID from, ID to) {
    return LineInput{.id = id,
                     .from_node = from,
                     .to_node = to,
                     .from_status = 1,
                     .to_status = 1,
                     .r1 = nan,
                     .x1 = nan,
                     .c1 = nan,
                     .tan1 = nan,
                     .r0 = nan,
                     .x0 = nan,
                     .c0 = nan,
                     .tan0 = nan,
                     .i_n = nan};
}
TransformerTapRegulatorInput get_regulator(ID id, ID regulated_object, ControlSide control_side) {
    return TransformerTapRegulatorInput{.id = id,
                                        .regulated_object = regulated_object,
                                        .status = 1,
                                        .control_side = control_side,
                                        .u_set = nan,
                                        .u_band = nan,
                                        .line_drop_compensation_r = nan,
                                        .line_drop_compensation_x = nan};
}
} // namespace
} // namespace optimizer::tap_position_optimizer::test

namespace {
namespace pgm_tap = optimizer::tap_position_optimizer;
namespace test = pgm_tap::test;
} // namespace

TEST_CASE("Test Transformer ranking") {
    using test::get_line_input;
    using test::get_regulator;
    using test::get_transformer;
    using test::get_transformer3w;
    using test::TestState;

    SUBCASE("Single transformer") {
        auto const get_state = [](ID source, ID node_a, ID node_b, ID trafo, ID regulator) {
            TestState state;

            std::vector<NodeInput> nodes{{node_a, 10e3}, {node_b, 400}};
            main_core::add_component<Node>(state, nodes.begin(), nodes.end(), 50.0);

            std::vector const sources{SourceInput{.id = source, .node = node_a, .status = IntS{1}, .u_ref = 1.0}};
            main_core::add_component<Source>(state, sources.begin(), sources.end(), 50.0);

            std::vector const transformers{get_transformer(trafo, node_a, node_b, BranchSide::from)};
            main_core::add_component<Transformer>(state, transformers.begin(), transformers.end(), 50.0);

            std::vector const regulators{get_regulator(regulator, trafo, ControlSide::to)};
            main_core::add_component<TransformerTapRegulator>(state, regulators.begin(), regulators.end(), 50.0);

            state.components.set_construction_complete();

            return state;
        };

        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(6, 1, 2, 3, 5)));
        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(4, 1, 3, 2, 5)));
        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(1, 2, 3, 4, 5)));
        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(1, 3, 2, 4, 5)));
        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(6, 3, 2, 4, 5)));
        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(6, 2, 3, 4, 5)));
        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(6, 1, 2, 4, 5)));
        CHECK_NOTHROW(pgm_tap::build_transformer_graph(get_state(6, 2, 1, 4, 5)));
    }

    SUBCASE("Full grid") {
        TestState state;
        std::vector<NodeInput> nodes{{0, 150e3}, {1, 10e3}, {2, 10e3}, {3, 10e3}, {4, 10e3},
                                     {5, 50e3},  {6, 10e3}, {7, 10e3}, {8, 10e3}, {9, 10e3}};
        main_core::add_component<Node>(state, nodes.begin(), nodes.end(), 50.0);

        std::vector<TransformerInput> transformers{
            get_transformer(11, 0, 1, BranchSide::from), get_transformer(12, 0, 1, BranchSide::from),
            get_transformer(13, 5, 7, BranchSide::from), get_transformer(14, 2, 3, BranchSide::from),
            get_transformer(15, 8, 9, BranchSide::from)};
        main_core::add_component<Transformer>(state, transformers.begin(), transformers.end(), 50.0);

        std::vector<ThreeWindingTransformerInput> transformers3w{get_transformer3w(16, 0, 4, 5)};
        main_core::add_component<ThreeWindingTransformer>(state, transformers3w.begin(), transformers3w.end(), 50.0);

        std::vector<LineInput> lines{get_line_input(17, 3, 6), get_line_input(18, 3, 9)};
        main_core::add_component<Line>(state, lines.begin(), lines.end(), 50.0);

        std::vector<LinkInput> links{{19, 2, 1, 1, 1}, {20, 6, 4, 1, 1}, {21, 8, 7, 1, 1}};
        main_core::add_component<Link>(state, links.begin(), links.end(), 50.0);

        std::vector<SourceInput> sources{{22, 0, 1, 1.0, 0, nan, nan, nan}};
        main_core::add_component<Source>(state, sources.begin(), sources.end(), 50.0);

        std::vector<TransformerTapRegulatorInput> regulators{
            get_regulator(23, 11, ControlSide::from), get_regulator(24, 12, ControlSide::from),
            get_regulator(25, 13, ControlSide::from), get_regulator(26, 14, ControlSide::from),
            get_regulator(27, 15, ControlSide::from), get_regulator(28, 16, ControlSide::side_1)};
        main_core::add_component<TransformerTapRegulator>(state, regulators.begin(), regulators.end(), 50.0);

        state.components.set_construction_complete();

        // Subcases
        SUBCASE("Building the graph") {
            using pgm_tap::unregulated_idx;
            using vertex_iterator = boost::graph_traits<pgm_tap::TransformerGraph>::vertex_iterator;

            // reference graph creation
            // Inserted in order of transformer, transformer3w, line and link
            std::vector<std::pair<Idx, Idx>> expected_edges;
            expected_edges.insert(expected_edges.end(), {{0, 1}, {0, 1}, {5, 7}, {2, 3}, {8, 9}});
            expected_edges.insert(expected_edges.end(), {{0, 4}, {4, 5}, {5, 4}, {0, 5}});
            expected_edges.insert(expected_edges.end(), {{3, 6}, {6, 3}, {3, 9}, {9, 3}});
            expected_edges.insert(expected_edges.end(), {{2, 1}, {1, 2}, {6, 4}, {4, 6}, {8, 7}, {7, 8}});

            pgm_tap::TrafoGraphEdgeProperties expected_edges_prop;
            expected_edges_prop.insert(expected_edges_prop.end(),
                                       {{{3, 0}, 1}, {{3, 1}, 1}, {{3, 2}, 1}, {{3, 3}, 1}, {{3, 4}, 1}});
            expected_edges_prop.insert(expected_edges_prop.end(),
                                       {{{4, 0}, 1}, {unregulated_idx, 1}, {unregulated_idx, 1}, {unregulated_idx, 1}});
            expected_edges_prop.insert(expected_edges_prop.end(), 10, {unregulated_idx, 0});

            std::vector<pgm_tap::TrafoGraphVertex> const expected_vertex_props{
                {true}, {false}, {false}, {false}, {false}, {false}, {false}, {false}, {false}, {false}};

            pgm_tap::TransformerGraph actual_graph = pgm_tap::build_transformer_graph(state);
            pgm_tap::TrafoGraphEdgeProperties actual_edges_prop;

            vertex_iterator vi;
            vertex_iterator vi_end;
            for (boost::tie(vi, vi_end) = vertices(actual_graph); vi != vi_end; ++vi) {
                CHECK(actual_graph[*vi].is_source == expected_vertex_props[*vi].is_source);
            }

            BGL_FORALL_EDGES(e, actual_graph, pgm_tap::TransformerGraph) {
                actual_edges_prop.push_back(actual_graph[e]);
            }

            std::sort(actual_edges_prop.begin(), actual_edges_prop.end());
            std::sort(expected_edges_prop.begin(), expected_edges_prop.end());
            CHECK(actual_edges_prop == expected_edges_prop);
        }

        SUBCASE("Automatic tap unsupported tap side at LV") {
            TestState bad_state;
            std::vector<NodeInput> bad_nodes{{0, 50e3}, {1, 10e3}};
            main_core::add_component<Node>(bad_state, bad_nodes.begin(), bad_nodes.end(), 50.0);

            std::vector<TransformerInput> bad_trafo{get_transformer(2, 0, 1, BranchSide::to)};
            main_core::add_component<Transformer>(bad_state, bad_trafo.begin(), bad_trafo.end(), 50.0);

            std::vector<TransformerTapRegulatorInput> bad_regulators{get_regulator(3, 2, ControlSide::from)};

            CHECK_THROWS_AS(main_core::add_component<TransformerTapRegulator>(bad_state, bad_regulators.begin(),
                                                                              bad_regulators.end(), 50.0),
                            AutomaticTapCalculationError);
        }

        SUBCASE("Process edge weights") {
            using vertex_iterator = boost::graph_traits<pgm_tap::TransformerGraph>::vertex_iterator;

            // Dummy graph
            pgm_tap::TrafoGraphEdges const edge_array = {{0, 1}, {0, 2}, {2, 3}};
            pgm_tap::TrafoGraphEdgeProperties const edge_prop{{{0, 1}, 1}, {{-1, -1}, 2}, {{2, 3}, 3}};
            std::vector<pgm_tap::TrafoGraphVertex> vertex_props{{true}, {false}, {false}, {false}};

            pgm_tap::TransformerGraph g{boost::edges_are_unsorted_multi_pass, edge_array.cbegin(), edge_array.cend(),
                                        edge_prop.cbegin(), 4};

            // Vertex properties can not be set during graph creation
            vertex_iterator vi;
            vertex_iterator vi_end;
            for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
                g[*vi].is_source = vertex_props[*vi].is_source;
            }

            pgm_tap::TrafoGraphEdgeProperties const regulated_edge_weights = get_edge_weights(g);
            pgm_tap::TrafoGraphEdgeProperties const ref_regulated_edge_weights{{{0, 1}, 0}, {{2, 3}, 2}};
            CHECK(regulated_edge_weights == ref_regulated_edge_weights);
        }

        SUBCASE("Sorting transformer edges") {
            pgm_tap::TrafoGraphEdgeProperties const trafoList{
                {Idx2D{1, 1}, pgm_tap::infty}, {Idx2D{1, 2}, 5}, {Idx2D{1, 3}, 4}, {Idx2D{2, 1}, 4}};

            pgm_tap::RankedTransformerGroups const referenceList{
                {Idx2D{1, 3}, Idx2D{2, 1}}, {Idx2D{1, 2}}, {Idx2D{1, 1}}};

            pgm_tap::RankedTransformerGroups const sortedTrafoList = pgm_tap::rank_transformers(trafoList);
            REQUIRE(sortedTrafoList.size() == referenceList.size());
            for (Idx idx : boost::counting_range(Idx{0}, static_cast<Idx>(sortedTrafoList.size()))) {
                CAPTURE(idx);
                CHECK(sortedTrafoList[idx] == referenceList[idx]);
            }
        }

        SUBCASE("Multiple source grid") {
            using vertex_iterator = boost::graph_traits<pgm_tap::TransformerGraph>::vertex_iterator;

            // Grid with multiple sources and symetric graph
            pgm_tap::TrafoGraphEdges const edge_array = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
            pgm_tap::TrafoGraphEdgeProperties const edge_prop{
                {{0, 1}, 1}, {{1, 2}, 1}, {{2, 3}, 1}, {{3, 4}, 1}, {{4, 5}, 1}};
            std::vector<pgm_tap::TrafoGraphVertex> vertex_props{{true}, {false}, {false}, {false}, {false}, {true}};

            pgm_tap::TransformerGraph g{boost::edges_are_unsorted_multi_pass, edge_array.cbegin(), edge_array.cend(),
                                        edge_prop.cbegin(), 6};

            // Vertex properties can not be set during graph creation
            vertex_iterator vi;
            vertex_iterator vi_end;
            for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
                g[*vi].is_source = vertex_props[*vi].is_source;
            }

            pgm_tap::TrafoGraphEdgeProperties const regulated_edge_weights = get_edge_weights(g);
            pgm_tap::TrafoGraphEdgeProperties const ref_regulated_edge_weights{
                {{0, 1}, 0}, {{1, 2}, 1}, {{2, 3}, 2}, {{3, 4}, 1}, {{4, 5}, 0}};
            CHECK(regulated_edge_weights == ref_regulated_edge_weights);
        }

        SUBCASE("Ranking complete the graph") {
            pgm_tap::RankedTransformerGroups order = pgm_tap::rank_transformers(state);
            pgm_tap::RankedTransformerGroups const ref_order{{Idx2D{3, 0}, Idx2D{3, 1}, Idx2D{4, 0}},
                                                             {Idx2D{3, 3}, Idx2D{3, 2}, Idx2D{3, 4}}};
            CHECK(order == ref_order);
        }
    }
}

namespace optimizer::tap_position_optimizer::test {
namespace {
using power_grid_model::optimizer::test::ConstDatasetUpdate;
using power_grid_model::optimizer::test::OptStrategyMethodSearch;
using power_grid_model::optimizer::test::search_methods;
using power_grid_model::optimizer::test::strategies_and_methods;
using power_grid_model::optimizer::test::strategies_and_sides;
using power_grid_model::optimizer::test::strategy_search_and_sides;
using power_grid_model::optimizer::test::StubTransformer;
using power_grid_model::optimizer::test::StubTransformerInput;
using power_grid_model::optimizer::test::StubTransformerUpdate;

template <typename ContainerType>
    requires main_core::main_model_state_c<main_core::MainModelState<ContainerType>>
class MockSolverOutput : public SolverOutput<symmetric_t> {
  public:
    using type = solver_output_t;
    using sym = typename SolverOutput<symmetric_t>::sym;

    Idx call_index{-1};
    CalculationMethod method;
    std::optional<std::reference_wrapper<main_core::MainModelState<ContainerType> const>> state;
    std::map<ID, IntS> state_tap_positions;
    std::map<ID, IntS> output_tap_positions;

    template <component_c... Ts>
    MockSolverOutput(Idx call_index_, CalculationMethod method_,
                     std::reference_wrapper<main_core::MainModelState<ContainerType> const> state_)
        : call_index{call_index_}, method{method_}, state{state_} {
        add_tap_positions(state.value().get());
    }

  private:
    void add_tap_positions(main_core::MainModelState<ContainerType> const& state) {
        AddTapPositions<typename ContainerType::gettable_types>{}(state, state_tap_positions);
    }

    template <typename... T> struct AddTapPositions;
    template <typename T> struct AddTapPositions<T> {
        void operator()(main_core::MainModelState<ContainerType> const& state, std::map<ID, IntS>& target) {
            if constexpr (transformer_c<T> && main_core::component_container_c<ContainerType, T>) {
                for (auto const& component : state.components.template citer<T>()) {
                    target[component.id()] = component.tap_pos();
                }
            }
        }
    };
    template <typename... Ts> struct AddTapPositions<std::tuple<Ts...>> {
        void operator()(main_core::MainModelState<ContainerType> const& state, std::map<ID, IntS>& target) {
            (AddTapPositions<Ts>{}(state, target), ...);
        }
    };
};

template <typename ContainerType>
using MockStateCalculator = std::vector<MockSolverOutput<ContainerType>> (*)(
    main_core::MainModelState<ContainerType> const& state, CalculationMethod method);

struct A {};
struct B {};
struct C {};
static_assert(std::same_as<transformer_types_t<A, A>, std::tuple<>>);
static_assert(std::same_as<transformer_types_t<A, B>, std::tuple<>>);
static_assert(std::same_as<transformer_types_t<A, Transformer>, std::tuple<Transformer>>);
static_assert(std::same_as<transformer_types_t<Transformer, ThreeWindingTransformer>,
                           std::tuple<Transformer, ThreeWindingTransformer>>);
static_assert(std::same_as<transformer_types_t<A, Transformer, A, B, ThreeWindingTransformer, C>,
                           std::tuple<Transformer, ThreeWindingTransformer>>);
static_assert(std::same_as<transformer_types_t<std::tuple<A, Transformer, A, B, ThreeWindingTransformer, C>>,
                           std::tuple<Transformer, ThreeWindingTransformer>>);
static_assert(std::same_as<transformer_types_t<std::tuple<A, StubTransformer, A, B, StubTransformer, C>>,
                           std::tuple<StubTransformer, StubTransformer>>);

struct MockTransformerState {
    using SideType = ControlSide;

    static constexpr auto unregulated{-1};

    ID id{};

    std::function<ID(SideType)> node = [](SideType /*side*/) { return ID{}; };
    std::function<bool(SideType)> status = [](SideType /*side*/) { return true; };

    SideType tap_side{};
    IntS tap_pos{};
    IntS tap_min{};
    IntS tap_max{};
    IntS tap_nom{};

    Idx topology_index{};
    Idx rank{unregulated};
    Idx2D math_id{};

    std::function<ComplexValue<symmetric_t>(SideType)> i_pu = [](SideType /*side*/) {
        return ComplexValue<symmetric_t>{};
    };
    std::function<ComplexValue<symmetric_t>(SideType)> u_pu = [](SideType /*side*/) {
        return ComplexValue<symmetric_t>{};
    };
};

struct MockTransformer {
    using InputType = StubTransformerInput;
    using UpdateType = StubTransformerUpdate;
    using SideType = typename MockTransformerState::SideType;

    MockTransformer() = default;
    MockTransformer(MockTransformerState state_) : state{std::move(state_)} {}

    static constexpr auto name = "MockTransformer";

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static) // mocks non-static functions
    constexpr auto math_model_type() const { return ComponentType::test; }

    constexpr auto id() const { return state.id; }
    auto node(SideType side) const { return state.node(side); }
    auto status(SideType side) const { return state.status(side); }

    constexpr auto tap_side() const { return state.tap_side; }
    constexpr auto tap_pos() const { return state.tap_pos; }
    constexpr auto tap_min() const { return state.tap_min; }
    constexpr auto tap_max() const { return state.tap_max; }
    constexpr auto tap_nom() const { return state.tap_nom; }

    auto update(UpdateType const& update) {
        CHECK(update.id == id());
        UpdateChange result;
        if (!is_nan(update.tap_pos)) {
            CHECK(update.tap_pos >= std::min(state.tap_min, state.tap_max));
            CHECK(update.tap_pos <= std::max(state.tap_min, state.tap_max));

            result.param = state.tap_pos != update.tap_pos;
            state.tap_pos = update.tap_pos;
        }
        return result;
    }

    auto inverse(UpdateType update) const {
        CHECK(update.id == state.id);
        auto const tap_pos_update = is_nan(update.tap_pos) ? na_IntS : tap_pos();
        return UpdateType{.id = id(), .tap_pos = tap_pos_update};
    }

    MockTransformerState state{};
};
static_assert(transformer_c<MockTransformer>);

template <std::derived_from<MockTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topology_index(State const& state, auto const& id_or_index) {
    auto const& transformer = main_core::get_component<ComponentType>(state, id_or_index);
    return transformer.state.math_id.pos;
}

template <std::derived_from<MockTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& state, Idx topology_index) {
    return main_core::get_component_by_sequence<MockTransformer>(state, topology_index).state.math_id;
}

template <std::derived_from<MockTransformer> ComponentType, typename ContainerType>
inline DoubleComplex i_pu(std::vector<MockSolverOutput<ContainerType>> const& solver_output, Idx2D const& math_id,
                          ControlSide side) {
    REQUIRE(math_id.group >= 0);
    REQUIRE(math_id.group < solver_output.size());

    CHECK(solver_output[math_id.group].call_index >= 0);

    auto const& state = solver_output[math_id.group].state;
    REQUIRE(state.has_value());
    if (state.has_value()) { // necessary for clang-tidy
        return main_core::get_component_by_sequence<MockTransformer>(state.value().get(), math_id.pos).state.i_pu(side);
    }
    FAIL("Unreachable");
    return {};
}

template <std::derived_from<MockTransformer> ComponentType, typename State,
          steady_state_solver_output_type SolverOutputType>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
inline auto u_pu(State const& state, std::vector<SolverOutputType> const& /* solver_output */, Idx topology_index,
                 ControlSide side) {
    return main_core::get_component_by_sequence<MockTransformer>(state, topology_index).state.u_pu(side);
}

template <std::derived_from<MockTransformer> ComponentType, typename State>
inline auto get_topo_node(State const& /*state*/, Idx /*topology_index*/, ControlSide /*control_side*/) {
    return 0;
}

template <typename ComponentType, typename State>
inline auto get_math_id(State const& /*state*/, Idx /*topology_index*/) {
    return Idx2D{0, 0};
}

template <typename ContainerType>
std::vector<MockSolverOutput<ContainerType>>
mock_state_calculator(main_core::MainModelState<ContainerType> const& state, CalculationMethod method) {
    static Idx call_count{};

    return {{call_count++, method, std::cref(state)}};
}

template <main_core::main_model_state_c State> class MockTransformerRanker {
  private:
    template <typename... Ts> struct Impl;
    template <typename ComponentType> struct Impl<ComponentType> {
        void operator()(State const& state, RankedTransformerGroups& ranking) const {
            if constexpr (std::derived_from<ComponentType, MockTransformer>) {
                for (Idx const idx :
                     boost::counting_range(Idx{0}, main_core::get_component_size<ComponentType>(state))) {
                    auto const& comp = main_core::get_component_by_sequence<ComponentType>(state, idx);
                    auto const rank = comp.state.rank;
                    if (rank == MockTransformerState::unregulated) {
                        continue;
                    }

                    REQUIRE(rank >= 0);
                    if (rank >= static_cast<Idx>(ranking.size())) {
                        ranking.resize(rank + 1);
                    }
                    ranking[rank].push_back(
                        {.group = main_core::get_component_type_index<ComponentType>(state), .pos = idx});
                }
            }
        }
    };
    template <typename... ComponentTypes> struct Impl<std::tuple<ComponentTypes...>> {
        auto operator()(State const& state) const -> RankedTransformerGroups {
            RankedTransformerGroups ranking;
            (Impl<ComponentTypes>{}(state, ranking), ...);
            return ranking;
        }
    };

  public:
    auto operator()(State const& state) const -> RankedTransformerGroups {
        return Impl<typename State::ComponentContainer::gettable_types>{}(state);
    }
};

using TapPositionCheckFunc = std::function<void(IntS, OptimizerStrategy)>;

auto check_exact(IntS tap_pos) -> TapPositionCheckFunc {
    return [tap_pos](IntS value, OptimizerStrategy /*strategy*/) { CHECK(value == tap_pos); };
};
auto check_exact_per_strategy(IntS tap_pos_any, IntS tap_pos_min, IntS tap_pos_max) -> TapPositionCheckFunc {
    return [tap_pos_any, tap_pos_min, tap_pos_max](IntS value, OptimizerStrategy strategy) {
        using enum OptimizerStrategy;

        switch (strategy) {
        case any:
            CHECK(value == tap_pos_any);
            break;
        case local_maximum:
        case global_maximum:
            CHECK(value == tap_pos_max);
            break;
        case local_minimum:
        case global_minimum:
            CHECK(value == tap_pos_min);
            break;
        default:
            FAIL("Unreachable");
        }
    };
}
auto normalized_lerp(IntS value, IntS start, IntS stop) {
    REQUIRE(start != stop);
    return (static_cast<double>(value) - static_cast<double>(start)) /
           (static_cast<double>(stop) - static_cast<double>(start));
}
void checkNormalTapRange(MockTransformerState& state_b, TransformerTapRegulatorUpdate& update_data, auto& check_b) {
    state_b.tap_min = 1;
    state_b.tap_max = 5;
    state_b.tap_pos = 3;

    SUBCASE("unique value in band") {
        update_data.u_band = 0.01;
        check_b = test::check_exact(3);
    }
    SUBCASE("large compact band") {
        update_data.u_band = 1.01;
        check_b = test::check_exact_per_strategy(3, 5, 1);
    }
    SUBCASE("small open band") {
        update_data.u_band = 0.76;
        check_b = test::check_exact_per_strategy(3, 4, 2);
    }
}

void checkInvertedTapRange(MockTransformerState& state_b, TransformerTapRegulatorUpdate& update_data, auto& check_b) {
    state_b.tap_min = 5;
    state_b.tap_max = 1;
    state_b.tap_pos = 3;

    SUBCASE("unique value in band") {
        update_data.u_band = 0.01;
        check_b = test::check_exact(3);
    }
    SUBCASE("large compact band") {
        update_data.u_band = 1.01;
        check_b = test::check_exact_per_strategy(3, 1, 5);
    }
    SUBCASE("small open band") {
        update_data.u_band = 0.76;
        check_b = test::check_exact_per_strategy(3, 2, 4);
    }
}
} // namespace
} // namespace optimizer::tap_position_optimizer::test

namespace {
namespace meta_gen = meta_data::meta_data_gen;
} // namespace

TEST_CASE("Test Tap position optimizer") {
    using MockTransformerState = test::MockTransformerState;
    using MockTransformer = test::MockTransformer;
    using MockContainer = Container<ExtraRetrievableTypes<Regulator>, MockTransformer, TransformerTapRegulator>;
    using MockState = main_core::MainModelState<MockContainer>;
    using MockStateCalculator = test::MockStateCalculator<MockContainer>;
    using MockTransformerRanker = test::MockTransformerRanker<MockState>;

    auto const meta_data = meta_gen::get_meta_data<ComponentList<MockTransformer, TransformerTapRegulator>,
                                                   meta_data::update_getter_s>::value;

    MockState state;

    auto strategy_method_searches = [&] {
        std::array<test::OptStrategyMethodSearch, test::strategies_and_methods.size() * test::search_methods.size()>
            result;
        size_t idx{};
        for (auto strategy_method : test::strategies_and_methods) {
            for (auto search_method : test::search_methods) {
                result[idx++] = {strategy_method.strategy, strategy_method.method, search_method}; // NOSONAR
            }
        }
        return result;
    }();

    auto const updater = [&state](ConstDataset const& update_dataset) {
        REQUIRE(!update_dataset.empty());
        REQUIRE(update_dataset.n_components() == 1);
        REQUIRE(update_dataset.contains_component(MockTransformer::name));
        auto const& transformers_dataset =
            update_dataset.get_buffer_span<meta_data::update_getter_s, MockTransformer>();
        auto changed_components = std::vector<Idx2D>{};
        main_core::update::update_component<MockTransformer>(
            state, transformers_dataset.begin(), transformers_dataset.end(), std::back_inserter(changed_components));
    };

    auto twoStatesEqual = [](const MockState& state1, const MockState& state2) {
        if (state1.components.template size<MockTransformer>() != state2.components.template size<MockTransformer>()) {
            return false;
        }
        std::vector<std::pair<IntS, IntS>> trafo_state_1;
        std::vector<std::pair<IntS, IntS>> trafo_state_2;
        for (auto const& transformer : state1.components.template citer<MockTransformer>()) {
            trafo_state_1.emplace_back(transformer.id(), transformer.tap_pos());
        }
        for (auto const& transformer : state2.components.template citer<MockTransformer>()) {
            trafo_state_2.emplace_back(transformer.id(), transformer.tap_pos());
        }

        return trafo_state_1 == trafo_state_2;
    };

    auto const get_optimizer = [&](OptimizerStrategy strategy, SearchMethod tap_search) {
        return pgm_tap::TapPositionOptimizer<MockStateCalculator, decltype(updater), MockState, MockTransformerRanker>{
            test::mock_state_calculator, updater, strategy, meta_data, tap_search};
    };

    SUBCASE("empty state") {
        state.components.set_construction_complete();
        auto optimizer = get_optimizer(OptimizerStrategy::any, SearchMethod::linear_search);
        auto result = optimizer.optimize(state, CalculationMethod::default_method);
        CHECK(result.solver_output.size() == 1);
        CHECK(result.solver_output[0].method == CalculationMethod::default_method);
    }

    SUBCASE("Calculation method") {
        main_core::emplace_component<test::MockTransformer>(
            state, 1, MockTransformerState{.id = 1, .math_id = {.group = 0, .pos = 0}});
        main_core::emplace_component<test::MockTransformer>(
            state, 2, MockTransformerState{.id = 2, .math_id = {.group = 0, .pos = 1}});
        state.components.set_construction_complete();

        for (auto strategy_method_search : strategy_method_searches) {
            auto strategy = strategy_method_search.strategy;
            auto method = strategy_method_search.method;
            auto search = strategy_method_search.search;
            CAPTURE(strategy);
            CAPTURE(method);
            CAPTURE(search);

            if (strategy == OptimizerStrategy::any && search == SearchMethod::binary_search) {
                CHECK_THROWS_AS(get_optimizer(strategy, search), TapSearchStrategyIncompatibleError);

            } else {
                auto optimizer = get_optimizer(strategy, search);
                auto result = optimizer.optimize(state, method);

                CHECK(result.solver_output.size() == 1);
                CHECK(result.solver_output[0].method == method);
            }
        }
    }

    SUBCASE("optimization") {
        main_core::emplace_component<test::MockTransformer>(
            state, 1, MockTransformerState{.id = 1, .math_id = {.group = 0, .pos = 0}});
        main_core::emplace_component<test::MockTransformer>(
            state, 2, MockTransformerState{.id = 2, .math_id = {.group = 0, .pos = 1}});

        auto& transformer_a = main_core::get_component<MockTransformer>(state, 1);
        auto& transformer_b = main_core::get_component<MockTransformer>(state, 2);

        main_core::emplace_component<TransformerTapRegulator>(
            state, 3,
            TransformerTapRegulatorInput{.id = 3,
                                         .regulated_object = 1,
                                         .status = 1,
                                         .control_side = ControlSide::side_1,
                                         .u_set = 0.0,
                                         .u_band = 0.0,
                                         .line_drop_compensation_r = 0.0,
                                         .line_drop_compensation_x = 0.0},
            transformer_a.math_model_type(), 1.0);
        main_core::emplace_component<TransformerTapRegulator>(
            state, 4,
            TransformerTapRegulatorInput{.id = 4,
                                         .regulated_object = 2,
                                         .status = 1,
                                         .control_side = ControlSide::side_2,
                                         .u_set = 0.0,
                                         .u_band = 0.0,
                                         .line_drop_compensation_r = 0.0,
                                         .line_drop_compensation_x = 0.0},
            transformer_b.math_model_type(), 1.0);

        auto& regulator_a = main_core::get_component<TransformerTapRegulator>(state, 3);
        auto& regulator_b = main_core::get_component<TransformerTapRegulator>(state, 4);

        state.components.set_construction_complete();

        auto& state_a = transformer_a.state;
        auto& state_b = transformer_b.state;

        SUBCASE("tap position in range") {
            auto check_a = test::check_exact(0);
            auto check_b = test::check_exact(0);

            SUBCASE("not regulatable") {
                state_b.tap_pos = 1;
                state_b.tap_min = 1;
                state_b.tap_max = 1;
                state_b.rank = MockTransformerState::unregulated;
                check_b = [&state_b](IntS value, OptimizerStrategy /*strategy*/) { CHECK(value == state_b.tap_pos); };
                auto const control_side = main_core::get_component<TransformerTapRegulator>(state, 4).control_side();

                SUBCASE("not regulated") {}

                SUBCASE("not connected at tap side") {
                    state_b.status = [&state_b](ControlSide side) { return side != state_b.tap_side; };
                }

                SUBCASE("not connected at control side") {
                    state_b.status = [control_side](ControlSide side) { return side != control_side; };
                }

                SUBCASE("not connected at third side doesn't matter") {
                    check_b = test::check_exact(1);
                    state_b.rank = 0;
                    state_b.status = [control_side, &state_b](ControlSide side) {
                        return side == control_side || side == state_b.tap_side;
                    };
                }
            }

            SUBCASE("single valid value") {
                state_b.tap_pos = 1;
                state_b.tap_min = state_b.tap_pos;
                state_b.tap_max = state_b.tap_pos;
                state_b.rank = 0;
                check_b = test::check_exact(state_b.tap_pos);
            }

            SUBCASE("multipe valid values") {
                state_b.rank = 0;
                check_b = [&state_b](IntS value, OptimizerStrategy strategy) {
                    switch (strategy) {
                        using enum OptimizerStrategy;
                    case any:
                        CHECK(value == state_b.tap_pos);
                        break;
                    case local_maximum:
                    case global_maximum:
                        // max voltage => min tap pos
                        CHECK(value == state_b.tap_min);
                        break;
                    case local_minimum:
                    case global_minimum:
                        // min voltage => max tap pos
                        CHECK(value == state_b.tap_max);
                        break;
                    default:
                        FAIL("unreachable");
                    }
                };
                SUBCASE("normal tap range") {
                    state_b.tap_min = 1;
                    state_b.tap_max = 3;
                    SUBCASE("start low in range") { state_b.tap_pos = state_b.tap_min; }
                    SUBCASE("start high in range") { state_b.tap_pos = state_b.tap_max; }
                    SUBCASE("start mid range") { state_b.tap_pos = narrow_cast<IntS>(state_b.tap_min + 1); }
                }
                SUBCASE("inverted tap range") {
                    state_b.tap_min = 3;
                    state_b.tap_max = 1;
                    SUBCASE("start low in range") { state_b.tap_pos = state_b.tap_min; }
                    SUBCASE("start high in range") { state_b.tap_pos = state_b.tap_max; }
                    SUBCASE("start mid range") { state_b.tap_pos = narrow_cast<IntS>(state_b.tap_min - 1); }
                }
                SUBCASE("extreme tap range") {
                    state_b.tap_min = IntS{0};
                    state_b.tap_max = IntS{127};
                    SUBCASE("start low in range") { state_b.tap_pos = state_b.tap_min; }
                    SUBCASE("start high in range") { state_b.tap_pos = state_b.tap_max; }
                    SUBCASE("start mid range") { state_b.tap_pos = 64; }
                }
                SUBCASE("extreme inverted tap range") {
                    state_b.tap_min = IntS{127};
                    state_b.tap_max = IntS{0};
                    SUBCASE("start low in range") { state_b.tap_pos = state_b.tap_min; }
                    SUBCASE("start high in range") { state_b.tap_pos = state_b.tap_max; }
                    SUBCASE("start mid range") { state_b.tap_pos = 64; }
                }
            }

            SUBCASE("voltage band") {
                state_b.rank = 0;
                state_b.u_pu = [&state_b, &regulator_b](ControlSide side) {
                    CHECK(side == regulator_b.control_side());

                    // tap pos closer to tap_max at tap side <=> lower voltage at control side
                    return static_cast<DoubleComplex>(
                        test::normalized_lerp(state_b.tap_pos, state_b.tap_max, state_b.tap_min));
                };

                auto update_data = TransformerTapRegulatorUpdate{.id = 4, .u_set = 0.5, .u_band = 0.0};

                SUBCASE("normal tap range") { checkNormalTapRange(state_b, update_data, check_b); }
                SUBCASE("inverted tap range") { checkInvertedTapRange(state_b, update_data, check_b); }

                regulator_b.update(update_data);
            }

            SUBCASE("line drop compensation") {
                state_b.rank = 0;
                state_b.u_pu = [&state_b, &regulator_b](ControlSide side) {
                    CHECK(side == regulator_b.control_side());

                    // tap pos closer to tap_max at tap side <=> lower voltage at control side
                    return static_cast<DoubleComplex>(
                        test::normalized_lerp(state_b.tap_pos, state_b.tap_max, state_b.tap_min));
                };
                state_b.i_pu = [&state_b, &regulator_b](ControlSide side) {
                    CHECK(side == regulator_b.control_side());
                    auto const value = test::normalized_lerp(state_b.tap_pos, state_b.tap_min, state_b.tap_max);
                    return DoubleComplex{value, value};
                };

                auto update_data = TransformerTapRegulatorUpdate{.id = 4, .u_set = 0.5, .u_band = 0.76};

                state_b.tap_min = 1;
                state_b.tap_max = 5;
                state_b.tap_pos = 3;

                SUBCASE("no line drop compensation") { check_b = test::check_exact_per_strategy(3, 4, 2); }
                SUBCASE("resistance") {
                    update_data.line_drop_compensation_r = 0.5 / base_power_3p;
                    check_b = test::check_exact_per_strategy(3, 5, 3);
                }
                SUBCASE("positive reactance") {
                    update_data.line_drop_compensation_x = 0.125 / base_power_3p;
                    check_b = test::check_exact_per_strategy(3, 5, 2);
                }
                SUBCASE("negative reactance") {
                    update_data.line_drop_compensation_x = -0.5 / base_power_3p;
                    check_b = test::check_exact_per_strategy(3, 5, 3);
                }

                regulator_b.update(update_data);
            }

            SUBCASE("multiple transformers with control function based on ranking") {
                state_a.rank = 0;
                state_b.rank = 1;
                state_a.tap_min = -5;
                state_a.tap_max = 5;
                state_b.tap_min = -5;
                state_b.tap_max = 5;

                state_a.u_pu = [&state_a, &regulator_a](ControlSide side) {
                    CHECK(side == regulator_a.control_side());

                    // u_2a = f(tap_pos_a) when rank is 0
                    // u_2a = (u_1a * n_1) / (1.0 + relative_tap_pos_a)
                    // consider u_1a = n_1 = 1.0
                    // For a tap_size of 0.1 and tap_nom of 0, tap_pos_relative_a = 0.1 * (tap_pos_a - 0)
                    auto const relative_tap_a = static_cast<double>(state_a.tap_pos) * 0.1;
                    return static_cast<DoubleComplex>(1.0 / (1.0 + relative_tap_a));
                };

                state_b.u_pu = [&state_a, &regulator_a, &state_b, &regulator_b](ControlSide side) {
                    CHECK(side == regulator_b.control_side());

                    // u_2b = f(tap_pos_a, tap_pos_b) when rank is 1
                    // u_2b = (u_1b * n_2) / (1.0 + relative_tap_pos_b)
                    // consider n_2 = 1. Also u_1a = u_2b
                    // For a tap_size of 0.1 and tap_nom of 0, tap_pos_relative_b = 0.1 * (tap_pos_b - 0)
                    auto const relative_tap_b = static_cast<double>(state_b.tap_pos) * 0.1;
                    return state_a.u_pu(regulator_a.control_side()) / (1.0 + relative_tap_b);
                };

                SUBCASE("Situation 1") {
                    regulator_a.update({.id = 3, .u_set = 1.25, .u_band = 0.01});
                    regulator_b.update({.id = 4, .u_set = 1.13636, .u_band = 0.01});
                    check_a = test::check_exact(-2);
                    check_b = test::check_exact(1);
                }
                SUBCASE("Situation 2") {
                    regulator_a.update({.id = 3, .u_set = 1.1111, .u_band = 0.01});
                    regulator_b.update({.id = 4, .u_set = 1.5873, .u_band = 0.01});
                    check_a = test::check_exact(-1);
                    check_b = test::check_exact(-3);
                }
                SUBCASE("Situation 3") {
                    regulator_a.update({.id = 3, .u_set = 1.0, .u_band = 0.01});
                    regulator_b.update({.id = 4, .u_set = 0.7142, .u_band = 0.01});
                    check_a = test::check_exact(0);
                    check_b = test::check_exact(4);
                }
            }

            SUBCASE("multiple transformers with generic control function") {
                state_a.tap_min = 0;
                state_a.tap_max = 2;
                state_b.tap_min = 0;
                state_b.tap_max = 2;
                regulator_a.update({.id = 3, .u_set = 1.0, .u_band = 0.2});
                regulator_b.update({.id = 4, .u_set = 1.0, .u_band = 0.2});

                // Both control side voltages have a function which follows this table
                // t_a \ t_b |  0   |  1   |  2   |  3
                // --------- | ---- | ---- | ---- | ----
                // 0         | 1.5  | 1.25 | 1.0  | 0.75
                // 1         | 1.25 | 1.0  | 0.75 | 0.5
                // 2         | 1.0  | 0.75 | 0.5  | 0.25
                // 3         | 0.75 | 0.5  | 0.25 | 0.0

                state_a.u_pu = [&state_a, &state_b, &regulator_a](ControlSide side) {
                    CHECK(side == regulator_a.control_side());
                    auto const tap_sum = static_cast<double>(state_a.tap_pos + state_b.tap_pos);
                    return static_cast<DoubleComplex>(1.5 - tap_sum / 4.0);
                };

                state_b.u_pu = [&state_a, &state_b, &regulator_b](ControlSide side) {
                    CHECK(side == regulator_b.control_side());
                    auto const tap_sum = static_cast<double>(state_a.tap_pos + state_b.tap_pos);
                    return static_cast<DoubleComplex>(1.5 - tap_sum / 4.0);
                };

                SUBCASE("Rank a < Rank b") {
                    state_a.rank = 0;
                    state_b.rank = 1;
                    check_a = test::check_exact_per_strategy(2, 0, 2);
                    check_b = test::check_exact_per_strategy(0, 2, 0);
                }
                SUBCASE("Rank a > Rank b") {
                    state_a.rank = 1;
                    state_b.rank = 0;
                    check_a = test::check_exact_per_strategy(0, 2, 0);
                    check_b = test::check_exact_per_strategy(2, 0, 2);
                }
                SUBCASE("Rank a == Rank b") {
                    state_a.rank = 0;
                    state_b.rank = 0;
                    check_a = test::check_exact(1);
                    check_b = test::check_exact(1);
                }
            }

            auto const initial_a{transformer_a.tap_pos()};
            auto const initial_b{transformer_b.tap_pos()};

            for (auto strategy_search_side : test::strategy_search_and_sides) {
                auto strategy = strategy_search_side.strategy;
                auto search = strategy_search_side.search;
                auto tap_side = strategy_search_side.side;
                CAPTURE(strategy);
                CAPTURE(search);
                CAPTURE(tap_side);

                state_b.tap_side = tap_side;
                state_a.tap_side = tap_side;

                auto optimizer = get_optimizer(strategy, search);
                auto const result = optimizer.optimize(state, CalculationMethod::default_method);

                auto const get_state_tap_pos = [&](ID const id) {
                    REQUIRE(!result.solver_output.empty());
                    return result.solver_output.front().state_tap_positions.at(id);
                };
                auto const get_output_tap_pos = [&](ID const id) {
                    REQUIRE(!result.optimizer_output.transformer_tap_positions.empty());
                    auto const it = std::ranges::find_if(result.optimizer_output.transformer_tap_positions,
                                                         [id](auto const& x) { return x.transformer_id == id; });
                    REQUIRE(it != std::end(result.optimizer_output.transformer_tap_positions));
                    CHECK(it->transformer_id == id);
                    return it->tap_position;
                };

                // check optimal state
                CHECK(result.solver_output.size() == 1);
                check_a(get_state_tap_pos(state_a.id), strategy);
                check_b(get_state_tap_pos(state_b.id), strategy);

                // check optimal output
                if (state_a.rank != MockTransformerState::unregulated) {
                    check_a(get_output_tap_pos(state_a.id), strategy);
                }
                if (state_b.rank != MockTransformerState::unregulated) {
                    check_b(get_output_tap_pos(state_b.id), strategy);
                }

                // reset
                CHECK(transformer_a.tap_pos() == initial_a);
                CHECK(transformer_b.tap_pos() == initial_b);
            }
        }

        SUBCASE("Check throw as MaxIterationReached") { // This only applies to non-binary search
            state_b.rank = 0;
            state_b.u_pu = [&state_b, &regulator_b](ControlSide side) {
                CHECK(side == regulator_b.control_side());

                // tap pos closer to tap_max at tap side <=> lower voltage at control side
                return static_cast<DoubleComplex>(
                    test::normalized_lerp(state_b.tap_pos, state_b.tap_max, state_b.tap_min));
            };

            auto update_data = TransformerTapRegulatorUpdate{.id = 4, .u_set = 0.4, .u_band = 0.0};

            // tap pos will jump between 3 and 4 in linear_search method
            state_b.tap_min = 1;
            state_b.tap_max = 5;
            state_b.tap_pos = 5;

            regulator_b.update(update_data);

            for (auto strategy_side : test::strategies_and_sides) {
                auto strategy = strategy_side.strategy;
                auto tap_side = strategy_side.side;
                CAPTURE(strategy);
                CAPTURE(tap_side);

                state_b.tap_side = tap_side;
                state_a.tap_side = tap_side;

                auto optimizer = get_optimizer(strategy, SearchMethod::linear_search);
                auto const cached_state = MockState{state}; // explicit copy
                CHECK_THROWS_AS(optimizer.optimize(state, CalculationMethod::default_method), MaxIterationReached);
                CHECK(twoStatesEqual(cached_state, state));
            }
        }
    }
}

TEST_CASE("Test tap position optmizer I/O") {
    SUBCASE("transformer duplicatively regulated") {
        test::TestState state_mini;
        std::vector<NodeInput> nodes{{0, 150e3}, {1, 10e3}, {2, 10e3}, {3, 10e3}};
        main_core::add_component<Node>(state_mini, nodes.begin(), nodes.end(), 50.0);

        std::vector<TransformerInput> transformers{test::get_transformer(4, 0, 1, BranchSide::from, 0),
                                                   test::get_transformer(5, 1, 2, BranchSide::from, -1),
                                                   test::get_transformer(6, 2, 3, BranchSide::from, 1)};
        main_core::add_component<Transformer>(state_mini, transformers.begin(), transformers.end(), 50.0);

        std::vector<TransformerTapRegulatorInput> bad_regulators{
            TransformerTapRegulatorInput{.id = 7,
                                         .regulated_object = 4,
                                         .status = 1,
                                         .control_side = ControlSide::side_1,
                                         .u_set = 0.0,
                                         .u_band = 0.0,
                                         .line_drop_compensation_r = 0.0,
                                         .line_drop_compensation_x = 0.0},
            TransformerTapRegulatorInput{.id = 8,
                                         .regulated_object = 5,
                                         .status = 1,
                                         .control_side = ControlSide::side_2,
                                         .u_set = 0.0,
                                         .u_band = 0.0,
                                         .line_drop_compensation_r = 0.0,
                                         .line_drop_compensation_x = 0.0},
            TransformerTapRegulatorInput{.id = 9,
                                         .regulated_object = 5,
                                         .status = 1,
                                         .control_side = ControlSide::side_2,
                                         .u_set = 0.0,
                                         .u_band = 0.0,
                                         .line_drop_compensation_r = 0.0,
                                         .line_drop_compensation_x = 0.0},
        };

        CHECK_THROWS_AS(main_core::add_component<TransformerTapRegulator>(state_mini, bad_regulators.begin(),
                                                                          bad_regulators.end(), 50.0),
                        DuplicativelyRegulatedObject);
    }
}

TEST_CASE("Test RankIterator") {
    std::vector<std::vector<IntS>> const regulator_order = {{0, 0, 0}, {0, 0, 0}};
    bool tap_changed{false};
    std::vector<IntS> iterations_per_rank = {2, 4, 6};
    Idx rank_index{0};
    bool update{false};
    optimizer::tap_position_optimizer::RankIteration rank_iterator(iterations_per_rank, rank_index);

    auto const mock_lambda = [&](Idx const& /*rank_idx*/, Idx const& /*transformer_idx*/,
                                 std::vector<IntS> const& /*same_rank_regulators*/) { return update; };
    SUBCASE("Test tap not changed") {
        tap_changed = rank_iterator.iterate_ranks(regulator_order, mock_lambda, tap_changed);
        iterations_per_rank = rank_iterator.iterations_per_rank();
        rank_index = rank_iterator.rank_index();
        CHECK(iterations_per_rank[0] == 2);
        CHECK(iterations_per_rank[1] == 4);
        CHECK(iterations_per_rank[2] == 6);
        CHECK(rank_index == 2);
        CHECK(tap_changed == false);
    }
    SUBCASE("Test tap changed") {
        update = true;
        tap_changed = rank_iterator.iterate_ranks(regulator_order, mock_lambda, tap_changed);
        iterations_per_rank = rank_iterator.iterations_per_rank();
        rank_index = rank_iterator.rank_index();
        CHECK(iterations_per_rank[0] == 3);
        CHECK(iterations_per_rank[1] == 0);
        CHECK(iterations_per_rank[2] == 0);
        CHECK(rank_index == 0);
    }
    SUBCASE("Test tap changed last rank") {
        update = true;
        rank_iterator.set_rank_index(2);
        tap_changed = rank_iterator.iterate_ranks(regulator_order, mock_lambda, tap_changed);
        iterations_per_rank = rank_iterator.iterations_per_rank();
        rank_index = rank_iterator.rank_index();
        CHECK(iterations_per_rank[0] == 2);
        CHECK(iterations_per_rank[1] == 4);
        CHECK(iterations_per_rank[2] == 7);
        CHECK(rank_index == 2);
    }
    SUBCASE("Test set rank_index") {
        rank_iterator.set_rank_index(1);
        CHECK(rank_iterator.rank_index() == 1);
    }
}

} // namespace power_grid_model

TEST_SUITE_END();
