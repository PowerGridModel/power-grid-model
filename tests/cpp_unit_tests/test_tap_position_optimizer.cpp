// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/optimizer/tap_position_optimizer.hpp>

#include <boost/graph/graph_utility.hpp>

#include <doctest/doctest.h>

namespace pgm_tap = power_grid_model::optimizer::tap_position_optimizer;

namespace power_grid_model {
namespace {} // namespace

TEST_CASE("Test Transformer ranking") {
    // The grid

    // Subcases
    SUBCASE("Building the graph") {
        // graph creation
    }

    std::vector<pgm_tap::TrafoGraphEdge> edge_array;
    edge_array.push_back(pgm_tap::TrafoGraphEdge({{0, 1}, 1}));
    edge_array.push_back(pgm_tap::TrafoGraphEdge({{1, 2}, 2}));

    pgm_tap::TransformerGraph g;
    //{boost::edges_are_unsorted_multi_pass, edge_array.cbegin(), edge_array.cend(), 3};

    SUBCASE("Process edge weights") { auto const res = get_edge_weights(g); }

    SUBCASE("Dijkstra shortest path") {
        std::vector<pgm_tap::EdgeWeight> edge_weight(3, pgm_tap::infty);
        std::vector<Idx2D> edge_pos(3);
        // process_edges_dijkstra(0, edge_weight, edge_pos, g);
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