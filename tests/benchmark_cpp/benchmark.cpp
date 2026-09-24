// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "fictional_grid_generator.hpp"

#include <power_grid_model_cpp/dataset.hpp>
#include <power_grid_model_cpp/logger.hpp>
#include <power_grid_model_cpp/meta_data.hpp>
#include <power_grid_model_cpp/model.hpp>
#include <power_grid_model_cpp/options.hpp>

#include <power_grid_model_c/basics.h>
#include <power_grid_model_c/dataset_definitions.h>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/logging.hpp>

#include <concepts>
#include <cstddef>
#include <exception>
#include <format>
#include <iterator>
#include <map>
#include <memory>
#include <print>
#include <sstream>
#include <string>
#include <utility>
#include <xstring>
#include <xutility>

namespace power_grid_model::benchmark {
namespace {
using power_grid_model_cpp::DatasetConst;
using power_grid_model_cpp::DatasetMutable;
using power_grid_model_cpp::Logger;
using power_grid_model_cpp::MetaData;
using power_grid_model_cpp::Model;
using power_grid_model_cpp::Options;

// accumulated timings per log event, mirroring the layout of the benchmark logger output
using Report = std::map<LogEvent, double>;

struct BenchmarkOptions {
    static constexpr Idx sequential = -1;

    PGM_CalculationType calculation_type{PGM_power_flow};
    PGM_SymmetryType calculation_symmetry{PGM_symmetric};
    PGM_CalculationMethod calculation_method{PGM_default_method};
    PGM_TapChangingStrategy tap_changing_strategy{PGM_tap_changing_strategy_disabled};

    double err_tol{1e-8};
    Idx max_iter{20};
    Idx threading{sequential};

    PGM_ShortCircuitVoltageScaling short_circuit_voltage_scaling{PGM_short_circuit_voltage_scaling_maximum};
};

constexpr double system_frequency = 50.0;

constexpr std::string to_string(LogEvent tag) {
    using enum LogEvent;
    using namespace std::string_literals;

    switch (tag) {
    case total:
        return "Total"s;
    case build_model:
        return "Build model"s;
    case total_single_calculation_in_thread:
        return "Total single calculation in thread"s;
    case total_batch_calculation_in_thread:
        return "Total batch calculation in thread"s;
    case copy_model:
        return "Copy model"s;
    case update_model:
        return "Update model"s;
    case restore_model:
        return "Restore model"s;
    case scenario_exception:
        return "Scenario exception"s;
    case recover_from_bad:
        return "Recover from bad"s;
    case prepare:
        return "Prepare"s;
    case create_math_solver:
        return "Create math solver"s;
    case math_calculation:
        return "Math Calculation"s; // TODO(mgovers): make capitalization consistent
    case math_solver:
        return "Math solver"s;
    case initialize_calculation:
        return "Initialize calculation"s;
    case preprocess_measured_value:
        return "Pre-process measured value"s; // TODO(mgovers): make plural
    case prepare_matrix:
        return "Prepare matrix"s;
    case prepare_matrix_including_prefactorization:
        return "Prepare matrix, including pre-factorization"s;
    case prepare_matrices:
        return "Prepare the matrices"s; // TODO(mgovers): combine or properly split up?
    case initialize_voltages:
        return "Initialize voltages"s;
    case calculate_rhs:
        return "Calculate rhs"s; // TODO(mgovers): capitalize?
    case prepare_lhs_rhs:
        return "Prepare LHS rhs"s;
    case solve_sparse_linear_equation:
        return "Solve sparse linear equation"s;
    case solve_sparse_linear_equation_prefactorized:
        return "Solve sparse linear equation (pre-factorized)"s;
    case iterate_unknown:
        return "Iterate unknown"s;
    case calculate_math_result:
        return "Calculate math result"s;
    case produce_output:
        return "Produce output"s;
    case iterative_pf_solver_max_num_iter:
        // return "Max number of iterations"s; // TODO(mgovers): different messages?
        [[fallthrough]];
    case max_num_iter:
        return "Max number of iterations"s; // TODO(mgovers): different messages?
    case unknown:
        [[fallthrough]];
    default:
        return "unknown"s;
    }
}

std::string make_key(LogEvent code) {
    auto key = std::format("{:04}.", std::to_underlying(code));
    for (size_t i = 0, n = key.length() - 1; i < n; ++i) {
        if (key[i] == '0') {
            break;
        }
        key += "\t";
    }
    key += to_string(code);
    return key;
}

// the benchmark logger emits one 'EVENT_CODE<TAB>VALUE' line per logged event
void merge_logger_output(Report& report, std::string const& logger_output) {
    std::istringstream stream{logger_output};
    for (std::string line; std::getline(stream, line);) {
        auto const separator = line.find('\t');
        if (separator == std::string::npos) {
            continue;
        }
        auto const code = static_cast<LogEvent>(std::stoi(line.substr(0, separator)));
        report[code] += std::stod(line.substr(separator + 1));
    }
}

void print_report(Report const& report) {
    for (auto const& [key, val] : report) {
        std::println("{}: {:g}", make_key(key), val);
    }
}

// the output components the fictional grid generator produces, per output dataset flavor
struct OutputComponents {
    PGM_MetaDataset const* dataset{};
    PGM_MetaComponent const* node{};
    PGM_MetaComponent const* transformer{};
    PGM_MetaComponent const* line{};
    PGM_MetaComponent const* source{};
    PGM_MetaComponent const* sym_load{};
    PGM_MetaComponent const* asym_load{};
    PGM_MetaComponent const* shunt{};
};

template <typename OutputDataType> OutputComponents output_components() {
    if constexpr (std::same_as<OutputDataType, ShortCircuitOutputData>) {
        return {.dataset = PGM_def_sc_output,
                .node = PGM_def_sc_output_node,
                .transformer = PGM_def_sc_output_transformer,
                .line = PGM_def_sc_output_line,
                .source = PGM_def_sc_output_source,
                .sym_load = PGM_def_sc_output_sym_load,
                .asym_load = PGM_def_sc_output_asym_load,
                .shunt = PGM_def_sc_output_shunt};
    } else if constexpr (std::same_as<OutputDataType, OutputData<symmetric_t>>) {
        return {.dataset = PGM_def_sym_output,
                .node = PGM_def_sym_output_node,
                .transformer = PGM_def_sym_output_transformer,
                .line = PGM_def_sym_output_line,
                .source = PGM_def_sym_output_source,
                .sym_load = PGM_def_sym_output_sym_load,
                .asym_load = PGM_def_sym_output_asym_load,
                .shunt = PGM_def_sym_output_shunt};
    } else {
        static_assert(std::same_as<OutputDataType, OutputData<asymmetric_t>>);
        return {.dataset = PGM_def_asym_output,
                .node = PGM_def_asym_output_node,
                .transformer = PGM_def_asym_output_transformer,
                .line = PGM_def_asym_output_line,
                .source = PGM_def_asym_output_source,
                .sym_load = PGM_def_asym_output_sym_load,
                .asym_load = PGM_def_asym_output_asym_load,
                .shunt = PGM_def_asym_output_shunt};
    }
}

DatasetConst make_input_dataset(InputData const& input) {
    DatasetConst dataset{MetaData::dataset_name(PGM_def_input), false, 1};
    auto const add = [&dataset](PGM_MetaComponent const* component, auto const& buffer) {
        dataset.add_buffer(MetaData::component_name(component), std::ssize(buffer), std::ssize(buffer), nullptr,
                           buffer.data());
    };
    add(PGM_def_input_node, input.node);
    add(PGM_def_input_transformer, input.transformer);
    add(PGM_def_input_line, input.line);
    add(PGM_def_input_source, input.source);
    add(PGM_def_input_sym_load, input.sym_load);
    add(PGM_def_input_asym_load, input.asym_load);
    add(PGM_def_input_shunt, input.shunt);
    add(PGM_def_input_sym_voltage_sensor, input.sym_voltage_sensor);
    add(PGM_def_input_asym_voltage_sensor, input.asym_voltage_sensor);
    add(PGM_def_input_sym_power_sensor, input.sym_power_sensor);
    add(PGM_def_input_asym_power_sensor, input.asym_power_sensor);
    add(PGM_def_input_fault, input.fault);
    add(PGM_def_input_transformer_tap_regulator, input.transformer_tap_regulator);
    return dataset;
}

template <typename OutputDataType> DatasetMutable make_output_dataset(OutputDataType& output) {
    auto const components = output_components<OutputDataType>();
    DatasetMutable dataset{MetaData::dataset_name(components.dataset), true, output.batch_size};
    auto const add = [&dataset, batch_size = output.batch_size](PGM_MetaComponent const* component, auto& buffer) {
        dataset.add_buffer(MetaData::component_name(component), std::ssize(buffer) / batch_size, std::ssize(buffer),
                           nullptr, buffer.data());
    };
    add(components.node, output.node);
    add(components.transformer, output.transformer);
    add(components.line, output.line);
    add(components.source, output.source);
    add(components.sym_load, output.sym_load);
    add(components.asym_load, output.asym_load);
    add(components.shunt, output.shunt);
    return dataset;
}

DatasetConst make_update_dataset(BatchData const& batch_data) {
    DatasetConst dataset{MetaData::dataset_name(PGM_def_update), true, batch_data.batch_size};
    if (batch_data.batch_size == 0) {
        return dataset;
    }
    auto const add = [&dataset, batch_size = batch_data.batch_size](PGM_MetaComponent const* component,
                                                                    auto const& buffer) {
        dataset.add_buffer(MetaData::component_name(component), std::ssize(buffer) / batch_size, std::ssize(buffer),
                           nullptr, buffer.data());
    };
    add(PGM_def_update_sym_load, batch_data.sym_load);
    add(PGM_def_update_asym_load, batch_data.asym_load);
    add(PGM_def_update_sym_power_sensor, batch_data.sym_power_sensor);
    add(PGM_def_update_asym_power_sensor, batch_data.asym_power_sensor);
    return dataset;
}

Options to_api_options(BenchmarkOptions const& benchmark_options) {
    Options options{};
    options.set_calculation_type(benchmark_options.calculation_type);
    options.set_calculation_method(benchmark_options.calculation_method);
    options.set_symmetric(benchmark_options.calculation_symmetry);
    options.set_err_tol(benchmark_options.err_tol);
    options.set_max_iter(benchmark_options.max_iter);
    options.set_threading(benchmark_options.threading);
    options.set_short_circuit_voltage_scaling(benchmark_options.short_circuit_voltage_scaling);
    options.set_tap_changing_strategy(benchmark_options.tap_changing_strategy);
    return options;
}

auto get_benchmark_run_title(Option const& option, BenchmarkOptions const& benchmark_options) {
    using namespace std::string_literals;
    auto const mv_ring_type = option.has_mv_ring ? "meshed grid"s : "radial grid"s;
    auto const sym_type = benchmark_options.calculation_symmetry == PGM_symmetric ? "symmetric"s : "asymmetric"s;
    auto const method = [calculation_method = benchmark_options.calculation_method] {
        switch (calculation_method) {
        case PGM_newton_raphson:
            return "Newton-Raphson method"s;
        case PGM_linear:
            return "Linear method"s;
        case PGM_linear_current:
            return "Linear current method"s;
        case PGM_iterative_current:
            return "Iterative current method"s;
        case PGM_iterative_linear:
            return "Iterative linear method"s;
        case PGM_iec60909:
            return "IEC 60909 method"s;
        default:
            throw MissingCaseForEnumError{"get_benchmark_run_title", calculation_method};
        }
    }();

    return std::format("============= Benchmark case: {}, {}, {} =============\n", mv_ring_type, sym_type, method);
}

struct PowerGridBenchmark {
    static constexpr auto single_scenario = -1;

    template <typename OutputDataType>
    void run_calculation(BenchmarkOptions const& benchmark_options, Idx batch_size) noexcept {
        if (!model) {
            std::println("\nNo main model available: skipping benchmark.");
            return;
        }

        auto output = generator.generate_output_data<OutputDataType>(batch_size);
        BatchData const batch_data = generator.generate_batch_input(batch_size, 0);
        std::println("Number of nodes: {}", generator.input_data().node.size());

        try {
            // calculate
            auto output_dataset = make_output_dataset(output);
            model->calculate(to_api_options(benchmark_options), output_dataset, make_update_dataset(batch_data));
        } catch (std::exception const& e) {
            std::println("\nAn exception was raised during execution: {}", e.what());
        }
    }

    void run_benchmark(Option const& option, BenchmarkOptions const& benchmark_options, Idx batch_size = single_scenario) {
        generator.generate_grid(option, 0);
        InputData const& input = generator.input_data();

        std::println("{}", get_benchmark_run_title(option, benchmark_options));

        auto const run = [this, &benchmark_options](Idx batch_size_) {
            switch (benchmark_options.calculation_type) {
            case PGM_short_circuit:
                run_calculation<ShortCircuitOutputData>(benchmark_options, batch_size_);
                break;
            case PGM_power_flow:
                [[fallthrough]];
            case PGM_state_estimation: {
                switch (benchmark_options.calculation_symmetry) {
                case PGM_symmetric:
                    run_calculation<OutputData<symmetric_t>>(benchmark_options, batch_size_);
                    break;
                case PGM_asymmetric:
                    run_calculation<OutputData<asymmetric_t>>(benchmark_options, batch_size_);
                    break;
                default:
                    throw MissingCaseForEnumError{"run_benchmark<calculation_symmetry>",
                                                  benchmark_options.calculation_symmetry};
                }
                break;
            }
            default:
                throw MissingCaseForEnumError{"run_benchmark<calculation_type>", benchmark_options.calculation_type};
            }
        };

        {
            std::println("*****Run with initialization*****");
            create_model(input);
            run(single_scenario);
            print_report(collect_report());
        }
        {
            std::println("\n*****Run without initialization*****");
            run(single_scenario);
            print_report(collect_report());
        }
        if (batch_size > 0) {
            std::println("\n*****Run with batch calculation*****");
            run(batch_size);
            print_report(collect_report());
        }

        std::println("\n");
    }

    void create_model(InputData const& input) {
        model = std::make_unique<Model>(system_frequency, make_input_dataset(input), logger);
    }

    Report collect_report() {
        Report report;
        merge_logger_output(report, logger.get_output());
        logger.clear();
        return report;
    }

    Logger logger{PGM_benchmark_logger};
    std::unique_ptr<Model> model;
    FictionalGridGenerator generator;
};
} // namespace
} // namespace power_grid_model::benchmark

int main(int /* argc */, char** /* argv */) {
    using enum power_grid_model::CalculationType;
    using enum power_grid_model::CalculationMethod;
    using enum power_grid_model::CalculationSymmetry;
    using enum power_grid_model::OptimizerType;

    power_grid_model::benchmark::PowerGridBenchmark benchmarker{};
    power_grid_model::benchmark::Option option{};

#ifndef NDEBUG
    option.n_node_total_specified = 200;
    option.n_mv_feeder = 3;
    option.n_node_per_mv_feeder = 6;
    option.n_lv_feeder = 2;
    option.n_connection_per_lv_feeder = 4;
    power_grid_model::Idx constexpr batch_size = 10;
#else
    option.n_node_total_specified = 1500;
    option.n_mv_feeder = 20;
    option.n_node_per_mv_feeder = 10;
    option.n_lv_feeder = 10;
    option.n_connection_per_lv_feeder = 40;
    power_grid_model::Idx constexpr batch_size = 1000;
#endif

    std::println("\n\n##### BENCHMARK POWER FLOW #####\n");
    option.has_measurements = false;
    option.has_fault = false;
    option.has_tap_changer = false;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .threading = 6},
                              batch_size);
    benchmarker.run_benchmark(
        option,
        {.calculation_type = PGM_power_flow, .calculation_symmetry = PGM_symmetric, .calculation_method = PGM_linear});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_linear_current});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_iterative_current,
                                       .max_iter = 100});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_asymmetric,
                                       .calculation_method = PGM_newton_raphson});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = PGM_power_flow, .calculation_symmetry = PGM_asymmetric, .calculation_method = PGM_linear});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_asymmetric,
                                       .calculation_method = PGM_linear_current});
    // benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
    //                                    .calculation_symmetry = PGM_asymmetric,
    //                                    .calculation_method = PGM_iterative_current,
    //                                    .max_iter = 100});

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_newton_raphson});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = PGM_power_flow, .calculation_symmetry = PGM_symmetric, .calculation_method = PGM_linear});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_linear_current});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_iterative_current,
                                       .max_iter = 100});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_asymmetric,
                                       .calculation_method = PGM_newton_raphson});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = PGM_power_flow, .calculation_symmetry = PGM_asymmetric, .calculation_method = PGM_linear});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
                                       .calculation_symmetry = PGM_asymmetric,
                                       .calculation_method = PGM_linear_current});
    // benchmarker.run_benchmark(option, {.calculation_type = PGM_power_flow,
    //                                    .calculation_symmetry = PGM_asymmetric,
    //                                    .calculation_method = PGM_iterative_current,
    //                                    .max_iter = 100});

    std::println("\n\n##### BENCHMARK POWER FLOW WITH AUTOMATIC TAP CHANGER #####\n");
    option.has_measurements = false;
    option.has_fault = false;
    option.has_tap_changer = true;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_fast_any_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_linear,
                               .tap_changing_strategy = PGM_tap_changing_strategy_fast_any_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_asymmetric,
                               .calculation_method = PGM_linear,
                               .tap_changing_strategy = PGM_tap_changing_strategy_fast_any_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_any_valid_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_fast_any_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_min_voltage_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_max_voltage_tap},
                              batch_size);
    benchmarker.run_benchmark(option, // TODO(mgovers): local_minimum is not exposed as a public API yet
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_min_voltage_tap},
                              batch_size);
    benchmarker.run_benchmark(option, // TODO(mgovers): local_minimum is not exposed as a public API yet
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_max_voltage_tap},
                              batch_size);

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .tap_changing_strategy = PGM_tap_changing_strategy_fast_any_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_linear,
                               .tap_changing_strategy = PGM_tap_changing_strategy_fast_any_tap},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_power_flow,
                               .calculation_symmetry = PGM_asymmetric,
                               .calculation_method = PGM_linear,
                               .tap_changing_strategy = PGM_tap_changing_strategy_fast_any_tap},
                              batch_size);

    std::println("\n\n##### BENCHMARK STATE ESTIMATION #####\n");
    option.has_measurements = true;
    option.has_fault = false;
    option.has_tap_changer = false;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_state_estimation,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_state_estimation,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_newton_raphson,
                               .threading = 6},
                              batch_size);
    benchmarker.run_benchmark(option, {.calculation_type = PGM_state_estimation,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_iterative_linear});
    // benchmarker.run_benchmark(option, {.calculation_type = PGM_state_estimation,
    //                                    .calculation_symmetry = PGM_asymmetric,
    //                                    .calculation_method = PGM_newton_raphson});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_state_estimation,
                                       .calculation_symmetry = PGM_asymmetric,
                                       .calculation_method = PGM_iterative_linear});

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(option, {.calculation_type = PGM_state_estimation,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_newton_raphson});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_state_estimation,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_iterative_linear});
    // benchmarker.run_benchmark(option, {.calculation_type = PGM_state_estimation,
    //                                    .calculation_symmetry = PGM_asymmetric,
    //                                    .calculation_method = PGM_newton_raphson});
    benchmarker.run_benchmark(option, {.calculation_type = PGM_state_estimation,
                                       .calculation_symmetry = PGM_asymmetric,
                                       .calculation_method = PGM_iterative_linear});

    std::println("\n\n##### BENCHMARK SHORT CIRCUIT #####\n");
    option.has_measurements = false;
    option.has_fault = true;
    option.has_tap_changer = false;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_short_circuit,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_iec60909},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = PGM_short_circuit,
                               .calculation_symmetry = PGM_symmetric,
                               .calculation_method = PGM_iec60909,
                               .threading = 6},
                              batch_size);
    benchmarker.run_benchmark(option, {.calculation_type = PGM_short_circuit,
                                       .calculation_symmetry = PGM_symmetric,
                                       .calculation_method = PGM_iec60909});

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(option, {.calculation_type = PGM_short_circuit,
                                       .calculation_symmetry = PGM_asymmetric,
                                       .calculation_method = PGM_iec60909});

    return 0;
}
