// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "calculation_parameters.hpp"
#include "common/common.hpp"
#include "common/counting_iterator.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/property.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

namespace power_grid_model::supernodes {
class TopologicalNodeMapping {
  public:
    explicit TopologicalNodeMapping(Idx n_topo_nodes, IdxVector mapping)
        : mapping_{std::move(mapping)}, n_topo_nodes_{n_topo_nodes} {
        check_sanity();
    }

    auto mapping() const& -> IdxVector const& { return mapping_; }
    auto mapping() && -> IdxVector { return std::move(mapping_); }

    constexpr Idx n_topo_nodes() const { return n_topo_nodes_; }
    constexpr Idx n_user_nodes() const { return std::ssize(mapping_); }

  private:
    IdxVector mapping_;
    Idx n_topo_nodes_{};

    constexpr void check_sanity() const noexcept { // NOLINT(readability-convert-member-functions-to-static)
        // the mapping should be a valid partition of user nodes into topological nodes
        assert(n_topo_nodes_ == (std::ranges::empty(mapping_) ? 0 : std::ranges::max(mapping_) + 1));
    }
};

namespace detail {

inline TopologicalNodeMapping find_link_connected_components(Idx n_nodes, std::vector<BranchIdx> const& edges,
                                                             std::vector<BranchConnected> const& edge_connections) {
    struct GlobalEdge {};
    struct GlobalVertex {};

    // sparse directed graph
    // edge i -> j
    using GlobalGraph = boost::compressed_sparse_row_graph<boost::bidirectionalS, GlobalVertex, GlobalEdge,
                                                           boost::no_property, Idx, Idx>;

    IdxVector vertices = IdxRange{n_nodes} | std::ranges::to<IdxVector>();

    std::vector<std::pair<Idx, Idx>> internal_edges;
    internal_edges.reserve(2 * edges.size());
    std::ranges::for_each(std::views::zip(edges, edge_connections),
                          [&internal_edges](auto const& vertices_and_connectivity) {
                              if (BranchConnected const& connectivity = std::get<1>(vertices_and_connectivity);
                                  connectivity[0] == 0 || connectivity[1] == 0) {
                                  return; // only add edge if both sides are connected
                              }
                              BranchIdx const& vertices = std::get<0>(vertices_and_connectivity);
                              auto const from = vertices[0];
                              auto const to = vertices[1];
                              internal_edges.emplace_back(from, to);
                              internal_edges.emplace_back(to, from);
                          });

    assert(std::ranges::all_of(internal_edges, [n_nodes](auto const& edge) {
        auto const& [from, to] = edge;
        return 0 <= from && from < n_nodes && 0 <= to && to < n_nodes;
    }));

    auto const global_graph_ =
        GlobalGraph{boost::edges_are_unsorted_multi_pass, internal_edges.cbegin(), internal_edges.cend(), n_nodes};

    Idx const num_connected_components = boost::connected_components(global_graph_, vertices.data());
    return TopologicalNodeMapping{num_connected_components, std::move(vertices)};
}

// Find connected components in the link-only grid (similar to union-find)
//   n nodes = 6
//   lines/trafos = [[0, 1], [3, 4], [5, 4]]
//   links = [[1, 3], [5, 2], [2, 1]]
//
// Start: [0, 1, 2, 3, 4, 5]
//   Process links in comp topo and comp conn
// Result:
//   [0, 1, 1, 1, 4, 1]
// Mapping (explanation):
//   [N0, SN1, SN1, SN1, N4, SN1]
//   [TN0, TN1, TN1, TN1, TN2, TN1]
//
// Note: we can't use a naive forward-sweep union-find approach as that would result in
//   [0, 1, 1, 1, 4, 2]  (2 should be part of the super node)
//   Fixing this retroactively is O(N_links^2)
//   The solution is to use the boost connected components algorithm, which is linear
inline TopologicalNodeMapping create_map(ComponentTopology const& comp_topo, ComponentConnections const& comp_conn) {
    return find_link_connected_components(comp_topo.n_node, comp_topo.link_node_idx, comp_conn.link_connected);
};

inline TopologicalNodesAndCoupling create_topological_nodes(ComponentTopology const& comp_topo,
                                                            ComponentConnections const& comp_conn,
                                                            TopologicalNodeMapping const& topo_node_mapping) {
    auto const& node_mapping = topo_node_mapping.mapping();

    std::vector<TopologicalNode> topo_nodes(topo_node_mapping.n_topo_nodes());
    std::vector<Idx2D> user_node_topo_node_coup = enumerate(node_mapping) |
                                                  std::views::transform([&topo_nodes](auto const& idx_and_topo) {
                                                      auto const& [user_node, topo_node] = idx_and_topo;
                                                      topo_nodes[topo_node].user_nodes.push_back(user_node);
                                                      return Idx2D{.group = topo_node, .pos = user_node};
                                                  }) |
                                                  std::ranges::to<std::vector>();

    std::vector<Idx2D> user_link_topo_node_coup =
        enumerate(std::views::zip(comp_topo.link_node_idx, comp_conn.link_connected)) |
        std::views::transform([&node_mapping, &topo_nodes](auto const& idx_link_and_connectivity) {
            auto const& [link_idx, link_nodes_and_connectivity] = idx_link_and_connectivity;
            auto const& [link_nodes, link_connected] = link_nodes_and_connectivity;

            auto const [from, to] = link_nodes;
            auto const from_conn = link_connected[0] == 0 ? disconnected : from;
            auto const to_conn = link_connected[1] == 0 ? disconnected : to;

            assert((from_conn == disconnected || to_conn == disconnected || node_mapping[from] == node_mapping[to]) &&
                   "if both sides are connected, they should belong to the same topo node");

            auto const topo_node = [from_conn, to_conn, &node_mapping] {
                if (from_conn != disconnected) {
                    return node_mapping[from_conn];
                }
                if (to_conn != disconnected) {
                    return node_mapping[to_conn];
                }
                return disconnected;
            }();

            if (topo_node == disconnected) {
                return Idx2D{.group = disconnected, .pos = disconnected};
            }

            auto& user_links = topo_nodes[topo_node].user_links;
            Idx const pos = std::ssize(user_links);
            user_links.push_back(BranchIdx{from_conn, to_conn}); // can't emplace_back because BranchIdx is std::array
            return Idx2D{.group = topo_node, .pos = pos};
        }) |
        std::ranges::to<std::vector>();

    return TopologicalNodesAndCoupling{.topo_nodes = std::move(topo_nodes),
                                       .coupling = {.n_topo_nodes = topo_node_mapping.n_topo_nodes(),
                                                    .user_nodes_to_topo_nodes = std::move(user_node_topo_node_coup),
                                                    .user_links_to_topo_nodes = std::move(user_link_topo_node_coup)}};
};

// Use the topo_node_mapping to remap every part of comp_topo such that the output nodes are the topological nodes;
// not the user nodes. This effectively removes all link-related stuff from the math topology.
inline ReducedComponentTopology construct_reduced_topology(ComponentTopology const& comp_topo,
                                                           TopologicalNodeMapping const& topo_node_mapping) {
    auto remap_nodes = [&topo_node_mapping]<typename IdxType>(std::vector<IdxType> const& user_nodes) {
        return user_nodes | std::views::transform([&mapping = topo_node_mapping.mapping()](IdxType const& user_node) {
                   if constexpr (std::same_as<IdxType, Idx>) {
                       return mapping[user_node];
                   } else {
                       static_assert(
                           requires { std::tuple_size_v<IdxType>; },
                           "IdxType must have a static size (e.g. std::array)");
                       return [&]<std::size_t... I>(std::index_sequence<I...>) {
                           return IdxType{mapping[user_node[I]]...};
                       }(std::make_index_sequence<std::tuple_size_v<IdxType>>{});
                   }
               }) |
               std::ranges::to<std::vector<IdxType>>();
    };

    return ReducedComponentTopology{
        .n_node = topo_node_mapping.n_topo_nodes(),
        .branch_node_idx = remap_nodes(comp_topo.branch_node_idx),
        .branch3_node_idx = remap_nodes(comp_topo.branch3_node_idx),
        .shunt_node_idx = remap_nodes(comp_topo.shunt_node_idx),
        .source_node_idx = remap_nodes(comp_topo.source_node_idx),
        .load_gen_node_idx = remap_nodes(comp_topo.load_gen_node_idx),
        .load_gen_type = std::span{comp_topo.load_gen_type},
        .voltage_sensor_node_idx = remap_nodes(comp_topo.voltage_sensor_node_idx),
        .power_sensor_object_idx = std::span{comp_topo.power_sensor_object_idx},
        .power_sensor_terminal_type = std::span{comp_topo.power_sensor_terminal_type},
        .current_sensor_object_idx = std::span{comp_topo.current_sensor_object_idx},
        .current_sensor_terminal_type = std::span{comp_topo.current_sensor_terminal_type},
        .regulator_type = std::span{comp_topo.regulator_type},
        .regulated_object_idx = std::span{comp_topo.regulated_object_idx},
        .regulated_object_type = std::span{comp_topo.regulated_object_type},
    };
}

inline ReducedTopology dont_reduce_topology(ComponentTopology const& comp_topo,
                                            ComponentConnections const& /*comp_conn*/) {
    using namespace detail;

    return ReducedTopology{
        .reduced_comp_topo = ReducedComponentTopology::from_component_topology(comp_topo),
        .topo_node_coup = {
            .topo_nodes = IdxRange{comp_topo.n_node_total()} | std::views::transform([](Idx idx) {
                              return TopologicalNode{
                                  .user_nodes = std::vector{idx},
                                  .user_links = {},
                              };
                          }) |
                          std::ranges::to<std::vector>(),
            .coupling =
                {
                    .n_topo_nodes = comp_topo.n_node_total(),
                    .user_nodes_to_topo_nodes = IdxRange{comp_topo.n_node_total()} | std::views::transform([](Idx idx) {
                                                    return Idx2D{.group = idx, .pos = 0};
                                                }) |
                                                std::ranges::to<std::vector>(),
                    .user_links_to_topo_nodes = {},
                },
        }};
}
} // namespace detail

inline ReducedTopology reduce_topology(ComponentTopology const& comp_topo, ComponentConnections const& comp_conn) {
    using namespace detail;

    if (std::ranges::empty(comp_topo.link_node_idx)) {
        return dont_reduce_topology(comp_topo, comp_conn);
    }

    auto topo_node_mapping = create_map(comp_topo, comp_conn);
    return ReducedTopology{.reduced_comp_topo = construct_reduced_topology(comp_topo, topo_node_mapping),
                           .topo_node_coup = create_topological_nodes(comp_topo, comp_conn, topo_node_mapping)};
}

} // namespace power_grid_model::supernodes
