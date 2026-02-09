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
    Idx n_nodes;
};

std::vector<TestCase> discover_test_cases() {
    std::vector<TestCase> cases;

    // Test cases with similar sizes to the original DTC and SCH networks
    cases.push_back({"Radial-120-nodes", 120});
    cases.push_back({"Radial-109-nodes", 109});

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
    Idx n_iterations = 5; // Default

    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "-h" || arg1 == "--help") {
            std::cout << "Usage: " << argv[0] << " [ITERATIONS]\n\n";
            std::cout << "Arguments:\n";
            std::cout << "  ITERATIONS     Number of iterations per algorithm (default: 5)\n\n";
            return 0;
        }
        n_iterations = std::stoll(argv[1]);
    }

    print_header();

    std::cout << std::format("Iterations per test: {}\n\n", n_iterations);

    // Discover test cases
    auto test_cases = discover_test_cases();

    std::cout << std::format("Test cases: {}\n", test_cases.size());
    for (auto const& test_case : test_cases) {
        std::cout << std::format("  - {} ({} nodes)\n", test_case.name, test_case.n_nodes);
    }
    std::cout << "\n";

    Idx successful_runs = 0;

    // Run benchmarks on each test case
    for (auto& test_case : test_cases) {
        std::cout << std::string(80, '-') << "\n";
        std::cout << std::format("Test Case: {}\n", test_case.name);
        std::cout << std::format("Nodes:     {}\n", test_case.n_nodes);
        std::cout << std::string(80, '-') << "\n";

        try {
            std::cout << "Generating test model...\n";

            // Configure grid generation options
            Option option{};
            option.n_node_total_specified = test_case.n_nodes;
            option.n_mv_feeder = 5;
            option.n_node_per_mv_feeder = 10;
            option.n_lv_feeder = 3;
            option.n_connection_per_lv_feeder = 4;
            option.has_mv_ring = false;     // Radial topology
            option.has_measurements = true; // Generate sensors for observability
            option.has_tap_changer = false;
            option.has_fault = false;

            // Create generator and generate grid
            FictionalGridGenerator generator;
            generator.generate_grid(option, 42); // Use seed 42 for reproducibility

            // Get input data and create model
            auto const& input = generator.input_data();
            static constexpr MathSolverDispatcher math_solver_dispatcher{
                math_solver::math_solver_tag<math_solver::MathSolver>{}};
            MainModel model{50.0, input.get_dataset(), math_solver_dispatcher};

            std::cout << std::format("Model created: {} nodes\n", input.node.size());
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
