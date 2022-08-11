// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <iostream>
#include <random>

#include "fictional_grid_generator.hpp"
#include "power_grid_model/main_model.hpp"
#include "power_grid_model/timer.hpp"

namespace power_grid_model::benchmark {

struct PowerGridBenchmark {
    PowerGridBenchmark() : main_model{50.0} {
    }

    template <bool sym>
    void run_pf(CalculationMethod calculation_method, CalculationInfo& info, Idx batch_size = -1) {
        OutputData<sym> output = generator.generate_output_data<sym>(batch_size);
        BatchData const batch_data = generator.generate_batch_input(batch_size, 0);
        // calculate
        main_model.value().calculate_power_flow<sym>(1e-8, 20, calculation_method, output.get_dataset(),
                                                     batch_data.get_dataset());
        CalculationInfo info_extra = main_model.value().calculation_info();
        info.merge(info_extra);
        std::cout << "Number of nodes: " << generator.input_data().node.size() << '\n';
    }

    template <bool sym>
    void run_benchmark(Option const& option, CalculationMethod calculation_method, Idx batch_size = -1) {
        CalculationInfo info;
        generator.generate_grid(option, 0);
        InputData const& input = generator.input_data();

        std::string title = "Benchmark case: ";
        title += option.has_mv_ring ? "meshed grid, " : "radial grid, ";
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
                main_model.emplace(50.0, input.get_dataset());
            }
            run_pf<sym>(calculation_method, info);
        }
        print(info);
        info.clear();
        {
            std::cout << "\n*****Run without initialization*****\n";
            Timer t_total(info, 0000, "Total");
            run_pf<sym>(calculation_method, info);
        }
        print(info);

        if (batch_size > 0) {
            info.clear();
            std::cout << "\n*****Run with batch calculation*****\n";
            Timer t_total(info, 0000, "Total");
            run_pf<sym>(calculation_method, info, batch_size);
        }
        print(info);

        std::cout << "\n\n";
    }

    void print(CalculationInfo const& info) {
        for (auto const& [key, val] : info) {
            std::cout << key << ": " << val << '\n';
        }
    }

    std::optional<MainModel> main_model;
    FictionalGridGenerator generator;
};

}  // namespace power_grid_model::benchmark

int main(int, char**) {
    using power_grid_model::CalculationMethod;
    power_grid_model::benchmark::PowerGridBenchmark benchmarker{};
    power_grid_model::benchmark::Option option{};

#ifndef NDEBUG
    option.n_node_total_specified = 200;
    option.n_mv_feeder = 3;
    option.n_node_per_mv_feeder = 6;
    option.n_lv_feeder = 2;
    option.n_connection_per_lv_feeder = 4;
#else
    option.n_node_total_specified = 1000000;
    option.n_mv_feeder = 40;
    option.n_node_per_mv_feeder = 30;
    option.n_lv_feeder = 10;
    option.n_connection_per_lv_feeder = 100;
#endif
    option.has_mv_ring = false;
    option.has_lv_ring = false;

    // radial
    benchmarker.run_benchmark<true>(option, CalculationMethod::newton_raphson, 5);
    // benchmarker.run_benchmark(option, true, CalculationMethod::linear);
    // benchmarker.run_benchmark(option, true, CalculationMethod::iterative_current);
    // benchmarker.run_benchmark<false>(option, CalculationMethod::newton_raphson);
    // benchmarker.run_benchmark(option, false, CalculationMethod::linear);
    // benchmarker.run_benchmark(option, false, CalculationMethod::iterative_current);
    //// with meshed ring
    // benchmarker.run_benchmark(n_node, true, CalculationMethod::newton_raphson, true);
    // benchmarker.run_benchmark(n_node, true, CalculationMethod::linear, true);
    // benchmarker.run_benchmark(n_node, true, CalculationMethod::iterative_current, true);
    // benchmarker.run_benchmark(n_node, false, CalculationMethod::newton_raphson, true);
    // benchmarker.run_benchmark(n_node, false, CalculationMethod::linear, true);
    // benchmarker.run_benchmark(n_node, false, CalculationMethod::iterative_current, true);
    return 0;
}
