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
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#ifdef __linux__
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace power_grid_model::benchmark {

/**
 * @brief Get current memory usage in kilobytes
 * Returns RSS (Resident Set Size) on Linux, 0 on other platforms
 */
inline size_t get_memory_usage_kb() {
#ifdef __linux__
    // Read from /proc/self/status for accurate RSS
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string key;
            size_t value;
            std::string unit;
            iss >> key >> value >> unit;
            return value; // Already in kB
        }
    }
#endif
    return 0;
}

/**
 * @brief Get peak (maximum) memory usage in kilobytes
 */
inline size_t get_peak_memory_kb() {
#ifdef __linux__
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return static_cast<size_t>(usage.ru_maxrss); // In kB on Linux
#else
    return 0;
#endif
}

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
 * @return Tuple: {radial_mean_ns, meshed_mean_ns, radial_mem_kb, meshed_mem_kb}
 */
template <symmetry_tag sym>
inline std::tuple<double, double, size_t, size_t>
benchmark_observability_algorithms(YBus<sym> const& y_bus, math_solver::MeasuredValues<sym> const& measured_values,
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

    // Memory measurement
    size_t mem_before_radial = get_memory_usage_kb();
    size_t mem_before_meshed = 0;
    size_t mem_after_radial = 0;
    size_t mem_after_meshed = 0;

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
    mem_after_radial = get_memory_usage_kb();

    // Benchmark meshed algorithm
    std::cout << "Benchmarking meshed algorithm...\n";
    mem_before_meshed = get_memory_usage_kb();
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
    mem_after_meshed = get_memory_usage_kb();

    // Restore original setting
    topo.is_radial = original_is_radial;

    // Calculate and display statistics
    if (radial_times.empty() || meshed_times.empty()) {
        std::cout << "\nBenchmark failed - one or both algorithms did not complete\n";
        return {0.0, 0.0, 0, 0};
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

    // Calculate memory usage
    size_t radial_mem_delta = (mem_after_radial > mem_before_radial) ? (mem_after_radial - mem_before_radial) : 0;
    size_t meshed_mem_delta = (mem_after_meshed > mem_before_meshed) ? (mem_after_meshed - mem_before_meshed) : 0;
    double mem_overhead = (radial_mem_delta > 0)
                              ? ((static_cast<double>(meshed_mem_delta) - radial_mem_delta) * 100.0 / radial_mem_delta)
                              : 0.0;

    double time_overhead = ((meshed_mean.count() - radial_mean.count()) * 100.0) / radial_mean.count();

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Results:\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << std::format("Radial Algorithm:\n");
    std::cout << std::format("  Mean:   {:.2f} μs\n", radial_mean.count() / 1000.0);
    std::cout << std::format("  Median: {:.2f} μs\n", radial_median.count() / 1000.0);
    std::cout << std::format("  Min:    {:.2f} μs\n", radial_min.count() / 1000.0);
    std::cout << std::format("  Max:    {:.2f} μs\n", radial_max.count() / 1000.0);
    if (radial_mem_delta > 0) {
        std::cout << std::format("  Memory: {:.1f} MB\n", radial_mem_delta / 1024.0);
    }

    std::cout << std::format("\nMeshed Algorithm:\n");
    std::cout << std::format("  Mean:   {:.2f} μs\n", meshed_mean.count() / 1000.0);
    std::cout << std::format("  Median: {:.2f} μs\n", meshed_median.count() / 1000.0);
    std::cout << std::format("  Min:    {:.2f} μs\n", meshed_min.count() / 1000.0);
    std::cout << std::format("  Max:    {:.2f} μs\n", meshed_max.count() / 1000.0);
    if (meshed_mem_delta > 0) {
        std::cout << std::format("  Memory: {:.1f} MB\n", meshed_mem_delta / 1024.0);
    }

    std::cout << std::format("\nTime Overhead: {:+.2f}%\n", time_overhead);
    if (radial_mem_delta > 0 && meshed_mem_delta > 0) {
        std::cout << std::format("Memory Overhead: {:+.2f}%\n", mem_overhead);
    }
    std::cout << std::string(60, '=') << "\n\n";

    return {radial_mean.count(), meshed_mean.count(), radial_mem_delta, meshed_mem_delta};
}

} // namespace power_grid_model::benchmark

namespace {
using namespace power_grid_model;
namespace fs = std::filesystem;

enum class BenchmarkMode {
    json_data,      // Load from JSON files
    generated_grid, // Single generated grid
    scaling_study   // Scaling experiment with varying grid sizes
};

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

void print_header(BenchmarkMode mode) {
    std::cout << std::string(80, '=') << "\n";
    std::cout << "Observability Algorithm Performance Benchmark\n";
    std::cout << "Comparing Radial vs Meshed Algorithm on Radial Networks\n";
    std::cout << std::string(80, '=') << "\n";

    std::cout << "Mode: ";
    switch (mode) {
    case BenchmarkMode::json_data:
        std::cout << "JSON Data Loading\n";
        break;
    case BenchmarkMode::generated_grid:
        std::cout << "Generated Grid\n";
        break;
    case BenchmarkMode::scaling_study:
        std::cout << "Scaling Study\n";
        break;
    }
    std::cout << "\n";
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

void print_summary_simple(Idx successful_runs) {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "Summary\n";
    std::cout << std::string(80, '=') << "\n";
    std::cout << std::format("Successful benchmark runs: {}\n", successful_runs);
    std::cout << std::string(80, '=') << "\n";
}

std::tuple<double, double, size_t, size_t> run_benchmark_on_generated_grid(benchmark::Option const& grid_option,
                                                                           Idx n_iterations) {
    using namespace power_grid_model::benchmark;

    std::cout << std::string(80, '-') << "\n";
    std::cout << "Generating Grid\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::format("Grid parameters:\n");
    std::cout << std::format("  MV feeders:              {}\n", grid_option.n_mv_feeder);
    std::cout << std::format("  Nodes per MV feeder:     {}\n", grid_option.n_node_per_mv_feeder);
    std::cout << std::format("  LV feeders:              {}\n", grid_option.n_lv_feeder);
    std::cout << std::format("  Connections per LV:      {}\n", grid_option.n_connection_per_lv_feeder);
    std::cout << std::format("  Has measurements:        {}\n", grid_option.has_measurements ? "Yes" : "No");
    std::cout << std::format("  Has MV ring:             {}\n", grid_option.has_mv_ring ? "Yes" : "No");
    std::cout << std::format("  Has LV ring:             {}\n", grid_option.has_lv_ring ? "Yes" : "No");

    FictionalGridGenerator generator;
    generator.generate_grid(grid_option);

    auto const& input = generator.input_data();
    std::cout << std::format("\nGenerated grid with {} nodes, {} lines, {} transformers, {} sensors\n",
                             input.node.size(), input.line.size(), input.transformer.size(),
                             input.sym_voltage_sensor.size() + input.sym_power_sensor.size());

    // Create model from generated data
    static constexpr MathSolverDispatcher math_solver_dispatcher{
        math_solver::math_solver_tag<math_solver::MathSolver>{}};
    MainModel model{50.0, input.get_dataset(), math_solver_dispatcher};

    std::cout << "Model created successfully\n";
    std::cout << "Running benchmark...\n\n";

    // Run benchmark
    auto [radial_mean_ns, meshed_mean_ns, radial_mem_kb, meshed_mem_kb] =
        model.get_impl_for_benchmark().run_observability_benchmark<symmetric_t>(
            benchmark_observability_algorithms<symmetric_t>, n_iterations);

    return std::make_tuple(radial_mean_ns, meshed_mean_ns, radial_mem_kb, meshed_mem_kb);
}

void run_scaling_study(Idx n_iterations, Idx max_nodes = 0) {
    using namespace power_grid_model::benchmark;

    std::cout << "\nRunning Scaling Study\n";
    if (max_nodes > 0) {
        std::cout << std::format("Varying grid size from ~10 to ~{} nodes\n", max_nodes);
        std::cout << "Running filtered test set\n\n";
    } else {
        std::cout << "Varying grid size from ~10 to ~10,000 nodes\n";
        std::cout << "Testing multiple feeder configurations per size\n\n";
    }

    // Define test configurations for scaling study
    // Format: target_size, feeders, nodes_per_feeder (all +1 for source node)
    struct ScalingConfig {
        std::string name;
        Idx n_mv_feeder;
        Idx n_node_per_mv_feeder;
        Idx n_lv_feeder;
        Idx n_connection_per_lv_feeder;
        Idx approx_nodes;
    };

    std::vector<ScalingConfig> configs = {
        // Exact sizes as specified: 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 500, 1000, 5000, 10000
        {"10 nodes", 2, 4, 0, 0, 10},       {"20 nodes", 3, 6, 0, 0, 20},        {"30 nodes", 3, 9, 0, 0, 30},
        {"40 nodes", 4, 9, 0, 0, 40},       {"50 nodes", 5, 9, 0, 0, 50},        {"60 nodes", 5, 11, 0, 0, 60},
        {"70 nodes", 6, 11, 0, 0, 70},      {"80 nodes", 7, 11, 0, 0, 80},       {"90 nodes", 8, 11, 0, 0, 90},
        {"100 nodes", 10, 10, 0, 0, 102},   {"200 nodes", 20, 10, 0, 0, 202},    {"500 nodes", 25, 20, 0, 0, 502},
        {"1000 nodes", 20, 50, 0, 0, 1002}, {"5000 nodes", 50, 100, 0, 0, 5002}, {"10000 nodes", 100, 100, 0, 0, 10002},
    };

    Idx successful_runs = 0;
    Idx total_configs = 0; // Track total configs to be run

    // Store results for summary table
    struct BenchmarkResult {
        std::string name;
        Idx nodes;
        Idx feeders;
        Idx nodes_per_feeder;
        double radial_mean_us;
        double meshed_mean_us;
        double overhead_pct;
        size_t radial_mem_mb;
        size_t meshed_mem_mb;
        double mem_overhead_pct;
    };
    std::vector<BenchmarkResult> results;

    // Count total configs to run (for summary)
    for (auto const& config : configs) {
        if (max_nodes > 0 && config.approx_nodes > max_nodes) {
            continue;
        }
        total_configs++;
    }

    for (auto const& config : configs) {
        // Filter by max_nodes if specified
        if (max_nodes > 0 && config.approx_nodes > max_nodes) {
            continue; // Skip this config
        }

        Option grid_option{
            .n_node_total_specified = config.approx_nodes, // Set to actual target
            .n_mv_feeder = config.n_mv_feeder,
            .n_node_per_mv_feeder = config.n_node_per_mv_feeder,
            .n_lv_feeder = config.n_lv_feeder,
            .n_connection_per_lv_feeder = config.n_connection_per_lv_feeder,
            .n_parallel_hv_mv_transformer = 0, // Will be calculated
            .n_lv_grid = 0,                    // Will be calculated
            .ratio_lv_grid = 0.0,              // Will be calculated
            .has_mv_ring = false,              // Keep simple radial for observability
            .has_lv_ring = false,
            .has_tap_changer = false,
            .has_measurements = true, // Required for observability
            .has_fault = false,
            .has_tap_regulator = false,
        };

        try {
            std::cout << std::format("\n** Scaling Test: {} (target: ~{} nodes) **\n", config.name,
                                     config.approx_nodes);
            auto [radial_mean_ns, meshed_mean_ns, radial_mem_kb, meshed_mem_kb] =
                run_benchmark_on_generated_grid(grid_option, n_iterations);

            // Get actual node count from the generated grid
            FictionalGridGenerator temp_gen;
            temp_gen.generate_grid(grid_option);
            Idx actual_nodes = static_cast<Idx>(temp_gen.input_data().node.size());

            // Store result
            double radial_mean_us = radial_mean_ns / 1000.0;
            double meshed_mean_us = meshed_mean_ns / 1000.0;
            double overhead_pct = ((meshed_mean_us - radial_mean_us) * 100.0) / radial_mean_us;

            size_t radial_mem_mb = radial_mem_kb / 1024;
            size_t meshed_mem_mb = meshed_mem_kb / 1024;
            double mem_overhead_pct =
                (radial_mem_kb > 0) ? ((static_cast<double>(meshed_mem_kb) - radial_mem_kb) * 100.0 / radial_mem_kb)
                                    : 0.0;

            results.push_back({config.name, actual_nodes, config.n_mv_feeder, config.n_node_per_mv_feeder,
                               radial_mean_us, meshed_mean_us, overhead_pct, radial_mem_mb, meshed_mem_mb,
                               mem_overhead_pct});

            successful_runs++;
        } catch (std::exception const& e) {
            std::cerr << std::format("\nError in scaling test '{}': {}\n", config.name, e.what());
            std::cerr << "Continuing with next test...\n";
        }
    }

    // Print comprehensive summary table
    std::cout << "\n" << std::string(150, '=') << "\n";
    std::cout << "COMPREHENSIVE SCALING STUDY SUMMARY\n";
    std::cout << std::string(150, '=') << "\n\n";

    if (results.empty()) {
        std::cout << "No successful benchmark runs to report.\n";
        return;
    }

    // Print table headers
    std::cout << std::setw(30) << std::left << "Configuration" << std::setw(8) << std::right << "Nodes" << std::setw(10)
              << "Feeders" << std::setw(12) << "N/Feeder" << std::setw(15) << "Time (μs)" << std::setw(15) << "Time OH%"
              << std::setw(12) << "Mem (MB)" << std::setw(12) << "Mem OH%" << std::setw(10) << "Speedup\n";
    std::cout << std::string(150, '-') << "\n";
    std::cout << std::setw(30) << std::left << "" << std::setw(8) << std::right << "" << std::setw(10) << ""
              << std::setw(12) << "" << std::setw(8) << "Radial" << std::setw(7) << "Meshed" << std::setw(15) << ""
              << std::setw(6) << "R" << std::setw(6) << "M" << std::setw(12) << "" << std::setw(10) << "\n";
    std::cout << std::string(150, '-') << "\n";

    // Print each result
    for (const auto& result : results) {
        double speedup = result.meshed_mean_us / result.radial_mean_us;
        std::cout << std::setw(30) << std::left << result.name << std::setw(8) << std::right << result.nodes
                  << std::setw(10) << result.feeders << std::setw(12) << result.nodes_per_feeder << std::setw(8)
                  << std::fixed << std::setprecision(1) << result.radial_mean_us << std::setw(7) << std::fixed
                  << std::setprecision(1) << result.meshed_mean_us << std::setw(15) << std::fixed
                  << std::setprecision(1) << result.overhead_pct << std::setw(6) << result.radial_mem_mb << std::setw(6)
                  << result.meshed_mem_mb << std::setw(12) << std::fixed << std::setprecision(1)
                  << result.mem_overhead_pct << std::setw(10) << std::fixed << std::setprecision(1) << speedup << "x\n";
    }

    std::cout << std::string(150, '=') << "\n";
    std::cout << std::format("Successfully completed {}/{} scaling configurations\n", successful_runs, total_configs);
    std::cout << std::string(150, '=') << "\n";
}

} // anonymous namespace

int main(int argc, char** argv) {
    using namespace power_grid_model;
    using namespace power_grid_model::benchmark;

    // Parse command-line arguments
    BenchmarkMode mode = BenchmarkMode::json_data;
    fs::path benchmark_dir = "tests/data/benchmark/observability_benchmark";
    Idx n_iterations = 10; // Default: 10 runs per configuration
    Idx n_mv_feeder = 10;
    Idx n_node_per_mv_feeder = 20;
    Idx n_lv_feeder = 5;
    Idx n_connection_per_lv_feeder = 10;
    Idx max_nodes = 0; // 0 means no limit

    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "-h" || arg1 == "--help") {
            std::cout << "Usage: " << argv[0] << " [MODE] [OPTIONS...]\n\n";
            std::cout << "Modes:\n";
            std::cout << "  json [DIR] [ITERATIONS]\n";
            std::cout << "    Load test cases from JSON files in DIR\n";
            std::cout << "    DIR:        Directory path (default: tests/data/benchmark/observability_benchmark)\n";
            std::cout << "    ITERATIONS: Number of iterations per algorithm (default: 10)\n\n";
            std::cout << "  generated [ITERATIONS] [MV_FEEDERS] [NODES_PER_MV] [LV_FEEDERS] [CONN_PER_LV]\n";
            std::cout << "    Run benchmark on a single generated grid\n";
            std::cout << "    ITERATIONS:   Number of iterations per algorithm (default: 10)\n";
            std::cout << "    MV_FEEDERS:   Number of MV feeders (default: 10)\n";
            std::cout << "    NODES_PER_MV: Nodes per MV feeder (default: 20)\n";
            std::cout << "    LV_FEEDERS:   Number of LV feeders (default: 5)\n";
            std::cout << "    CONN_PER_LV:  Connections per LV feeder (default: 10)\n\n";
            std::cout << "  scaling [ITERATIONS] [MAX_NODES]\n";
            std::cout << "    Run scaling study with predefined grid sizes\n";
            std::cout << "    ITERATIONS: Number of iterations per algorithm (default: 10)\n";
            std::cout << "    MAX_NODES:  Maximum node count to test (default: all, 0 means no limit)\n";
            std::cout << "                Use 100 to test only sub-100 node grids (10, 20, 30, ..., 90)\n\n";
            std::cout << "Examples:\n";
            std::cout << "  " << argv[0]
                      << " scaling           # Full scaling study (10-10,000 nodes) with 10 iterations\n";
            std::cout << "  " << argv[0] << " scaling 20        # Full scaling study with 20 iterations\n";
            std::cout << "  " << argv[0] << " scaling 5 100     # Only sub-100 node tests with 5 iterations\n";
            std::cout << "  " << argv[0] << " scaling 10 1000   # Tests up to 1000 nodes with 10 iterations\n";
            std::cout << "  " << argv[0] << " generated 10 5 20 # Generate 5 feeders × 20 nodes\n\n";
            std::cout << "If no mode is specified, 'json' mode is used with default parameters.\n\n";
            return 0;
        }

        if (arg1 == "json") {
            mode = BenchmarkMode::json_data;
            if (argc > 2) {
                benchmark_dir = argv[2];
            }
            if (argc > 3) {
                n_iterations = std::stoll(argv[3]);
            }
        } else if (arg1 == "generated") {
            mode = BenchmarkMode::generated_grid;
            if (argc > 2) {
                n_iterations = std::stoll(argv[2]);
            }
            if (argc > 3) {
                n_mv_feeder = std::stoll(argv[3]);
            }
            if (argc > 4) {
                n_node_per_mv_feeder = std::stoll(argv[4]);
            }
            if (argc > 5) {
                n_lv_feeder = std::stoll(argv[5]);
            }
            if (argc > 6) {
                n_connection_per_lv_feeder = std::stoll(argv[6]);
            }
        } else if (arg1 == "scaling") {
            mode = BenchmarkMode::scaling_study;
            if (argc > 2) {
                n_iterations = std::stoll(argv[2]);
            }
            if (argc > 3) {
                max_nodes = std::stoll(argv[3]);
            }
        } else {
            // Assume old-style arguments: [BENCHMARK_DIR] [ITERATIONS]
            mode = BenchmarkMode::json_data;
            benchmark_dir = argv[1];
            if (argc > 2) {
                n_iterations = std::stoll(argv[2]);
            }
        }
    }

    print_header(mode);

    // Handle different benchmark modes
    if (mode == BenchmarkMode::scaling_study) {
        run_scaling_study(n_iterations, max_nodes);
        return 0;
    }

    if (mode == BenchmarkMode::generated_grid) {
        Idx approx_nodes = n_mv_feeder * n_node_per_mv_feeder + n_lv_feeder * n_connection_per_lv_feeder * 2 + 2;
        Option grid_option{
            .n_node_total_specified = approx_nodes, // Set to actual target
            .n_mv_feeder = n_mv_feeder,
            .n_node_per_mv_feeder = n_node_per_mv_feeder,
            .n_lv_feeder = n_lv_feeder,
            .n_connection_per_lv_feeder = n_connection_per_lv_feeder,
            .n_parallel_hv_mv_transformer = 0, // Will be calculated
            .n_lv_grid = 0,                    // Will be calculated
            .ratio_lv_grid = 0.0,              // Will be calculated
            .has_mv_ring = false,
            .has_lv_ring = false,
            .has_tap_changer = false,
            .has_measurements = true, // Required for observability
            .has_fault = false,
            .has_tap_regulator = false,
        };

        try {
            run_benchmark_on_generated_grid(grid_option, n_iterations);
            print_summary_simple(1);
        } catch (std::exception const& e) {
            std::cerr << std::format("\nError: {}\n", e.what());
            print_summary_simple(0);
            return 1;
        }
        return 0;
    }

    // JSON mode
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
