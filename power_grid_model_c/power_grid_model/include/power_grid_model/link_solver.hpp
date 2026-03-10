// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "calculation_parameters.hpp"
#include "common/common.hpp"
#include "common/counting_iterator.hpp"

#include <array>
#include <complex>
#include <concepts>
#include <ranges>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::link_solver {

namespace detail {
constexpr uint8_t starting_row{0};

enum class EdgeEvent : std::uint8_t {
    Deleted = 0,          // pivot edge - used as pivot row
    Replaced = 1,         // reattached to a different node
    ContractedToPoint = 2 // from == to, becomes a degree of freedom
};

enum class EdgeDirection : IntS { Away = -1, Towards = 1 };

struct COOSparseMatrix {
    std::vector<IntS> data;
    std::vector<uint64_t> row;
    std::vector<uint64_t> col;

    void append(IntS value, uint64_t row_idx, uint64_t col_idx) {
        data.push_back(value);
        row.push_back(row_idx);
        col.push_back(col_idx);
    }
};

struct EdgeHistory {
    std::vector<uint64_t> rows{};
    std::vector<EdgeEvent> events{};
};

struct EliminationResult {
    COOSparseMatrix matrix;
    std::vector<DoubleComplex> rhs;          // RHS value at each pivot row
    std::vector<uint64_t> free_edge_indices; // index of degrees of freedom (self loop edges)
    std::vector<EdgeHistory> edge_history;   // edges elimination history
};

// convention: from node at position 0, to node at position 1
[[nodiscard]] constexpr Idx from_node(BranchIdx const& edge) { return edge[0]; }
[[nodiscard]] constexpr Idx& from_node(BranchIdx& edge) { return edge[0]; }
[[nodiscard]] constexpr Idx to_node(BranchIdx const& edge) { return edge[1]; }
[[nodiscard]] constexpr Idx& to_node(BranchIdx& edge) { return edge[1]; }

// map from node index to the set of adjacent edge indices
// unordered_map because node IDs may be sparse/non-contiguous
// unordered_set for O(1) insert/erase during reattachment
[[nodiscard]] auto build_adjacency_list(std::span<BranchIdx> edges) {
    std::unordered_map<Idx, std::unordered_set<uint64_t>> adjacency_list{};
    for (auto const& [raw_index, edge] : enumerate(edges)) {
        auto const index = static_cast<uint64_t>(raw_index);
        auto const [from_node, to_node] = edge;
        adjacency_list[from_node].insert(index);
        adjacency_list[to_node].insert(index);
    }
    return adjacency_list;
}

void write_edge_history(EdgeHistory& edge_history, EdgeEvent event, uint64_t row) {
    edge_history.events.push_back(event);
    edge_history.rows.push_back(row);
}

// forward elimination is performed via a modified Gaussian elimination procedure
// we name this procedure elimination-game
[[nodiscard]] EliminationResult forward_elimination(std::vector<BranchIdx> edges,
                                                    std::vector<DoubleComplex> node_loads) {
    EliminationResult result{.edge_history = std::vector<EdgeHistory>(edges.size())};
    using enum EdgeEvent;
    using enum EdgeDirection;

    auto& [matrix, rhs, free_edge_indices, edge_history] = result;

    std::unordered_set<uint64_t> edges_contracted_to_point{};
    uint64_t matrix_row{starting_row};
    auto adjacency_list = build_adjacency_list(edges);

    for (auto const& [raw_index, edge] : enumerate(edges)) {
        auto const index = static_cast<uint64_t>(raw_index);
        if (edges_contracted_to_point.contains(index)) {
            free_edge_indices.push_back(index);
        } else {
            write_edge_history(edge_history[index], Deleted, matrix_row); // Delete edge -> pivot there
            matrix.append(std::to_underlying(Towards), matrix_row, index);

            Idx const from_node_idx = from_node(edge);
            Idx const to_node_idx = to_node(edge);

            // update adjacency list for deleted edge
            adjacency_list[from_node_idx].erase(index);
            adjacency_list[to_node_idx].erase(index);

            // Gaussian elimination like steps
            node_loads[from_node_idx] += node_loads[to_node_idx];
            rhs.push_back(node_loads[to_node_idx]);

            auto const adjacent_edges_snapshot =
                std::vector<uint64_t>(adjacency_list[to_node_idx].begin(), adjacency_list[to_node_idx].end());

            for (uint64_t const adjacent_edge_idx : adjacent_edges_snapshot) {
                if (from_node(edges[adjacent_edge_idx]) == to_node_idx) {
                    from_node(edges[adjacent_edge_idx]) = from_node_idx; // re-attachment step
                    matrix.append(std::to_underlying(Away), matrix_row, adjacent_edge_idx);
                } else {
                    to_node(edges[adjacent_edge_idx]) = from_node_idx; // re-attachment step
                    matrix.append(std::to_underlying(Towards), matrix_row, adjacent_edge_idx);
                }

                // update adjacency list after re-attachment
                adjacency_list[to_node_idx].erase(adjacent_edge_idx);
                adjacency_list[from_node_idx].insert(adjacent_edge_idx);

                if (from_node(edges[adjacent_edge_idx]) == to_node(edges[adjacent_edge_idx])) {
                    write_edge_history(edge_history[adjacent_edge_idx], ContractedToPoint, matrix_row);
                    edges_contracted_to_point.insert(adjacent_edge_idx);
                } else {
                    write_edge_history(edge_history[adjacent_edge_idx], Replaced, matrix_row);
                }
            }
            ++matrix_row;
        }
    }

    return result;
}
} // namespace detail
} // namespace power_grid_model::link_solver
