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
    PowerGridBenchmark() : main_model{50.0} {}

    template <bool sym>
    void run_pf(CalculationMethod calculation_method, CalculationInfo& info, Idx batch_size = -1, Idx threading = -1) {
        try {
            OutputData<sym> output = generator.generate_output_data<sym>(batch_size);
            BatchData const batch_data = generator.generate_batch_input(batch_size, 0);
            // calculate
            main_model.value().calculate_power_flow<sym>(1e-8, 20, calculation_method, output.get_dataset(),
                                                         batch_data.get_dataset(), threading);
            CalculationInfo info_extra = main_model.value().calculation_info();
            info.merge(info_extra);
            std::cout << "Number of nodes: " << generator.input_data().node.size() << '\n';
        } catch (std::exception const& e) {
            std::cout << "\nAn exception was raised during execution: " << e.what() << '\n';
        }
    }

    template <bool sym>
    void run_benchmark(Option const& option, CalculationMethod calculation_method, Idx batch_size = -1,
                       Idx threading = -1) {
        CalculationInfo info;
        generator.generate_grid(option, 0);
        InputData const& input = generator.input_data();

        std::string title = "Benchmark case: ";
        title += option.has_mv_ring ? "meshed grid, " : "radial grid, ";
        title += sym ? "symmetric, " : "asymmetric, ";
        if (calculation_method == CalculationMethod::newton_raphson) {
            title += "Newton-Raphson method";
        } else if (calculation_method == CalculationMethod::linear) {
            title += "Linear method";
        } else {
            title += "Iterative current method";
        }
        std::cout << "=============" << title << "=============\n";

        {
            std::cout << "*****Run with initialization*****\n";
            Timer const t_total(info, 0000, "Total");
            {
                Timer const t_build(info, 1000, "Build model");
                main_model.emplace(50.0, input.get_dataset());
            }
            run_pf<sym>(calculation_method, info);
        }
        print(info);
        info.clear();
        {
            std::cout << "\n*****Run without initialization*****\n";
            Timer const t_total(info, 0000, "Total");
            run_pf<sym>(calculation_method, info);
        }
        print(info);

        if (batch_size > 0) {
            info.clear();
            std::cout << "\n*****Run with batch calculation*****\n";
            Timer const t_total(info, 0000, "Total");
            run_pf<sym>(calculation_method, info, batch_size, threading);
        }
        print(info);

        std::cout << "\n\n";
    }

    static void print(CalculationInfo const& info) {
        for (auto const& [key, val] : info) {
            std::cout << key << ": " << val << '\n';
        }
    }

    std::optional<MainModel> main_model;
    FictionalGridGenerator generator;
};

} // namespace power_grid_model::benchmark

int main(int, char**) {
    using enum power_grid_model::CalculationMethod;

    power_grid_model::benchmark::PowerGridBenchmark benchmarker{};
    power_grid_model::benchmark::Option option{};

#ifndef NDEBUG
    option.n_node_total_specified = 200;
    option.n_mv_feeder = 3;
    option.n_node_per_mv_feeder = 6;
    option.n_lv_feeder = 2;
    option.n_connection_per_lv_feeder = 4;
    power_grid_model::Idx batch_size = 10;
#else
    option.n_node_total_specified = 2000;
    option.n_mv_feeder = 40;
    option.n_node_per_mv_feeder = 10;
    option.n_lv_feeder = 20;
    option.n_connection_per_lv_feeder = 50;
    power_grid_model::Idx batch_size = 1000;
#endif

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark<true>(option, newton_raphson, batch_size);
    benchmarker.run_benchmark<true>(option, newton_raphson, batch_size, 6);
    benchmarker.run_benchmark<true>(option, linear);
    benchmarker.run_benchmark<true>(option, iterative_current);
    benchmarker.run_benchmark<false>(option, newton_raphson);
    benchmarker.run_benchmark<false>(option, linear);
    benchmarker.run_benchmark<false>(option, iterative_current);

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark<true>(option, newton_raphson);
    benchmarker.run_benchmark<true>(option, linear);
    benchmarker.run_benchmark<true>(option, iterative_current);
    benchmarker.run_benchmark<false>(option, newton_raphson);
    benchmarker.run_benchmark<false>(option, linear);
    benchmarker.run_benchmark<false>(option, iterative_current);
    return 0;
}
