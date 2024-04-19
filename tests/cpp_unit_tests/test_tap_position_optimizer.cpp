// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_core/input.hpp>
#include <power_grid_model/optimizer/tap_position_optimizer.hpp>

#include <algorithm>
#include <ranges>

#include <doctest/doctest.h>

namespace pgm_tap = power_grid_model::optimizer::tap_position_optimizer;
namespace main_core = power_grid_model::main_core;

namespace power_grid_model {

namespace {
using TestComponentContainer =
    Container<ExtraRetrievableTypes<Base, Node, Branch, Branch3, Appliance, Regulator>, Line, Link, Node, Transformer,
              ThreeWindingTransformer, TransformerTapRegulator, Source>;
using TestState = main_core::MainModelState<TestComponentContainer>;

TransformerInput get_transformer(ID id, ID from, ID to, BranchSide tap_side) {
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
                            .tap_side = tap_side};
}

ThreeWindingTransformerInput get_transformer3w(ID id, ID node_1, ID node_2, ID node_3) {
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
        .tap_pos = 0,
        .tap_min = 0,
        .tap_max = 0,
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

// TEST_SUITE("Automatic Tap Changer") {
TEST_CASE("Test Transformer ranking") {
    // Minimum test grid
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
        // reference graph creation
        // Inserted in order of transformer, transformer3w, line and link
        std::vector<std::pair<Idx, Idx>> expected_edges;
        expected_edges.insert(expected_edges.end(), {{0, 1}, {0, 1}, {5, 7}, {2, 3}, {8, 9}});
        expected_edges.insert(expected_edges.end(), {{0, 4}, {4, 5}, {5, 4}, {0, 5}});
        expected_edges.insert(expected_edges.end(), {{3, 6}, {6, 3}, {3, 9}, {9, 3}});
        expected_edges.insert(expected_edges.end(), {{2, 1}, {1, 2}, {6, 4}, {4, 6}, {8, 7}, {7, 8}});

        std::vector<pgm_tap::TrafoGraphEdge> expected_edges_prop;
        expected_edges_prop.insert(expected_edges_prop.end(),
                                   {{true, {0, 1}, 1}, {false, {1, 2}, 0}, {false, {2, 1}, 0}, {true, {2, 3}, 1}});
        expected_edges_prop.insert(expected_edges_prop.end(),
                                   {{true, {0, 4}, 1}, {true, {0, 5}, 1}, {true, {4, 5}, 1}, {true, {5, 4}, 1}});
        expected_edges_prop.insert(expected_edges_prop.end(), {{false, {4, 6}, 0},
                                                               {false, {6, 4}, 0},
                                                               {false, {3, 6}, 0},
                                                               {false, {6, 3}, 0},
                                                               {false, {3, 9}, 0},
                                                               {false, {9, 3}, 0},
                                                               {true, {0, 1}, 1}});
        expected_edges_prop.insert(expected_edges_prop.end(),
                                   {{false, {7, 8}, 0}, {false, {8, 7}, 0}, {true, {5, 7}, 1}, {true, {8, 9}, 1}});

        const std::vector<pgm_tap::TrafoGraphVertex> expected_vertex_props{{true},  {false}, {false}, {false}, {false},
                                                                           {false}, {false}, {false}, {false}, {false}};

        pgm_tap::TransformerGraph actual_graph = pgm_tap::build_transformer_graph(state);
        std::vector<pgm_tap::TrafoGraphEdge> actual_edges_prop;

        boost::graph_traits<pgm_tap::TransformerGraph>::vertex_iterator vi, vi_end;
        for (boost::tie(vi, vi_end) = vertices(actual_graph); vi != vi_end; ++vi) {
            CHECK(actual_graph[*vi].is_source == expected_vertex_props[*vi].is_source);
        }

        BGL_FORALL_EDGES(e, actual_graph, pgm_tap::TransformerGraph) { actual_edges_prop.push_back(actual_graph[e]); }

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
        main_core::add_component<TransformerTapRegulator>(bad_state, bad_regulators.begin(), bad_regulators.end(),
                                                          50.0);

        bad_state.components.set_construction_complete();
        CHECK_THROWS_AS(pgm_tap::build_transformer_graph(bad_state), AutomaticTapCalculationError);
    }

    SUBCASE("Process edge weights") {
        // Dummy graph
        std::vector<std::pair<Idx, Idx>> edge_array = {{0, 1}, {0, 2}, {2, 3}};
        std::vector<pgm_tap::TrafoGraphEdge> edge_prop{{true, {0, 1}, 1}, {true, {0, 2}, 2}, {true, {2, 3}, 3}};
        std::vector<pgm_tap::TrafoGraphVertex> vertex_props{{true}, {false}, {false}, {false}};

        pgm_tap::TransformerGraph g{boost::edges_are_unsorted_multi_pass, edge_array.cbegin(), edge_array.cend(),
                                    edge_prop.cbegin(), 4};

        // Vertex properties can not be set during graph creation
        boost::graph_traits<pgm_tap::TransformerGraph>::vertex_iterator vi, vi_end;
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
            g[*vi].is_source = vertex_props[*vi].is_source;
        }

        const pgm_tap::WeightedTrafoList all_edge_weights = get_edge_weights(g);
        const pgm_tap::WeightedTrafoList ref_edge_weights{
            {false, {0, 0}, 0}, {true, {0, 1}, 1}, {true, {0, 2}, 2}, {true, {2, 3}, 5}};
        CHECK(all_edge_weights == ref_edge_weights);
    }

    SUBCASE("Sorting transformer edges") {
        pgm_tap::WeightedTrafoList trafoList{{false, Idx2D{1, 1}, pgm_tap::infty},
                                             {true, Idx2D{1, 2}, 5},
                                             {true, Idx2D{1, 3}, 4},
                                             {true, Idx2D{2, 1}, 4},
                                             {true, Idx2D{2, 2}, 3},
                                             {true, Idx2D{3, 1}, 2},
                                             {true, Idx2D{3, 2}, 1},
                                             {true, Idx2D{3, 3}, 1}};

        const pgm_tap::RankedTransformerGroups referenceList{{Idx2D{3, 2}, Idx2D{3, 3}}, {Idx2D{3, 1}}, {Idx2D{2, 2}},
                                                             {Idx2D{1, 3}, Idx2D{2, 1}}, {Idx2D{1, 2}}, {Idx2D{1, 1}}};

        const pgm_tap::RankedTransformerGroups sortedTrafoList = pgm_tap::rank_transformers(trafoList);
        CHECK(sortedTrafoList == referenceList);
    }

    SUBCASE("Ranking complete the graph") {
        const pgm_tap::RankedTransformerGroups order = pgm_tap::rank_transformers(state);
        // CHECK (transformer[1] == 1);
    }
}

TEST_CASE("Test Tap position optimizer" * doctest::skip(true)) {
    // TODO: Implement unit tests for the tap position optimizer
}
//}
} // namespace power_grid_model