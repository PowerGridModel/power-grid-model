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
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
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
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{Deleted});
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
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edges_history[1].rows == std::vector<Idx>{0, 1});
            CHECK(result.edges_history[2].events == std::vector<EdgeEvent>{ContractedToPoint});
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
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{ContractedToPoint});
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
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<Idx>{0});
            CHECK(result.edges_history[1].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edges_history[1].rows == std::vector<Idx>{0, 1});
            CHECK(result.edges_history[2].events == std::vector<EdgeEvent>{Replaced, Replaced, Deleted});
            CHECK(result.edges_history[2].rows == std::vector<Idx>{0, 1, 2});
            CHECK(result.edges_history[3].events == std::vector<EdgeEvent>{Replaced, ContractedToPoint});
            CHECK(result.edges_history[3].rows == std::vector<Idx>{1, 2});
            CHECK(result.edges_history[4].events == std::vector<EdgeEvent>{ContractedToPoint});
            CHECK(result.edges_history[4].rows == std::vector<Idx>{2});
            CHECK(result.edges_history[5].events == std::vector<EdgeEvent>{Replaced, Deleted});
            CHECK(result.edges_history[5].rows == std::vector<Idx>{2, 3});
            CHECK(result.edges_history[6].events == std::vector<EdgeEvent>{Replaced, Replaced, ContractedToPoint});
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
            result.edges_history[0].events = {Deleted};
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
            result.edges_history[0].events = {Deleted};
            result.edges_history[0].rows = {0};
            result.edges_history[1].events = {Deleted};
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
            result.edges_history[0].events = {Deleted};
            result.edges_history[0].rows = {0};
            result.edges_history[1].events = {Replaced, Deleted};
            result.edges_history[1].rows = {0, 1};
            result.edges_history[2].events = {ContractedToPoint};
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
            result.edges_history[0].events = {Deleted};
            result.edges_history[0].rows = {0};
            result.edges_history[1].events = {ContractedToPoint};
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
            result.edges_history[0].events = std::vector<EdgeEvent>{Deleted};
            result.edges_history[0].rows = std::vector<Idx>{0};
            result.edges_history[1].events = std::vector<EdgeEvent>{Replaced, Deleted};
            result.edges_history[1].rows = std::vector<Idx>{0, 1};
            result.edges_history[2].events = std::vector<EdgeEvent>{Replaced, Replaced, Deleted};
            result.edges_history[2].rows = std::vector<Idx>{0, 1, 2};
            result.edges_history[3].events = std::vector<EdgeEvent>{Replaced, ContractedToPoint};
            result.edges_history[3].rows = std::vector<Idx>{1, 2};
            result.edges_history[4].events = std::vector<EdgeEvent>{ContractedToPoint};
            result.edges_history[4].rows = std::vector<Idx>{2};
            result.edges_history[5].events = std::vector<EdgeEvent>{Replaced, Deleted};
            result.edges_history[5].rows = std::vector<Idx>{2, 3};
            result.edges_history[6].events = std::vector<EdgeEvent>{Replaced, Replaced, ContractedToPoint};
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
        SUBCASE("Complex case with complex loads") {
            EliminationResult result{};

            result.free_edge_indices = {3, 4, 6};
            result.pivot_edge_indices = {0, 1, 2, 5};

            SolutionSet solution_set{};

            uint64_t dfs_rows = 7;
            uint64_t dfs_cols = 3;

            solution_set.dfs_matrix.prepare(dfs_rows, dfs_cols);

            std::vector<IntS> dfs_data = {1, 1, 1, -1, -1, -1, -1, -1, 1, -1};
            std::vector<uint64_t> dfs_row = {0, 0, 1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<uint64_t> dfs_col = {0, 2, 1, 2, 0, 1, 0, 1, 2, 2};

            for (uint64_t i = 0; i < dfs_data.size(); i++) {
                solution_set.dfs_matrix.set_value(dfs_data[i], dfs_row[i], dfs_col[i]);
            }

            solution_set.extended_rhs = {{0, 0}, {1, 1}, {-2, -2}, {0, 0}, {0, 0}, {-0, -0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> projection_system =
                set_projection_system(result.free_edge_indices, result.pivot_edge_indices, solution_set);

            std::vector<std::vector<DoubleComplex>> test_system = {{{3, 0}, {1, 0}, {1, 0}, {2, 2}},
                                                                   {{1, 0}, {3, 0}, {-1, 0}, {3, 3}},
                                                                   {{1, 0}, {-1, 0}, {4, 0}, {-1, -1}}};

            CHECK(projection_system == test_system);
        }

        SUBCASE("Four edges, four nodes, two real loads") {

            EliminationResult result{};

            result.free_edge_indices = {3};
            result.pivot_edge_indices = {0, 1, 2};

            SolutionSet solution_set{};

            uint64_t dfs_rows = 4;
            uint64_t dfs_cols = 1;

            std::vector<IntS> dfs_data = {1, 1, -1};
            std::vector<uint64_t> dfs_row = {1, 2, 3};
            std::vector<uint64_t> dfs_col = {0, 0, 0};

            solution_set.dfs_matrix.prepare(dfs_rows, dfs_cols);

            for (uint64_t i = 0; i < dfs_data.size(); i++) {
                solution_set.dfs_matrix.set_value(dfs_data[i], dfs_row[i], dfs_col[i]);
            }

            solution_set.extended_rhs = {{1, 0}, {-1, -0}, {-1, -0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> projection_system =
                set_projection_system(result.free_edge_indices, result.pivot_edge_indices, solution_set);

            std::vector<std::vector<DoubleComplex>> test_system = {{{3, 0}, {-2, 0}}};

            CHECK(projection_system == test_system);
        }
    }
}
} // namespace power_grid_model::link_solver
