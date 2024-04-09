// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <limits>
#include <power_grid_model/optimizer/tap_position_optimizer.hpp>
#include <utility>
#include <vector>

#include <boost/graph/graph_utility.hpp>

#include <doctest/doctest.h>

namespace pgm_tap = power_grid_model::optimizer::tap_position_optimizer;

namespace power_grid_model {

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

TEST_CASE("Test Tap position optimizer" * doctest::skip(true)) {
    // TODO: Implement unit tests for the tap position optimizer
}

} // namespace power_grid_model