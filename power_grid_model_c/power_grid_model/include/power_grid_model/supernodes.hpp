// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "calculation_parameters.hpp"
#include "common/common.hpp"
#include "common/counting_iterator.hpp"
#include "common/grouped_index_vector.hpp"

#include <cassert>
#include <ranges>
#include <utility>
#include <vector>

namespace power_grid_model::supernodes {
class TopologicalNode {
  public:
    TopologicalNode() = default;
    TopologicalNode(IdxVector user_nodes, std::vector<Idx2D> user_links)
        : user_nodes_{std::move(user_nodes)}, user_links_{std::move(user_links)} {
        assert(!std::ranges::empty(user_nodes_));
        assert(user_nodes_.size() > 1 || std::ranges::empty(user_links_));
    }

    constexpr bool is_supernode() const noexcept { return std::ranges::empty(user_links_); }

  private:
    IdxVector user_nodes_;
    std::vector<Idx2D> user_links_;
};

struct TopologicalNodeMapping : DenseGroupedIdxVector {
    using DenseGroupedIdxVector::DenseGroupedIdxVector;

    constexpr Idx n_topo_nodes() const { return size(); }
};

struct ReducedTopology {
    ComponentTopology comp_topo;
    std::vector<TopologicalNode> topological_nodes;
};

namespace detail {
// Union-find algorithm:
// n nodes = 4
// Start: [0, 1, 2, 3]
//   Process links in comp topo and comp conn
// Result: [0, 1, 1, 2]  => this is fed into the dense_group_elements of the DenseGroupedIdxVector
//   (user nodes -> TN)
// Mapping:
//   [N0, SN1, SN1, N2]
//   [TN0, TN1, TN1, TN2]
inline TopologicalNodeMapping create_map(ComponentTopology const& comp_topo,
                                         ComponentConnections const& /*comp_conn*/) {
    IdxVector node_mapping =
        IdxRange{comp_topo.n_node} | std::ranges::to<std::vector>(); // TODO(mgovers): replace with real implementation

    // TODO(marcvanraalte): the implementation goes here

    return TopologicalNodeMapping{from_dense, std::move(node_mapping), comp_topo.n_node};
};

inline std::vector<TopologicalNode> create_topological_nodes(ComponentTopology const& /*comp_topo*/,
                                                             ComponentConnections const& /*comp_conn*/,
                                                             TopologicalNodeMapping const& topo_node_mapping) {
    std::vector<TopologicalNode> topo_nodes(
        topo_node_mapping.n_topo_nodes()); // TODO(mgovers): replace with real implementation

    // TODO(marcvanraalte): the implementation goes here

    return topo_nodes;
};

// Use the topo_node_mapping to remap every part of comp_topo such that the output nodes are the topological nodes; not
// the user nodes. This effectively removes all link-related stuff from the math topology.
inline ComponentTopology construct_reduced_topology(ComponentTopology const& comp_topo,
                                                    TopologicalNodeMapping const& /*topo_node_mapping*/) {
    // TODO(mgovers): do we really require a full deep-copy of comp_topo here?
    ComponentTopology comp_topo_reduced{comp_topo}; // TODO(mgovers):  replace with real implementation

    // TODO(marcvanraalte): the implementation goes here

    return comp_topo_reduced;
}
} // namespace detail

inline ReducedTopology reduce_topology(ComponentTopology const& comp_topo,
                                              ComponentConnections const& comp_conn) {
    using namespace detail;

    auto topo_node_mapping = create_map(comp_topo, comp_conn);
    return ReducedTopology{.comp_topo = construct_reduced_topology(comp_topo, topo_node_mapping),
                           .topological_nodes = create_topological_nodes(comp_topo, comp_conn, topo_node_mapping)};
}

} // namespace power_grid_model::supernodes
