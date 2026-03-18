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
void check_value(IntS expected, uint64_t row_idx, uint64_t col_idx, detail::COOSparseMatrix const& matrix) {
    IntS value{};
    CHECK(matrix.get_value(value, row_idx, col_idx));
    CHECK(value == expected);
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
        EliminationResult result{};

        SUBCASE("One edge, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            uint64_t const edge_number{edges.size()};
            uint64_t const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 1);
            check_value(1, 0, 0, result.matrix);
            CHECK(result.rhs == std::vector<DoubleComplex>{{1.0, 0.0}});
            CHECK(result.free_edge_indices.empty());
            REQUIRE(result.edges_history.size() == 1);
            CHECK(result.edges_history[0].events == std::vector<EdgeEvent>{Deleted});
            CHECK(result.edges_history[0].rows == std::vector<uint64_t>{0});
        }

        SUBCASE("Two edges, three nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}};
            uint64_t const edge_number{edges.size()};
            uint64_t const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 2);
            check_value(1, 0, 0, result.matrix);
            check_value(1, 1, 1, result.matrix);
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
            uint64_t const edge_number{edges.size()};
            uint64_t const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 4);
            check_value(1, 0, 0, result.matrix);
            check_value(-1, 0, 1, result.matrix);
            check_value(1, 1, 1, result.matrix);
            check_value(-1, 1, 2, result.matrix);
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
            uint64_t const edge_number{edges.size()};
            uint64_t const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 2);
            check_value(1, 0, 0, result.matrix);
            check_value(1, 0, 1, result.matrix);
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
            uint64_t const edge_number{edges.size()};
            uint64_t const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(edge_number, node_number);
            forward_elimination(result, edges, node_loads);

            REQUIRE(result.matrix.data_map.size() == 14);
            check_value(1, 0, 0, result.matrix);
            check_value(1, 0, 2, result.matrix);
            check_value(1, 0, 1, result.matrix);
            check_value(1, 1, 1, result.matrix);
            check_value(1, 1, 2, result.matrix);
            check_value(-1, 1, 6, result.matrix);
            check_value(-1, 1, 3, result.matrix);
            check_value(1, 2, 2, result.matrix);
            check_value(-1, 2, 3, result.matrix);
            check_value(-1, 2, 6, result.matrix);
            check_value(-1, 2, 5, result.matrix);
            check_value(-1, 2, 4, result.matrix);
            check_value(1, 3, 5, result.matrix);
            check_value(1, 3, 6, result.matrix);
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
            check_value(1, 0, 0, result.matrix);
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
            check_value(1, 0, 0, result.matrix);
            check_value(1, 1, 1, result.matrix);
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
            check_value(1, 0, 0, result.matrix);
            check_value(1, 1, 1, result.matrix);
            check_value(-1, 1, 2, result.matrix);
            check_value(-1, 0, 2, result.matrix);
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
            check_value(1, 0, 0, result.matrix);
            check_value(1, 0, 1, result.matrix);
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

            result.free_edge_indices = std::vector<uint64_t>{3, 4, 6};
            result.pivot_edge_indices = std::vector<uint64_t>{0, 1, 2, 5};

            result.edges_history.resize(7);
            result.edges_history[0].events = std::vector<EdgeEvent>{Deleted};
            result.edges_history[0].rows = std::vector<uint64_t>{0};
            result.edges_history[1].events = std::vector<EdgeEvent>{Replaced, Deleted};
            result.edges_history[1].rows = std::vector<uint64_t>{0, 1};
            result.edges_history[2].events = std::vector<EdgeEvent>{Replaced, Replaced, Deleted};
            result.edges_history[2].rows = std::vector<uint64_t>{0, 1, 2};
            result.edges_history[3].events = std::vector<EdgeEvent>{Replaced, ContractedToPoint};
            result.edges_history[3].rows = std::vector<uint64_t>{1, 2};
            result.edges_history[4].events = std::vector<EdgeEvent>{ContractedToPoint};
            result.edges_history[4].rows = std::vector<uint64_t>{2};
            result.edges_history[5].events = std::vector<EdgeEvent>{Replaced, Deleted};
            result.edges_history[5].rows = std::vector<uint64_t>{2, 3};
            result.edges_history[6].events = std::vector<EdgeEvent>{Replaced, Replaced, ContractedToPoint};
            result.edges_history[6].rows = std::vector<uint64_t>{1, 2, 3};

            backward_substitution(result);
            REQUIRE(result.matrix.data_map.size() == 11);
            check_value(1, 0, 0, result.matrix);
            check_value(1, 0, 3, result.matrix);
            check_value(1, 0, 6, result.matrix);
            check_value(1, 1, 1, result.matrix);
            check_value(1, 1, 4, result.matrix);
            check_value(-1, 1, 6, result.matrix);
            check_value(1, 2, 2, result.matrix);
            check_value(-1, 2, 3, result.matrix);
            check_value(-1, 2, 4, result.matrix);
            check_value(1, 3, 5, result.matrix);
            check_value(1, 3, 6, result.matrix);
            REQUIRE(result.rhs.size() == 5);
            CHECK(result.rhs ==
                  std::vector<DoubleComplex>{{0.0, 0.0}, {1.0, 1.0}, {-2.0, -2.0}, {0.0, 0.0}, {0.0, 0.0}});
        }
    }
    SUBCASE("Testing the set_solution_system routine"){
    std::vector<IntS> data = {1, 0, 0, 1, 0, 0, -1, 1, -1, -1, 0, 0, 1, 1, 1, 1, 0, 1 };
    std::vector<uint64_t> row = {0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 1, 0, 0, 0 };
    std::vector<uint64_t> col = {0, 1, 2, 1, 2, 3, 6, 2, 3, 4, 5, 6, 5, 6, 4, 3, 4, 6 };
	
	IntS value;
    uint64_t rows = 5;
	uint64_t cols = 7;

    EliminationResult result{};
	
	result.matrix.prepare(rows,cols);
	
	for (uint64_t i = 0; i < data.size(); i++){	
		result.matrix.set_value(data[i], row[i], col[i]);
	}
		
	result.rhs = {{0,0}, {1,1}, {-2,-2}, {-0,-0} };
	result.free_edge_indices = {3, 4, 6 };	
	
	EdgeHistory edge_history{};	
	
	edge_history.rows = {0}; //0
	edge_history.events = {EdgeEvent::Deleted};
	result.edges_history.push_back(edge_history);
	
	edge_history.rows = {0,1}; //1
	edge_history.events = {EdgeEvent::Replaced,EdgeEvent::Deleted};
	result.edges_history.push_back(edge_history);
	
	edge_history.rows = {0,1,2}; //2
	edge_history.events = {EdgeEvent::Replaced,EdgeEvent::Replaced,EdgeEvent::Deleted};
	result.edges_history.push_back(edge_history);
	
	edge_history.rows = {1,2}; //3
	edge_history.events = {EdgeEvent::Replaced,EdgeEvent::ContractedToPoint};
	result.edges_history.push_back(edge_history);
	
	edge_history.rows = {2}; //4
	edge_history.events = {EdgeEvent::ContractedToPoint};
	result.edges_history.push_back(edge_history);
	
	edge_history.rows = {2,3}; //5
	edge_history.events = {EdgeEvent::Replaced,EdgeEvent::Deleted};
	result.edges_history.push_back(edge_history);
	
	edge_history.rows = {1,2,3}; //6
	edge_history.events = {EdgeEvent::Replaced,EdgeEvent::Replaced,EdgeEvent::ContractedToPoint};
	result.edges_history.push_back(edge_history);
	
	SolutionSet solution_set = set_solution_system(result);

    bool test = solution_set.dfs_matrix.get_value(value,0,0);
    REQUIRE(test == true);
    CHECK(value == 1);
    test = solution_set.dfs_matrix.get_value(value,0,2);
    CHECK(value == 1);
    test = solution_set.dfs_matrix.get_value(value,1,1);
	CHECK(value == 1);
	test = solution_set.dfs_matrix.get_value(value,1,2);
	CHECK(value == -1);
	test = solution_set.dfs_matrix.get_value(value,2,0);
	CHECK(value == -1);
	test = solution_set.dfs_matrix.get_value(value,2,1);
	CHECK(value == -1);
	test = solution_set.dfs_matrix.get_value(value,3,0);
	CHECK(value == -1);
	test = solution_set.dfs_matrix.get_value(value,4,1);
	CHECK(value == -1);
	test = solution_set.dfs_matrix.get_value(value,5,2);
	CHECK(value == 1);
	test = solution_set.dfs_matrix.get_value(value,6,2);
    CHECK(value == -1);	
    }
}

} // namespace power_grid_model::link_solver
