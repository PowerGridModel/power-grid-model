// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/calculation_parameters.hpp"
#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/enum.hpp"
#include "power_grid_model/supernodes.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

namespace {

using namespace power_grid_model;
using namespace power_grid_model::supernodes;
namespace detail = power_grid_model::supernodes::detail;

TEST_CASE("Test Supernodes") {
    SUBCASE("create_map") {
        SUBCASE("No links => no remapping") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.branch_node_idx = {{0, 1}, {1, 2}, {2, 0}};
            ComponentConnections comp_conn;
            comp_conn.branch_connected = {{1, 1}, {1, 1}, {1, 1}};
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 3);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 1, 2});
        }
        SUBCASE("One link between node 0 and 1 => node 0 and 1 are remapped to the same topological node") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.branch_node_idx = {{0, 1}, {1, 2}, {2, 0}};
            comp_topo.link_node_idx = {{0, 1}};
            ComponentConnections comp_conn;
            comp_conn.branch_connected = {{1, 1}, {1, 1}, {1, 1}};
            comp_conn.link_connected = {{1, 1}};
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 2);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 0, 1});
        }
        SUBCASE("Disconnected link => not remapped") {
            for (auto const disconnection :
                 std::array{BranchConnected{1, 0}, BranchConnected{0, 1}, BranchConnected{0, 0}}) {
                ComponentTopology comp_topo;
                comp_topo.n_node = 3;
                comp_topo.branch_node_idx = {{0, 1}, {1, 2}, {2, 0}};
                comp_topo.link_node_idx = {{0, 1}};
                ComponentConnections comp_conn;
                comp_conn.branch_connected = {{1, 1}, {1, 1}, {1, 1}};
                comp_conn.link_connected = {disconnection};
                auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
                CHECK(topo_node_mapping.n_topo_nodes() == 3);
                REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
                REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
                REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
                CHECK(topo_node_mapping.mapping() == IdxVector{0, 1, 2});
            }
        }
        SUBCASE("Multiple links => all connected nodes are remapped to the same topological node") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 6;
            comp_topo.link_node_idx = {{2, 1}, {5, 3}, {5, 2}};
            ComponentConnections comp_conn;
            comp_conn.link_connected = {{1, 1}, {1, 1}, {1, 1}};
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 3);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 1, 1, 1, 2, 1});
        }
        SUBCASE("Multiple connected components map to different topological nodes") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 6;
            comp_topo.link_node_idx = {{0, 1}, {2, 4}, {3, 5}};
            ComponentConnections comp_conn;
            comp_conn.link_connected = {{1, 1}, {1, 1}, {1, 1}};
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            CHECK(topo_node_mapping.n_topo_nodes() == 3);
            REQUIRE(std::ranges::max(topo_node_mapping.mapping()) + 1 == topo_node_mapping.n_topo_nodes());
            REQUIRE(topo_node_mapping.n_user_nodes() == topo_node_mapping.mapping().size());
            REQUIRE(topo_node_mapping.n_user_nodes() == comp_topo.n_node);
            CHECK(topo_node_mapping.mapping() == IdxVector{0, 0, 1, 2, 1, 2});
        }
    }
    SUBCASE("create_topological_nodes") {
        SUBCASE("Multiple connected components map to different topological nodes") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 6;
            comp_topo.link_node_idx = {{0, 1}, {2, 4}, {3, 5}, {5, 3}, {3, 5}};
            ComponentConnections comp_conn;
            comp_conn.link_connected = {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}};
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            auto const topo_nodes = detail::create_topological_nodes(comp_topo, comp_conn, topo_node_mapping);

            REQUIRE(std::ssize(topo_nodes.topo_nodes) == topo_node_mapping.n_topo_nodes());
            CHECK(std::ranges::all_of(topo_nodes.topo_nodes,
                                      [](TopologicalNode const& node) { return node.is_supernode(); }));

            CHECK(topo_nodes.topo_nodes[0].user_nodes == IdxVector{0, 1});
            CHECK(topo_nodes.topo_nodes[1].user_nodes == IdxVector{2, 4});
            CHECK(topo_nodes.topo_nodes[2].user_nodes == IdxVector{3, 5});
            CHECK(topo_nodes.topo_nodes[0].user_links == std::vector<BranchIdx>{{0, 1}});
            CHECK(topo_nodes.topo_nodes[1].user_links == std::vector<BranchIdx>{{2, 4}});
            CHECK(topo_nodes.topo_nodes[2].user_links == std::vector<BranchIdx>{{3, 5}, {5, 3}, {3, 5}});

            CHECK(std::ranges::equal(topo_nodes.coupling.user_links_to_topo_nodes,
                                     std::vector<Idx2D>{{.group = 0, .pos = 0},
                                                        {.group = 1, .pos = 0},
                                                        {.group = 2, .pos = 0},
                                                        {.group = 2, .pos = 1},
                                                        {.group = 2, .pos = 2}}));
        }
        SUBCASE("Disconnected link => included in user link on connected side, abandoned if fully disconnected") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 6;
            comp_topo.link_node_idx = {{0, 1}, {2, 4}, {3, 5}, {0, 1}};
            ComponentConnections comp_conn;
            comp_conn.link_connected = {{1, 0}, {1, 1}, {0, 0}, {0, 1}};
            auto const topo_node_mapping = detail::create_map(comp_topo, comp_conn);
            auto const topo_nodes = detail::create_topological_nodes(comp_topo, comp_conn, topo_node_mapping);

            REQUIRE(std::ssize(topo_nodes.topo_nodes) == topo_node_mapping.n_topo_nodes());
            CHECK(std::ranges::equal(topo_nodes.topo_nodes | std::views::transform([](TopologicalNode const& node) {
                                         return node.is_supernode();
                                     }),
                                     std::array{false, false, true, false, false}));

            CHECK(std::ranges::equal(
                topo_nodes.topo_nodes |
                    std::views::transform([](TopologicalNode const& node) -> auto& { return node.user_nodes; }),
                std::vector<IdxVector>{{0}, {1}, {2, 4}, {3}, {5}}));
            CHECK(std::ranges::equal(
                topo_nodes.topo_nodes |
                    std::views::transform([](TopologicalNode const& node) -> auto& { return node.user_links; }),
                std::vector<std::vector<BranchIdx>>{{{0, disconnected}}, {{disconnected, 1}}, {{2, 4}}, {}, {}}));

            CHECK(std::ranges::equal(topo_nodes.coupling.user_links_to_topo_nodes,
                                     std::vector<Idx2D>{{.group = 0, .pos = 0},
                                                        {.group = 2, .pos = 0},
                                                        {.group = disconnected, .pos = disconnected},
                                                        {.group = 1, .pos = 0}}));
        }
    }
    SUBCASE("construct_reduced_topology") {
        SUBCASE("Single node => no remapping, no supernodes") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 1;
            auto const topo_node_mapping = detail::create_map(comp_topo, ComponentConnections{});
            auto const reduced = detail::construct_reduced_topology(comp_topo, topo_node_mapping);
            CHECK(reduced.n_node == 1);
            CHECK(reduced.branch_node_idx.empty());
        }
        SUBCASE("No links => identity mapping, no remapping") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.branch_node_idx = {{0, 1}, {1, 2}, {2, 0}};
            TopologicalNodeMapping const topo_node_mapping{3, IdxVector{0, 1, 2}};
            auto const reduced = detail::construct_reduced_topology(comp_topo, topo_node_mapping);
            CHECK(reduced.n_node == 3);
            CHECK(reduced.branch_node_idx == std::vector<BranchIdx>{{0, 1}, {1, 2}, {2, 0}});
        }
        SUBCASE("Only links => one supernode") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.link_node_idx = {{0, 1}, {1, 2}, {2, 0}};
            TopologicalNodeMapping const topo_node_mapping{1, IdxVector{0, 0, 0}};
            auto const reduced = detail::construct_reduced_topology(comp_topo, topo_node_mapping);
            CHECK(reduced.n_node == 1);
            CHECK(reduced.branch_node_idx.empty());
        }
        SUBCASE("Link merging => branch, shunt and source nodes remapped") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.branch_node_idx = {{0, 2}};
            comp_topo.link_node_idx = {{0, 1}};
            comp_topo.shunt_node_idx = {2};
            comp_topo.source_node_idx = {1};
            // node 0 and 1 are merged into topo node 0, node 2 becomes topo node 1
            TopologicalNodeMapping const topo_node_mapping{2, IdxVector{0, 0, 1}};
            auto const reduced = detail::construct_reduced_topology(comp_topo, topo_node_mapping);
            CHECK(reduced.n_node == 2);
            CHECK(reduced.branch_node_idx == std::vector<BranchIdx>{{0, 1}});
            CHECK(reduced.shunt_node_idx == IdxVector{1});
            CHECK(reduced.source_node_idx == IdxVector{0});
        }
        SUBCASE("Multiple links merging => all node index vectors remapped") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 6;
            comp_topo.branch_node_idx = {{0, 4}};
            comp_topo.link_node_idx = {{2, 1}, {5, 3}, {5, 2}};
            comp_topo.shunt_node_idx = {3};
            comp_topo.source_node_idx = {0};
            comp_topo.load_gen_node_idx = {4};
            comp_topo.voltage_sensor_node_idx = {1};
            // nodes 1, 2, 3, 5 are merged into topo node 1; node 0 => topo 0; node 4 => topo 2
            TopologicalNodeMapping const topo_node_mapping{3, IdxVector{0, 1, 1, 1, 2, 1}};
            auto const reduced = detail::construct_reduced_topology(comp_topo, topo_node_mapping);
            CHECK(reduced.n_node == 3);
            CHECK(reduced.branch_node_idx == std::vector<BranchIdx>{{0, 2}});
            CHECK(reduced.source_node_idx == IdxVector{0});
            CHECK(reduced.shunt_node_idx == IdxVector{1});
            CHECK(reduced.load_gen_node_idx == IdxVector{2});
            CHECK(reduced.voltage_sensor_node_idx == IdxVector{1});
        }
        SUBCASE("Complicated wheel grid with branches and links") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 6;
            comp_topo.branch_node_idx = {{0, 1}, {2, 3}, {5, 1}, {5, 3}};
            comp_topo.link_node_idx = {{1, 2}, {3, 4}, {5, 0}, {5, 2}};
            comp_topo.source_node_idx = {0};
            comp_topo.shunt_node_idx = {4};
            comp_topo.load_gen_node_idx = {3};
            comp_topo.voltage_sensor_node_idx = {2};
            comp_topo.power_sensor_object_idx = {1};
            comp_topo.power_sensor_terminal_type = {MeasuredTerminalType::branch_from};
            // all nodes are connected by links and should be merged into a single supernode
            TopologicalNodeMapping const topo_node_mapping{1, IdxVector{0, 0, 0, 0, 0, 0}};
            auto const reduced = detail::construct_reduced_topology(comp_topo, topo_node_mapping);
            CHECK(reduced.n_node == 1);
            CHECK(reduced.branch_node_idx == std::vector<BranchIdx>{{0, 0}, {0, 0}, {0, 0}, {0, 0}});
            CHECK(reduced.source_node_idx == IdxVector{0});
            CHECK(reduced.shunt_node_idx == IdxVector{0});
            CHECK(reduced.load_gen_node_idx == IdxVector{0});
            CHECK(reduced.voltage_sensor_node_idx == IdxVector{0});
            CHECK(std::ranges::equal(reduced.power_sensor_object_idx, IdxVector{1}));
            CHECK(std::ranges::equal(reduced.power_sensor_terminal_type,
                                     std::vector<MeasuredTerminalType>{MeasuredTerminalType::branch_from}));
        }
    }
    SUBCASE("reduce_topology") {
        SUBCASE("No links => no remapping, no supernodes") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.branch_node_idx = {{0, 1}, {1, 2}};
            comp_topo.source_node_idx = {0};
            ComponentConnections comp_conn;
            comp_conn.branch_connected = {{1, 1}, {1, 1}};
            auto const result = reduce_topology(comp_topo, comp_conn);
            CHECK(result.reduced_comp_topo.n_node == 3);
            CHECK(result.reduced_comp_topo.branch_node_idx == std::vector<BranchIdx>{{0, 1}, {1, 2}});
            CHECK(result.reduced_comp_topo.source_node_idx == IdxVector{0});
            REQUIRE(std::ssize(result.topo_node_coup.topo_nodes) == 3);
            CHECK(std::ranges::none_of(result.topo_node_coup.topo_nodes,
                                       [](TopologicalNode const& node) { return node.is_supernode(); }));
            CHECK(result.topo_node_coup.coupling.n_topo_nodes == 3);
        }
        SUBCASE("One link => supernode created, nodes and branches remapped") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.branch_node_idx = {{0, 2}};
            comp_topo.link_node_idx = {{0, 1}};
            comp_topo.shunt_node_idx = {2};
            comp_topo.source_node_idx = {0};
            ComponentConnections comp_conn;
            comp_conn.branch_connected = {{1, 1}};
            comp_conn.link_connected = {{1, 1}};
            auto const result = reduce_topology(comp_topo, comp_conn);
            CHECK(result.reduced_comp_topo.n_node == 2);
            CHECK(result.reduced_comp_topo.branch_node_idx == std::vector<BranchIdx>{{0, 1}});
            CHECK(result.reduced_comp_topo.source_node_idx == IdxVector{0});
            CHECK(result.reduced_comp_topo.shunt_node_idx == IdxVector{1});
            REQUIRE(std::ssize(result.topo_node_coup.topo_nodes) == 2);
            CHECK(result.topo_node_coup.topo_nodes[0].is_supernode());
            CHECK(!result.topo_node_coup.topo_nodes[1].is_supernode());
            CHECK(result.topo_node_coup.topo_nodes[0].user_nodes == IdxVector{0, 1});
            CHECK(result.topo_node_coup.topo_nodes[1].user_nodes == IdxVector{2});
            CHECK(result.topo_node_coup.coupling.n_topo_nodes == 2);
        }
        SUBCASE("Disconnected link => not merged, no supernode") {
            ComponentTopology comp_topo;
            comp_topo.n_node = 3;
            comp_topo.branch_node_idx = {{0, 2}};
            comp_topo.link_node_idx = {{0, 1}};
            ComponentConnections comp_conn;
            comp_conn.branch_connected = {{1, 1}};
            comp_conn.link_connected = {{1, 0}};
            auto const result = reduce_topology(comp_topo, comp_conn);
            CHECK(result.reduced_comp_topo.n_node == 3);
            CHECK(result.reduced_comp_topo.branch_node_idx == std::vector<BranchIdx>{{0, 2}});
            REQUIRE(std::ssize(result.topo_node_coup.topo_nodes) == 3);
            CHECK(std::ranges::none_of(result.topo_node_coup.topo_nodes,
                                       [](TopologicalNode const& node) { return node.is_supernode(); }));
        }
    }
}
} // namespace
