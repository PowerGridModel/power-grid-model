// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/link_solver.hpp>

#include <doctest/doctest.h>

#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::link_solver {
TEST_CASE("Test the link solver algorithm") {
    using namespace detail;

    SUBCASE("Test build adjacency list") {

        SUBCASE("One edge, two nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 2);
            CHECK(adjacency_map.at(0) == std::unordered_set<Idx>{0});
            CHECK(adjacency_map.at(1) == std::unordered_set<Idx>{0});
        }

        SUBCASE("Two edges, three nodes") {
            auto edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 3);
            CHECK(adjacency_map.at(0) == std::unordered_set<Idx>{0});
            CHECK(adjacency_map.at(1) == std::unordered_set<Idx>{0, 1});
            CHECK(adjacency_map.at(2) == std::unordered_set<Idx>{1});
        }

        SUBCASE("Three edges, three nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 3);
            CHECK(adjacency_map.at(0) == std::unordered_set<Idx>{0, 2});
            CHECK(adjacency_map.at(1) == std::unordered_set<Idx>{0, 1});
            CHECK(adjacency_map.at(2) == std::unordered_set<Idx>{1, 2});
        }

        SUBCASE("Two edges, two nodes") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 2);
            CHECK(adjacency_map.at(0) == std::unordered_set<Idx>{0, 1});
            CHECK(adjacency_map.at(1) == std::unordered_set<Idx>{0, 1});
        }

        SUBCASE("Seven edges, five nodes") {
            auto edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            AdjacencyMap const adjacency_map = build_adjacency_map(edges);

            REQUIRE(adjacency_map.size() == 5);
            CHECK(adjacency_map.at(0) == std::unordered_set<Idx>{0, 1, 2});
            CHECK(adjacency_map.at(1) == std::unordered_set<Idx>{1, 4, 5});
            CHECK(adjacency_map.at(2) == std::unordered_set<Idx>{2, 3, 4});
            CHECK(adjacency_map.at(3) == std::unordered_set<Idx>{0, 3, 6});
            CHECK(adjacency_map.at(4) == std::unordered_set<Idx>{5, 6});
        }
    }

    SUBCASE("Test forward elimination - elimination game") {
        using enum EdgeEvent;
        EliminationResult result{};

        SUBCASE("One edge, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            auto const edge_number{edges.size()};
            auto const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 1);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
            CHECK(result.free_edge_indices.empty());
            REQUIRE(result.edges_history.size() == 1);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
        }

        SUBCASE("Two edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}};
            auto const edge_number{edges.size()};
            auto const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 2);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(1 == result.matrix.get_value(1, 1));
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{-1.0, 0.0}, {0.0, 0.0}});
            CHECK(result.free_edge_indices.empty());
            REQUIRE(result.edges_history.size() == 2);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{deleted});
            CHECK(result.edges_history[1].rows == std::vector<Idx>{1});
        }

        SUBCASE("Three edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}};
            auto const edge_number{edges.size()};
            auto const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 4);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(-1 == result.matrix.get_value(0, 1));
            CHECK(1 == result.matrix.get_value(1, 1));
            CHECK(-1 == result.matrix.get_value(1, 2));
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}, {0.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 1);
            CHECK(result.free_edge_indices == std::vector<Idx>{2});
            REQUIRE(result.edges_history.size() == 3);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{replaced, deleted});
            CHECK(result.edges_history[1].rows == std::vector<Idx>{0, 1});
            CHECK(result.edges_history[2].events == std::vector<EdgeEvent>{contracted_to_point});
            CHECK(result.edges_history[2].rows == std::vector<Idx>{1});
        }

        SUBCASE("Two edges, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            auto const edge_number{edges.size()};
            auto const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 2);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(1 == result.matrix.get_value(0, 1));
            REQUIRE(result.rhs.size() == 1);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 1);
            CHECK(result.free_edge_indices == std::vector<Idx>{1});
            REQUIRE(result.edges_history.size() == 2);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{contracted_to_point});
            CHECK(result.edges_history[1].rows == std::vector<Idx>{0});
        }

        SUBCASE("Complex case with complex loads") {
            auto const edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            auto const node_loads =
                std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {2.0, 2.0}, {0.0, 0.0}, {0.0, 0.0}};
            auto const edge_number{edges.size()};
            auto const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 14);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(1 == result.matrix.get_value(0, 2));
            CHECK(1 == result.matrix.get_value(0, 1));
            CHECK(1 == result.matrix.get_value(1, 1));
            CHECK(1 == result.matrix.get_value(1, 2));
            CHECK(-1 == result.matrix.get_value(1, 6));
            CHECK(-1 == result.matrix.get_value(1, 3));
            CHECK(1 == result.matrix.get_value(2, 2));
            CHECK(-1 == result.matrix.get_value(2, 3));
            CHECK(-1 == result.matrix.get_value(2, 6));
            CHECK(-1 == result.matrix.get_value(2, 5));
            CHECK(-1 == result.matrix.get_value(2, 4));
            CHECK(1 == result.matrix.get_value(3, 5));
            CHECK(1 == result.matrix.get_value(3, 6));
            REQUIRE(result.rhs.size() == 4);
            CHECK(result.rhs == std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {-2.0, -2.0}, {0.0, 0.0}});
            REQUIRE(result.free_edge_indices.size() == 3);
            CHECK(result.free_edge_indices == std::vector<Idx>{3, 4, 6});
            REQUIRE(result.edges_history.size() == 7);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{replaced, deleted});
            CHECK(result.edges_history[1].rows == std::vector<Idx>{0, 1});
            CHECK(result.edges_history[2].events == std::vector<EdgeEvent>{replaced, replaced, deleted});
            CHECK(result.edges_history[2].rows == std::vector<Idx>{0, 1, 2});
            CHECK(result.edges_history[3].events == std::vector<EdgeEvent>{replaced, contracted_to_point});
            CHECK(result.edges_history[3].rows == std::vector<Idx>{1, 2});
            CHECK(result.edges_history[4].events == std::vector<EdgeEvent>{contracted_to_point});
            CHECK(result.edges_history[4].rows == std::vector<Idx>{2});
            CHECK(result.edges_history[5].events == std::vector<EdgeEvent>{replaced, deleted});
            CHECK(result.edges_history[5].rows == std::vector<Idx>{2, 3});
            CHECK(result.edges_history[6].events == std::vector<EdgeEvent>{replaced, replaced, contracted_to_point});
            CHECK(result.edges_history[6].rows == std::vector<Idx>{1, 2, 3});
        }
    }

    SUBCASE("Test backward substitution auxiliary functions") {
        std::vector<Idx> const indices_vector{1, 4, 5, 10};

        SUBCASE("Test backward substitution pivots") {
            auto const result = backward_substitution_pivots(indices_vector) | std::ranges::to<std::vector<Idx>>();
            CHECK(result.size() == 3);
            CHECK(result == std::vector<Idx>{10, 5, 4});
        }

        SUBCASE("Test backward substitution rows") {
            auto const result = backward_substitution_rows(indices_vector) | std::ranges::to<std::vector<Idx>>();
            CHECK(result.size() == 3);
            CHECK(result == std::vector<Idx>{5, 4, 1});
        }

        SUBCASE("Test backward substitution free right cols") {
            auto const pivot_col_idx = Idx{6};
            auto const result = backward_substitution_free_right_cols(indices_vector, pivot_col_idx) |
                                std::ranges::to<std::vector<Idx>>();
            CHECK(result.size() == 1);
            CHECK(result == std::vector<Idx>{10});
        }
    }

    SUBCASE("Test backward substitution") {
        using enum EdgeEvent;

        SUBCASE("One edge, two nodes, two real loads") {
            EliminationResult result{};
            result.matrix.prepare(1, 2);
            result.matrix.set_value(1, 0, 0);
            result.rhs = std::vector<DoubleComplex>{{1.0, 0.0}};
            result.free_edge_indices = {};
            result.pivot_edge_indices = {0};
            result.edges_history.resize(1);
            result.edges_history[0].events = {deleted};
            result.edges_history[0].rows = {0};

            backward_substitution(result);
            REQUIRE(result.matrix.data_map.size() == 1);
            CHECK(1 == result.matrix.get_value(0, 0));
            REQUIRE(result.rhs.size() == 1);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
        }

        SUBCASE("Two edges, three nodes, two real loads") {
            EliminationResult result{};
            result.matrix.prepare(2, 3);
            result.matrix.set_value(1, 0, 0);
            result.matrix.set_value(1, 1, 1);
            result.rhs = std::vector<DoubleComplex>{{-1.0, 0.0}, {0.0, 0.0}};
            result.free_edge_indices = {};
            result.pivot_edge_indices = {0, 1};
            result.edges_history.resize(2);
            result.edges_history[0].events = {deleted};
            result.edges_history[0].rows = {0};
            result.edges_history[1].events = {deleted};
            result.edges_history[1].rows = {1};

            backward_substitution(result);
            REQUIRE(result.matrix.data_map.size() == 2);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(1 == result.matrix.get_value(1, 1));
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{-1.0, 0.0}, {0.0, 0.0}});
        }

        SUBCASE("Three edges, three nodes, two real loads") {
            EliminationResult result{};
            result.matrix.prepare(4, 3);
            result.matrix.set_value(1, 0, 0);
            result.matrix.set_value(-1, 0, 1);
            result.matrix.set_value(1, 1, 1);
            result.matrix.set_value(-1, 1, 2);
            result.rhs = std::vector<DoubleComplex>{{1.0, 0.0}, {0.0, 0.0}};
            result.free_edge_indices = {2};
            result.pivot_edge_indices = {0, 1};
            result.edges_history.resize(3);
            result.edges_history[0].events = {deleted};
            result.edges_history[0].rows = {0};
            result.edges_history[1].events = {replaced, deleted};
            result.edges_history[1].rows = {0, 1};
            result.edges_history[2].events = {contracted_to_point};
            result.edges_history[2].rows = {1};

            backward_substitution(result);
            REQUIRE(result.matrix.data_map.size() == 4);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(1 == result.matrix.get_value(1, 1));
            CHECK(-1 == result.matrix.get_value(1, 2));
            CHECK(-1 == result.matrix.get_value(0, 2));
            REQUIRE(result.rhs.size() == 2);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}, {0.0, 0.0}});
        }

        SUBCASE("Two edges, two nodes, two real loads") {
            EliminationResult result{};
            result.matrix.prepare(2, 2);
            result.matrix.set_value(1, 0, 0);
            result.matrix.set_value(1, 0, 1);
            result.rhs = std::vector<DoubleComplex>{{1.0, 0.0}};
            result.free_edge_indices = {1};
            result.pivot_edge_indices = {0};
            result.edges_history.resize(2);
            result.edges_history[0].events = {deleted};
            result.edges_history[0].rows = {0};
            result.edges_history[1].events = {contracted_to_point};
            result.edges_history[1].rows = {0};

            backward_substitution(result);
            REQUIRE(result.matrix.data_map.size() == 2);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(1 == result.matrix.get_value(0, 1));
            REQUIRE(result.rhs.size() == 1);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
        }

        SUBCASE("Complex case with complex loads") {
            EliminationResult result{};
            result.matrix.prepare(5, 7);
            result.matrix.set_value(1, 0, 0);
            result.matrix.set_value(1, 0, 1);
            result.matrix.set_value(1, 0, 2);
            result.matrix.set_value(1, 1, 1);
            result.matrix.set_value(1, 1, 2);
            result.matrix.set_value(-1, 1, 3);
            result.matrix.set_value(-1, 1, 6);
            result.matrix.set_value(1, 2, 2);
            result.matrix.set_value(-1, 2, 3);
            result.matrix.set_value(-1, 2, 4);
            result.matrix.set_value(-1, 2, 5);
            result.matrix.set_value(-1, 2, 6);
            result.matrix.set_value(1, 3, 5);
            result.matrix.set_value(1, 3, 6);

            result.rhs = std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {-2.0, -2.0}, {0.0, 0.0}, {0.0, 0.0}};

            result.free_edge_indices = std::vector<Idx>{3, 4, 6};
            result.pivot_edge_indices = std::vector<Idx>{0, 1, 2, 5};

            result.edges_history.resize(7);
            result.edges_history[0].events = std::vector<EdgeEvent>{deleted};
            result.edges_history[0].rows = std::vector<Idx>{0};
            result.edges_history[1].events = std::vector<EdgeEvent>{replaced, deleted};
            result.edges_history[1].rows = std::vector<Idx>{0, 1};
            result.edges_history[2].events = std::vector<EdgeEvent>{replaced, replaced, deleted};
            result.edges_history[2].rows = std::vector<Idx>{0, 1, 2};
            result.edges_history[3].events = std::vector<EdgeEvent>{replaced, contracted_to_point};
            result.edges_history[3].rows = std::vector<Idx>{1, 2};
            result.edges_history[4].events = std::vector<EdgeEvent>{contracted_to_point};
            result.edges_history[4].rows = std::vector<Idx>{2};
            result.edges_history[5].events = std::vector<EdgeEvent>{replaced, deleted};
            result.edges_history[5].rows = std::vector<Idx>{2, 3};
            result.edges_history[6].events = std::vector<EdgeEvent>{replaced, replaced, contracted_to_point};
            result.edges_history[6].rows = std::vector<Idx>{1, 2, 3};

            backward_substitution(result);
            REQUIRE(result.matrix.data_map.size() == 11);
            CHECK(1 == result.matrix.get_value(0, 0));
            CHECK(1 == result.matrix.get_value(0, 3));
            CHECK(1 == result.matrix.get_value(0, 6));
            CHECK(1 == result.matrix.get_value(1, 1));
            CHECK(1 == result.matrix.get_value(1, 4));
            CHECK(-1 == result.matrix.get_value(1, 6));
            CHECK(1 == result.matrix.get_value(2, 2));
            CHECK(-1 == result.matrix.get_value(2, 3));
            CHECK(-1 == result.matrix.get_value(2, 4));
            CHECK(1 == result.matrix.get_value(3, 5));
            CHECK(1 == result.matrix.get_value(3, 6));
            REQUIRE(result.rhs.size() == 5);
            CHECK(result.rhs ==
                  std::vector<DoubleComplex>{{0.0, 0.0}, {1.0, 1.0}, {-2.0, -2.0}, {0.0, 0.0}, {0.0, 0.0}});
        }
    }
}
} // namespace power_grid_model::link_solver
