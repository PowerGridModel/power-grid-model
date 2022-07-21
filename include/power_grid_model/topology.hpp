// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_TOPOLOGY_HPP
#define POWER_GRID_MODEL_TOPOLOGY_HPP

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/compressed_sparse_row_graph.hpp"
#include "boost/graph/depth_first_search.hpp"
#include "boost/graph/iteration_macros.hpp"
#include "boost/graph/minimum_degree_ordering.hpp"
#include "calculation_parameters.hpp"
#include "enum.hpp"
#include "exception.hpp"
#include "power_grid_model.hpp"
#include "sparse_mapping.hpp"

// build topology of the grid
// divide grid into several math models
// start search from a source
// using BFS search

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

    using ReorderGraph = boost::adjacency_list<boost::vecS,  // vector as adjacency
                                               boost::vecS,  // vector for vertices
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
              back_edges_{back_edges} {
        }

        // accumulate phase shift
        // assign predecessor
        void tree_edge(GlobalGraph::edge_descriptor e, GlobalGraph const& g) {
            GraphIdx const source = boost::source(e, g), target = boost::target(e, g);
            phase_shift_[target] = phase_shift_[source] + g[e].phase_shift;
            predecessors_[target] = source;
        }

        // forward_or_cross_edge
        // in symmetric directed graph (equivalent to undirected)
        //    forward edge is ignored
        //    cross edge does not exist

        // back edge, judge if it forms a cycle
        void back_edge(GlobalGraph::edge_descriptor e, GlobalGraph const& g) {
            GraphIdx const source = boost::source(e, g), target = boost::target(e, g);
            // if this edge matches in the current tree as target->source
            // it does not form a cycle, but an anti-parallel edge
            // else it forms a cycle
            if (predecessors_[source] != target) {
                back_edges_.push_back({source, target});
            }
        }

        // assign node to math group
        // append node to dfs list
        void discover_vertex(GlobalGraph::vertex_descriptor u, GlobalGraph const&) {
            node_coupling_[u].group = (Idx)math_group_;
            dfs_node_.push_back((Idx)u);
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
              boost::counting_iterator<GraphIdx>{0},  // Predecessors is initialized as 0, 1, 2, ..., n_node_total() - 1
              boost::counting_iterator<GraphIdx>{(GraphIdx)comp_topo_.n_node_total()}),
          node_status_(comp_topo_.n_node_total(), -1) {
    }

    // build topology
    std::pair<std::vector<std::shared_ptr<MathModelTopology const>>, std::shared_ptr<ComponentToMathCoupling const>>
    build_topology() {
        reset_topology();
        build_sparse_graph();
        dfs_search();
        couple_branch();
        couple_all_appliance();
        couple_sensors();
        // create return pair with shared pointer
        std::pair<std::vector<std::shared_ptr<MathModelTopology const>>, std::shared_ptr<ComponentToMathCoupling const>>
            pair;
        for (Idx k = 0; k != (Idx)math_topology_.size(); ++k) {
            pair.first.emplace_back(std::make_shared<MathModelTopology const>(std::move(math_topology_[k])));
        }
        pair.second = std::make_shared<ComponentToMathCoupling const>(std::move(comp_coup_));
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
    ComponentToMathCoupling comp_coup_;

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
        for (Idx k = 0; k != (Idx)comp_topo_.branch_node_idx.size(); ++k) {
            auto const [i, j] = comp_topo_.branch_node_idx[k];
            auto const [i_status, j_status] = comp_conn_.branch_connected[k];
            // node_i - node_j
            double const phase_shift = comp_conn_.branch_phase_shift[k];
            if (i_status && j_status) {
                // node_j - node_i
                edges.push_back({(GraphIdx)i, (GraphIdx)j});
                edge_props.push_back({-phase_shift});
                // node_i - node_j
                edges.push_back({(GraphIdx)j, (GraphIdx)i});
                edge_props.push_back({phase_shift});
            }
        }
        // k as branch number for 3-way branch
        for (Idx k = 0; k != (Idx)comp_topo_.branch3_node_idx.size(); ++k) {
            auto const i = comp_topo_.branch3_node_idx[k];
            auto const i_status = comp_conn_.branch3_connected[k];
            // node_i - node_internal
            auto const& phase_shift = comp_conn_.branch3_phase_shift[k];
            // internal node number
            Idx const j_internal = comp_topo_.n_node + k;
            // loop 3 way as indices m
            for (Idx m = 0; m != 3; ++m) {
                if (i_status[m]) {
                    // node_internal - node_i
                    edges.push_back({(GraphIdx)i[m], (GraphIdx)j_internal});
                    edge_props.push_back({-phase_shift[m]});
                    // node_i - node_internal
                    edges.push_back({(GraphIdx)j_internal, (GraphIdx)i[m]});
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
        Idx m = 0;
        // loop all source as k
        for (Idx k = 0; k != (Idx)comp_topo_.source_node_idx.size(); ++k) {
            // skip disconnected source
            if (!comp_conn_.source_connected[k]) {
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
                GlobalDFSVisitor{m, comp_coup_.node, phase_shift_, dfs_node, predecessors_, back_edges},
                boost::get(&GlobalVertex::color, global_graph_));
            // reorder node number
            if (back_edges.empty()) {
                // no cycle, the graph is pure tree structure
                // just reverse the node
                std::reverse(dfs_node.begin(), dfs_node.end());
            }
            else {
                // with cycles, meshed graph
                // use minimum degree
                reorder_node(dfs_node, back_edges);
            }
            // assign bus number
            MathModelTopology math_topo_single{};
            // initialize phase shift
            math_topo_single.phase_shift.resize((Idx)dfs_node.size());
            // i as bus number
            Idx i = 0;
            for (auto it = dfs_node.cbegin(); it != dfs_node.cend(); ++it, ++i) {
                Idx const current_node = *it;
                // assign node coupling
                comp_coup_.node[current_node].pos = i;
                // assign phase shift
                math_topo_single.phase_shift[i] = phase_shift_[current_node];
                assert(comp_coup_.node[current_node].group == m);
            }
            assert(i == math_topo_single.n_bus());
            // assign slack bus as the source node
            math_topo_single.slack_bus_ = comp_coup_.node[source_node].pos;
            math_topology_.emplace_back(std::move(math_topo_single));
            // iterate math model sequence number
            ++m;
            assert(math_topology_.size() == (size_t)m);
        }
    }

    // re-order bfs_node using minimum degree
    void reorder_node(std::vector<Idx>& dfs_node, std::vector<std::pair<GraphIdx, GraphIdx>> const& back_edges) {
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
        std::copy_if(dfs_node_copy.crbegin(), dfs_node_copy.crend(), std::back_inserter(dfs_node), [this](Idx x) {
            return node_status_[x] == -1;
        });
        // copy all cyclic node
        std::vector<Idx> cyclic_node;
        std::copy_if(dfs_node_copy.cbegin(), dfs_node_copy.cend(), std::back_inserter(cyclic_node), [this](Idx x) {
            return node_status_[x] == -2;
        });
        GraphIdx const n_cycle_node = cyclic_node.size();
        // reorder does not make sense if number of cyclic nodes in a sub graph is smaller than 4
        if (n_cycle_node < 4) {
            std::copy(cyclic_node.crbegin(), cyclic_node.crend(), std::back_inserter(dfs_node));
            return;
        }

        // assign temporary bus number as increasing from 0, 1, 2, ..., n_cycle_node - 1
        for (GraphIdx i = 0; i != n_cycle_node; ++i) {
            node_status_[cyclic_node[i]] = (Idx)i;
        }
        // build graph
        ReorderGraph graph{n_cycle_node};
        // add edges
        for (GraphIdx i = 0; i != n_cycle_node; ++i) {
            // loop all edges of vertex i
            GraphIdx global_i = (GraphIdx)cyclic_node[i];
            auto const [vertex_begin, vertex_end] = boost::adjacent_vertices(global_i, global_graph_);
            for (auto it_vertex = vertex_begin; it_vertex != vertex_end; ++it_vertex) {
                GraphIdx const global_j = *it_vertex;
                // skip if j is not part of cyclic sub graph
                if (node_status_[global_j] == -1) {
                    continue;
                }
                GraphIdx const j = (GraphIdx)node_status_[global_j];
                if (!boost::edge(i, j, graph).second) {
                    boost::add_edge(i, j, graph);
                }
            }
        }
        // start minimum degree ordering
        std::vector<std::make_signed_t<GraphIdx>> perm(n_cycle_node), inverse_perm(n_cycle_node), degree(n_cycle_node),
            supernode_sizes(n_cycle_node, 1);
        boost::vec_adj_list_vertex_id_map<boost::no_property, std::make_signed_t<GraphIdx>> id{};
        int const delta = 0;
        boost::minimum_degree_ordering(graph, boost::make_iterator_property_map(degree.begin(), id),
                                       boost::make_iterator_property_map(inverse_perm.begin(), id),
                                       boost::make_iterator_property_map(perm.begin(), id),
                                       boost::make_iterator_property_map(supernode_sizes.begin(), id), delta, id);
        // loop to assign re-order sub graph
        for (GraphIdx i = 0; i != n_cycle_node; ++i) {
            dfs_node.push_back(cyclic_node[perm[i]]);
        }
    }

    void couple_branch() {
        // k as branch number for 2-way branch
        for (Idx k = 0; k != (Idx)comp_topo_.branch_node_idx.size(); ++k) {
            auto const [i, j] = comp_topo_.branch_node_idx[k];
            IntS const i_status = comp_conn_.branch_connected[k][0];
            IntS const j_status = comp_conn_.branch_connected[k][1];
            Idx2D const i_math = comp_coup_.node[i];
            Idx2D const j_math = comp_coup_.node[j];
            // m as math model group number
            Idx const m = [&]() {
                Idx group = -1;
                if (i_status && i_math.group != -1) {
                    group = i_math.group;
                }
                if (j_status && j_math.group != -1) {
                    group = j_math.group;
                }
                return group;
            }();
            // skip if no math model connected
            if (m == -1) {
                continue;
            }
            assert(i_status || j_status);
            // get and set branch idx in math model
            BranchIdx const branch_idx{i_status ? assert(m == i_math.group), i_math.pos : -1,
                                       j_status ? assert(m == j_math.group), j_math.pos : -1};
            // current branch position index in math model
            Idx const branch_pos = (Idx)math_topology_[m].n_branch();
            // push back
            math_topology_[m].branch_bus_idx.push_back(branch_idx);
            // set branch idx in coupling
            comp_coup_.branch[k] = Idx2D{m, branch_pos};
        }
        // k as branch number for 3-way branch
        for (Idx k = 0; k != (Idx)comp_topo_.branch3_node_idx.size(); ++k) {
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
            // m as math model group number
            Idx const m = [&]() {
                Idx group = -1;
                // loop 3 way as indices n
                for (size_t n = 0; n != 3; ++n) {
                    if (i_status[n] && i_math[n].group != -1) {
                        group = i_math[n].group;
                    }
                }
                return group;
            }();
            // skip if no math model connected
            if (m == -1) {
                assert(j_math.group == -1);
                continue;
            }
            assert(i_status[0] || i_status[1] || i_status[2]);
            assert(j_math.group == m);
            // branch3
            // TODO make this const
            Idx2DBranch3 idx_branch3{};
            idx_branch3.group = m;
            // loop 3 way as indices n
            for (size_t n = 0; n != 3; ++n) {
                // get and set branch idx in math model
                // j side is always connected
                // connect i side if i_status is true
                BranchIdx const branch_idx{i_status[n] ? assert(i_math[n].group == m), i_math[n].pos : -1, j_math.pos};
                // current branch position index in math model
                Idx const branch_pos = (Idx)math_topology_[m].n_branch();
                // push back
                math_topology_[m].branch_bus_idx.push_back(branch_idx);
                // set branch idx in coupling
                idx_branch3.pos[n] = branch_pos;
            }
            // set branch idx in coupling
            comp_coup_.branch3[k] = idx_branch3;
        }
    }

    static constexpr auto include_all = [](Idx) {
        return true;
    };

    // Couple one type of components (e.g. appliances or sensors)
    // The indptr in math topology will be modified
    // The coupling element should be pre-allocated in coupling
    // Only connect the component if include(component_i) returns true
    template <IdxVector MathModelTopology::*indptr, Idx (MathModelTopology::*n_obj_fn)() const,
              typename Predicate = decltype(include_all)>
    void couple_object_components(IdxVector const& component_obj_idx, std::vector<Idx2D>& objects,
                                  std::vector<Idx2D>& coupling, Predicate include = include_all) {
        auto const n_math_topologies((Idx)math_topology_.size());
        auto const n_components = (Idx)component_obj_idx.size();
        std::vector<IdxVector> topo_obj_idx(n_math_topologies);
        std::vector<IdxVector> topo_component_idx(n_math_topologies);

        // Collect objects and components per topology
        for (Idx component_i = 0; component_i != n_components; ++component_i) {
            if (!include(component_i)) {
                continue;
            }
            Idx const obj_idx = component_obj_idx[component_i];
            Idx2D const math_idx = objects[obj_idx];
            Idx const topo_idx = math_idx.group;
            if (topo_idx >= 0) {  // Consider non-isolated objects only
                topo_obj_idx[topo_idx].push_back(math_idx.pos);
                topo_component_idx[topo_idx].push_back(component_i);
            }
        }

        // Couple components per topology
        for (Idx topo_idx = 0; topo_idx != n_math_topologies; ++topo_idx) {
            auto const obj_idx = topo_obj_idx[topo_idx];
            auto const n_obj = (math_topology_[topo_idx].*n_obj_fn)();

            // Reorder to compressed format for each math topology
            auto map = build_sparse_mapping(obj_idx, n_obj);

            // Assign indptr
            math_topology_[topo_idx].*indptr = std::move(map.indptr);

            // Reorder components within the math model
            auto const& reorder = map.reorder;

            // Store component coupling for the current topology
            for (Idx new_math_comp_i = 0; new_math_comp_i != (Idx)reorder.size(); ++new_math_comp_i) {
                auto const old_math_comp_i = reorder[new_math_comp_i];
                auto const topo_comp_i = topo_component_idx[topo_idx][old_math_comp_i];
                coupling[topo_comp_i] = Idx2D{topo_idx, new_math_comp_i};
            }
        }
    }

    void couple_all_appliance() {
        // shunt
        couple_object_components<&MathModelTopology::shunt_bus_indptr, &MathModelTopology::n_bus>(
            comp_topo_.shunt_node_idx, comp_coup_.node, comp_coup_.shunt);

        // load gen
        couple_object_components<&MathModelTopology::load_gen_bus_indptr, &MathModelTopology::n_bus>(
            comp_topo_.load_gen_node_idx, comp_coup_.node, comp_coup_.load_gen);

        // set load gen type
        // resize vector
        std::for_each(math_topology_.begin(), math_topology_.end(), [](MathModelTopology& topo) {
            topo.load_gen_type.resize(topo.n_load_gen());
        });
        // assign load type
        for (Idx k = 0; k != (Idx)comp_topo_.load_gen_node_idx.size(); ++k) {
            Idx2D const idx_math = comp_coup_.load_gen[k];
            if (idx_math.group == -1) {
                continue;
            }
            math_topology_[idx_math.group].load_gen_type[idx_math.pos] = comp_topo_.load_gen_type[k];
        }

        // source
        couple_object_components<&MathModelTopology::source_bus_indptr, &MathModelTopology::n_bus>(
            comp_topo_.source_node_idx, comp_coup_.node, comp_coup_.source, [&](Idx i) {
                return comp_conn_.source_connected[i];
            });
    }

    void couple_sensors() {
        // voltage sensors
        couple_object_components<&MathModelTopology::voltage_sensor_indptr, &MathModelTopology::n_bus>(
            comp_topo_.voltage_sensor_node_idx, comp_coup_.node, comp_coup_.voltage_sensor);

        // source power sensors
        couple_object_components<&MathModelTopology::source_power_sensor_indptr, &MathModelTopology::n_source>(
            comp_topo_.power_sensor_object_idx, comp_coup_.source, comp_coup_.power_sensor, [&](Idx i) {
                return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::source;
            });

        // shunt power sensors
        couple_object_components<&MathModelTopology::shunt_power_sensor_indptr, &MathModelTopology::n_shunt>(
            comp_topo_.power_sensor_object_idx, comp_coup_.shunt, comp_coup_.power_sensor, [&](Idx i) {
                return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::shunt;
            });

        // load + generator power sensors
        couple_object_components<&MathModelTopology::load_gen_power_sensor_indptr, &MathModelTopology::n_load_gen>(
            comp_topo_.power_sensor_object_idx, comp_coup_.load_gen, comp_coup_.power_sensor, [&](Idx i) {
                return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::load ||
                       comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::generator;
            });

        // branch 'from' power sensors
        couple_object_components<&MathModelTopology::branch_from_power_sensor_indptr, &MathModelTopology::n_branch>(
            comp_topo_.power_sensor_object_idx, comp_coup_.branch, comp_coup_.power_sensor, [&](Idx i) {
                return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::branch_from;
            });

        // branch 'to' power sensors
        couple_object_components<&MathModelTopology::branch_to_power_sensor_indptr, &MathModelTopology::n_branch>(
            comp_topo_.power_sensor_object_idx, comp_coup_.branch, comp_coup_.power_sensor, [&](Idx i) {
                return comp_topo_.power_sensor_terminal_type[i] == MeasuredTerminalType::branch_to;
            });
    }
};

}  // namespace power_grid_model

#endif
