// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/counting_iterator.hpp>
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
namespace {
void compare_vectors(std::vector<DoubleComplex>& result_vector, std::vector<DoubleComplex>& reference_vector,
                     double tolerance) {
    auto const loads_size = narrow_cast<Idx>(result_vector.size());
    auto const test_loads_size = narrow_cast<Idx>(reference_vector.size());
    REQUIRE(loads_size == test_loads_size);

    for (Idx const idx : IdxRange(loads_size)) {
        CHECK(result_vector[idx].real() == doctest::Approx(reference_vector[idx].real()).epsilon(tolerance));
        CHECK(result_vector[idx].imag() == doctest::Approx(reference_vector[idx].imag()).epsilon(tolerance));
    }
};

template <typename T>
    requires std::same_as<T, detail::ReducedEchelonFormResult> || std::same_as<T, detail::SolutionSet>
T generate_input_result(std::span<const IntS> data, std::span<const Idx> rows, std::span<const Idx> cols,
                        Idx col_number) {
    T result{};

    detail::CooSparseMatrix& T_matrix = [&result]() -> detail::CooSparseMatrix& {
        if constexpr (std::same_as<T, detail::ReducedEchelonFormResult>) {
            return result.matrix;
        } else if constexpr (std::same_as<T, detail::SolutionSet>) {
            return result.dfs_matrix;
        }
    }();

    T_matrix.prepare(col_number);
    for (auto idx = size_t{0}; idx < data.size(); ++idx) {
        T_matrix.set_value(data[idx], rows[idx], cols[idx]);
    }
    return result;
}
} // namespace

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
        ReducedEchelonFormResult result{};

        SUBCASE("One edge, two nodes, two real loads") {
            auto const edges = std::vector<BranchIdx>{{0, 1}};
            auto const node_loads = std::vector<DoubleComplex>{{-1.0, 0.0}, {1.0, 0.0}};
            auto const edge_number{edges.size()};
            auto const node_number{node_loads.size()};
            result.edges_history.resize(edge_number);
            result.matrix.prepare(node_number);
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
            result.matrix.prepare(node_number);
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
            result.matrix.prepare(node_number);
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
            result.matrix.prepare(node_number);
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
            result.matrix.prepare(node_number);
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
            ReducedEchelonFormResult result{};
            result.matrix.prepare(Idx{2});
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
            ReducedEchelonFormResult result{};
            result.matrix.prepare(Idx{3});
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
            ReducedEchelonFormResult result{};
            result.matrix.prepare(Idx{3});
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
            ReducedEchelonFormResult result{};
            result.matrix.prepare(Idx{2});
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
            ReducedEchelonFormResult result{};
            result.matrix.prepare(Idx{7});
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
        SUBCASE("Complex case with complex loads") {
            std::vector<IntS> data = {1, 0, 0, 1, 0, 0, -1, 1, -1, -1, 0, 0, 1, 1, 1, 1, 0, 1};
            std::vector<Idx> row = {0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 1, 0, 0, 0};
            std::vector<Idx> col = {0, 1, 2, 1, 2, 3, 6, 2, 3, 4, 5, 6, 5, 6, 4, 3, 4, 6};

            auto result = generate_input_result<ReducedEchelonFormResult>(data, row, col, Idx{7});

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

            auto result = generate_input_result<ReducedEchelonFormResult>(data, row, col, Idx{3});

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

            auto result = generate_input_result<ReducedEchelonFormResult>(data, row, col, Idx{4});

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
            // free_edge_indices = {3, 4, 6};
            // pivot_edge_indices = {0, 1, 2, 5};
            Idx const free_indices_number = 3;
            Idx const total_indices_number = 7;

            std::vector<IntS> dfs_data = {1, 1, 1, -1, -1, -1, -1, -1, 1, -1};
            std::vector<Idx> dfs_row = {0, 0, 1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<Idx> dfs_col = {0, 2, 1, 2, 0, 1, 0, 1, 2, 2};

            auto solution_set = generate_input_result<SolutionSet>(dfs_data, dfs_row, dfs_col, Idx{3});
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
            auto solution_set = generate_input_result<SolutionSet>(dfs_data, dfs_row, dfs_col, Idx{1});
            solution_set.extended_rhs = {{1, 0}, {-1, -0}, {-1, -0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> const projection_system =
                set_projection_system(free_indices_number, total_indices_number, solution_set);

            std::vector<std::vector<DoubleComplex>> test_system = {{{3, 0}, {-2, 0}}};

            CHECK(projection_system == test_system);
        }
    }

    SUBCASE("Testing the gauss elimination routine") {
        auto compare_systems = [](std::vector<std::vector<DoubleComplex>> const& solution,
                                  std::vector<std::vector<DoubleComplex>> const& reference, Idx col_number,
                                  Idx row_number, double tolerance) {
            for (Idx const col : IdxRange{col_number}) {
                for (Idx const row : IdxRange{row_number}) {
                    CHECK(solution[row][col].real() == doctest::Approx(reference[row][col].real()).epsilon(tolerance));
                    CHECK(solution[row][col].imag() == doctest::Approx(reference[row][col].imag()).epsilon(tolerance));
                }
            }
        };

        SUBCASE("Linear system of the complex case") {
            std::vector<std::vector<DoubleComplex>> system = {{{3, 0}, {1, 0}, {1, 0}, {2, 2}},
                                                              {{1, 0}, {3, 0}, {-1, 0}, {3, 3}},
                                                              {{1, 0}, {-1, 0}, {4, 0}, {-1, -1}}};
            naive_gauss_elimination(system);
            std::vector<std::vector<DoubleComplex>> const test_system = {
                {{3, 0}, {1, 0}, {1, 0}, {0.458333, 0.458333}},
                {{-0.333333, 0}, {2.66667, 0}, {-1.33333, 0}, {0.791667, 0.791667}},
                {{-0.333333, 0}, {0.5, -0}, {3, 0}, {-0.166667, -0.166667}}};

            // the tolerance is set to 1e-5 because that's the test system tolerance
            compare_systems(system, test_system, Idx{4}, Idx{3}, 1e-5);
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

            naive_gauss_elimination(system);

            // error tolerance is increased as this is a stress test
            // the test system solution now includes for significant digits for this reason
            // this is important because we are using naive_gauss_elimination, which skips pivoting
            // so this test makes sure that assumption holds
            // in addition, we only test against the last column of the solution as this lambda is intended for large
            // edge test cases this makes our lifes easier
            auto const system_size = narrow_cast<Idx>(system.size());
            auto last_column = system |
                               std::views::transform([system_size](auto const& row) { return row[system_size]; }) |
                               std::ranges::to<std::vector<DoubleComplex>>();
            compare_vectors(last_column, test_solution, 1e-7);
        }
    }

    SUBCASE("Testing the compute_internal_loads_routine") {
        SUBCASE("Complex case with complex loads") {
            std::vector<IntS> data = {1, 1, 1, -1, -1, -1, -1, -1, 1, -1};
            std::vector<Idx> row = {0, 0, 1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<Idx> col = {0, 2, 1, 2, 0, 1, 0, 1, 2, 2};

            auto solution_set = generate_input_result<SolutionSet>(data, row, col, Idx{3});

            solution_set.extended_rhs = {{0, 0}, {1, 1}, {-2, -2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> const test_system = {
                {{3, 0}, {1, 0}, {1, 0}, {0.458333, 0.458333}},
                {{-0.333333, 0}, {2.66667, 0}, {-1.33333, 0}, {0.791667, 0.791667}},
                {{-0.333333, 0}, {0.5, -0}, {3, 0}, {-0.166667, -0.166667}}};

            std::vector<DoubleComplex> internal_loads = compute_internal_loads(solution_set, test_system);

            std::vector<DoubleComplex> test_loads = {
                {-0.291667, -0.291667}, {0.0416667, 0.0416667}, {-0.75, -0.75},        {0.458333, 0.458333},
                {0.791667, 0.791667},   {0.166667, 0.166667},   {-0.166667, -0.166667}};

            compare_vectors(internal_loads, test_loads, 1e-5);
        }

        SUBCASE("Four edges, four nodes, two real loads") {
            std::vector<IntS> data = {1, 1, -1};
            std::vector<Idx> row = {1, 2, 3};
            std::vector<Idx> col = {0, 0, 0};

            auto solution_set = generate_input_result<SolutionSet>(data, row, col, Idx{1});
            solution_set.extended_rhs = {{1, 0}, {-1, -0}, {-1, -0}, {0, 0}};

            std::vector<std::vector<DoubleComplex>> const test_system = {{{3, 0}, {-0.666667, 0}}};

            std::vector<DoubleComplex> internal_loads = compute_internal_loads(solution_set, test_system);

            std::vector<DoubleComplex> test_loads = {{1, 0}, {-0.333333, -0}, {-0.333333, -0}, {-0.666667, 0}};

            compare_vectors(internal_loads, test_loads, 1.e-5);
        }
    }

    SUBCASE("Testing the compute_loads_link_elements - end to end test") {
        SUBCASE("Complex case with complex loads") {
            auto edges = std::vector<BranchIdx>{{3, 0}, {1, 0}, {2, 0}, {3, 2}, {1, 2}, {1, 4}, {3, 4}};
            auto node_loads = std::vector<DoubleComplex>{{1.0, 1.0}, {1.0, 1.0}, {-2.0, -2.0}, {0.0, 0.0}, {0.0, 0.0}};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {
                {-0.291667, -0.291667}, {0.0416667, 0.0416667}, {-0.75, -0.75},        {0.458333, 0.458333},
                {0.791667, 0.791667},   {0.166667, 0.166667},   {-0.166667, -0.166667}};

            compare_vectors(internal_loads, test_loads, 1.e-6);
        }

        SUBCASE("One edge, two nodes, two real loads") {
            auto edges = std::vector<BranchIdx>{{0, 1}};
            auto node_loads = std::vector<DoubleComplex>{1, -1};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{1, -0}};

            compare_vectors(internal_loads, test_loads, 1.e-8);
        }

        SUBCASE("Two edges, three nodes, two real loads") {
            auto edges = std::vector<BranchIdx>{{1, 0}, {1, 2}};
            auto node_loads = std::vector<DoubleComplex>{1, -1, 0};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{-1, -0}, {-0, -0}};

            compare_vectors(internal_loads, test_loads, 1.e-8);
        }

        SUBCASE("Three edges, three nodes, two real loads") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}};
            auto node_loads = std::vector<DoubleComplex>{1, -1, 0};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{0.666667, -0}, {-0.333333, -0}, {-0.333333, 0}};

            compare_vectors(internal_loads, test_loads, 1.e-6);
        }

        SUBCASE("Two edges, two nodes, two real loads") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {0, 1}};
            auto node_loads = std::vector<DoubleComplex>{1, -1};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{0.5, -0}, {0.5, 0}};

            compare_vectors(internal_loads, test_loads, 1.e-8);
        }

        SUBCASE("Four edges, four nodes, two real loads") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {2, 1}, {3, 2}, {3, 1}};
            auto node_loads = std::vector<DoubleComplex>{1, 0, 0, -1};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{1, 0}, {-0.333333, -0}, {-0.333333, -0}, {-0.666667, 0}};

            compare_vectors(internal_loads, test_loads, 1.e-6);
        }
        SUBCASE("Eight edges, five nodes, two real loads") {
            auto edges = std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 3}, {0, 3}, {0, 4}, {1, 4}, {2, 4}, {3, 4}};
            auto node_loads = std::vector<DoubleComplex>{1, -1, 0, 0, 0};

            std::vector<DoubleComplex> internal_loads = compute_loads_link_elements(edges, node_loads);

            std::vector<DoubleComplex> test_loads = {{0.53333333, 0},  {-0.2, 0},       {-0.13333333, 0},
                                                     {0.2, 0},         {0.26666667, 0}, {-0.26666667, 0},
                                                     {-0.06666667, 0}, {0.06666667, 0}};

            compare_vectors(internal_loads, test_loads, 1.e-8);
        }
    }
}
} // namespace power_grid_model::link_solver
