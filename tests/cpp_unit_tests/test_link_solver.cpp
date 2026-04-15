// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/typing.hpp>
#include <power_grid_model/link_solver.hpp>

#include <doctest/doctest.h>

#include <cmath>
#include <cstddef>
#include <ranges>
#include <span>
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

            result.rhs = std::vector<DoubleComplex>{{-1.0, -1.0}, {-1.0, -1.0}, {-2.0, -2.0}, {0.0, 0.0}};

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
            REQUIRE(result.rhs.size() == 4);

            CHECK(result.rhs == std::vector<DoubleComplex>{{0.0, 0.0}, {1.0, 1.0}, {-2.0, -2.0}, {0.0, 0.0}});
        }
    }
    SUBCASE("Testing the set_solution_system routine") {
        auto generate_input_result = [](std::span<const IntS> data, std::span<const Idx> row, std::span<const Idx> col,
                                        Idx row_number, Idx col_number) {
            EliminationResult result{};
            result.matrix.prepare(row_number, col_number);
            for (auto idx = size_t{0}; idx < data.size(); ++idx) {
                result.matrix.set_value(data[idx], row[idx], col[idx]);
            }
            return result;
        };

        SUBCASE("Complex case with complex loads") {
            std::vector<IntS> data = {1, 0, 0, 1, 0, 0, -1, 1, -1, -1, 0, 0, 1, 1, 1, 1, 0, 1};
            std::vector<Idx> row = {0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 1, 0, 0, 0};
            std::vector<Idx> col = {0, 1, 2, 1, 2, 3, 6, 2, 3, 4, 5, 6, 5, 6, 4, 3, 4, 6};

            auto result = generate_input_result(data, row, col, Idx{5}, Idx{7});

            result.rhs = {{0, 0}, {1, 1}, {-2, -2}, {-0, -0}};
            result.free_edge_indices = {3, 4, 6};
            result.pivot_edge_indices = {0, 1, 2, 5};

            SolutionSet const solution_set = set_solution_system(result);

            CHECK(1 == solution_set.dfs_matrix.get_value(0, 0));
            CHECK(1 == solution_set.dfs_matrix.get_value(0, 2));
            CHECK(1 == solution_set.dfs_matrix.get_value(1, 1));
            CHECK(-1 == solution_set.dfs_matrix.get_value(1, 2));
            CHECK(-1 == solution_set.dfs_matrix.get_value(2, 0));
            CHECK(-1 == solution_set.dfs_matrix.get_value(2, 1));
            CHECK(-1 == solution_set.dfs_matrix.get_value(3, 0));
            CHECK(-1 == solution_set.dfs_matrix.get_value(4, 1));
            CHECK(1 == solution_set.dfs_matrix.get_value(5, 2));
            CHECK(-1 == solution_set.dfs_matrix.get_value(6, 2));
            CHECK(solution_set.extended_rhs ==
                  std::vector<DoubleComplex>({{0, 0}, {1, 1}, {-2, -2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}));
        }

        SUBCASE("Two edges, two nodes, two real loads") {

            std::vector<IntS> data = {1, 0, 1, -1, -1};
            std::vector<Idx> row = {0, 0, 1, 1, 0};
            std::vector<Idx> col = {0, 1, 1, 2, 2};

            auto result = generate_input_result(data, row, col, Idx{1}, Idx{3});

            result.rhs = {{1, 0}, {0, 0}};
            result.free_edge_indices = {2};
            result.pivot_edge_indices = {0, 1};

            SolutionSet const solution_set = set_solution_system(result);

            CHECK(-1 == solution_set.dfs_matrix.get_value(0, 0));
            CHECK(-1 == solution_set.dfs_matrix.get_value(1, 0));
            CHECK(-1 == solution_set.dfs_matrix.get_value(2, 0));
            CHECK(solution_set.extended_rhs == std::vector<DoubleComplex>({{1, 0}, {0, 0}, {0, 0}}));
        }

        SUBCASE("Four edges, four nodes, two real loads") {
            std::vector<IntS> data = {1, 0, 0, 1, 1, 1, 1};
            std::vector<Idx> row = {0, 0, 0, 1, 1, 2, 2};
            std::vector<Idx> col = {0, 1, 3, 1, 3, 2, 3};

            auto result = generate_input_result(data, row, col, Idx{1}, Idx{4});

            result.rhs = {{1, 0}, {-1, 0}, {-1, 0}};
            result.free_edge_indices = {3};
            result.pivot_edge_indices = {0, 1, 2};

            const SolutionSet solution_set = set_solution_system(result);

            CHECK(1 == solution_set.dfs_matrix.get_value(1, 0));
            CHECK(1 == solution_set.dfs_matrix.get_value(2, 0));
            CHECK(-1 == solution_set.dfs_matrix.get_value(3, 0));
            CHECK(solution_set.extended_rhs == std::vector<DoubleComplex>({{1, 0}, {-1, 0}, {-1, 0}, {0, 0}}));
        }
    }

    SUBCASE("Testing the set_projection_system routine") {
        auto generate_solution_set = [](std::span<const IntS> data, std::span<const Idx> row, std::span<const Idx> col,
                                        Idx row_number, Idx col_number) {
            SolutionSet solution_set{};
            solution_set.dfs_matrix.prepare(row_number, col_number);
            for (auto idx = size_t{0}; idx < data.size(); ++idx) {
                solution_set.dfs_matrix.set_value(data[idx], row[idx], col[idx]);
            }
            return solution_set;
        };

        SUBCASE("Complex case with complex loads") {
            // free_edge_indices = {3, 4, 6};
            // pivot_edge_indices = {0, 1, 2, 5};
            Idx const free_indices_number = 3;
            Idx const total_indices_number = 7;

            std::vector<IntS> dfs_data = {1, 1, 1, -1, -1, -1, -1, -1, 1, -1};
            std::vector<Idx> dfs_row = {0, 0, 1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<Idx> dfs_col = {0, 2, 1, 2, 0, 1, 0, 1, 2, 2};

            auto solution_set = generate_solution_set(dfs_data, dfs_row, dfs_col, Idx{7}, Idx{3});
            solution_set.extended_rhs = {{0, 0}, {1, 1}, {-2, -2}, {0, 0}, {0, 0}, {-0, -0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> const projection_system =
                set_projection_system(free_indices_number, total_indices_number, solution_set);

            std::vector<std::vector<DoubleComplex>> test_system = {{{3, 0}, {1, 0}, {1, 0}, {2, 2}},
                                                                   {{1, 0}, {3, 0}, {-1, 0}, {3, 3}},
                                                                   {{1, 0}, {-1, 0}, {4, 0}, {-1, -1}}};

            CHECK(projection_system == test_system);
        }

        SUBCASE("Four edges, four nodes, two real loads") {
            // result.free_edge_indices = {3};
            // result.pivot_edge_indices = {0, 1, 2};
            Idx const free_indices_number = 1;
            Idx const total_indices_number = 4;

            std::vector<IntS> dfs_data = {1, 1, -1};
            std::vector<Idx> dfs_row = {1, 2, 3};
            std::vector<Idx> dfs_col = {0, 0, 0};
            SolutionSet solution_set = generate_solution_set(dfs_data, dfs_row, dfs_col, Idx{4}, Idx{1});
            solution_set.extended_rhs = {{1, 0}, {-1, -0}, {-1, -0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> const projection_system =
                set_projection_system(free_indices_number, total_indices_number, solution_set);

            std::vector<std::vector<DoubleComplex>> test_system = {{{3, 0}, {-2, 0}}};

            CHECK(projection_system == test_system);
        }
    }

    SUBCASE("Testing the gauss elimination routine") {
        auto compare_systems = [](std::vector<std::vector<DoubleComplex>>& system,
                                  std::vector<std::vector<DoubleComplex>>& test_system) {
            auto const system_size = narrow_cast<Idx>(system.size());
            double element_sum{};

            for (Idx column = 0; column < system_size + 1; column++) {
                for (Idx row = 0; row < system_size; row++) {
                    element_sum += abs(system[row][column] - test_system[row][column]);
                }
            }

            return element_sum;
        };

        auto compare_vectors = [](std::vector<std::vector<DoubleComplex>>& system,
                                  std::vector<DoubleComplex>& test_solution) {
            auto const system_size = narrow_cast<Idx>(system.size());
            double element_sum{};

            for (Idx row = 0; row < system_size; row++) {
                element_sum += abs(system[row][system_size] - test_solution[row]);
            }

            return element_sum;
        };

        SUBCASE("Linear system of the complex case") {
            std::vector<std::vector<DoubleComplex>> system = {{{3, 0}, {1, 0}, {1, 0}, {2, 2}},
                                                              {{1, 0}, {3, 0}, {-1, 0}, {3, 3}},
                                                              {{1, 0}, {-1, 0}, {4, 0}, {-1, -1}}};

            gauss_elimination(system);

            std::vector<std::vector<DoubleComplex>> test_system = {
                {{3, 0}, {1, 0}, {1, 0}, {0.458333, 0.458333}},
                {{-0.333333, 0}, {2.66667, 0}, {-1.33333, 0}, {0.791667, 0.791667}},
                {{-0.333333, 0}, {0.5, -0}, {3, 0}, {-0.166667, -0.166667}}};

            CHECK(compare_systems(system, test_system) < 1.e-5);
        }

        SUBCASE("A system that consisng of a 15 X 15 matrix with externally randomly generated elements") {
            std::vector<std::vector<DoubleComplex>> system = {{{7, 0},
                                                               {4, 0},
                                                               {7, 0},
                                                               {2, 0},
                                                               {14, 0},
                                                               {5, 0},
                                                               {2, 0},
                                                               {4, 0},
                                                               {14, 0},
                                                               {2, 0},
                                                               {6, 0},
                                                               {7, 0},
                                                               {7, 0},
                                                               {0, 0},
                                                               {3, 0},
                                                               {1, 0}},
                                                              {{7, 0},
                                                               {11, 0},
                                                               {9, 0},
                                                               {7, 0},
                                                               {0, 0},
                                                               {12, 0},
                                                               {1, 0},
                                                               {14, 0},
                                                               {7, 0},
                                                               {12, 0},
                                                               {0, 0},
                                                               {4, 0},
                                                               {4, 0},
                                                               {14, 0},
                                                               {13, 0},
                                                               {1, 0}},
                                                              {{6, 0},
                                                               {0, 0},
                                                               {13, 0},
                                                               {0, 0},
                                                               {0, 0},
                                                               {2, 0},
                                                               {4, 0},
                                                               {8, 0},
                                                               {8, 0},
                                                               {14, 0},
                                                               {9, 0},
                                                               {8, 0},
                                                               {0, 0},
                                                               {3, 0},
                                                               {11, 0},
                                                               {1, 0}},
                                                              {{9, 0},
                                                               {2, 0},
                                                               {2, 0},
                                                               {14, 0},
                                                               {5, 0},
                                                               {4, 0},
                                                               {14, 0},
                                                               {7, 0},
                                                               {4, 0},
                                                               {8, 0},
                                                               {4, 0},
                                                               {5, 0},
                                                               {11, 0},
                                                               {10, 0},
                                                               {4, 0},
                                                               {1, 0}},
                                                              {{10, 0},
                                                               {7, 0},
                                                               {12, 0},
                                                               {12, 0},
                                                               {12, 0},
                                                               {7, 0},
                                                               {13, 0},
                                                               {7, 0},
                                                               {14, 0},
                                                               {2, 0},
                                                               {14, 0},
                                                               {5, 0},
                                                               {2, 0},
                                                               {1, 0},
                                                               {0, 0},
                                                               {1, 0}},
                                                              {{1, 0},
                                                               {3, 0},
                                                               {0, 0},
                                                               {7, 0},
                                                               {3, 0},
                                                               {14, 0},
                                                               {11, 0},
                                                               {5, 0},
                                                               {6, 0},
                                                               {11, 0},
                                                               {3, 0},
                                                               {7, 0},
                                                               {0, 0},
                                                               {12, 0},
                                                               {1, 0},
                                                               {1, 0}},
                                                              {{2, 0},
                                                               {11, 0},
                                                               {9, 0},
                                                               {2, 0},
                                                               {0, 0},
                                                               {3, 0},
                                                               {0, 0},
                                                               {8, 0},
                                                               {0, 0},
                                                               {12, 0},
                                                               {8, 0},
                                                               {5, 0},
                                                               {14, 0},
                                                               {10, 0},
                                                               {4, 0},
                                                               {1, 0}},
                                                              {{9, 0},
                                                               {12, 0},
                                                               {2, 0},
                                                               {13, 0},
                                                               {0, 0},
                                                               {11, 0},
                                                               {8, 0},
                                                               {1, 0},
                                                               {1, 0},
                                                               {13, 0},
                                                               {2, 0},
                                                               {8, 0},
                                                               {10, 0},
                                                               {2, 0},
                                                               {13, 0},
                                                               {1, 0}},
                                                              {{14, 0},
                                                               {9, 0},
                                                               {13, 0},
                                                               {13, 0},
                                                               {13, 0},
                                                               {12, 0},
                                                               {11, 0},
                                                               {2, 0},
                                                               {0, 0},
                                                               {3, 0},
                                                               {11, 0},
                                                               {3, 0},
                                                               {6, 0},
                                                               {6, 0},
                                                               {13, 0},
                                                               {1, 0}},
                                                              {{3, 0},
                                                               {14, 0},
                                                               {4, 0},
                                                               {7, 0},
                                                               {10, 0},
                                                               {14, 0},
                                                               {6, 0},
                                                               {13, 0},
                                                               {11, 0},
                                                               {12, 0},
                                                               {6, 0},
                                                               {7, 0},
                                                               {14, 0},
                                                               {12, 0},
                                                               {0, 0},
                                                               {1, 0}},
                                                              {{13, 0},
                                                               {11, 0},
                                                               {9, 0},
                                                               {14, 0},
                                                               {14, 0},
                                                               {14, 0},
                                                               {12, 0},
                                                               {13, 0},
                                                               {1, 0},
                                                               {10, 0},
                                                               {8, 0},
                                                               {8, 0},
                                                               {11, 0},
                                                               {14, 0},
                                                               {14, 0},
                                                               {1, 0}},
                                                              {{10, 0},
                                                               {14, 0},
                                                               {9, 0},
                                                               {3, 0},
                                                               {7, 0},
                                                               {11, 0},
                                                               {8, 0},
                                                               {8, 0},
                                                               {11, 0},
                                                               {3, 0},
                                                               {0, 0},
                                                               {6, 0},
                                                               {12, 0},
                                                               {5, 0},
                                                               {2, 0},
                                                               {1, 0}},
                                                              {{7, 0},
                                                               {13, 0},
                                                               {8, 0},
                                                               {7, 0},
                                                               {0, 0},
                                                               {12, 0},
                                                               {4, 0},
                                                               {9, 0},
                                                               {8, 0},
                                                               {13, 0},
                                                               {3, 0},
                                                               {8, 0},
                                                               {0, 0},
                                                               {5, 0},
                                                               {2, 0},
                                                               {1, 0}},
                                                              {{2, 0},
                                                               {2, 0},
                                                               {3, 0},
                                                               {10, 0},
                                                               {10, 0},
                                                               {14, 0},
                                                               {0, 0},
                                                               {5, 0},
                                                               {7, 0},
                                                               {5, 0},
                                                               {6, 0},
                                                               {10, 0},
                                                               {8, 0},
                                                               {11, 0},
                                                               {6, 0},
                                                               {1, 0}},
                                                              {{9, 0},
                                                               {6, 0},
                                                               {0, 0},
                                                               {2, 0},
                                                               {2, 0},
                                                               {12, 0},
                                                               {13, 0},
                                                               {13, 0},
                                                               {12, 0},
                                                               {9, 0},
                                                               {8, 0},
                                                               {5, 0},
                                                               {12, 0},
                                                               {9, 0},
                                                               {7, 0},
                                                               {1, 0}}};

            std::vector<DoubleComplex> test_solution = {
                {0.05461404, 0},  {0.03584441, 0},  {-0.00895461, 0}, {-0.00979037, 0}, {-0.01083266, 0},
                {-0.03845678, 0}, {-0.00652489, 0}, {-0.08356931, 0}, {0.05730963, 0},  {0.01390954, 0},
                {0.026622, 0},    {0.04469859, 0},  {-0.00946348, 0}, {0.08945877, 0},  {0.00377452, 0}};

            gauss_elimination(system);

            CHECK(compare_vectors(system, test_solution) < 1.e-7);
        }
    }

    SUBCASE("Testing the compute_internal_loads_routine") {

        auto generate_input_solution_set = [](std::span<const IntS> data, std::span<const Idx> row,
                                              std::span<const Idx> col, Idx row_number, Idx col_number) {
            SolutionSet solution_set{};
            solution_set.dfs_matrix.prepare(row_number, col_number);
            for (auto idx = size_t{0}; idx < data.size(); ++idx) {
                solution_set.dfs_matrix.set_value(data[idx], row[idx], col[idx]);
            }
            return solution_set;
        };

        auto compare_vectors = [](std::vector<DoubleComplex>& load, std::vector<DoubleComplex>& test_load) {
            auto const load_size = narrow_cast<Idx>(load.size());
            double element_sum{};

            for (Idx idx = 0; idx < load_size; idx++) {
                element_sum += abs(load[idx] - test_load[idx]);
            }

            return element_sum;
        };

        SUBCASE("Complex case with complex loads") {
            std::vector<IntS> data = {1, 1, 1, -1, -1, -1, -1, -1, 1, -1};
            std::vector<Idx> row = {0, 0, 1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<Idx> col = {0, 2, 1, 2, 0, 1, 0, 1, 2, 2};

            auto solution_set = generate_input_solution_set(data, row, col, Idx{7}, Idx{3});

            solution_set.extended_rhs = {{0, 0}, {1, 1}, {-2, -2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> test_system = {
                {{3, 0}, {1, 0}, {1, 0}, {0.458333, 0.458333}},
                {{-0.333333, 0}, {2.66667, 0}, {-1.33333, 0}, {0.791667, 0.791667}},
                {{-0.333333, 0}, {0.5, -0}, {3, 0}, {-0.166667, -0.166667}}};

            std::vector<DoubleComplex> internal_loads = compute_internal_loads(solution_set, test_system);

            std::vector<DoubleComplex> test_loads = {
                {-0.291667, -0.291667}, {0.0416667, 0.0416667}, {-0.75, -0.75},        {0.458333, 0.458333},
                {0.791667, 0.791667},   {0.166667, 0.166667},   {-0.166667, -0.166667}};
            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-5);
        }

        SUBCASE("Four edges, four nodes, two real loads") {

            std::vector<IntS> data = {1, 1, -1};
            std::vector<Idx> row = {1, 2, 3};
            std::vector<Idx> col = {0, 0, 0};

            auto solution_set = generate_input_solution_set(data, row, col, Idx{4}, Idx{1});
            solution_set.extended_rhs = {{1, 0}, {-1, -0}, {-1, -0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> test_system = {{{3, 0}, {-0.666667, 0}}};

            std::vector<DoubleComplex> internal_loads = compute_internal_loads(solution_set, test_system);

            std::vector<DoubleComplex> test_loads = {{1, 0}, {-0.333333, -0}, {-0.333333, -0}, {-0.666667, 0}};
            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-5);
        }
    }
    SUBCASE("Testing the compute_loads_link_elements") {
        auto compare_vectors = [](std::vector<DoubleComplex>& load, std::vector<DoubleComplex>& test_load) {
            auto const load_size = narrow_cast<Idx>(load.size());
            double element_sum{};

            for (Idx idx = 0; idx < load_size; idx++) {
                element_sum += abs(load[idx] - test_load[idx]);
            }

            return element_sum;
        };

        SUBCASE("Complex case with complex loads") {
            auto const edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            auto const node_loads =
                std::vector<DoubleComplex>{{1.0, 1.0}, {1.0, 1.0}, {-2.0, -2.0}, {0.0, 0.0}, {0.0, 0.0}};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {
                {-0.291667, -0.291667}, {0.0416667, 0.0416667}, {-0.75, -0.75},        {0.458333, 0.458333},
                {0.791667, 0.791667},   {0.166667, 0.166667},   {-0.166667, -0.166667}};

            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-5);
        }

        SUBCASE("One edge, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{1, -1};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{1, -0}};

            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-8);
        }

        SUBCASE("Two edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            auto const node_loads = std::vector<DoubleComplex>{1, -1, 0};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{-1, -0}, {-0, -0}};

            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-8);
        }

        SUBCASE("Three edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            auto const node_loads = std::vector<DoubleComplex>{1, -1, 0};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{0.666667, -0}, {-0.333333, -0}, {-0.333333, 0}};

            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-5);
        }

        SUBCASE("Two edges, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{1, -1};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{0.5, -0}, {0.5, 0}};

            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-8);
        }

        SUBCASE("Four edges, four nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {2, 1}, {3, 2}, {3, 1}};
            auto const node_loads = std::vector<DoubleComplex>{1, 0, 0, -1};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{1, 0}, {-0.333333, -0}, {-0.333333, -0}, {-0.666667, 0}};

            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-5);
        }
        SUBCASE("Eight edges, five nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 3}, {0, 3}, {0, 4}, {1, 4}, {2, 4}, {3, 4}};
            auto const node_loads = std::vector<DoubleComplex>{1, -1, 0, 0, 0};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{0.53333333, 0},  {-0.2, 0},       {-0.13333333, 0},
                                                     {0.2, 0},         {0.26666667, 0}, {-0.26666667, 0},
                                                     {-0.06666667, 0}, {0.06666667, 0}};

            CHECK(compare_vectors(internal_loads, test_loads) < 1.e-7);
        }
    }
}
} // namespace power_grid_model::link_solver
