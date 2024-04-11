// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_core/input.hpp>
#include <power_grid_model/optimizer/tap_position_optimizer.hpp>

#include <doctest/doctest.h>

namespace pgm_tap = power_grid_model::optimizer::tap_position_optimizer;
namespace main_core = power_grid_model::main_core;

namespace power_grid_model {

namespace {
using TestComponentContainer = Container<Line, Link, Node, Transformer, Source>;
using TestState = main_core::MainModelState<TestComponentContainer>;
} // namespace

TEST_CASE("Test Transformer ranking") {
    // The grid from OntNote page
    TestState state;
    std::vector<NodeInput> nodes{{0, 150e3}, {1, 10e3}, {2, 10e3}, {3, 10e3}, {4, 10e3},
                                 {5, 50e3},  {6, 10e3}, {7, 10e3}, {8, 10e3}, {9, 10e3}};
    main_core::add_component<Node>(state, nodes.begin(), nodes.end(), 50.0);

    std::vector<TransformerInput> transformers{{11, 0, 1, 1, 1, 150e3, 10e3},
                                               {12, 0, 1, 1, 1, 150e3, 10e3},
                                               {13, 5, 7, 1, 1, 50e3, 10e3},
                                               {14, 2, 3, 1, 1, 10e3, 10e3},
                                               {15, 8, 9, 1, 1, 10e3, 10e3}};
    main_core::add_component<Transformer>(state, transformers.begin(), transformers.end(), 50.0);

    std::vector<LineInput> lines{{16, 1, 2, 1, 1}, {17, 4, 6, 1, 1}, {18, 7, 8, 1, 1}, {19, 3, 6, 1, 1}};
    main_core::add_component<Line>(state, lines.begin(), lines.end(), 50.0);

    std::vector<LinkInput> links{{20, 2, 1, 1, 1}, {21, 6, 4, 1, 1}, {22, 8, 7, 1, 1}, {23, 9, 3, 1, 1}};
    main_core::add_component<Link>(state, links.begin(), links.end(), 50.0);

    // Subcases
    SUBCASE("Building the graph") {
        // reference graph creation
        std::vector<std::pair<Idx, Idx>> edge_array = {{0, 1}, {0, 2}, {2, 3}};

        std::vector<pgm_tap::TrafoGraphEdge> edge_prop{{{0, 1}, 1}, {{0, 1}, 1}, {{0, 1}, 1}};
        pgm_tap::TransformerGraph expected_graph{boost::edges_are_unsorted_multi_pass, edge_array.cbegin(),
                                                 edge_array.cend(), edge_prop.cbegin(), 10};
        pgm_tap::TransformerGraph actual_graph = pgm_tap::build_transformer_graph(state);
        // CHECK (actual_graph == expected_graph);
    }

    SUBCASE("Process edge weights") {
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

    SUBCASE("Ranking complete the graph") {
        // CHECK (transformer[1] == 1);
    }
}

TEST_CASE("Test Tap position optimizer" * doctest::skip(true)) {
    // TODO: Implement unit tests for the tap position optimizer
}

} // namespace power_grid_model