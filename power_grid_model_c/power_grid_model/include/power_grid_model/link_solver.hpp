// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "calculation_parameters.hpp"
#include "common/common.hpp"
#include "common/counting_iterator.hpp"
#include "common/typing.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace power_grid_model::link_solver {
namespace detail {

using AdjacencyMap = std::unordered_map<Idx, std::unordered_set<Idx>>;
using ContractedEdgesSet = std::unordered_set<Idx>;

enum class EdgeEvent : std::uint8_t {
    deleted = 0,            // pivot edge - used as pivot row
    replaced = 1,           // reattached to a different node
    contracted_to_point = 2 // from == to, becomes a degree of freedom
};

enum class EdgeDirection : IntS { outgoing = -1, incoming = 1 };

// Coordinate list (COO) sparse matrix representation, optimized for incremental construction during forward elimination
struct CooSparseMatrix {
    Idx row_number{};
    std::unordered_map<Idx, IntS> data_map{};

    void prepare(auto row_size, auto col_size) {
        row_number = col_size;
        data_map.reserve(row_size *
                         col_size); // worst case to guarantee O(1) insertion/retrieval, but typically much sparser
    }
    void set_value(IntS value, Idx row_idx, Idx col_idx) {
        assert(row_number != 0 && "row_number must be set before setting values in data_map");
        data_map[row_idx * row_number + col_idx] = value;
    }
    std::optional<IntS> get_value(Idx row_idx, Idx col_idx) const {
        assert(row_number != 0 && "row_number must be set before getting values from data_map");
        if (auto const it = data_map.find(row_idx * row_number + col_idx); it != data_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    void add_to_value(IntS value, Idx row_idx, Idx col_idx) {
        assert(row_number != 0 && "row_number must be set before adding values in data_map");
        auto const key = row_idx * row_number + col_idx;
        if (auto const it = data_map.find(key); it != data_map.end()) {
            auto const new_value = narrow_cast<IntS>(it->second + value);
            if (new_value == 0) {
                data_map.erase(it); // maintain sparsity by erasing zero entries
                return;
            }
            it->second = new_value;
        } else if (value != 0) {
            data_map[key] = value;
        }
    }
};

struct EdgeHistory {
    std::vector<Idx> rows{};
    std::vector<EdgeEvent> events{};
};

struct ReducedEchelonFormResult {
    CooSparseMatrix matrix{};
    std::vector<DoubleComplex> rhs{};         // RHS value at each pivot row
    std::vector<Idx> free_edge_indices{};     // index of degrees of freedom (self loop edges)
    std::vector<Idx> pivot_edge_indices{};    // index of pivot edges
    std::vector<EdgeHistory> edges_history{}; // edges elimination history
};

// convention: from node at position 0, to node at position 1
constexpr Idx from_node(BranchIdx const& edge) { return edge[0]; }
constexpr void re_attach_from_node(Idx new_node, BranchIdx& edge) { edge[0] = new_node; }
constexpr Idx to_node(BranchIdx const& edge) { return edge[1]; }
constexpr void re_attach_to_node(Idx new_node, BranchIdx& edge) { edge[1] = new_node; }

// map from node index to the set of adjacent edge indices
// unordered_map because node IDs may be sparse/non-contiguous
// unordered_set for O(1) insert/erase during reattachment
inline auto build_adjacency_map(std::span<BranchIdx const> edges) -> AdjacencyMap {
    AdjacencyMap adjacency_map{};
    for (auto const& [index, edge] : enumerate(edges)) {
        auto const [from_node, to_node] = edge;
        adjacency_map[from_node].insert(index);
        adjacency_map[to_node].insert(index);
    }
    return adjacency_map;
}

inline void write_edge_history(EdgeHistory& edges_history, EdgeEvent event, Idx row) {
    edges_history.events.push_back(event);
    edges_history.rows.push_back(row);
}

inline void replace_and_write(Idx edge_idx, Idx from_node_idx, Idx to_node_idx, Idx matrix_row,
                              std::vector<BranchIdx>& edges, CooSparseMatrix& matrix) {
    using enum EdgeDirection;

    if (from_node(edges[edge_idx]) == to_node_idx) {
        re_attach_from_node(from_node_idx, edges[edge_idx]);
        matrix.set_value(std::to_underlying(outgoing), matrix_row, edge_idx);
    } else {
        re_attach_to_node(from_node_idx, edges[edge_idx]);
        matrix.set_value(std::to_underlying(incoming), matrix_row, edge_idx);
    }
}

inline void update_edge_info(Idx edge_idx, Idx matrix_row, std::vector<BranchIdx>& edges,
                             std::vector<EdgeHistory>& edges_history, ContractedEdgesSet& edges_contracted_to_point) {
    using enum EdgeEvent;

    if (from_node(edges[edge_idx]) == to_node(edges[edge_idx])) {
        write_edge_history(edges_history[edge_idx], contracted_to_point, matrix_row);
        edges_contracted_to_point.insert(edge_idx);
    } else {
        write_edge_history(edges_history[edge_idx], replaced, matrix_row);
    }
}

// forward elimination is performed via a modified Gaussian elimination procedure
// we name this procedure elimination-game
// node_loads convention: caller passes negated external loads (as in RHS of power flow equations))
inline void forward_elimination(ReducedEchelonFormResult& result, std::vector<BranchIdx> edges,
                                std::vector<DoubleComplex> node_loads) {
    using enum EdgeEvent;
    using enum EdgeDirection;

    Idx matrix_row{};
    auto adjacency_map = build_adjacency_map(edges);
    ContractedEdgesSet edges_contracted_to_point{};

    for (auto const& [index, edge] : enumerate(std::as_const(edges))) {
        if (edges_contracted_to_point.contains(index)) {
            result.free_edge_indices.push_back(index);
        } else {
            write_edge_history(result.edges_history[index], deleted, matrix_row); // Delete edge -> pivot there
            result.pivot_edge_indices.push_back(index);
            result.matrix.set_value(std::to_underlying(incoming), matrix_row, index);

            Idx const from_node_idx = from_node(edge);
            Idx const to_node_idx = to_node(edge);

            // update adjacency list for deleted edge
            adjacency_map[from_node_idx].erase(index);
            adjacency_map[to_node_idx].erase(index);

            // Gaussian elimination like steps
            node_loads[from_node_idx] += node_loads[to_node_idx];
            result.rhs.push_back(node_loads[to_node_idx]);

            auto const adjacent_edges_snapshot =
                std::vector<Idx>(adjacency_map[to_node_idx].begin(), adjacency_map[to_node_idx].end());

            for (Idx const adjacent_edge_idx : adjacent_edges_snapshot) {
                // re-attach edge and write to matrix
                replace_and_write(adjacent_edge_idx, from_node_idx, to_node_idx, matrix_row, edges, result.matrix);

                // update adjacency list after re-attachment
                adjacency_map[to_node_idx].erase(adjacent_edge_idx);
                adjacency_map[from_node_idx].insert(adjacent_edge_idx);

                // update edges_history and edges_contracted_to_point (if needed) after re-attachment
                update_edge_info(adjacent_edge_idx, matrix_row, edges, result.edges_history, edges_contracted_to_point);
            }
            ++matrix_row;
        }
    }
}

inline auto backward_substitution_pivots(std::span<Idx const> pivot_edge_indices) {
    return pivot_edge_indices | std::views::drop(1) | std::views::reverse;
}

inline auto backward_substitution_rows(std::span<Idx const> edge_history_rows) {
    return edge_history_rows | std::views::reverse | std::views::drop(1);
}

inline auto backward_substitution_free_right_cols(std::span<Idx const> free_col_indices, Idx pivot_col_idx) {
    return std::ranges::subrange(std::ranges::upper_bound(free_col_indices, pivot_col_idx), free_col_indices.end());
}

// backward substitution is performed in a sparse way
// using the result from the elimination game
inline void backward_substitution(ReducedEchelonFormResult& elimination_result) {
    auto free_col_indices = std::span<Idx const>(elimination_result.free_edge_indices);

    for (auto const pivot_col_idx : backward_substitution_pivots(elimination_result.pivot_edge_indices)) {
        auto const& edge_history = elimination_result.edges_history[pivot_col_idx];
        Idx const pivot_row_idx = edge_history.rows.back();

        for (auto const row_idx : backward_substitution_rows(edge_history.rows)) {
            auto const multiplier_value =
                elimination_result.matrix.get_value(row_idx, pivot_col_idx).value(); // must always exist
            elimination_result.matrix.add_to_value(narrow_cast<IntS>(-multiplier_value), row_idx, pivot_col_idx);

            // only iterate over free columns to the right of the pivot column
            // as these are the only ones that can be affected by the backward substitution
            for (auto const backward_col_idx : backward_substitution_free_right_cols(free_col_indices, pivot_col_idx)) {
                elimination_result.matrix.get_value(pivot_row_idx, backward_col_idx)
                    .transform([&elimination_result, multiplier_value, row_idx, backward_col_idx](IntS pivot_value) {
                        elimination_result.matrix.add_to_value(static_cast<IntS>(-multiplier_value * pivot_value),
                                                               row_idx, backward_col_idx);
                        return pivot_value;
                    });
            }
            elimination_result.rhs[row_idx] -=
                static_cast<DoubleComplex>(multiplier_value) * elimination_result.rhs[pivot_row_idx];
        }
    }
}

// reduced echelon form based on custom forward elimination and backward substitution procedures
// in other words, this performs the Penrose inverse on the adjacency matrix
inline ReducedEchelonFormResult reduced_echelon_form(std::vector<BranchIdx>& edges,
                                                     std::vector<DoubleComplex>& node_loads) {
    ReducedEchelonFormResult result{};
    auto const edge_number{edges.size()};
    auto const node_number{node_loads.size()};

    result.edges_history.resize(edge_number);
    result.matrix.prepare(node_number, edge_number);

    // -1 because the loads represent the RHS
    std::ranges::for_each(node_loads, [](auto& load) { load = -load; });

    // both edges and node_loads are modified and consumed in the forward sweep
    forward_elimination(result, std::move(edges), std::move(node_loads));
    backward_substitution(result);
    return result;
}

// The degrees of freedom matrix (dfs_matrix) is associated with the degrees of freedom vector according
// internal_loads = extended_rhs - dfs_matrix * lambda
struct SolutionSet {
    CooSparseMatrix dfs_matrix{};
    std::vector<DoubleComplex> extended_rhs{};
};

// Constructs the dfs_matrix and the extended_rhs for the set of solutions.
// The dfs_matrix is constructed from the matrix rows associated with the free column indices
// and from negative unit elements at locations where the variables are associated with the free parameters.
inline SolutionSet set_solution_system(ReducedEchelonFormResult& result) {
    SolutionSet solution_set{};

    auto& [dfs_matrix, extended_rhs] = solution_set;
    auto const pivot_indices_size = narrow_cast<Idx>(result.pivot_edge_indices.size());
    auto const free_indices_size = narrow_cast<Idx>(result.free_edge_indices.size());
    Idx const total_indices_size = pivot_indices_size + free_indices_size;
    dfs_matrix.prepare(total_indices_size, free_indices_size);
    extended_rhs.resize(total_indices_size);
    constexpr auto const free_matrix_element = IntS{-1};
    Idx free_edge_idx;

    // The part constructed from result.matrix and result.rhs.
    for (auto matrix_row : std::views::iota(Idx{}, pivot_indices_size)) {
        auto const pivot_edge_idx = result.pivot_edge_indices[matrix_row];
        for (auto dfs_matrix_col : std::views::iota(Idx{}, free_indices_size)) {
            free_edge_idx = result.free_edge_indices[dfs_matrix_col];
            result.matrix.get_value(matrix_row, free_edge_idx)
                .transform([&dfs_matrix, pivot_edge_idx, dfs_matrix_col](IntS matrix_element) {
                    dfs_matrix.set_value(matrix_element, pivot_edge_idx, dfs_matrix_col);
                    return matrix_element;
                });
        }
        extended_rhs[pivot_edge_idx] = result.rhs[matrix_row];
    }

    // The part constructed from negative unit elements.
    for (auto dfs_matrix_col : std::views::iota(Idx{}, free_indices_size)) {
        free_edge_idx = result.free_edge_indices[dfs_matrix_col];
        dfs_matrix.set_value(free_matrix_element, free_edge_idx, dfs_matrix_col);
        extended_rhs[result.free_edge_indices[dfs_matrix_col]] = DoubleComplex{};
    }
    return solution_set;
};

inline std::vector<std::vector<DoubleComplex>> set_projection_system(Idx free_indices_number, Idx total_indices_number,
                                                                     SolutionSet& solution_set) {
    std::vector<std::vector<DoubleComplex>> projection_system(free_indices_number,
                                                              std::vector<DoubleComplex>(free_indices_number + 1));

    for (Idx dfs_matrix_col = 0; dfs_matrix_col < free_indices_number; dfs_matrix_col++) {
        auto dot_product_rhs = DoubleComplex{};
        for (Idx dfs_matrix_row = 0; dfs_matrix_row < total_indices_number; dfs_matrix_row++) {
            solution_set.dfs_matrix.get_value(dfs_matrix_row, dfs_matrix_col)
                .transform(
                    [&dot_product_rhs, &extended_rhs_ = solution_set.extended_rhs, &dfs_matrix_row](IntS first_value) {
                        dot_product_rhs += static_cast<DoubleComplex>(first_value) * extended_rhs_[dfs_matrix_row];
                        return first_value;
                    });
        }
        projection_system[dfs_matrix_col][free_indices_number] = dot_product_rhs;
        for (Idx second_dfs_matrix_col = dfs_matrix_col; second_dfs_matrix_col < free_indices_number;
             second_dfs_matrix_col++) {
            auto dot_product_matrix = DoubleComplex{};
            for (Idx dfs_matrix_row = 0; dfs_matrix_row < total_indices_number; dfs_matrix_row++) {
                auto const first_value = solution_set.dfs_matrix.get_value(dfs_matrix_row, dfs_matrix_col);
                auto const second_value = solution_set.dfs_matrix.get_value(dfs_matrix_row, second_dfs_matrix_col);
                if (first_value && second_value) {
                    dot_product_matrix += static_cast<DoubleComplex>(first_value.value() * second_value.value());
                }
            }
            projection_system[dfs_matrix_col][second_dfs_matrix_col] = dot_product_matrix;
            projection_system[second_dfs_matrix_col][dfs_matrix_col] = dot_product_matrix;
        }
    }
    return projection_system;
};

inline void naive_gauss_elimination(std::vector<std::vector<DoubleComplex>>& system) {
    auto const system_size = narrow_cast<Idx>(std::ssize(system));

    // we skip pivoting since the matrix system is mostly diagonally dominant
    // and given the real power systems topology of super nodes, this should not introduce
    // any numerical instabilities in practice

    // forward elimination
    for (Idx const column : IdxRange{system_size}) {
        auto const& pivot = system[column][column];
        for (Idx const row : IdxRange{column + Idx{1}, system_size}) {
            auto& row_col_value = system[row][column];
            row_col_value /= -pivot;
            for (Idx const column_part : IdxRange{column + Idx{1}, system_size + Idx{1}}) {
                system[row][column_part] += row_col_value * system[column][column_part];
            }
        }
    }

    // backward substitution
    system[system_size - 1][system_size] /= system[system_size - 1][system_size - 1];
    for (Idx const row : IdxRange{system_size - 1} | std::views::reverse) {
        auto element_sum = DoubleComplex{};
        for (Idx const column : IdxRange{row + Idx{1}, system_size}) {
            element_sum -= system[row][column] * system[column][system_size];
        }
        system[row][system_size] = (system[row][system_size] + element_sum) / system[row][row];
    }
};

inline std::vector<DoubleComplex> compute_internal_loads(SolutionSet const& solution_set,
                                                         std::vector<std::vector<DoubleComplex>> const& system) {
    auto const number_of_rows = narrow_cast<Idx>(solution_set.extended_rhs.size());
    auto const number_of_columns = narrow_cast<Idx>(system.size());
    std::vector<DoubleComplex> internal_loads(number_of_rows);

    for (auto const row : IdxRange{number_of_rows}) {
        internal_loads[row] = solution_set.extended_rhs[row];
        auto sum_value = DoubleComplex{};
        for (auto const column : IdxRange{number_of_columns}) {
            solution_set.dfs_matrix.get_value(row, column).transform([&sum_value, &system, column](IntS value) {
                sum_value += static_cast<DoubleComplex>(value) * system[column].back();
                return value;
            });
        }
        internal_loads[row] -= sum_value;
    }

    return internal_loads;
};
} // namespace detail

inline std::vector<DoubleComplex> compute_loads_link_elements(std::vector<BranchIdx>& edges,
                                                              std::vector<DoubleComplex>& node_loads) {
    using namespace detail;

    auto reduced_echelon_result = reduced_echelon_form(edges, node_loads);
    auto solution_set = set_solution_system(reduced_echelon_result);

    if (solution_set.dfs_matrix.data_map.empty()) {
        return solution_set.extended_rhs;
    }

    auto const free_indices_number = narrow_cast<Idx>(reduced_echelon_result.free_edge_indices.size());
    auto const total_indices_number = narrow_cast<Idx>(reduced_echelon_result.free_edge_indices.size() +
                                                       reduced_echelon_result.pivot_edge_indices.size());
    std::vector<std::vector<DoubleComplex>> projection_system =
        set_projection_system(free_indices_number, total_indices_number, solution_set);

    naive_gauss_elimination(projection_system);

    return compute_internal_loads(solution_set, projection_system);
};

// TODO(figueroa1395): look for cache improvements in the code

} // namespace power_grid_model::link_solver
