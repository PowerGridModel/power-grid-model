// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <iostream>
#include <random>

#include "power_grid_model/main_model.hpp"
#include "power_grid_model/timer.hpp"

namespace power_grid_model {

#ifndef NDEBUG
constexpr Idx nodes_per_feeder = 10;
#else
constexpr Idx nodes_per_feeder = 100;
#endif

constexpr Idx ring_node_pos = 2;

struct PowerGridBenchmark {
    PowerGridBenchmark() : main_model{50.0} {
    }

    void generate_network(Idx n_nodes, bool meshed) {
        sym_load_input.clear();
        asym_load_input.clear();
        transformer_input.clear();
        line_input.clear();
        node_input.clear();
        source_input.clear();

        Idx const n_feeders = n_nodes / nodes_per_feeder;

        // source node
        ID const id_source_node = 0;
        NodeInput const source_node{{id_source_node}, 150.0e3};
        NodeInput const cycle_node_1{{id_source_node + 1}, 150.0e3}, cycle_node_2{{id_source_node + 2}, 150.0e3};
        LinkInput const link_1{{{3}, 0, 1, true, true}};
        LinkInput const link_2{{{4}, 1, 2, true, true}};
        LinkInput const link_3{{{5}, 2, 0, true, true}};

        SourceInput const source{{{6}, 0, true}, 1.05, nan, 1e20, nan, nan};
        // template input
        NodeInput const node{{0}, 10.0e3};
        SymLoadGenInput const sym_load{{{{0}, 0, true}, LoadGenType::const_i}, 0.4e6, 0.3e6};
        AsymLoadGenInput const asym_load{
            {{{0}, 0, true}, LoadGenType::const_i}, RealValue<false>{0.0}, RealValue<false>{0.0}};
        // transformer, 150/10.5kV, 30MVA, uk=20.3%
        TransformerInput const tranformer{{{0}, 0, 0, true, true},
                                          150.0e3,
                                          10.5e3,
                                          30.0e6,
                                          0.203,
                                          100e3,
                                          0.01,
                                          20e3,
                                          WindingType::delta,
                                          WindingType::wye_n,
                                          5,
                                          BranchSide::from,
                                          0,
                                          -10,
                                          10,
                                          0,
                                          2.5e3,
                                          nan,
                                          nan,
                                          nan,
                                          nan,
                                          nan,
                                          nan,
                                          nan,
                                          nan};
        // cable 630Al XLPE 10kV with neutral conductor, 1 km
        LineInput const line{{{0}, 0, 0, true, true}, 0.063, 0.103, 0.4e-6, 0.0, 0.156, 0.1, 0.66e-6, 0.0, 1e3};

        // random generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<Idx> load_type_gen{0, 2};
        // total 10 km feeder, divided by nodes per feeder
        std::uniform_real_distribution<double> real_gen{0.8 * 10.0 / nodes_per_feeder, 1.2 * 10.0 / nodes_per_feeder};

        // input vector
        node_input = {source_node, cycle_node_1, cycle_node_2};
        source_input = {source};
        link_input = {link_1, link_2, link_3};

        // vector of node ids at far end
        std::vector<ID> ring_nodes;

        // current id
        ID id_gen = 10;
        for (Idx i = 0; i < n_feeders; i++) {
            NodeInput feeder_node = node;
            feeder_node.id = id_gen++;
            TransformerInput transformer_s = tranformer;
            transformer_s.id = id_gen++;
            transformer_s.from_node = id_source_node;
            transformer_s.to_node = feeder_node.id;
            node_input.push_back(feeder_node);
            transformer_input.push_back(transformer_s);
            ID prev_node_id = feeder_node.id;
            for (Idx j = 0; j < nodes_per_feeder; j++) {
                // node
                ID const current_node_id = id_gen++;
                NodeInput node_s = node;
                node_s.id = current_node_id;
                // line
                LineInput line_s = line;
                line_s.id = id_gen++;
                line_s.from_node = prev_node_id;
                line_s.to_node = current_node_id;
                // scale r1, x1, r0, x0
                double const cable_ratio = real_gen(gen);
                line_s.r1 *= cable_ratio;
                line_s.x1 *= cable_ratio;
                line_s.c1 *= cable_ratio;
                line_s.r0 *= cable_ratio;
                line_s.x0 *= cable_ratio;
                line_s.c0 *= cable_ratio;
                // load
                SymLoadGenInput sym_load_s = sym_load;
                AsymLoadGenInput asym_load_s = asym_load;
                // id
                sym_load_s.id = id_gen++;
                asym_load_s.id = id_gen++;
                // node id
                sym_load_s.node = current_node_id;
                asym_load_s.node = current_node_id;
                // type
                sym_load_s.type = static_cast<LoadGenType>((IntS)load_type_gen(gen));
                asym_load_s.type = static_cast<LoadGenType>((IntS)load_type_gen(gen));
                // value
                double const sym_scale = real_gen(gen);
                sym_load_s.p_specified *= sym_scale;
                sym_load_s.q_specified *= sym_scale;
                double const asym_scale = real_gen(gen);
                Idx const phase = load_type_gen(gen);
                std::array<double, 3> p{}, q{};
                p[phase] = asym_scale * sym_load.p_specified;
                q[phase] = asym_scale * sym_load.q_specified;
                asym_load_s.p_specified << p[0], p[1], p[2];
                asym_load_s.q_specified << q[0], q[1], q[2];
                // push to vector
                node_input.push_back(node_s);
                line_input.push_back(line_s);
                sym_load_input.push_back(sym_load_s);
                asym_load_input.push_back(asym_load_s);
                prev_node_id = current_node_id;
                // push to rind node
                if (j == ring_node_pos) {
                    ring_nodes.push_back(node_input.back().id);
                }
            }
        }
        // add loop if needed, and there are more than one feeder, and there are ring nodes
        if (n_feeders > 1 && meshed && !ring_nodes.empty()) {
            ring_nodes.push_back(ring_nodes.front());
            // loop all far end nodes
            for (auto it = ring_nodes.cbegin(); it != ring_nodes.cend() - 1; ++it) {
                // line
                LineInput line_s = line;
                line_s.id = id_gen++;
                line_s.from_node = *it;
                line_s.to_node = *(it + 1);
                // scale r1, x1, r0, x0
                double const cable_ratio = real_gen(gen);
                line_s.r1 *= cable_ratio;
                line_s.x1 *= cable_ratio;
                line_s.c1 *= cable_ratio;
                line_s.r0 *= cable_ratio;
                line_s.x0 *= cable_ratio;
                line_s.c0 *= cable_ratio;
                line_input.push_back(line_s);
            }
        }
    }

    void build_network() {
        main_model = MainModel{50.0};
        main_model.add_component<Node>(node_input.cbegin(), node_input.cend());
        main_model.add_component<Source>(source_input.cbegin(), source_input.cend());
        main_model.add_component<SymLoad>(sym_load_input.cbegin(), sym_load_input.cend());
        main_model.add_component<AsymLoad>(asym_load_input.cbegin(), asym_load_input.cend());
        main_model.add_component<Transformer>(transformer_input.cbegin(), transformer_input.cend());
        main_model.add_component<Line>(line_input.cbegin(), line_input.cend());
        main_model.add_component<Link>(link_input.cbegin(), link_input.cend());
        main_model.set_construction_complete();
    }

    template <bool sym>
    void run_pf(CalculationMethod calculation_method, CalculationInfo& info) {
        std::vector<NodeOutput<sym>> node(node_input.size());
        std::vector<BranchOutput<sym>> branch(line_input.size() + transformer_input.size() + link_input.size());
        std::vector<ApplianceOutput<sym>> appliance(source_input.size() + sym_load_input.size() +
                                                    asym_load_input.size());
        auto const math_output = main_model.calculate_power_flow<sym>(1e-8, 20, calculation_method);
        {
            Timer t_output(info, 3000, "Calculate output");
            main_model.output_result<sym, Node>(math_output, node.begin());
            main_model.output_result<sym, Branch>(math_output, branch.begin());
            main_model.output_result<sym, Appliance>(math_output, appliance.begin());
        }
        CalculationInfo info_extra = main_model.calculation_info();
        info.merge(info_extra);
        std::cout << "Number of nodes: " << node.size() << '\n';
        auto const [min_l, max_l] = std::minmax_element(branch.cbegin(), branch.cend(), [](auto x, auto y) {
            return x.loading < y.loading;
        });
        std::cout << "Min loading: " << min_l->loading << ", max loading: " << max_l->loading << '\n';
    }

    void run_benchmark(Idx n_node, bool sym, CalculationMethod calculation_method, bool meshed) {
        CalculationInfo info;
        generate_network(n_node, meshed);
        std::string title = "Benchmark case: ";
        title += meshed ? "meshed grid, " : "radial grid, ";
        title += sym ? "symmetric, " : "asymmetric, ";
        if (calculation_method == CalculationMethod::newton_raphson) {
            title += "Newton-Raphson method";
        }
        else if (calculation_method == CalculationMethod::linear) {
            title += "Linear method";
        }
        else {
            title += "Iterative current method";
        }
        std::cout << "=============" << title << "=============\n";

        {
            std::cout << "*****Run with initialization*****\n";
            Timer t_total(info, 0000, "Total");
            {
                Timer t_build(info, 1000, "Build model");
                build_network();
            }
            if (sym) {
                run_pf<true>(calculation_method, info);
            }
            else {
                run_pf<false>(calculation_method, info);
            }
        }
        print(info);

        info.clear();
        {
            std::cout << "\n*****Run without initialization*****\n";
            Timer t_total(info, 0000, "Total");
            if (sym) {
                run_pf<true>(calculation_method, info);
            }
            else {
                run_pf<false>(calculation_method, info);
            }
        }
        print(info);
        std::cout << "\n\n";
    }

    void print(CalculationInfo const& info) {
        for (auto const& [key, val] : info) {
            std::cout << key << ": " << val << '\n';
        }
    }

    MainModel main_model;
    // input vector
    std::vector<NodeInput> node_input;
    std::vector<SourceInput> source_input;
    std::vector<SymLoadGenInput> sym_load_input;
    std::vector<AsymLoadGenInput> asym_load_input;
    std::vector<TransformerInput> transformer_input;
    std::vector<LineInput> line_input;
    std::vector<LinkInput> link_input;
};

}  // namespace power_grid_model

int main(int, char**) {
#ifndef NDEBUG
    constexpr power_grid_model::Idx n_node = 100;
#else
    constexpr power_grid_model::Idx n_node = 1000000;
#endif
    using power_grid_model::CalculationMethod;
    power_grid_model::PowerGridBenchmark benchmarker{};
    // radial
    benchmarker.run_benchmark(n_node, true, CalculationMethod::newton_raphson, false);
    benchmarker.run_benchmark(n_node, true, CalculationMethod::linear, false);
    benchmarker.run_benchmark(n_node, true, CalculationMethod::iterative_current, false);
    benchmarker.run_benchmark(n_node, false, CalculationMethod::newton_raphson, false);
    benchmarker.run_benchmark(n_node, false, CalculationMethod::linear, false);
    benchmarker.run_benchmark(n_node, false, CalculationMethod::iterative_current, false);
    // with meshed ring
    benchmarker.run_benchmark(n_node, true, CalculationMethod::newton_raphson, true);
    benchmarker.run_benchmark(n_node, true, CalculationMethod::linear, true);
    benchmarker.run_benchmark(n_node, true, CalculationMethod::iterative_current, true);
    benchmarker.run_benchmark(n_node, false, CalculationMethod::newton_raphson, true);
    benchmarker.run_benchmark(n_node, false, CalculationMethod::linear, true);
    benchmarker.run_benchmark(n_node, false, CalculationMethod::iterative_current, true);
    return 0;
}
