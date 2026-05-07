// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/grouped_index_vector.hpp"
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/supernodes.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <array>

namespace {

using namespace power_grid_model;
using namespace power_grid_model::supernodes;
namespace detail = power_grid_model::supernodes::detail;

TEST_CASE("Test Supernodes") {
    SUBCASE("create_map") {
        SUBCASE("No links => no remapping") {
            ComponentTopology const comp_topo{
                .n_node = 3,
                .branch_node_idx = {{0, 1}, {1, 2}, {2, 0}},
            };
            ComponentConnections const comp_conn{
                .branch_connected = {{1, 1}, {1, 1}, {1, 1}},
            };
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 3);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 1, 2});
        }
        SUBCASE("One link between node 0 and 1 => node 0 and 1 are remapped to the same topological node") {
            ComponentTopology const comp_topo{
                .n_node = 3,
                .branch_node_idx = {{0, 1}, {1, 2}, {2, 0}},
                .link_node_idx = {{0, 1}},
            };
            ComponentConnections const comp_conn{
                .branch_connected = {{1, 1}, {1, 1}, {1, 1}},
                .link_connected = {{1, 1}},
            };
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 2);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 0, 1});
        }
        SUBCASE("Disconnected link => not remapped") {
            for (auto const disconnected :
                 std::array{BranchConnected{1, 0}, BranchConnected{0, 1}, BranchConnected{0, 0}}) {
                ComponentTopology const comp_topo{
                    .n_node = 3,
                    .branch_node_idx = {{0, 1}, {1, 2}, {2, 0}},
                    .link_node_idx = {{0, 1}},
                };
                ComponentConnections const comp_conn{
                    .branch_connected = {{1, 1}, {1, 1}, {1, 1}},
                    .link_connected = {disconnected},
                };
                auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
                CHECK(topo_node_mapping.n_topo_nodes() == 3);
                REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
                REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
                REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
                CHECK(topo_node_mapping.mapping() == IdxVector{0, 1, 2});
            }
        }
        SUBCASE("Multiple links => all connected nodes are remapped to the same topological node") {
            ComponentTopology const comp_topo{
                .n_node = 6,
                .link_node_idx = {{2, 1}, {5, 3}, {5, 2}},
            };
            ComponentConnections const comp_conn{
                .link_connected = {{1, 1}, {1, 1}, {1, 1}},
            };
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 3);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 1, 1, 1, 2, 1});
        }
        SUBCASE("Multiple connected components map to different topological nodes") {
            ComponentTopology const comp_topo{
                .n_node = 6,
                .link_node_idx = {{0, 1}, {2, 4}, {3, 5}},
            };
            ComponentConnections const comp_conn{
                .link_connected = {{1, 1}, {1, 1}, {1, 1}},
            };
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 3);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 0, 1, 2, 1, 2});
        }
    }
    SUBCASE("create_topological_nodes") {
        // TODO: Add test implementation
    }
    SUBCASE("construct_reduced_topology") {
        // TODO: Add test implementation
    }
    SUBCASE("reduce_topology") {
        // TODO: Add test implementation
    }
}
} // namespace
