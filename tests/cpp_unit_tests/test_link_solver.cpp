// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/link_solver.hpp>

#include <doctest/doctest.h>

#include <cstdint>
#include <vector>

namespace power_grid_model::link_solver {
TEST_CASE("Test the link solver algorithm") {
    using namespace detail;

    SUBCASE("Test build adjacency list") {

        SUBCASE("One edge, two nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}};
            AdjacencyList const adjacency_list = build_adjacency_list(edges);

            REQUIRE(adjacency_list.size() == 2);
            CHECK(adjacency_list.at(0) == std::unordered_set<uint64_t>{0});
            CHECK(adjacency_list.at(1) == std::unordered_set<uint64_t>{0});
        }

        SUBCASE("Two edges, three nodes") {
            auto edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            AdjacencyList const adjacency_list = build_adjacency_list(edges);

            REQUIRE(adjacency_list.size() == 3);
            CHECK(adjacency_list.at(0) == std::unordered_set<uint64_t>{0});
            CHECK(adjacency_list.at(1) == std::unordered_set<uint64_t>{0, 1});
            CHECK(adjacency_list.at(2) == std::unordered_set<uint64_t>{1});
        }

        SUBCASE("Three edges, three nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            AdjacencyList const adjacency_list = build_adjacency_list(edges);

            REQUIRE(adjacency_list.size() == 3);
            CHECK(adjacency_list.at(0) == std::unordered_set<uint64_t>{0, 2});
            CHECK(adjacency_list.at(1) == std::unordered_set<uint64_t>{0, 1});
            CHECK(adjacency_list.at(2) == std::unordered_set<uint64_t>{1, 2});
        }

        SUBCASE("Two edges, two nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            AdjacencyList const adjacency_list = build_adjacency_list(edges);

            REQUIRE(adjacency_list.size() == 2);
            CHECK(adjacency_list.at(0) == std::unordered_set<uint64_t>{0, 1});
            CHECK(adjacency_list.at(1) == std::unordered_set<uint64_t>{0, 1});
        }

        SUBCASE("Seven edges, five nodes") {
            auto edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            AdjacencyList const adjacency_list = build_adjacency_list(edges);

            REQUIRE(adjacency_list.size() == 5);
            CHECK(adjacency_list.at(0) == std::unordered_set<uint64_t>{0, 1, 2});
            CHECK(adjacency_list.at(1) == std::unordered_set<uint64_t>{1, 4, 5});
            CHECK(adjacency_list.at(2) == std::unordered_set<uint64_t>{2, 3, 4});
            CHECK(adjacency_list.at(3) == std::unordered_set<uint64_t>{0, 3, 6});
            CHECK(adjacency_list.at(4) == std::unordered_set<uint64_t>{5, 6});
        }
    }

    SUBCASE("Test forward elimination - elimination game") {
        using enum EdgeEvent;

        SUBCASE("One edge, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);

            REQUIRE(result.matrix.data.size() == 1);
            CHECK(result.matrix.data == std::vector<IntS>{1});
            REQUIRE(result.matrix.row.size() == 1);
            CHECK(result.matrix.row == std::vector<uint64_t>{0});
            REQUIRE(result.matrix.col.size() == 1);
            CHECK(result.matrix.col == std::vector<uint64_t>{0});
            REQUIRE(result.rhs.size() == 1);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
            CHECK(result.free_edge_indices.empty());
            REQUIRE(result.edge_history.size() == 1);
            CHECK(result.edge_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edge_history[0].rows == std::vector<uint64_t>{0});
        }

        SUBCASE("Two edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);

            REQUIRE(result.matrix.data.size() == 2);
            CHECK(result.matrix.data == std::vector<IntS>{1, 1});
            REQUIRE(result.matrix.row.size() == 2);
            CHECK(result.matrix.row == std::vector<uint64_t>{0, 1});
            REQUIRE(result.matrix.col.size() == 2);
            CHECK(result.matrix.col == std::vector<uint64_t>{0, 1});
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{-1.0, 0.0}, {0.0, 0.0}});
            CHECK(result.free_edge_indices.empty());
            REQUIRE(result.edge_history.size() == 2);
            CHECK(result.edge_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edge_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edge_history[1].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edge_history[1].rows == std::vector<uint64_t>{1});
        }

        SUBCASE("Three edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);

            REQUIRE(result.matrix.data.size() == 4);
            CHECK(result.matrix.data == std::vector<IntS>{1, -1, 1, -1});
            REQUIRE(result.matrix.row.size() == 4);
            CHECK(result.matrix.row == std::vector<uint64_t>{0, 0, 1, 1});
            REQUIRE(result.matrix.col.size() == 4);
            CHECK(result.matrix.col == std::vector<uint64_t>{0, 1, 1, 2});
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}, {0.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 1);
            CHECK(result.free_edge_indices == std::vector<uint64_t>{2});
            REQUIRE(result.edge_history.size() == 3);
            CHECK(result.edge_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edge_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edge_history[1].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edge_history[1].rows == std::vector<uint64_t>{0, 1});
            CHECK(result.edge_history[2].events == std::vector<EdgeEvent>{ContractedToPoint});
            CHECK(result.edge_history[2].rows == std::vector<uint64_t>{1});
        }

        SUBCASE("Two edges, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);

            REQUIRE(result.matrix.data.size() == 2);
            CHECK(result.matrix.data == std::vector<IntS>{1, 1});
            REQUIRE(result.matrix.row.size() == 2);
            CHECK(result.matrix.row == std::vector<uint64_t>{0, 0});
            REQUIRE(result.matrix.col.size() == 2);
            CHECK(result.matrix.col == std::vector<uint64_t>{0, 1});
            REQUIRE(result.rhs.size() == 1);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 1);
            CHECK(result.free_edge_indices == std::vector<uint64_t>{1});
            REQUIRE(result.edge_history.size() == 2);
            CHECK(result.edge_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edge_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edge_history[1].events == std::vector<EdgeEvent>{ContractedToPoint});
            CHECK(result.edge_history[1].rows == std::vector<uint64_t>{0});
        }

        SUBCASE("Complex case with complex loads") {
            auto const edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            auto const node_loads =
                std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {2.0, 2.0}, {0.0, 0.0}, {0.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);

            REQUIRE(result.matrix.data.size() == 14);
            // TODO(figueroa1395): Some checks are commented out because the use of unordered_set in adjacency list
            // makes the order of re-attachments non-deterministic (per-platform - locally it's fine), which affects the
            // order of entries in the COO matrix should we use an ordered set instead to test this fine grained
            // details, or is it enough to test overall structure and result correctness? CHECK(result.matrix.data ==
            // std::vector<IntS>{1, 1, 1, 1, 1, -1, -1, 1, -1, -1, -1, -1, 1, 1});
            REQUIRE(result.matrix.row.size() == 14);
            CHECK(result.matrix.row == std::vector<uint64_t>{0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3});
            REQUIRE(result.matrix.col.size() == 14);
            // CHECK(result.matrix.col == std::vector<uint64_t>{0, 2, 1, 1, 2, 6, 3, 2, 3, 6, 5, 4, 5, 6});
            REQUIRE(result.rhs.size() == 4);
            CHECK(result.rhs == std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {-2.0, -2.0}, {0.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 3);
            CHECK(result.free_edge_indices == std::vector<uint64_t>{3, 4, 6});
            REQUIRE(result.edge_history.size() == 7);
            CHECK(result.edge_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edge_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edge_history[1].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edge_history[1].rows == std::vector<uint64_t>{0, 1});
            CHECK(result.edge_history[2].events == std::vector<EdgeEvent>{Replaced, Replaced, Deleted});
            CHECK(result.edge_history[2].rows == std::vector<uint64_t>{0, 1, 2});
            CHECK(result.edge_history[3].events == std::vector<EdgeEvent>{Replaced, ContractedToPoint});
            CHECK(result.edge_history[3].rows == std::vector<uint64_t>{1, 2});
            CHECK(result.edge_history[4].events == std::vector<EdgeEvent>{ContractedToPoint});
            CHECK(result.edge_history[4].rows == std::vector<uint64_t>{2});
            CHECK(result.edge_history[5].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edge_history[5].rows == std::vector<uint64_t>{2, 3});
            CHECK(result.edge_history[6].events == std::vector<EdgeEvent>{Replaced, Replaced, ContractedToPoint});
            CHECK(result.edge_history[6].rows == std::vector<uint64_t>{1, 2, 3});
        }
    }
}

} // namespace power_grid_model::link_solver
