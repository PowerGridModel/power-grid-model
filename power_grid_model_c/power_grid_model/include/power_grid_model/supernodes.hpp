// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "calculation_parameters.hpp"
#include "common/common.hpp"
#include "common/counting_iterator.hpp"
#include "common/grouped_index_vector.hpp"
#include "common/typing.hpp"
#include "index_mapping.hpp"

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/property.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace power_grid_model::supernodes {
class TopologicalNode {
  public:
    TopologicalNode() = default;
    TopologicalNode(IdxVector user_nodes, std::vector<Idx2D> user_links)
        : user_nodes_{std::move(user_nodes)}, user_links_{std::move(user_links)} {
        check_sanity();
    }

    constexpr bool is_supernode() const noexcept {
        check_sanity();
        return !std::ranges::empty(user_links_);
    }

  private:
    IdxVector user_nodes_;
    std::vector<Idx2D> user_links_;

    constexpr void check_sanity() const noexcept {
        // a supernode has multiple user nodes and at least one user link; a non-supernode only contains one user node
        assert(!std::ranges::empty(user_nodes_));
        assert(is_supernode() ? (user_nodes_.size() > 1 && !std::ranges::empty(user_links_))
                              : (user_nodes_.size() == 1 && std::ranges::empty(user_links_)));
    }
};

class TopologicalNodeMapping {
  public:
    TopologicalNodeMapping() = default;
    explicit TopologicalNodeMapping(Idx n_topo_nodes_, IdxVector mapping)
        : mapping_{std::move(mapping)}, n_topo_nodes_{n_topo_nodes_} {
        check_sanity();
    }

    auto mapping() const& -> IdxVector const& { return mapping_; }
    auto mapping() && -> IdxVector { return std::move(mapping_); }

    constexpr Idx n_topo_nodes() const { return n_topo_nodes_; }
    constexpr Idx n_user_nodes() const { return mapping_.size(); }

  private:
    IdxVector mapping_;
    Idx n_topo_nodes_{};

    constexpr void check_sanity() const noexcept {
        // the mapping should be a valid partition of user nodes into topological nodes
        assert(n_topo_nodes_ == 0 || (n_topo_nodes_ == std::ranges::max(mapping_) + 1));
    }
};

struct ReducedTopology {
    ComponentTopology comp_topo;
    std::vector<TopologicalNode> topological_nodes;
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

    auto const global_graph_ =
        GlobalGraph{boost::edges_are_unsorted_multi_pass, internal_edges.cbegin(), internal_edges.cend(), n_nodes};

    Idx const num_connected_components = boost::connected_components(global_graph_, vertices.data());
    return TopologicalNodeMapping{num_connected_components, std::move(vertices)};
}

// Union-find algorithm:
//   n nodes = 6
//   lines/trafos = [[0, 1], [3, 4], [5, 4]]
//   links = [[1, 3], [5, 2], [2, 1]]
//
// Start: [0, 1, 2, 3, 4, 5]
//   Process links in comp topo and comp conn
// NAIVE (WRONG): [0, 1, 1, 1, 4, 2]  (2 should be super node)
//   Fixing this retroactively is O(N_links^2)
//   Unless we use counting sort, which is O(N_nodes + N_links)
// Result:
//   [0, 1, 1, 1, 4, 1]
// Mapping (explanation):
//   [N0, SN1, SN1, SN1, N4, SN1]
//   [TN0, TN1, TN1, TN1, TN2, TN1]
inline TopologicalNodeMapping create_map(ComponentTopology const& comp_topo, ComponentConnections const& comp_conn) {
    return find_link_connected_components(comp_topo.n_node, comp_topo.link_node_idx, comp_conn.link_connected);
};

inline std::vector<TopologicalNode> create_topological_nodes(ComponentTopology const& comp_topo,
                                                             ComponentConnections const& /*comp_conn*/,
                                                             TopologicalNodeMapping const& topo_node_mapping) {
    // auto topo_nodes = std::ranges::transform(topo_node_mapping.mapping(),
    //                                          [](IdxRange const& user_nodes) {
    //                                              return TopologicalNode{user_nodes | std::ranges::to<IdxVector>(),
    //                                              {}};
    //                                          }) |
    //                   std::ranges::to<std::vector<TopologicalNode>>();

    // std::ranges::for_each(comp_topo.link_node_idx, [&topo_nodes](Idx2D const& link) {
    //     auto& [i, j] = link;
    //     topo_nodes[i].user_links_.push_back(link);
    //     topo_nodes[j].user_links_.push_back(link);
    // });

    // return topo_nodes;
    return {};
};

// Use the topo_node_mapping to remap every part of comp_topo such that the output nodes are the topological nodes;
// not the user nodes. This effectively removes all link-related stuff from the math topology.
inline ComponentTopology construct_reduced_topology(ComponentTopology const& comp_topo,
                                                    TopologicalNodeMapping const& /*topo_node_mapping*/) {
    // TODO(mgovers): do we really require a full deep-copy of comp_topo here?
    ComponentTopology comp_topo_reduced{comp_topo}; // TODO(mgovers):  replace with real implementation

    // TODO(marcvanraalte): the implementation goes here

    return comp_topo_reduced;
}
} // namespace detail

inline ReducedTopology reduce_topology(ComponentTopology const& comp_topo, ComponentConnections const& comp_conn) {
    using namespace detail;

    auto topo_node_mapping = create_map(comp_topo, comp_conn);
    return ReducedTopology{.comp_topo = construct_reduced_topology(comp_topo, topo_node_mapping),
                           .topological_nodes = create_topological_nodes(comp_topo, comp_conn, topo_node_mapping)};
}

} // namespace power_grid_model::supernodes
