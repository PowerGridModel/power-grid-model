// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_TOPOLOGY_HPP
#define POWER_GRID_MODEL_TOPOLOGY_HPP

#include "calculation_parameters.hpp"
#include "enum.hpp"
#include "exception.hpp"
#include "power_grid_model.hpp"
#include "sparse_mapping.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/minimum_degree_ordering.hpp>

// build topology of the grid
// divide grid into several math models
// start search from a source
// using DFS search

namespace power_grid_model {

class Topology {
    using GraphIdx = size_t;

    struct GlobalEdge {
        double phase_shift;
    };

    struct GlobalVertex {
        boost::default_color_type color;
    };

    // sparse directed graph
    // edge i -> j, the phase shift is node_j - node_i
    // so to move forward from i to j, the phase shift is appended by value at (i, j)
    // for 3-way branch, the internal node is appended at the end one by one
    // n_node + k, k as branch3 sequence number
    // branch3 #0, has internal node idx n_node
    // branch3 #1, has internal node idx n_node + 1
    using GlobalGraph = boost::compressed_sparse_row_graph<boost::directedS, GlobalVertex, GlobalEdge,
                                                           boost::no_property, GraphIdx, GraphIdx>;

    using ReorderGraph = boost::adjacency_list<boost::vecS, // vector as adjacency
                                               boost::vecS, // vector for vertices
                                               boost::directedS>;

    // dfs visitor for global graph
    class GlobalDFSVisitor : public boost::dfs_visitor<> {
      public:
        GlobalDFSVisitor(Idx math_group, std::vector<Idx2D>& node_coupling, std::vector<double>& phase_shift,
                         std::vector<Idx>& dfs_node, std::vector<GraphIdx>& predecessors,
                         std::vector<std::pair<GraphIdx, GraphIdx>>& back_edges)
            : math_group_{math_group},
              node_coupling_{node_coupling},
              phase_shift_{phase_shift},
              dfs_node_{dfs_node},
              predecessors_{predecessors},
              back_edges_{back_edges} {}

        // accumulate phase shift
        // assign predecessor
        void tree_edge(GlobalGraph::edge_descriptor e, GlobalGraph const& g) {
            GraphIdx const source = boost::source(e, g);
            GraphIdx const target = boost::target(e, g);
            phase_shift_[target] = phase_shift_[source] + g[e].phase_shift;
            predecessors_[target] = source;
        }

        // forward_or_cross_edge
        // in symmetric directed graph (equivalent to undirected)
        //    forward edge is ignored
        //    cross edge does not exist

        // back edge, judge if it forms a cycle
        void back_edge(GlobalGraph::edge_descriptor e, GlobalGraph const& g) {
            GraphIdx const source = boost::source(e, g);
            GraphIdx const target = boost::target(e, g);
            // if this edge matches in the current tree as target->source
            // it does not form a cycle, but an anti-parallel edge
            // else it forms a cycle
            if (predecessors_[source] != target) {
                back_edges_.emplace_back(source, target);
            }
        }

        // assign node to math group
        // append node to dfs list
        void discover_vertex(GlobalGraph::vertex_descriptor u, GlobalGraph const& /* unused_value */) {
            node_coupling_[u].group = static_cast<Idx>(math_group_);
            dfs_node_.push_back(static_cast<Idx>(u));
        }

      private:
        Idx math_group_;
        std::vector<Idx2D>& node_coupling_;
        std::vector<double>& phase_shift_;
        std::vector<Idx>& dfs_node_;
        std::vector<GraphIdx>& predecessors_;
        std::vector<std::pair<GraphIdx, GraphIdx>>& back_edges_;
    };

  public:
    Topology(ComponentTopology const& comp_topo, ComponentConnections const& comp_conn)
        : comp_topo_{comp_topo},
          comp_conn_{comp_conn},
          phase_shift_(comp_topo_.n_node_total(), 0.0),
          predecessors_(
              boost::counting_iterator<GraphIdx>{0}, // Predecessors is initialized as 0, 1, 2, ..., n_node_total() - 1
              boost::counting_iterator<GraphIdx>{(GraphIdx)comp_topo_.n_node_total()}),
          node_status_(comp_topo_.n_node_total(), -1) {}

    // build topology
    std::pair<std::vector<std::shared_ptr<MathModelTopology const>>,
              std::shared_ptr<TopologicalComponentToMathCoupling const>>
    build_topology() {
        reset_topology();
        build_sparse_graph();
        dfs_search();
        couple_branch();
        couple_all_appliance();
        couple_sensors();
        // create return pair with shared pointer
        std::pair<std::vector<std::shared_ptr<MathModelTopology const>>,
                  std::shared_ptr<TopologicalComponentToMathCoupling const>>
            pair;
        for (Idx k = 0; k != static_cast<Idx>(math_topology_.size()); ++k) {
            pair.first.emplace_back(std::make_shared<MathModelTopology const>(std::move(math_topology_[k])));
        }
        pair.second = std::make_shared<TopologicalComponentToMathCoupling const>(std::move(comp_coup_));
        return pair;
    }

  private:
    // input
    ComponentTopology const& comp_topo_;
    ComponentConnections const& comp_conn_;
    // intermediate
    GlobalGraph global_graph_;
    DoubleVector phase_shift_;
    std::vector<GraphIdx> predecessors_;
    // node status
    // -1, node not processed, assuming that node in the far end of a tree structure
    // -2, node in cycles or between the source and cycles, reordering not yet happened
    // >=0, temporary internal bus number for minimum degree reordering
    std::vector<Idx> node_status_;
    // output
    std::vector<MathModelTopology> math_topology_;
    TopologicalComponentToMathCoupling comp_coup_;

    void reset_topology() {
        comp_coup_.node.resize(comp_topo_.n_node_total(), Idx2D{-1, -1});
        comp_coup_.branch.resize(comp_topo_.branch_node_idx.size(), Idx2D{-1, -1});
        comp_coup_.branch3.resize(comp_topo_.branch3_node_idx.size(), Idx2DBranch3{-1, {-1, -1, -1}});
        comp_coup_.shunt.resize(comp_topo_.shunt_node_idx.size(), Idx2D{-1, -1});
        comp_coup_.load_gen.resize(comp_topo_.load_gen_node_idx.size(), Idx2D{-1, -1});
        comp_coup_.source.resize(comp_topo_.source_node_idx.size(), Idx2D{-1, -1});
        comp_coup_.voltage_sensor.resize(comp_topo_.voltage_sensor_node_idx.size(), Idx2D{-1, -1});
        comp_coup_.power_sensor.resize(comp_topo_.power_sensor_object_idx.size(), Idx2D{-1, -1});
    }

    void build_sparse_graph() {
        std::vector<std::pair<GraphIdx, GraphIdx>> edges;
        std::vector<GlobalEdge> edge_props;
        // k as branch number for 2-way branch
        for (Idx k = 0; k != static_cast<Idx>(comp_topo_.branch_node_idx.size()); ++k) {
            auto const [i, j] = comp_topo_.branch_node_idx[k];
            auto const [i_status, j_status] = comp_conn_.branch_connected[k];
            // node_i - node_j
            double const phase_shift = comp_conn_.branch_phase_shift[k];
            if (i_status != 0 && j_status != 0) {
                // node_j - node_i
                edges.emplace_back((GraphIdx)i, (GraphIdx)j);
                edge_props.push_back({-phase_shift});
                // node_i - node_j
                edges.emplace_back((GraphIdx)j, (GraphIdx)i);
                edge_props.push_back({phase_shift});
            }
        }
        // k as branch number for 3-way branch
        for (Idx k = 0; k != static_cast<Idx>(comp_topo_.branch3_node_idx.size()); ++k) {
            auto const i = comp_topo_.branch3_node_idx[k];
            auto const i_status = comp_conn_.branch3_connected[k];
            // node_i - node_internal
            auto const& phase_shift = comp_conn_.branch3_phase_shift[k];
            // internal node number
            Idx const j_internal = comp_topo_.n_node + k;
            // loop 3 way as indices m
            for (Idx m = 0; m != 3; ++m) {
                if (i_status[m] != 0) {
                    // node_internal - node_i
                    edges.emplace_back((GraphIdx)i[m], (GraphIdx)j_internal);
                    edge_props.push_back({-phase_shift[m]});
                    // node_i - node_internal
                    edges.emplace_back((GraphIdx)j_internal, (GraphIdx)i[m]);
                    edge_props.push_back({phase_shift[m]});
                }
            }
        }
        // build graph
        global_graph_ = GlobalGraph{boost::edges_are_unsorted_multi_pass, edges.cbegin(), edges.cend(),
                                    edge_props.cbegin(), (GraphIdx)comp_topo_.n_node_total()};
        // set color
        BGL_FORALL_VERTICES(v, global_graph_, GlobalGraph) {
            global_graph_[v].color = boost::default_color_type::white_color;
        }
    }

    void dfs_search() {
        // m as math solver sequence number
        Idx math_solver_idx = 0;
        // loop all source as k
        for (Idx k = 0; k != static_cast<Idx>(comp_topo_.source_node_idx.size()); ++k) {
            // skip disconnected source
            if (static_cast<int>(comp_conn_.source_connected[k]) == 0) {
                continue;
            }
            Idx const source_node = comp_topo_.source_node_idx[k];
            // if the source node is already part of a graph
            if (comp_coup_.node[source_node].group != -1) {
                // skip the source
                continue;
            }
            // temporary vector to store the node in this dfs search
            std::vector<Idx> dfs_node;
            // back edges
            std::vector<std::pair<GraphIdx, GraphIdx>> back_edges;
            // start dfs search
            boost::depth_first_visit(
                global_graph_, (GraphIdx)source_node,
                GlobalDFSVisitor{math_solver_idx, comp_coup_.node, phase_shift_, dfs_node, predecessors_, back_edges},
                boost::get(&GlobalVertex::color, global_graph_));

            // begin to construct math topology
            MathModelTopology math_topo_single{};
            // reorder node number
            if (back_edges.empty()) {
                // no cycle, the graph is pure tree structure
                // just reverse the node
                std::reverse(dfs_node.begin(), dfs_node.end());
            } else {
                // with cycles, meshed graph
                // use minimum degree
                math_topo_single.fill_in = reorder_node(dfs_node, back_edges);
            }
            // initialize phase shift
            math_topo_single.phase_shift.resize(dfs_node.size());
            // i as bus number
            Idx i = 0;
            for (auto it = dfs_node.cbegin(); it != dfs_node.cend(); ++it, ++i) {
                Idx const current_node = *it;
                // assign node coupling
                comp_coup_.node[current_node].pos = i;
                // assign phase shift
                math_topo_single.phase_shift[i] = phase_shift_[current_node];
                assert(comp_coup_.node[current_node].group == math_solver_idx);
            }
            assert(i == math_topo_single.n_bus());
            // assign slack bus as the source node
            math_topo_single.slack_bus_ = comp_coup_.node[source_node].pos;
            math_topology_.emplace_back(std::move(math_topo_single));
            // iterate math model sequence number
            ++math_solver_idx;
            assert(math_solver_idx == static_cast<Idx>(math_topology_.size()));
        }
    }

    // re-order dfs_node using minimum degree
    // return list of fill-ins when factorize the matrix
    std::vector<BranchIdx> reorder_node(std::vector<Idx>& dfs_node,
                                        std::vector<std::pair<GraphIdx, GraphIdx>> const& back_edges) {
        std::vector<BranchIdx> fill_in;
        // make a copy and clear current vector
        std::vector<Idx> const dfs_node_copy(dfs_node);
        dfs_node.clear();

        // loop all back edges assign all nodes before the back edges as inside cycle
        for (auto const& back_edge : back_edges) {
            GraphIdx node_in_cycle = back_edge.first;
            // loop back from source in the predecessor tree
            // stop if it is already marked as in cycle
            while (node_status_[node_in_cycle] != -2) {
                // assign cycle status and go to predecessor
                node_status_[node_in_cycle] = -2;
                node_in_cycle = predecessors_[node_in_cycle];
            }
        }

        // copy all the far-end non-cyclic node, in reverse order
        std::copy_if(dfs_node_copy.crbegin(), dfs_node_copy.crend(), std::back_inserter(dfs_node),
                     [this](Idx x) { return node_status_[x] == -1; });
        // copy all cyclic node
        std::vector<Idx> cyclic_node;
        std::copy_if(dfs_node_copy.cbegin(), dfs_node_copy.cend(), std::back_inserter(cyclic_node),
                     [this](Idx x) { return node_status_[x] == -2; });
        GraphIdx const n_cycle_node = cyclic_node.size();
        // reorder does not make sense if number of cyclic nodes in a sub graph is smaller than 4
        if (n_cycle_node < 4) {
            std::copy(cyclic_node.crbegin(), cyclic_node.crend(), std::back_inserter(dfs_node));
            return fill_in;
        }

        // assign temporary bus number as increasing from 0, 1, 2, ..., n_cycle_node - 1
        for (GraphIdx i = 0; i != n_cycle_node; ++i) {
            node_status_[cyclic_node[i]] = static_cast<Idx>(i);
        }
        // build graph lambda
        auto const build_graph = [&](ReorderGraph& g) {
            // add edges
            for (GraphIdx i = 0; i != n_cycle_node; ++i) {
                // loop all edges of vertex i
                auto const global_i = static_cast<GraphIdx>(cyclic_node[i]);
                BGL_FORALL_ADJ(global_i, global_j, global_graph_, GlobalGraph) {
                    // skip if j is not part of cyclic sub graph
                    if (node_status_[global_j] == -1) {
                        continue;
                    }
                    auto const j = static_cast<GraphIdx>(node_status_[global_j]);
                    if (!boost::edge(i, j, g).second) {
                        boost::add_edge(i, j, g);
                    }
                }
            }
        };
        ReorderGraph meshed_graph{n_cycle_node};
        build_graph(meshed_graph);
        // start minimum degree ordering
        std::vector<std::make_signed_t<GraphIdx>> perm(n_cycle_node);
        std::vector<std::make_signed_t<GraphIdx>> inverse_perm(n_cycle_node);
        std::vector<std::make_signed_t<GraphIdx>> degree(n_cycle_node);
        std::vector<std::make_signed_t<GraphIdx>> supernode_sizes(n_cycle_node, 1);
        boost::vec_adj_list_vertex_id_map<boost::no_property, std::make_signed_t<GraphIdx>> const id{};
        int const delta = 0;
        boost::minimum_degree_ordering(meshed_graph, boost::make_iterator_property_map(degree.begin(), id),
                                       boost::make_iterator_property_map(inverse_perm.begin(), id),
                                       boost::make_iterator_property_map(perm.begin(), id),
                                       boost::make_iterator_property_map(supernode_sizes.begin(), id), delta, id);
        // re-order cyclic node
        std::vector<Idx> const cyclic_node_copy{cyclic_node};
        for (GraphIdx i = 0; i != n_cycle_node; ++i) {
            cyclic_node[i] = cyclic_node_copy[perm[i]];
        }
        // copy back to dfs node
        std::copy(cyclic_node.cbegin(), cyclic_node.cend(), std::back_inserter(dfs_node));

        // analyze and record fill-ins
        // re-assign temporary bus number as increasing from 0, 1, 2, ..., n_cycle_node - 1
        for (GraphIdx i = 0; i != n_cycle_node; ++i) {
            node_status_[cyclic_node[i]] = static_cast<Idx>(i);
        }
        // re-build graph with reordered cyclic node
        meshed_graph.clear();
        meshed_graph = ReorderGraph{n_cycle_node};
        build_graph(meshed_graph);
        // begin to remove vertices from graph, create fill-ins
        BGL_FORALL_VERTICES(i, meshed_graph, ReorderGraph) {
            // double loop to loop all pairs of adjacent vertices
            BGL_FORALL_ADJ(i, j1, meshed_graph, ReorderGraph) {
                // skip for already removed vertices
                if (j1 < i) {
                    continue;
                }
                BGL_FORALL_ADJ(i, j2, meshed_graph, ReorderGraph) {
                    // no self edges
                    assert(i != j1);
                    assert(i != j2);
                    // skip for already removed vertices
                    if (j2 < i) {
                        continue;
                    }
                    // only keep pair with j1 < j2
                    if (j1 >= j2) {
                        continue;
                    }
                    // if edge j1 -> j2 does not already exists
                    // it is a fill-in
                    if (!boost::edge(j1, j2, meshed_graph).second) {
                        // anti edge should also not exist
                        assert(!boost::edge(j2, j1, meshed_graph).second);
                        // add both edges to the graph
                        boost::add_edge(j1, j2, meshed_graph);
                        boost::add_edge(j2, j1, meshed_graph);
                        // add to fill-in
                        fill_in.push_back({static_cast<Idx>(j1), static_cast<Idx>(j2)});
                    }
                }
            }
        }
        // offset fill-in indices by n_node - n_cycle_node
        auto const offset = static_cast<Idx>(dfs_node.size() - n_cycle_node);
        std::for_each(fill_in.begin(), fill_in.end(), [offset](BranchIdx& b) {
            b[0] += offset;
            b[1] += offset;
        });
        return fill_in;
    }

    void couple_branch() {
        auto const get_group_pos_if = []([[maybe_unused]] Idx math_group, IntS status, Idx2D const& math_idx) {
            if (status == 0) {
                return Idx{-1};
            }
            assert(math_group == math_idx.group);
            return math_idx.pos;
        };
        // k as branch number for 2-way branch
        for (Idx k = 0; k != static_cast<Idx>(comp_topo_.branch_node_idx.size()); ++k) {
            auto const [i, j] = comp_topo_.branch_node_idx[k];
            IntS const i_status = comp_conn_.branch_connected[k][0];
            IntS const j_status = comp_conn_.branch_connected[k][1];
            Idx2D const i_math = comp_coup_.node[i];
            Idx2D const j_math = comp_coup_.node[j];
            Idx const math_group = [&]() {
                Idx group = -1;
                if (i_status != 0 && i_math.group != -1) {
                    group = i_math.group;
                }
                if (j_status != 0 && j_math.group != -1) {
                    group = j_math.group;
                }
                return group;
            }();
            // skip if no math model connected
            if (math_group == -1) {
                continue;
            }
            assert(i_status || j_status);
            // get and set branch idx in math model
            BranchIdx const branch_idx{get_group_pos_if(math_group, i_status, i_math),
                                       get_group_pos_if(math_group, j_status, j_math)};
            // current branch position index in math model
            auto const branch_pos = math_topology_[math_group].n_branch();
            // push back
            math_topology_[math_group].branch_bus_idx.push_back(branch_idx);
            // set branch idx in coupling
            comp_coup_.branch[k] = Idx2D{math_group, branch_pos};
        }
        // k as branch number for 3-way branch
        for (Idx k = 0; k != static_cast<Idx>(comp_topo_.branch3_node_idx.size()); ++k) {
            auto const i = comp_topo_.branch3_node_idx[k];
            auto const i_status = comp_conn_.branch3_connected[k];
            std::array<Idx2D, 3> const i_math{
                comp_coup_.node[i[0]],
                comp_coup_.node[i[1]],
                comp_coup_.node[i[2]],
            };
            // internal node number as j
            Idx const j = comp_topo_.n_node + k;
            Idx2D const j_math = comp_coup_.node[j];
            Idx const math_group = [&]() {
                Idx group = -1;
                // loop 3 way as indices n
                for (size_t n = 0; n != 3; ++n) {
                    if (i_status[n] != 0 && i_math[n].group != -1) {
                        group = i_math[n].group;
                    }
                }
                return group;
            }();
            // skip if no math model connected
            if (math_group == -1) {
                assert(j_math.group == -1);
                continue;
            }
            assert(i_status[0] || i_status[1] || i_status[2]);
            assert(j_math.group == math_group);
            // branch3
            // TODO make this const
            Idx2DBranch3 idx_branch3{};
            idx_branch3.group = math_group;
            // loop 3 way as indices n
            for (size_t n = 0; n != 3; ++n) {
                // get and set branch idx in math model
                // j side is always connected
                // connect i side if i_status is true
                BranchIdx const branch_idx{get_group_pos_if(math_group, i_status[n], i_math[n]), j_math.pos};
                // current branch position index in math model
                auto const branch_pos = math_topology_[math_group].n_branch();
                // push back
                math_topology_[math_group].branch_bus_idx.push_back(branch_idx);
                // set branch idx in coupling
                idx_branch3.pos[n] = branch_pos;
            }
            // set branch idx in coupling
            comp_coup_.branch3[k] = idx_branch3;
        }
    }

    static constexpr auto include_all = [](Idx) { return true; };

    // proxy class to find the coupled object in math model in the coupling process to a single type object
    //    given a particular component index
    struct SingleTypeObjectFinder {
        Idx size() const { return static_cast<Idx>(component_obj_idx.size()); }
        Idx2D find_math_object(Idx component_i) const { return objects_coupling[component_obj_idx[component_i]]; }
        IdxVector const& component_obj_idx;
        std::vector<Idx2D> const& objects_coupling;
    };

    // proxy class to find coupled branch in math model for sensor measured at from side, or at 1/2/3 side of branch3
    // they are all coupled to the from-side of some branches in math model
    // the key is to find relevant coupling, either via branch or branch3
    struct SensorBranchObjectFinder {
        Idx size() const { return static_cast<Idx>(sensor_obj_idx.size()); }
        Idx2D find_math_object(Idx component_i) const {
            Idx const obj_idx = sensor_obj_idx[component_i];
            switch (power_sensor_terminal_type[component_i]) {
                using enum MeasuredTerminalType;

            case branch_from:
                return branch_coupling[obj_idx];
            // return relevant branch mapped from branch3
            case branch3_1:
                return {branch3_coupling[obj_idx].group, branch3_coupling[obj_idx].pos[0]};
            case branch3_2:
                return {branch3_coupling[obj_idx].group, branch3_coupling[obj_idx].pos[1]};
            case branch3_3:
                return {branch3_coupling[obj_idx].group, branch3_coupling[obj_idx].pos[2]};
            default:
                assert(false);
                return {};
            }
        }
        IdxVector const& sensor_obj_idx;
        std::vector<MeasuredTerminalType> const& power_sensor_terminal_type;
        std::vector<Idx2D> const& branch_coupling;
        std::vector<Idx2DBranch3> const& branch3_coupling;
    };

    // Couple one type of components (e.g. appliances or sensors)
    // The indptr in math topology will be modified
    // The coupling element should be pre-allocated in coupling
    // Only connect the component if include(component_i) returns true
    template <IdxVector MathModelTopology::*indptr, Idx (MathModelTopology::*n_obj_fn)() const,
              typename ObjectFinder = SingleTypeObjectFinder, typename Predicate = decltype(include_all)>
    void couple_object_components(ObjectFinder object_finder, std::vector<Idx2D>& coupling,
                                  Predicate include = include_all) {
        auto const n_math_topologies(static_cast<Idx>(math_topology_.size()));
        auto const n_components = object_finder.size();
        std::vector<IdxVector> topo_obj_idx(n_math_topologies);
        std::vector<IdxVector> topo_component_idx(n_math_topologies);

        // Collect objects and components per topology
        for (Idx component_i = 0; component_i != n_components; ++component_i) {
            if (!include(component_i)) {
                continue;
            }
            Idx2D const math_idx = object_finder.find_math_object(component_i);
            Idx const topo_idx = math_idx.group;
            if (topo_idx >= 0) { // Consider non-isolated objects only
                topo_obj_idx[topo_idx].push_back(math_idx.pos);
                topo_component_idx[topo_idx].push_back(component_i);
            }
        }

        // Couple components per topology
        for (Idx topo_idx = 0; topo_idx != n_math_topologies; ++topo_idx) {
            IdxVector const& obj_idx = topo_obj_idx[topo_idx];
            Idx const n_obj = (math_topology_[topo_idx].*n_obj_fn)();

            // Reorder to compressed format for each math topology
            SparseMapping map = build_sparse_mapping(obj_idx, n_obj);

            // Assign indptr
            math_topology_[topo_idx].*indptr = std::move(map.indptr);

            // Reorder components within the math model
            IdxVector const& reorder = map.reorder;

            // Store component coupling for the current topology
            for (Idx new_math_comp_i = 0; new_math_comp_i != static_cast<Idx>(reorder.size()); ++new_math_comp_i) {
                Idx const old_math_comp_i = reorder[new_math_comp_i];
                Idx const topo_comp_i = topo_component_idx[topo_idx][old_math_comp_i];
                coupling[topo_comp_i] = Idx2D{topo_idx, new_math_comp_i};
            }
        }
    }

    void couple_all_appliance() {
        // shunt
        couple_object_components<&MathModelTopology::shunt_bus_indptr, &MathModelTopology::n_bus>(
            {comp_topo_.shunt_node_idx, comp_coup_.node}, comp_coup_.shunt);

        // load gen
        couple_object_components<&MathModelTopology::load_gen_bus_indptr, &MathModelTopology::n_bus>(
            {comp_topo_.load_gen_node_idx, comp_coup_.node}, comp_coup_.load_gen);

        // set load gen type
        // resize vector
        std::for_each(math_topology_.begin(), math_topology_.end(),
                      [](MathModelTopology& topo) { topo.load_gen_type.resize(topo.n_load_gen()); });
        // assign load type
        for (Idx k = 0; k != static_cast<Idx>(comp_topo_.load_gen_node_idx.size()); ++k) {
            Idx2D const idx_math = comp_coup_.load_gen[k];
            if (idx_math.group == -1) {
                continue;
            }
            math_topology_[idx_math.group].load_gen_type[idx_math.pos] = comp_topo_.load_gen_type[k];
        }

        // source
        couple_object_components<&MathModelTopology::source_bus_indptr, &MathModelTopology::n_bus>(
            {comp_topo_.source_node_idx, comp_coup_.node}, comp_coup_.source,
            [this](Idx i) { return comp_conn_.source_connected[i]; });
    }

    void couple_sensors() {
        // voltage sensors
        couple_object_components<&MathModelTopology::voltage_sensor_indptr, &MathModelTopology::n_bus>(
            {comp_topo_.voltage_sensor_node_idx, comp_coup_.node}, comp_coup_.voltage_sensor);

        // source power sensors
        couple_object_components<&MathModelTopology::source_power_sensor_indptr, &MathModelTopology::n_source>(
            {comp_topo_.power_sensor_object_idx, comp_coup_.source}, comp_coup_.power_sensor,
            [this](Idx i) { return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::source; });

        // shunt power sensors
        couple_object_components<&MathModelTopology::shunt_power_sensor_indptr, &MathModelTopology::n_shunt>(
            {comp_topo_.power_sensor_object_idx, comp_coup_.shunt}, comp_coup_.power_sensor,
            [this](Idx i) { return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::shunt; });

        // load + generator power sensors
        couple_object_components<&MathModelTopology::load_gen_power_sensor_indptr, &MathModelTopology::n_load_gen>(
            {comp_topo_.power_sensor_object_idx, comp_coup_.load_gen}, comp_coup_.power_sensor, [this](Idx i) {
                return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::load ||
                       comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::generator;
            });

        // branch 'from' power sensors
        // include all branch3 sensors
        auto const predicate_from_sensor = [this](Idx i) {
            using enum MeasuredTerminalType;

            return comp_topo_.power_sensor_terminal_type[i] == branch_from ||
                   // all branch3 sensors are at from side in the mathemtical model
                   comp_topo_.power_sensor_terminal_type[i] == branch3_1 ||
                   comp_topo_.power_sensor_terminal_type[i] == branch3_2 ||
                   comp_topo_.power_sensor_terminal_type[i] == branch3_3;
        };
        SensorBranchObjectFinder const object_finder_from_sensor{comp_topo_.power_sensor_object_idx,
                                                                 comp_topo_.power_sensor_terminal_type,
                                                                 comp_coup_.branch, comp_coup_.branch3};
        couple_object_components<&MathModelTopology::branch_from_power_sensor_indptr, &MathModelTopology::n_branch>(
            object_finder_from_sensor, comp_coup_.power_sensor, predicate_from_sensor);

        // branch 'to' power sensors
        couple_object_components<&MathModelTopology::branch_to_power_sensor_indptr, &MathModelTopology::n_branch>(
            {comp_topo_.power_sensor_object_idx, comp_coup_.branch}, comp_coup_.power_sensor,
            [this](Idx i) { return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::branch_to; });

        // node injection power sensors
        couple_object_components<&MathModelTopology::bus_power_sensor_indptr, &MathModelTopology::n_bus>(
            {comp_topo_.power_sensor_object_idx, comp_coup_.node}, comp_coup_.power_sensor,
            [this](Idx i) { return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::node; });
    }
};

} // namespace power_grid_model

#endif
