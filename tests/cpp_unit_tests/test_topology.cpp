// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <ostream>

#include "doctest/doctest.h"
#include "power_grid_model/topology.hpp"

/*
 *  [0]   = Node
 * --0--> = Branch (from --id--> to)
 * -(b0)- = Virtual node, representing Branch3,
 *          the sequence within the branch 3 is indicated by $0 $1 $2
 *  -X-   = Open switch / not connected
 *  s0    = source
 *  s0X   = disconnected source
 *  lg0   = load_gen
 *  h0    = shunt
 *  v0    = voltage sensor
 *  +p0   = power sensor
 *  +p?0  = components power sensor (e.g. +ps1 = source power sensor 1)
 *          (f = branch_from, t = branch_to, s = source, h = shunt, l = load, g = generator)
 *
 * Topology:
 *
 *                                              7 -> [5:s1+p1+p12,lg2+p4+p8,v4]     [6:h1+p5+p9] -X-4-> [7] -3-> [8,v5]
 *      0 ----->+p13 [1:lg3+p7,v1]             /     /                       \        /                  \        /
 *     /          +p14\            5 ---X--- [4] <- 6                         $2     $1                   $1     $2
 *    /                $0         /          ^                                 \    /                      \    /
 *   /                  \        v          /                                   (b2)                        (b1)
 *  [0:s0,lg0]         (b0)-$2- [2:v0,v2]  /                                     |                           |
 *   +p0+p11            /    +p15         X                                      $0                          $0
 *    \                $1                /         [9:s2X+p3,h2]                 X                           |
 *     \          +p16/                 /                                       [10]                        [11:lg1+p6]
 *      1 -->+p2+p10 [3:s3X,h0,v3] -- 2
 *
 *
 * Math model #0:                       Math model #1:
 *
 *      0 ----->+pt0 [2:lg0+pg0,v3]             1 -> [3:s0+ps0+ps1,lg0+pl0+pl1,v0] [0:h1+ps0+ps1]
 *     /          +pf2\             3 --X      /       /                     \       /
 *    /                4           /         [2] <- 0                         3     4
 *   /                  v         v                                            v   v
 *  [4:s0,lg1]          [3] <-6- [0:v0,v1]                                      [1]
 *   +pf0+pf1           ^     +pf4                                               ^
 *    \                5                                                         2
 *     \          +pf3/                                                          X
 *      1 ->+pt1+pt2 [1:h0,v2] -- 2 --X
 *
 * Extra fill-in:
 * (3, 4)  by removing node 1
 *
 *
 * Topology for cycle reodering
 *
 *
 *   [5]  <---4--[4] <--3- [3]
 *    ^ \         ^       /  ^
 *    |   9----   |     /    |
 *    5        \  6   10     2
 *    |         v |  v       |
 * [0:s0] --0--> [1] --1--> [2]
 *    ^        ^    <- 12-   ^
 *    |   -11-/     parallel |
 *    7  /                   |
 *    | /                    |
 *   [6] -----------------8--
 *
 * Math model after reodering
 *
 *   [1]  <---4--[6] <--3- [2]
 *    ^ \         ^       /  ^
 *    |   9----   |     /    |
 *    5        \  6   10     2
 *    |         v |  v       |
 * [3:s0] --0--> [5] --1--> [4]
 *    ^        ^    <- 12-   ^
 *    |   -11-/     parallel |
 *    7  /                   |
 *    | /                    |
 *   [0] -----------------8--
 *
 * Extra fill-in:
 * (3, 4)  by removing node 0
 * (3, 6)  by removing node 1
 * (4, 6)  by removing node 2
 */

namespace power_grid_model {

// define operators
inline bool operator==(Idx2DBranch3 x, Idx2DBranch3 y) {
    return x.group == y.group && x.pos == y.pos;
}

std::ostream& operator<<(std::ostream& s, Idx2D const& idx) {
    s << "(" << idx.group << ", " << idx.pos << ")";
    return s;
}

TEST_CASE("Test topology") {
    // component topology
    ComponentTopology comp_topo{};
    comp_topo.n_node = 12;

    comp_topo.branch_node_idx = {
        {0, 1},  // 0
        {0, 3},  // 1
        {3, 4},  // 2
        {7, 8},  // 3
        {6, 7},  // 4
        {4, 2},  // 5
        {5, 4},  // 6
        {4, 5}   // 7
    };
    comp_topo.branch3_node_idx = {
        {1, 3, 2},   // b0
        {11, 7, 8},  // b1
        {10, 6, 5}   // b2
    };
    comp_topo.source_node_idx = {0, 5, 9, 3};
    comp_topo.load_gen_node_idx = {0, 11, 5, 1};
    comp_topo.load_gen_type = {LoadGenType::const_pq, LoadGenType::const_pq, LoadGenType::const_i,
                               LoadGenType::const_y};
    comp_topo.shunt_node_idx = {3, 6, 9};
    comp_topo.voltage_sensor_node_idx = {2, 1, 2, 3, 5, 8};
    comp_topo.power_sensor_object_idx = {1, 1, 1, 2, 2, 1, 1, 3, 2, 1, 1, 1, 1, 0, 0, 0, 0};
    comp_topo.power_sensor_terminal_type = {
        MeasuredTerminalType::branch_from,  // 0 (branch   1)
        MeasuredTerminalType::source,       // 1 (source   1)
        MeasuredTerminalType::branch_to,    // 2 (branch   1)
        MeasuredTerminalType::source,       // 3 (source   2)
        MeasuredTerminalType::load,         // 4 (load_gen 2)
        MeasuredTerminalType::shunt,        // 5 (shunt    1)
        MeasuredTerminalType::load,         // 6 (load_gen 1)
        MeasuredTerminalType::generator,    // 7 (load_gen 3)
        MeasuredTerminalType::load,         // 8 (load_gen 2)
        MeasuredTerminalType::shunt,        // 9 (shunt    1)
        MeasuredTerminalType::branch_to,    // 10 (branch   1)
        MeasuredTerminalType::branch_from,  // 11 (branch   1)
        MeasuredTerminalType::source,       // 12 (source   1)
        MeasuredTerminalType::branch_to,    // 13 (branch   0)
        MeasuredTerminalType::branch3_1,    // 14 (branch3  0)
        MeasuredTerminalType::branch3_3,    // 15 (branch3  0)
        MeasuredTerminalType::branch3_2,    // 16 (branch3  0)
    };

    // component connection
    ComponentConnections comp_conn{};
    comp_conn.branch_connected = {
        // {from, to}
        {1, 1},  // 0
        {1, 1},  // 1
        {1, 0},  // 2
        {1, 1},  // 3
        {0, 1},  // 4
        {0, 1},  // 5
        {1, 1},  // 6
        {1, 1},  // 7
    };
    comp_conn.branch3_connected = {
        {1, 1, 1},  // b0
        {1, 1, 1},  // b1
        {0, 1, 1},  // b2
    };
    comp_conn.branch_phase_shift = {0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    comp_conn.branch3_phase_shift = {
        {0.0, -1.0, 0.0},
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
    };
    comp_conn.source_connected = {1, 1, 0, 0};

    // result
    ComponentToMathCoupling comp_coup_ref{};
    comp_coup_ref.node = {         // 0 1 2 3
                          {0, 4},  // Topological node 0 has become node 4 in mathematical model (group) 0
                          {0, 2},
                          {0, 0},
                          {0, 1},
                          // 4 5 6
                          {1, 2},  // Topological node 4 has become node 2 in mathematical model (group) 1
                          {1, 3},
                          {1, 0},
                          // 7, 8, 9, 10, 11
                          {-1, -1},  // Topological node 7 is not included in the mathematical model, because it was not
                                     // connected to any power source
                          {-1, -1},
                          {-1, -1},
                          {-1, -1},
                          {-1, -1},
                          // b0, b1, b2
                          {0, 3},  // Branch3 b0 is replaced by a virtual node 3, in mathematical model 0
                          {-1, -1},
                          {1, 1}};
    comp_coup_ref.source = {
        {0, 0},    // 0
        {1, 0},    // 1
        {-1, -1},  // 2
        {-1, -1},  // 3
    };
    comp_coup_ref.branch = {
        {0, 0},    // 0
        {0, 1},    // 1
        {0, 2},    // 2
        {-1, -1},  // 3
        {-1, -1},  // 4
        {0, 3},    // 5
        {1, 0},    // 6
        {1, 1},    // 7
    };
    comp_coup_ref.branch3 = {
        {0, {4, 5, 6}},      // b0
        {-1, {-1, -1, -1}},  // b1
        {1, {2, 3, 4}},      // b2
    };
    comp_coup_ref.load_gen = {{0, 1}, {-1, -1}, {1, 0}, {0, 0}};
    comp_coup_ref.shunt = {{0, 0}, {1, 0}, {-1, -1}};
    comp_coup_ref.voltage_sensor = {{0, 0}, {0, 3}, {0, 1}, {0, 2}, {1, 0}, {-1, -1}};
    comp_coup_ref.power_sensor = {
        {0, 0},    // 0 branch_from
        {1, 0},    // 1 source
        {0, 1},    // 2 branch_to
        {-1, -1},  // 3 source
        {1, 0},    // 4 load       = load power sensor 0 in math model 1
        {1, 0},    // 5 shunt      = shunt power sensor 0 in math model 1
        {-1, -1},  // 6 load
        {0, 0},    // 7 generator
        {1, 1},    // 8 load
        {1, 1},    // 9 shunt
        {0, 2},    // 10 branch_to
        {0, 1},    // 11 branch_from
        {1, 1},    // 12 source
        {0, 0},    // 13 branch_to
        {0, 2},    // 14 branch_from
        {0, 4},    // 15 branch_from
        {0, 3}     // 16 branch_from
    };

    // Sub graph / math model 0
    MathModelTopology math0;
    math0.slack_bus_ = 4;
    math0.source_bus_indptr = {0, 0, 0, 0, 0, 1};
    math0.branch_bus_idx = {{4, 2}, {4, 1}, {1, -1}, {-1, 0}, {2, 3}, {1, 3}, {0, 3}};
    math0.phase_shift = {0.0, -1.0, 0.0, 0.0, 0.0};
    math0.load_gen_bus_indptr = {0, 0, 0, 1, 1, 2};
    math0.load_gen_type = {LoadGenType::const_y, LoadGenType::const_pq};
    math0.shunt_bus_indptr = {0, 0, 1, 1, 1, 1};
    math0.voltage_sensor_indptr = {0, 2, 3, 4, 4, 4};
    math0.source_power_sensor_indptr = {0, 0};
    math0.shunt_power_sensor_indptr = {0, 0};
    math0.load_gen_power_sensor_indptr = {0, 1, 1};
    math0.branch_from_power_sensor_indptr = {0, 0, 2, 2, 2, 3, 4, 5};
    // 7 branches, 3 branch-to power sensors
    // sensor 0 is connected to branch 0
    // sensor 1 and 2 are connected to branch 1
    math0.branch_to_power_sensor_indptr = {0, 1, 3, 3, 3, 3, 3, 3};
    math0.fill_in = {{3, 4}};

    // Sub graph / math model 1
    MathModelTopology math1;
    math1.slack_bus_ = 3;
    math1.source_bus_indptr = {0, 0, 0, 0, 1};
    math1.branch_bus_idx = {
        {3, 2}, {2, 3}, {-1, 1}, {0, 1}, {3, 1},
    };
    math1.phase_shift = {0, 0, 0, 0};
    math1.load_gen_bus_indptr = {0, 0, 0, 0, 1};
    math1.load_gen_type = {LoadGenType::const_i};
    math1.shunt_bus_indptr = {0, 1, 1, 1, 1};
    math1.voltage_sensor_indptr = {0, 0, 0, 0, 1};
    math1.source_power_sensor_indptr = {0, 2};
    math1.shunt_power_sensor_indptr = {0, 2};
    math1.load_gen_power_sensor_indptr = {0, 2};
    math1.branch_from_power_sensor_indptr = {0, 0, 0, 0, 0, 0};
    math1.branch_to_power_sensor_indptr = {0, 0, 0, 0, 0, 0};

    std::vector<MathModelTopology> math_topology_ref = {math0, math1};

    SUBCASE("Test topology result") {
        Topology topo{comp_topo, comp_conn};
        auto pair = topo.build_topology();
        auto const& math_topology = pair.first;
        auto const& comp_coup = *pair.second;

        CHECK(math_topology.size() == 2);
        // test component coupling
        CHECK(comp_coup.node == comp_coup_ref.node);
        CHECK(comp_coup.source == comp_coup_ref.source);
        CHECK(comp_coup.branch == comp_coup_ref.branch);
        CHECK(comp_coup.branch3 == comp_coup_ref.branch3);
        CHECK(comp_coup.load_gen == comp_coup_ref.load_gen);
        CHECK(comp_coup.voltage_sensor == comp_coup_ref.voltage_sensor);
        CHECK(comp_coup.power_sensor == comp_coup_ref.power_sensor);

        for (size_t i = 0; i < math_topology.size(); i++) {
            auto const& math = *math_topology[i];
            auto const& math_ref = math_topology_ref[i];
            CHECK(math.slack_bus_ == math_ref.slack_bus_);
            CHECK(math.n_bus() == math_ref.n_bus());
            CHECK(math.source_bus_indptr == math_ref.source_bus_indptr);
            CHECK(math.branch_bus_idx == math_ref.branch_bus_idx);
            CHECK(math.phase_shift == math_ref.phase_shift);
            CHECK(math.load_gen_bus_indptr == math_ref.load_gen_bus_indptr);
            CHECK(math.load_gen_type == math_ref.load_gen_type);
            CHECK(math.shunt_bus_indptr == math_ref.shunt_bus_indptr);
            CHECK(math.voltage_sensor_indptr == math_ref.voltage_sensor_indptr);
            CHECK(math.source_power_sensor_indptr == math_ref.source_power_sensor_indptr);
            CHECK(math.shunt_power_sensor_indptr == math_ref.shunt_power_sensor_indptr);
            CHECK(math.load_gen_power_sensor_indptr == math_ref.load_gen_power_sensor_indptr);
            CHECK(math.branch_from_power_sensor_indptr == math_ref.branch_from_power_sensor_indptr);
            CHECK(math.branch_to_power_sensor_indptr == math_ref.branch_to_power_sensor_indptr);
            CHECK(math.fill_in == math_ref.fill_in);
        }
    }
}

TEST_CASE("Test cycle reorder") {
    // component topology
    ComponentTopology comp_topo{};
    comp_topo.n_node = 7;
    comp_topo.branch_node_idx = {
        {0, 1},  // 0
        {1, 2},  // 1
        {2, 3},  // 2
        {3, 4},  // 3
        {4, 5},  // 4
        {0, 5},  // 5
        {1, 4},  // 6
        {6, 0},  // 7
        {6, 2},  // 8
        {5, 1},  // 9
        {3, 1},  // 10
        {6, 1},  // 11
        {2, 1},  // 12
    };
    comp_topo.source_node_idx = {0};
    // component connection
    ComponentConnections comp_conn{};
    comp_conn.branch_connected = std::vector<BranchConnected>(13, {1, 1});
    comp_conn.branch_phase_shift = std::vector<double>(13, 0.0);
    comp_conn.source_connected = {1};
    // result
    ComponentToMathCoupling comp_coup_ref{};
    comp_coup_ref.node = {{0, 3}, {0, 5}, {0, 4}, {0, 2}, {0, 6}, {0, 1}, {0, 0}};
    std::vector<BranchIdx> const fill_in_ref{{3, 4}, {3, 6}, {4, 6}};

    Topology topo{comp_topo, comp_conn};
    auto pair = topo.build_topology();
    auto const& comp_coup = *pair.second;
    auto const& math_topo = *pair.first[0];
    CHECK(comp_coup.node == comp_coup_ref.node);
    CHECK(math_topo.fill_in == fill_in_ref);
}

}  // namespace power_grid_model