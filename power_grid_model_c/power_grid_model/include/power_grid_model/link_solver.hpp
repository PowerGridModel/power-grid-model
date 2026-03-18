// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "calculation_parameters.hpp"
#include "common/common.hpp"
#include "common/counting_iterator.hpp"

#include <array>
#include <cstdint>
#include <ranges>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace power_grid_model::link_solver::detail {

using AdjacencyMap = std::unordered_map<Idx, std::unordered_set<uint64_t>>;
using ContractedEdgesSet = std::unordered_set<uint64_t>;

enum class EdgeEvent : std::uint8_t {
    Deleted = 0,          // pivot edge - used as pivot row
    Replaced = 1,         // reattached to a different node
    ContractedToPoint = 2 // from == to, becomes a degree of freedom
};

enum class EdgeDirection : IntS { Away = -1, Towards = 1 };

struct COOSparseMatrix {
    uint64_t row_number{};
    std::unordered_map<uint64_t, IntS> data_map{};

    void prepare(uint64_t row_size, uint64_t col_size) {
        row_number = col_size;
        data_map.reserve(row_size *
                         col_size); // worst case to guarantee O(1) insertion/retrieval, but typically much sparser
    }
    void set_value(IntS value, uint64_t row_idx, uint64_t col_idx) {
        assert(row_number != 0 && "row_number must be set before setting values in data_map");
        data_map[row_idx * row_number + col_idx] = value;
    }
    bool get_value(IntS& value, uint64_t row_idx, uint64_t col_idx) const {
        assert(row_number != 0 && "row_number must be set before getting values from data_map");
        if (auto const it = data_map.find(row_idx * row_number + col_idx); it != data_map.end()) {
            value = it->second;
            return true;
        }
        return false;
    }
    void add_to_value(IntS value, uint64_t row_idx, uint64_t col_idx) {
        assert(row_number != 0 && "row_number must be set before adding values in data_map");
        auto const key = row_idx * row_number + col_idx;
        if (auto const it = data_map.find(key); it != data_map.end()) {
            it->second = static_cast<IntS>(it->second + value);
            if (it->second == 0) {
                data_map.erase(it); // maintain sparsity by erasing zero entries
            }
        } else if (value != 0) {
            data_map[key] = value;
        }
    }
};

struct EdgeHistory {
    std::vector<uint64_t> rows{};
    std::vector<EdgeEvent> events{};
};

struct EliminationResult {
    COOSparseMatrix matrix{};
    std::vector<DoubleComplex> rhs{};           // RHS value at each pivot row
    std::vector<uint64_t> free_edge_indices{};  // index of degrees of freedom (self loop edges)
    std::vector<uint64_t> pivot_edge_indices{}; // index of pivot edges
    std::vector<EdgeHistory> edges_history{};   // edges elimination history
};

// convention: from node at position 0, to node at position 1
[[nodiscard]] constexpr Idx from_node(BranchIdx const& edge) { return edge[0]; }
[[nodiscard]] constexpr Idx& from_node(BranchIdx& edge) { return edge[0]; }
[[nodiscard]] constexpr Idx to_node(BranchIdx const& edge) { return edge[1]; }
[[nodiscard]] constexpr Idx& to_node(BranchIdx& edge) { return edge[1]; }

// map from node index to the set of adjacent edge indices
// unordered_map because node IDs may be sparse/non-contiguous
// unordered_set for O(1) insert/erase during reattachment
[[nodiscard]] inline auto build_adjacency_map(std::span<BranchIdx> edges) -> AdjacencyMap {
    AdjacencyMap adjacency_map{};
    for (auto const& [raw_index, edge] : enumerate(std::as_const(edges))) {
        auto const index = static_cast<uint64_t>(raw_index);
        auto const [from_node, to_node] = edge;
        adjacency_map[from_node].insert(index);
        adjacency_map[to_node].insert(index);
    }
    return adjacency_map;
}

inline void write_edge_history(EdgeHistory& edges_history, EdgeEvent event, uint64_t row) {
    edges_history.events.push_back(event);
    edges_history.rows.push_back(row);
}

inline void replace_and_write(uint64_t edge_idx, Idx from_node_idx, Idx to_node_idx, uint64_t matrix_row,
                              std::vector<BranchIdx>& edges, COOSparseMatrix& matrix) {
    using enum EdgeDirection;

    if (from_node(edges[edge_idx]) == to_node_idx) {
        from_node(edges[edge_idx]) = from_node_idx; // re-attachment step
        matrix.set_value(std::to_underlying(Away), matrix_row, edge_idx);
    } else {
        to_node(edges[edge_idx]) = from_node_idx; // re-attachment step
        matrix.set_value(std::to_underlying(Towards), matrix_row, edge_idx);
    }
}

inline void update_edge_info(uint64_t edge_idx, uint64_t matrix_row, std::vector<BranchIdx>& edges,
                             std::vector<EdgeHistory>& edges_history, ContractedEdgesSet& edges_contracted_to_point) {
    using enum EdgeEvent;

    if (from_node(edges[edge_idx]) == to_node(edges[edge_idx])) {
        write_edge_history(edges_history[edge_idx], ContractedToPoint, matrix_row);
        edges_contracted_to_point.insert(edge_idx);
    } else {
        write_edge_history(edges_history[edge_idx], Replaced, matrix_row);
    }
}

// forward elimination is performed via a modified Gaussian elimination procedure
// we name this procedure elimination-game
// node_loads convention: caller passes negated external loads (as in RHS of power flow equations))
[[nodiscard]] inline EliminationResult forward_elimination(std::vector<BranchIdx> edges,
                                                           std::vector<DoubleComplex> node_loads) {
    EliminationResult result{};
    uint64_t const edge_number{edges.size()};
    uint64_t const node_number{node_loads.size()};

    result.edges_history.resize(edge_number);
    using enum EdgeEvent;
    using enum EdgeDirection;

    auto& [matrix, rhs, free_edge_indices, pivot_edge_indices, edges_history] = result;
    matrix.prepare(edge_number, node_number);

    constexpr uint8_t starting_row{};
    uint64_t matrix_row{starting_row};
    auto adjacency_map = build_adjacency_map(edges);
    ContractedEdgesSet edges_contracted_to_point{}; // TODO(figueroa1395): is this really needed?

    for (auto const& [raw_index, edge] : enumerate(std::as_const(edges))) {
        auto const index = static_cast<uint64_t>(raw_index);
        if (edges_contracted_to_point.contains(index)) {
            free_edge_indices.push_back(index);
        } else {
            write_edge_history(edges_history[index], Deleted, matrix_row); // Delete edge -> pivot there
            pivot_edge_indices.push_back(index);
            matrix.set_value(std::to_underlying(Towards), matrix_row, index);

            Idx const from_node_idx = from_node(edge);
            Idx const to_node_idx = to_node(edge);

            // update adjacency list for deleted edge
            adjacency_map[from_node_idx].erase(index);
            adjacency_map[to_node_idx].erase(index);

            // Gaussian elimination like steps
            node_loads[from_node_idx] += node_loads[to_node_idx];
            rhs.push_back(node_loads[to_node_idx]);

            auto const adjacent_edges_snapshot =
                std::vector<uint64_t>(adjacency_map[to_node_idx].begin(), adjacency_map[to_node_idx].end());

            for (uint64_t const adjacent_edge_idx : adjacent_edges_snapshot) {
                // re-attach edge and write to matrix
                replace_and_write(adjacent_edge_idx, from_node_idx, to_node_idx, matrix_row, edges, matrix);

                // update adjacency list after re-attachment
                adjacency_map[to_node_idx].erase(adjacent_edge_idx);
                adjacency_map[from_node_idx].insert(adjacent_edge_idx);

                // update edges_history and edges_contracted_to_point (if needed) after re-attachment
                update_edge_info(adjacent_edge_idx, matrix_row, edges, edges_history, edges_contracted_to_point);
            }
            ++matrix_row;
        }
    }
    return result;
}

// backward substitution is performed in a sparse way
// using the result from the elimination game
inline void backward_substitution(EliminationResult& elimination_result) {
    auto pivot_col_indices = elimination_result.pivot_edge_indices | std::views::drop(1) | std::ranges::views::reverse;
    auto free_col_indices = std::span(elimination_result.free_edge_indices);

    for (auto const pivot_col_idx : pivot_col_indices) {
        auto const& edge_history = elimination_result.edges_history[pivot_col_idx];
        uint64_t const pivot_row_idx = edge_history.rows.back();

        for (auto const row_idx : edge_history.rows | std::views::reverse | std::views::drop(1)) {
            IntS multiplier_value{};
            elimination_result.matrix.get_value(multiplier_value, row_idx, pivot_col_idx);
            elimination_result.matrix.add_to_value(
                static_cast<IntS>(-multiplier_value), row_idx,
                pivot_col_idx); // the initial pivot is always one because of the forward sweep

            for (auto const backward_col_idx : std::ranges::subrange(
                     std::ranges::upper_bound(free_col_indices, pivot_col_idx), free_col_indices.end())) {
                IntS pivot_value{};
                if (elimination_result.matrix.get_value(pivot_value, pivot_row_idx, backward_col_idx)) {
                    elimination_result.matrix.add_to_value(static_cast<IntS>(-multiplier_value * pivot_value), row_idx,
                                                           backward_col_idx);
                }
            }

            elimination_result.rhs[row_idx] +=
                -static_cast<DoubleComplex>(multiplier_value) * elimination_result.rhs[pivot_row_idx];
        }
    }
}

struct SolutionSet{
	COOSparseMatrix dfs_matrix{};
    std::vector<DoubleComplex> extended_rhs{}; 
	
};

[[nodiscard]] inline SolutionSet set_solution_system(EliminationResult & result){
    
	using enum EdgeEvent;
	IntS value;
    constexpr uint8_t starting_row{};
	auto& [matrix, rhs, free_edge_indices, edges_history] = result;
	
	SolutionSet solution_set{};
	
	auto& [dfs_matrix, extended_rhs] =  solution_set;
	dfs_matrix.prepare(edges_history.size(),free_edge_indices.size());
	
	uint64_t matrix_row{starting_row};
	uint64_t dfs_matrix_row{starting_row};
	uint64_t dfs_index{starting_row};
	for (auto edge_history : edges_history){
		if (edge_history.events.back() == Deleted) {
			for (uint64_t j = 0; j < free_edge_indices.size(); j++){
				if (matrix.get_value(value, matrix_row, free_edge_indices[j])){
					dfs_matrix.set_value(value,dfs_matrix_row,j);
				}
			}
			extended_rhs.push_back(rhs[matrix_row]);
			++matrix_row;
		} else {
			dfs_matrix.set_value(-1,dfs_matrix_row,dfs_index);
			extended_rhs.push_back(0);
			++dfs_index;
		}
		
		++dfs_matrix_row;
	}	
	return solution_set;	
};

} // namespace power_grid_model::link_solver::detail
