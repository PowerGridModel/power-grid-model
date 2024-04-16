// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_optimizer.hpp"

#include <power_grid_model/optimizer/tap_position_optimizer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace optimizer::tap_position_optimizer::test {
namespace {
using power_grid_model::optimizer::test::StubTransformer;
using power_grid_model::optimizer::test::StubTransformerUpdate;

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

enum class MockTransformerSideType : IntS { side_a = 0, side_b = 1, side_c = 2, side_d = 3 };

struct MockTransformerState {
    using SideType = MockTransformerSideType;

    ID id{};

    std::function<ID(SideType)> node = [](SideType /*side*/) { return ID{}; };
    std::function<bool(SideType)> status = [](SideType /*side*/) { return bool{}; };

    SideType tap_side{};
    IntS tap_pos{};
    IntS tap_min{};
    IntS tap_max{};
    IntS tap_nom{};

    Idx topology_index{};
};

struct MockTransformer {
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
    auto const& transformer = get_component_item<ComponentType>(id_or_index);
    return transformer.state.topology_index;
}

template <std::derived_from<StubTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& /* state */, Idx /* topology_sequence_idx */) {
    return Idx2D{};
}

template <std::derived_from<StubTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topo_node(State const& /* state */, Idx /* topology_index */, ControlSide /* side */) {
    return Idx{};
}

template <std::derived_from<StubTransformer> ComponentType, steady_state_math_output_type MathOutputType>
inline auto i_pu(std::vector<MathOutputType> const& /* math_output */, Idx2DBranch3 const& /* math_id */,
                 ControlSide /* side */) {
    return ComplexValue<typename MathOutputType::sym>{};
}
} // namespace
} // namespace optimizer::tap_position_optimizer::test

namespace pgm_tap = optimizer::tap_position_optimizer;

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

TEST_CASE("Test Tap position optimizer") {}

} // namespace power_grid_model