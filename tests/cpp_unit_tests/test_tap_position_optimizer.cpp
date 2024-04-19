// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_optimizer.hpp"

#include <power_grid_model/main_core/input.hpp>
#include <power_grid_model/main_core/update.hpp>
#include <power_grid_model/optimizer/tap_position_optimizer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace optimizer::tap_position_optimizer::test {
namespace {
using power_grid_model::optimizer::test::ConstDatasetUpdate;
using power_grid_model::optimizer::test::strategies;
using power_grid_model::optimizer::test::strategies_and_methods;
using power_grid_model::optimizer::test::StubTransformer;
using power_grid_model::optimizer::test::StubTransformerInput;
using power_grid_model::optimizer::test::StubTransformerUpdate;

template <typename ContainerType>
    requires main_core::main_model_state_c<main_core::MainModelState<ContainerType>>
struct MockMathOutput : public MathOutput<symmetric_t> {
    Idx call_index;
    std::optional<std::reference_wrapper<main_core::MainModelState<ContainerType> const>> state;
    CalculationMethod method;
};

template <typename ContainerType>
using MockStateCalculator = std::vector<MockMathOutput<ContainerType>> (*)(
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
    std::function<bool(SideType)> status = [](SideType /*side*/) { return bool{}; };

    SideType tap_side{};
    IntS tap_pos{};
    IntS tap_min{};
    IntS tap_max{};
    IntS tap_nom{};

    Idx topology_index{};
    Idx rank{unregulated};

    Idx2D math_id{};
    std::function<RealValue<symmetric_t>(SideType)> i_pu = [](SideType /*side*/) { return RealValue<symmetric_t>{}; };
};

struct MockTransformer {
    using InputType = StubTransformerInput;
    using UpdateType = StubTransformerUpdate;
    using SideType = typename MockTransformerState::SideType;

    static constexpr auto name = "MockTransformer";
    constexpr auto math_model_type() const { return ComponentType::test; }

    constexpr auto id() const { return state.id; }
    constexpr auto node(SideType side) const { return state.node(side); }
    constexpr auto status(SideType side) const { return state.status(side); }

    constexpr auto tap_side() const { return state.tap_side; }
    constexpr auto tap_pos() const { return state.tap_pos; }
    constexpr auto tap_min() const { return state.tap_min; }
    constexpr auto tap_max() const { return state.tap_max; }
    constexpr auto tap_nom() const { return state.tap_nom; }

    auto update(UpdateType const& update) {
        CHECK(update.id == id());
        if (!is_nan(update.tap_pos)) {
            CHECK(update.tap_pos >= tap_min());
            CHECK(update.tap_pos <= tap_max());
            state.tap_pos = update.tap_pos;
        }
        return UpdateChange{};
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
    return transformer.state.topology_index;
}

template <std::derived_from<MockTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& state, Idx topology_index) {
    return main_core::get_component_by_sequence<MockTransformer>(state, topology_index).state.math_id;
}

template <std::derived_from<MockTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topo_node(State const& state, Idx topology_index, ControlSide side) {
    return main_core::get_component_by_sequence<MockTransformer>(state, topology_index).node(side);
}

template <std::derived_from<MockTransformer> ComponentType, typename ContainerType>
inline auto i_pu(std::vector<MockMathOutput<ContainerType>> const& math_output, Idx2D const& math_id,
                 ControlSide side) {
    REQUIRE(math_id.group >= 0);
    REQUIRE(math_id.group < math_output.size());
    REQUIRE(math_output[math_id.group].state.has_value());
    return main_core::get_component_by_sequence<MockTransformer>(math_output[math_id.group].state.value().get(),
                                                                 math_id.pos)
        .state.i_pu(side);
}

template <typename ContainerType>
std::vector<MockMathOutput<ContainerType>> mock_state_calculator(main_core::MainModelState<ContainerType> const& state,
                                                                 CalculationMethod method) {
    static Idx call_count{};

    return {{.call_index = call_count++, .state = std::cref(state), .method = method}};
}

template <main_core::main_model_state_c State> class MockTransformerRanker {
  private:
    template <typename... Ts> struct Impl;
    template <component_c ComponentType> struct Impl<ComponentType> {
        void operator()(State const& state, RankedTransformerGroups& ranking) const {
            if constexpr (std::derived_from<ComponentType, MockTransformer>) {
                for (Idx idx = 0; idx < main_core::get_component_size<ComponentType>(state); ++idx) {
                    auto const& comp = main_core::get_component_by_sequence<ComponentType>(state, idx);
                    auto const rank = comp.state.rank;
                    if (rank == MockTransformerState::unregulated) {
                        continue;
                    }

                    REQUIRE(rank >= 0);
                    if (rank >= ranking.size()) {
                        ranking.resize(rank + 1);
                    }
                    ranking[rank].emplace_back(main_core::get_component_type_index<ComponentType>(state), idx);
                }
            }
        }
    };
    template <component_c... ComponentTypes> struct Impl<std::tuple<ComponentTypes...>> {
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
} // namespace
} // namespace optimizer::tap_position_optimizer::test

namespace pgm_tap = optimizer::tap_position_optimizer;
namespace test = pgm_tap::test;

TEST_CASE("Test Transformer ranking") {
    // ToDo: The grid from OntNote page

    // Subcases
    SUBCASE("Building the graph") {
        // ToDo: graph creation
    }

    // Dummy graph
    std::vector<std::pair<Idx, Idx>> edge_array = {{0, 1}, {0, 2}, {2, 3}};

    std::vector<pgm_tap::TrafoGraphEdge> edge_prop;
    edge_prop.push_back(pgm_tap::TrafoGraphEdge({{0, 1}, 1}));
    edge_prop.push_back(pgm_tap::TrafoGraphEdge({{0, 2}, 2}));
    edge_prop.push_back(pgm_tap::TrafoGraphEdge({{2, 3}, 3}));

    std::vector<pgm_tap::TrafoGraphVertex> vertex_props{{true}, {false}, {false}, {false}};

    pgm_tap::TransformerGraph g{boost::edges_are_unsorted_multi_pass, edge_array.cbegin(), edge_array.cend(),
                                edge_prop.cbegin(), 4};

    // Vertex properties can not be set during graph creation
    boost::graph_traits<pgm_tap::TransformerGraph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
        g[*vi].is_source = vertex_props[*vi].is_source;
    }

    SUBCASE("Process edge weights") {
        auto const all_edge_weights = get_edge_weights(g);
        const pgm_tap::WeightedTrafoList ref_edge_weights{{{0, 0}, 0}, {{0, 1}, 1}, {{0, 2}, 2}, {{2, 3}, 5}};
        CHECK(all_edge_weights == ref_edge_weights);
    }

    SUBCASE("Sorting transformer edges") {
        pgm_tap::WeightedTrafoList trafoList{{Idx2D{1, 1}, pgm_tap::infty},
                                             {Idx2D{1, 2}, 5},
                                             {Idx2D{1, 3}, 4},
                                             {Idx2D{2, 1}, 4},
                                             {Idx2D{2, 2}, 3},
                                             {Idx2D{3, 1}, 2},
                                             {Idx2D{3, 2}, 1},
                                             {Idx2D{3, 3}, 1}};

        const pgm_tap::RankedTransformerGroups referenceList{{Idx2D{3, 2}, Idx2D{3, 3}}, {Idx2D{3, 1}}, {Idx2D{2, 2}},
                                                             {Idx2D{1, 3}, Idx2D{2, 1}}, {Idx2D{1, 2}}, {Idx2D{1, 1}}};

        auto const sortedTrafoList = pgm_tap::rank_transformers(trafoList);
        CHECK(sortedTrafoList == referenceList);
    }
}

TEST_CASE("Test Tap position optimizer") {
    using MockTransformerState = test::MockTransformerState;
    using MockTransformer = test::MockTransformer;
    using MockContainer = Container<MockTransformer, TransformerTapRegulator>;
    using MockState = main_core::MainModelState<MockContainer>;
    using MockStateCalculator = test::MockStateCalculator<MockContainer>;
    using MockStateUpdater = void (*)(ConstDataset const& update_dataset);
    using MockTransformerRanker = test::MockTransformerRanker<MockState>;

    MockState state;

    auto const updater = [&state](ConstDataset const& update_dataset) {
        REQUIRE(!update_dataset.empty());
        REQUIRE(update_dataset.size() == 1);
        REQUIRE(update_dataset.contains(MockTransformer::name));
        auto const& transformers_dataset = update_dataset.at(MockTransformer::name);
        auto const [transformers_update_begin, transformers_update_end] =
            transformers_dataset.get_iterators<typename MockTransformer::UpdateType>(-1);
        auto changed_components = std::vector<Idx2D>{};
        main_core::update_component<MockTransformer>(state, transformers_update_begin, transformers_update_end,
                                                     std::back_inserter(changed_components));
    };

    auto const get_optimizer = [&](OptimizerStrategy strategy) {
        return pgm_tap::TapPositionOptimizer<MockStateCalculator, decltype(updater), MockState, MockTransformerRanker>{
            test::mock_state_calculator, updater, strategy};
    };

    SUBCASE("empty state") {
        state.components.set_construction_complete();
        auto optimizer = get_optimizer(OptimizerStrategy::any);
        auto result = optimizer.optimize(state, CalculationMethod::default_method);
        CHECK(result.size() == 1);
        CHECK(result[0].method == CalculationMethod::default_method);
    }

    SUBCASE("Calculation method") {
        main_core::emplace_component<test::MockTransformer>(state, 1, MockTransformerState{.id = 1});
        main_core::emplace_component<test::MockTransformer>(state, 2, MockTransformerState{.id = 2});
        state.components.set_construction_complete();

        for (auto [strategy, calculation_method] : test::strategies_and_methods) {
            CAPTURE(strategy);
            CAPTURE(calculation_method);

            auto optimizer = get_optimizer(strategy);
            auto result = optimizer.optimize(state, calculation_method);

            CHECK(result.size() == 1);
            CHECK(result[0].method == calculation_method);
        }
    }

    SUBCASE("tap max/min") {
        main_core::emplace_component<test::MockTransformer>(state, 1, MockTransformerState{.id = 1});
        main_core::emplace_component<test::MockTransformer>(state, 2, MockTransformerState{.id = 2});

        auto& transformer_a = main_core::get_component<MockTransformer>(state, 1);
        auto& transformer_b = main_core::get_component<MockTransformer>(state, 2);

        main_core::emplace_component<TransformerTapRegulator>(
            state, 3, TransformerTapRegulatorInput{.id = 3, .regulated_object = 1}, transformer_a.math_model_type(),
            1.0);
        main_core::emplace_component<TransformerTapRegulator>(
            state, 4, TransformerTapRegulatorInput{.id = 4, .regulated_object = 2}, transformer_b.math_model_type(),
            1.0);

        state.components.set_construction_complete();

        auto strategy = OptimizerStrategy::any;
        auto& state_a = transformer_a.state;
        auto& state_b = transformer_b.state;
        auto expected_a{initial_a};
        auto expected_b{initial_b};

        SUBCASE("if initial state valid, keep tap pos") {
            state_b.tap_pos = 1;
            state_b.tap_min = 1;
            state_b.tap_max = 1;
            expected_b = 1;
        }
        SUBCASE("any strategy") {
            SUBCASE("tap pos should be in range") {
                state_b.tap_min = 1;
                state_b.tap_max = 1;
                state_b.tap_pos = 0;
                expected_b = 1;
            }
        }
        SUBCASE("any strategy") {
            SUBCASE("tap pos should be in range") {
                state_b.tap_min = 1;
                state_b.tap_max = 1;
                state_b.tap_pos = 0;
                expected_b = 1;
            }
        }

        auto const initial_a{transformer_a.tap_pos()};
        auto const initial_b{transformer_b.tap_pos()};

        auto optimizer = get_optimizer(strategy);
        auto result = optimizer.optimize(state, CalculationMethod::default_method);

        CHECK(result.size() == 1);
        CHECK(result.tap_pos(0) == expected_a);
        CHECK(result.tap_pos(1) == expected_b);
        CHECK(transformer_a.tap_pos() == initial_a);
        CHECK(transformer_b.tap_pos() == initial_b);
    }
}

} // namespace power_grid_model
