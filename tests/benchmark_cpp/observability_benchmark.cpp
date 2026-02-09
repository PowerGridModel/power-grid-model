// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * Observability Benchmark Framework
 *
 * This benchmark compares the performance of radial vs meshed observability algorithms.
 *
 * See the `benchmark_observability_algorithms` function below for the actual benchmark code.
 */

#include "fictional_grid_generator.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/main_model.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>
#include <power_grid_model/math_solver/measured_values.hpp>
#include <power_grid_model/math_solver/observability.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace power_grid_model::benchmark {

/**
 * @brief Benchmark observability check algorithms
 *
 * This function demonstrates the actual benchmarking code that compares radial
 * and meshed observability check performance on the same network.
 *
 * @tparam sym Symmetry type (symmetric_t or asymmetric_t)
 * @param y_bus The Y-bus structure
 * @param measured_values The sensor measurements
 * @param topo The network topology (mutable to toggle is_radial)
 * @param n_iterations Number of iterations to run for each algorithm
 */
template <symmetry_tag sym>
inline void benchmark_observability_algorithms(YBus<sym> const& y_bus,
                                               math_solver::MeasuredValues<sym> const& measured_values,
                                               MathModelTopology& topo, // mutable to toggle is_radial
                                               Idx n_iterations = 1000) {

    using namespace std::chrono;

    std::vector<nanoseconds> radial_times;
    std::vector<nanoseconds> meshed_times;

    radial_times.reserve(n_iterations);
    meshed_times.reserve(n_iterations);

    // Remember original topology setting
    bool const original_is_radial = topo.is_radial;
    Idx const n_bus = topo.n_bus();

    std::cout << std::format("\nBenchmarking network with {} buses\n", n_bus);
    std::cout << std::format("Running {} iterations per algorithm...\n", n_iterations);

    // Benchmark radial algorithm
    std::cout << "Benchmarking radial algorithm...\n";
    topo.is_radial = true;
    for (Idx i = 0; i < n_iterations; ++i) {
        auto start = high_resolution_clock::now();
        try {
            [[maybe_unused]] auto result =
                math_solver::observability::observability_check(measured_values, topo, y_bus.y_bus_structure());
        } catch (NotObservableError const&) {
            std::cerr << "  Warning: Network not observable with radial algorithm\n";
            break;
        }
        auto end = high_resolution_clock::now();
        radial_times.push_back(duration_cast<nanoseconds>(end - start));
    }

    // Benchmark meshed algorithm
    std::cout << "Benchmarking meshed algorithm...\n";
    topo.is_radial = false;
    for (Idx i = 0; i < n_iterations; ++i) {
        auto start = high_resolution_clock::now();
        try {
            [[maybe_unused]] auto result =
                math_solver::observability::observability_check(measured_values, topo, y_bus.y_bus_structure());
        } catch (NotObservableError const&) {
            std::cerr << "  Warning: Network not observable with meshed algorithm\n";
            break;
        }
        auto end = high_resolution_clock::now();
        meshed_times.push_back(duration_cast<nanoseconds>(end - start));
    }

    // Restore original setting
    topo.is_radial = original_is_radial;

    // Calculate and display statistics
    if (radial_times.empty() || meshed_times.empty()) {
        std::cout << "\nBenchmark failed - one or both algorithms did not complete\n";
        return;
    }

    auto calc_mean = [](auto const& times) {
        return std::accumulate(times.begin(), times.end(), nanoseconds{0}) / static_cast<Idx>(times.size());
    };

    auto calc_median = [](auto times) { // copy for sorting
        std::sort(times.begin(), times.end());
        if (times.size() % 2 == 0) {
            return (times[times.size() / 2 - 1] + times[times.size() / 2]) / 2;
        }
        return times[times.size() / 2];
    };

    auto radial_mean = calc_mean(radial_times);
    auto radial_median = calc_median(radial_times);
    auto radial_min = *std::min_element(radial_times.begin(), radial_times.end());
    auto radial_max = *std::max_element(radial_times.begin(), radial_times.end());

    auto meshed_mean = calc_mean(meshed_times);
    auto meshed_median = calc_median(meshed_times);
    auto meshed_min = *std::min_element(meshed_times.begin(), meshed_times.end());
    auto meshed_max = *std::max_element(meshed_times.begin(), meshed_times.end());

    double time_overhead = ((meshed_mean.count() - radial_mean.count()) * 100.0) / radial_mean.count();

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Results:\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << std::format("Radial Algorithm:\n");
    std::cout << std::format("  Mean:   {:.2f} μs\n", radial_mean.count() / 1000.0);
    std::cout << std::format("  Median: {:.2f} μs\n", radial_median.count() / 1000.0);
    std::cout << std::format("  Min:    {:.2f} μs\n", radial_min.count() / 1000.0);
    std::cout << std::format("  Max:    {:.2f} μs\n", radial_max.count() / 1000.0);

    std::cout << std::format("\nMeshed Algorithm:\n");
    std::cout << std::format("  Mean:   {:.2f} μs\n", meshed_mean.count() / 1000.0);
    std::cout << std::format("  Median: {:.2f} μs\n", meshed_median.count() / 1000.0);
    std::cout << std::format("  Min:    {:.2f} μs\n", meshed_min.count() / 1000.0);
    std::cout << std::format("  Max:    {:.2f} μs\n", meshed_max.count() / 1000.0);

    std::cout << std::format("\nOverhead: {:+.2f}%\n", time_overhead);
    std::cout << std::string(60, '=') << "\n\n";
}

} // namespace power_grid_model::benchmark

namespace {
using namespace power_grid_model;
namespace fs = std::filesystem;

struct TestCase {
    std::string name;
    fs::path json_path;
};

std::vector<TestCase> discover_test_cases(fs::path const& benchmark_dir) {
    std::vector<TestCase> cases;

    if (!fs::exists(benchmark_dir)) {
        std::cerr << std::format("Benchmark directory not found: {}\n", benchmark_dir.string());
        return cases;
    }

    for (auto const& dir_entry : fs::directory_iterator(benchmark_dir)) {
        if (!dir_entry.is_directory()) {
            continue;
        }

        auto const folder_name = dir_entry.path().filename().string();
        auto json_file = dir_entry.path() / "input.json";

        if (!fs::exists(json_file)) {
            json_file = dir_entry.path() / (folder_name + ".json");
        }

        if (fs::exists(json_file)) {
            cases.push_back({folder_name, json_file});
        }
    }

    std::sort(cases.begin(), cases.end(), [](TestCase const& a, TestCase const& b) { return a.name < b.name; });

    return cases;
}

void print_header() {
    std::cout << std::string(80, '=') << "\n";
    std::cout << "Observability Algorithm Performance Benchmark\n";
    std::cout << "Comparing Radial vs Meshed Algorithm on Radial Networks\n";
    std::cout << std::string(80, '=') << "\n\n";
}

void print_summary(std::vector<TestCase> const& cases, Idx successful_runs) {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "Summary\n";
    std::cout << std::string(80, '=') << "\n";
    std::cout << std::format("Total test cases discovered: {}\n", cases.size());
    std::cout << std::format("Successful benchmark runs:   {}\n", successful_runs);

    if (successful_runs == 0) {
        std::cout << "\nNo benchmarks were executed.\n";
        std::cout << "This requires extending MainModel with benchmark data access.\n";
        std::cout << "See OBSERVABILITY_BENCHMARK_README.md for implementation options.\n";
    }

    std::cout << std::string(80, '=') << "\n";
}

} // anonymous namespace

int main(int argc, char** argv) {
    using namespace power_grid_model;
    using namespace power_grid_model::benchmark;

    // Parse command-line arguments
    fs::path benchmark_dir = "tests/data/benchmark/observability_benchmark";
    Idx n_iterations = 5; // Default

    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "-h" || arg1 == "--help") {
            std::cout << "Usage: " << argv[0] << " [BENCHMARK_DIR] [ITERATIONS]\n\n";
            std::cout << "Arguments:\n";
            std::cout << "  BENCHMARK_DIR  Path to benchmark data directory (default: "
                         "tests/data/benchmark/observability_benchmark)\n";
            std::cout << "  ITERATIONS     Number of iterations per algorithm (default: 5)\n\n";
            return 0;
        }
        benchmark_dir = argv[1];
    }
    if (argc > 2) {
        n_iterations = std::stoll(argv[2]);
    }

    print_header();

    std::cout << std::format("Benchmark directory: {}\n", benchmark_dir.string());
    std::cout << std::format("Iterations per test: {}\n\n", n_iterations);

    // Discover test cases
    auto test_cases = discover_test_cases(benchmark_dir);

    if (test_cases.empty()) {
        std::cerr << "No test cases found.\n";
        return 1;
    }

    std::cout << std::format("Test cases: {}\n", test_cases.size());
    for (auto const& test_case : test_cases) {
        std::cout << std::format("  - {}\n", test_case.name);
    }
    std::cout << "\n";

    Idx successful_runs = 0;

    // Run benchmarks on each test case
    for (auto& test_case : test_cases) {
        std::cout << std::string(80, '-') << "\n";
        std::cout << std::format("Test Case: {}\n", test_case.name);
        std::cout << std::format("File:      {}\n", test_case.json_path.string());
        std::cout << std::string(80, '-') << "\n";

        try {
            std::cout << "Loading test data...\n";

            // Read JSON file
            std::ifstream json_file(test_case.json_path);
            if (!json_file) {
                throw std::runtime_error(std::format("Failed to open: {}", test_case.json_path.string()));
            }

            nlohmann::json json_data;
            json_file >> json_data;
            json_file.close();

            // Parse JSON into InputData structure
            InputData input;
            auto const& data = json_data["data"];

            // Parse nodes
            if (data.contains("node")) {
                for (auto const& node_json : data["node"]) {
                    NodeInput node{};
                    node.id = node_json["id"];
                    node.u_rated = node_json["u_rated"];
                    input.node.push_back(node);
                }
            }

            // Parse lines
            if (data.contains("line")) {
                for (auto const& line_json : data["line"]) {
                    LineInput line{};
                    line.id = line_json["id"];
                    line.from_node = line_json["from_node"];
                    line.to_node = line_json["to_node"];
                    line.from_status = line_json["from_status"];
                    line.to_status = line_json["to_status"];
                    line.r1 = line_json["r1"];
                    line.x1 = line_json["x1"];
                    line.c1 = line_json["c1"];
                    line.tan1 = line_json["tan1"];
                    input.line.push_back(line);
                }
            }

            // Parse transformers
            if (data.contains("transformer")) {
                for (auto const& trafo_json : data["transformer"]) {
                    TransformerInput trafo{};
                    trafo.id = trafo_json["id"];
                    trafo.from_node = trafo_json["from_node"];
                    trafo.to_node = trafo_json["to_node"];
                    trafo.from_status = trafo_json["from_status"];
                    trafo.to_status = trafo_json["to_status"];
                    trafo.u1 = trafo_json["u1"];
                    trafo.u2 = trafo_json["u2"];
                    trafo.sn = trafo_json["sn"];
                    trafo.uk = trafo_json["uk"];
                    trafo.pk = trafo_json["pk"];
                    trafo.i0 = trafo_json["i0"];
                    trafo.p0 = trafo_json["p0"];
                    trafo.winding_from = trafo_json["winding_from"];
                    trafo.winding_to = trafo_json["winding_to"];
                    trafo.clock = trafo_json["clock"];
                    trafo.tap_side = trafo_json["tap_side"];
                    trafo.tap_pos = trafo_json["tap_pos"];
                    trafo.tap_min = trafo_json.value("tap_min", na_IntS);
                    trafo.tap_max = trafo_json.value("tap_max", na_IntS);
                    trafo.tap_nom = trafo_json.value("tap_nom", na_IntS);
                    trafo.tap_size = trafo_json["tap_size"];
                    input.transformer.push_back(trafo);
                }
            }

            // Parse sources
            if (data.contains("source")) {
                for (auto const& source_json : data["source"]) {
                    SourceInput source{};
                    source.id = source_json["id"];
                    source.node = source_json["node"];
                    source.status = source_json["status"];
                    source.u_ref = source_json["u_ref"];
                    source.sk = source_json.value("sk", power_grid_model::nan);
                    source.rx_ratio = source_json.value("rx_ratio", power_grid_model::nan);
                    input.source.push_back(source);
                }
            }

            // Parse loads
            if (data.contains("sym_load")) {
                for (auto const& load_json : data["sym_load"]) {
                    SymLoadGenInput load{};
                    load.id = load_json["id"];
                    load.node = load_json["node"];
                    load.status = load_json["status"];
                    load.type = load_json["type"];
                    load.p_specified = load_json["p_specified"];
                    load.q_specified = load_json["q_specified"];
                    input.sym_load.push_back(load);
                }
            }

            // Parse voltage sensors
            if (data.contains("sym_voltage_sensor")) {
                for (auto const& sensor_json : data["sym_voltage_sensor"]) {
                    SymVoltageSensorInput sensor{};
                    sensor.id = sensor_json["id"];
                    sensor.measured_object = sensor_json["measured_object"];
                    sensor.u_measured = sensor_json["u_measured"];
                    sensor.u_sigma = sensor_json["u_sigma"];
                    input.sym_voltage_sensor.push_back(sensor);
                }
            }

            // Parse power sensors
            if (data.contains("sym_power_sensor")) {
                for (auto const& sensor_json : data["sym_power_sensor"]) {
                    SymPowerSensorInput sensor{};
                    sensor.id = sensor_json["id"];
                    sensor.measured_object = sensor_json["measured_object"];
                    sensor.measured_terminal_type = sensor_json["measured_terminal_type"];
                    sensor.power_sigma = sensor_json.value("power_sigma", power_grid_model::nan);
                    sensor.p_measured = sensor_json["p_measured"];
                    sensor.q_measured = sensor_json["q_measured"];
                    sensor.p_sigma = sensor_json.value("p_sigma", power_grid_model::nan);
                    sensor.q_sigma = sensor_json.value("q_sigma", power_grid_model::nan);
                    input.sym_power_sensor.push_back(sensor);
                }
            }

            std::cout << std::format("Loaded {} nodes, {} lines, {} transformers, {} sensors\n", input.node.size(),
                                     input.line.size(), input.transformer.size(),
                                     input.sym_voltage_sensor.size() + input.sym_power_sensor.size());

            // Create model from loaded data
            static constexpr MathSolverDispatcher math_solver_dispatcher{
                math_solver::math_solver_tag<math_solver::MathSolver>{}};
            MainModel model{50.0, input.get_dataset(), math_solver_dispatcher};

            std::cout << "Model created successfully\n";
            std::cout << "Running benchmark...\n\n";

            // Run benchmark
            model.get_impl_for_benchmark().run_observability_benchmark<symmetric_t>(
                benchmark_observability_algorithms<symmetric_t>, n_iterations);

            // Results are already printed by benchmark_observability_algorithms
            successful_runs++;

        } catch (std::exception const& e) {
            std::cerr << std::format("\nError processing test case: {}\n", e.what());
            std::cerr << "Continuing with next test case...\n";
        }

        std::cout << "\n";
    }

    print_summary(test_cases, successful_runs);

    return 0;
}
