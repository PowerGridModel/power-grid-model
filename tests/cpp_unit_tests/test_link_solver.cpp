// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/link_solver.hpp>

#include <doctest/doctest.h>

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::link_solver {
namespace {
[[nodiscard]] uint64_t data_map_key_gen(uint64_t row_idx, uint64_t col_idx, uint64_t row_number) {
    return row_idx * row_number + col_idx;
}
} // namespace
TEST_CASE("Test the link solver algorithm") {
    using namespace detail;

    SUBCASE("Test build adjacency list") {

        SUBCASE("One edge, two nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 2);
            CHECK(adjacency_map.at(0) == std::unordered_set<uint64_t>{0});
            CHECK(adjacency_map.at(1) == std::unordered_set<uint64_t>{0});
        }

        SUBCASE("Two edges, three nodes") {
            auto edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 3);
            CHECK(adjacency_map.at(0) == std::unordered_set<uint64_t>{0});
            CHECK(adjacency_map.at(1) == std::unordered_set<uint64_t>{0, 1});
            CHECK(adjacency_map.at(2) == std::unordered_set<uint64_t>{1});
        }

        SUBCASE("Three edges, three nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 3);
            CHECK(adjacency_map.at(0) == std::unordered_set<uint64_t>{0, 2});
            CHECK(adjacency_map.at(1) == std::unordered_set<uint64_t>{0, 1});
            CHECK(adjacency_map.at(2) == std::unordered_set<uint64_t>{1, 2});
        }

        SUBCASE("Two edges, two nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 2);
            CHECK(adjacency_map.at(0) == std::unordered_set<uint64_t>{0, 1});
            CHECK(adjacency_map.at(1) == std::unordered_set<uint64_t>{0, 1});
        }

        SUBCASE("Seven edges, five nodes") {
            auto edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 5);
            CHECK(adjacency_map.at(0) == std::unordered_set<uint64_t>{0, 1, 2});
            CHECK(adjacency_map.at(1) == std::unordered_set<uint64_t>{1, 4, 5});
            CHECK(adjacency_map.at(2) == std::unordered_set<uint64_t>{2, 3, 4});
            CHECK(adjacency_map.at(3) == std::unordered_set<uint64_t>{0, 3, 6});
            CHECK(adjacency_map.at(4) == std::unordered_set<uint64_t>{5, 6});
        }
    }

    SUBCASE("Test forward elimination - elimination game") {
        using enum EdgeEvent;

        SUBCASE("One edge, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);
            uint64_t const node_number = node_loads.size();

            REQUIRE(result.matrix.data_map.size() == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 0, node_number)) == 1);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
            CHECK(result.free_edge_indices.empty());
            REQUIRE(result.edges_history.size() == 1);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<uint64_t>{0});
        }

        SUBCASE("Two edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);
            uint64_t const node_number = node_loads.size();

            REQUIRE(result.matrix.data_map.size() == 2);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 0, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(1, 1, node_number)) == 1);
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{-1.0, 0.0}, {0.0, 0.0}});
            CHECK(result.free_edge_indices.empty());
            REQUIRE(result.edges_history.size() == 2);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[1].rows == std::vector<uint64_t>{1});
        }

        SUBCASE("Three edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);
            uint64_t const node_number = node_loads.size();

            REQUIRE(result.matrix.data_map.size() == 4);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 0, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 1, node_number)) == -1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(1, 1, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(1, 2, node_number)) == -1);
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}, {0.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 1);
            CHECK(result.free_edge_indices == std::vector<uint64_t>{2});
            REQUIRE(result.edges_history.size() == 3);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edges_history[1].rows == std::vector<uint64_t>{0, 1});
            CHECK(result.edges_history[2].events == std::vector<EdgeEvent>{ContractedToPoint});
            CHECK(result.edges_history[2].rows == std::vector<uint64_t>{1});
        }

        SUBCASE("Two edges, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);
            uint64_t const node_number = node_loads.size();

            REQUIRE(result.matrix.data_map.size() == 2);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 0, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 1, node_number)) == 1);
            REQUIRE(result.rhs.size() == 1);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 1);
            CHECK(result.free_edge_indices == std::vector<uint64_t>{1});
            REQUIRE(result.edges_history.size() == 2);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{ContractedToPoint});
            CHECK(result.edges_history[1].rows == std::vector<uint64_t>{0});
        }

        SUBCASE("Complex case with complex loads") {
            auto const edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            auto const node_loads =
                std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {2.0, 2.0}, {0.0, 0.0}, {0.0, 0.0}};
            auto const result = forward_elimination(edges, node_loads);
            uint64_t const node_number = node_loads.size();

            REQUIRE(result.matrix.data_map.size() == 14);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 0, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 2, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(0, 1, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(1, 1, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(1, 2, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(1, 6, node_number)) == -1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(1, 3, node_number)) == -1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(2, 2, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(2, 3, node_number)) == -1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(2, 6, node_number)) == -1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(2, 5, node_number)) == -1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(2, 4, node_number)) == -1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(3, 5, node_number)) == 1);
            CHECK(result.matrix.data_map.at(data_map_key_gen(3, 6, node_number)) == 1);
            REQUIRE(result.rhs.size() == 4);
            CHECK(result.rhs == std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {-2.0, -2.0}, {0.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 3);
            CHECK(result.free_edge_indices == std::vector<uint64_t>{3, 4, 6});
            REQUIRE(result.edges_history.size() == 7);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<uint64_t>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edges_history[1].rows == std::vector<uint64_t>{0, 1});
            CHECK(result.edges_history[2].events == std::vector<EdgeEvent>{Replaced, Replaced, Deleted});
            CHECK(result.edges_history[2].rows == std::vector<uint64_t>{0, 1, 2});
            CHECK(result.edges_history[3].events == std::vector<EdgeEvent>{Replaced, ContractedToPoint});
            CHECK(result.edges_history[3].rows == std::vector<uint64_t>{1, 2});
            CHECK(result.edges_history[4].events == std::vector<EdgeEvent>{ContractedToPoint});
            CHECK(result.edges_history[4].rows == std::vector<uint64_t>{2});
            CHECK(result.edges_history[5].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edges_history[5].rows == std::vector<uint64_t>{2, 3});
            CHECK(result.edges_history[6].events == std::vector<EdgeEvent>{Replaced, Replaced, ContractedToPoint});
            CHECK(result.edges_history[6].rows == std::vector<uint64_t>{1, 2, 3});
        }
    }
}

} // namespace power_grid_model::link_solver
