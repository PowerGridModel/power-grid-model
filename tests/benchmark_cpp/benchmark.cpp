// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "fictional_grid_generator.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/timer.hpp>
#include <power_grid_model/main_model.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>

#include <iomanip>
#include <iostream>
#include <random>

namespace power_grid_model::benchmark {
namespace {
MathSolverDispatcher const& get_math_solver_dispatcher() {
    static constexpr MathSolverDispatcher math_solver_dispatcher{math_solver::math_solver_tag<MathSolver>{}};
    return math_solver_dispatcher;
}

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
    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << static_cast<std::underlying_type_t<LogEvent>>(code) << ".";
    auto key = ss.str();
    for (size_t i = 0, n = key.length() - 1; i < n; ++i) {
        if (key[i] == '0') {
            break;
        }
        key += "\t";
    }
    key += to_string(code);
    return key;
}

auto get_benchmark_run_title(Option const& option, MainModelOptions const& model_options) {
    using namespace std::string_literals;
    auto const mv_ring_type = option.has_mv_ring ? "meshed grid"s : "radial grid"s;
    auto const sym_type =
        model_options.calculation_symmetry == CalculationSymmetry::symmetric ? "symmetric"s : "asymmetric"s;
    auto const method = [calculation_method = model_options.calculation_method] {
        using enum CalculationMethod;

        switch (calculation_method) {
        case newton_raphson:
            return "Newton-Raphson method"s;
        case linear:
            return "Linear method"s;
        case linear_current:
            return "Linear current method"s;
        case iterative_current:
            return "Iterative current method"s;
        case iterative_linear:
            return "Iterative linear method"s;
        case iec60909:
            return "IEC 60909 method"s;
        default:
            throw MissingCaseForEnumError{"get_benchmark_run_title", calculation_method};
        }
    }();

    return std::format("============= Benchmark case: {}, {}, {} =============\n", mv_ring_type, sym_type, method);
}

struct PowerGridBenchmark {
    static constexpr auto single_scenario = -1;

    PowerGridBenchmark()
        : main_model{
              std::make_unique<MainModel>(50.0, meta_data::meta_data_gen::meta_data, get_math_solver_dispatcher())} {}

    template <typename OutputDataType>
    void run_calculation(MainModelOptions model_options, Idx batch_size, CalculationInfo& info) noexcept {
        if (!main_model) {
            std::cout << "\nNo main model available: skipping benchmark.\n";
            return;
        }

        auto output = generator.generate_output_data<OutputDataType>(batch_size);
        BatchData const batch_data = generator.generate_batch_input(batch_size, 0);
        std::cout << "Number of nodes: " << generator.input_data().node.size() << '\n';

        try {
            // calculate
            main_model->calculate(model_options, output.get_dataset(), batch_data.get_dataset());
            main_model->calculation_info().merge_into(info);
        } catch (std::exception const& e) {
            std::cout << std::format("\nAn exception was raised during execution: {}\n", e.what());
        }
    }

    void run_benchmark(Option const& option, MainModelOptions const& model_options, Idx batch_size = single_scenario) {
        using enum CalculationType;
        using enum CalculationMethod;

        CalculationInfo info;
        generator.generate_grid(option, 0);
        InputData const& input = generator.input_data();

        std::cout << get_benchmark_run_title(option, model_options) << '\n';

        auto const run = [this, &model_options, &info](Idx batch_size_) {
            switch (model_options.calculation_type) {
            case short_circuit:
                run_calculation<ShortCircuitOutputData>(model_options, batch_size_, info);
                break;
            case power_flow:
                [[fallthrough]];
            case state_estimation: {
                switch (model_options.calculation_symmetry) {
                case CalculationSymmetry::symmetric:
                    run_calculation<OutputData<symmetric_t>>(model_options, batch_size_, info);
                    break;
                case CalculationSymmetry::asymmetric:
                    run_calculation<OutputData<asymmetric_t>>(model_options, batch_size_, info);
                    break;
                default:
                    throw MissingCaseForEnumError{"run_benchmark<calculation_symmetry>",
                                                  model_options.calculation_symmetry};
                }
                break;
            }
            default:
                throw MissingCaseForEnumError{"run_benchmark<calculation_type>", model_options.calculation_type};
            }
        };

        {
            std::cout << "*****Run with initialization*****\n";
            Timer const t_total{info, LogEvent::total};
            {
                Timer const t_build{info, LogEvent::build_model};
                main_model = std::make_unique<MainModel>(50.0, input.get_dataset(), get_math_solver_dispatcher());
            }
            run(single_scenario);
        }
        print_info(info);
        info.clear();
        {
            std::cout << "\n*****Run without initialization*****\n";
            Timer const t_total{info, LogEvent::total};
            run(single_scenario);
        }
        print_info(info);

        if (batch_size > 0) {
            info.clear();
            std::cout << "\n*****Run with batch calculation*****\n";
            Timer const t_total{info, LogEvent::total};
            run(batch_size);
        }
        print_info(info);

        std::cout << "\n\n";
    }

    static void print_info(CalculationInfo const& info) {
        for (auto const& [key, val] : info.report()) {
            std::cout << make_key(key) << ": " << val << '\n';
        }
    }

    std::unique_ptr<MainModel> main_model;
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

    std::cout << "\n\n##### BENCHMARK POWER FLOW #####\n\n";
    option.has_measurements = false;
    option.has_fault = false;
    option.has_tap_changer = false;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = symmetric, .calculation_method = newton_raphson},
        batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .threading = 6},
                              batch_size);
    benchmarker.run_benchmark(
        option, {.calculation_type = power_flow, .calculation_symmetry = symmetric, .calculation_method = linear});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = symmetric, .calculation_method = linear_current});
    benchmarker.run_benchmark(option, {.calculation_type = power_flow,
                                       .calculation_symmetry = symmetric,
                                       .calculation_method = iterative_current,
                                       .max_iter = 100});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = asymmetric, .calculation_method = newton_raphson});
    benchmarker.run_benchmark(
        option, {.calculation_type = power_flow, .calculation_symmetry = asymmetric, .calculation_method = linear});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = asymmetric, .calculation_method = linear_current});
    // benchmarker.run_benchmark(option, {.calculation_type = power_flow,
    //                                    .calculation_symmetry = asymmetric,
    //                                    .calculation_method = iterative_current,
    //                                    .max_iter = 100});

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = symmetric, .calculation_method = newton_raphson});
    benchmarker.run_benchmark(
        option, {.calculation_type = power_flow, .calculation_symmetry = symmetric, .calculation_method = linear});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = symmetric, .calculation_method = linear_current});
    benchmarker.run_benchmark(option, {.calculation_type = power_flow,
                                       .calculation_symmetry = symmetric,
                                       .calculation_method = iterative_current,
                                       .max_iter = 100});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = asymmetric, .calculation_method = newton_raphson});
    benchmarker.run_benchmark(
        option, {.calculation_type = power_flow, .calculation_symmetry = asymmetric, .calculation_method = linear});
    benchmarker.run_benchmark(
        option,
        {.calculation_type = power_flow, .calculation_symmetry = asymmetric, .calculation_method = linear_current});
    // benchmarker.run_benchmark(option, {.calculation_type = power_flow,
    //                                    .calculation_symmetry = asymmetric,
    //                                    .calculation_method = iterative_current,
    //                                    .max_iter = 100});

    std::cout << "\n\n##### BENCHMARK POWER FLOW WITH AUTOMATIC TAP CHANGER #####\n\n";
    option.has_measurements = false;
    option.has_fault = false;
    option.has_tap_changer = true;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = linear,
                               .optimizer_type = automatic_tap_adjustment},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = asymmetric,
                               .calculation_method = linear,
                               .optimizer_type = automatic_tap_adjustment},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment,
                               .optimizer_strategy = power_grid_model::OptimizerStrategy::any},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment,
                               .optimizer_strategy = power_grid_model::OptimizerStrategy::fast_any},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment,
                               .optimizer_strategy = power_grid_model::OptimizerStrategy::global_minimum},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment,
                               .optimizer_strategy = power_grid_model::OptimizerStrategy::global_maximum},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment,
                               .optimizer_strategy = power_grid_model::OptimizerStrategy::local_minimum},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment,
                               .optimizer_strategy = power_grid_model::OptimizerStrategy::local_maximum},
                              batch_size);

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .optimizer_type = automatic_tap_adjustment},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = symmetric,
                               .calculation_method = linear,
                               .optimizer_type = automatic_tap_adjustment},
                              batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = power_flow,
                               .calculation_symmetry = asymmetric,
                               .calculation_method = linear,
                               .optimizer_type = automatic_tap_adjustment},
                              batch_size);

    std::cout << "\n\n##### BENCHMARK STATE ESTIMATION #####\n\n";
    option.has_measurements = true;
    option.has_fault = false;
    option.has_tap_changer = false;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(
        option,
        {.calculation_type = state_estimation, .calculation_symmetry = symmetric, .calculation_method = newton_raphson},
        batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = state_estimation,
                               .calculation_symmetry = symmetric,
                               .calculation_method = newton_raphson,
                               .threading = 6},
                              batch_size);
    benchmarker.run_benchmark(option, {.calculation_type = state_estimation,
                                       .calculation_symmetry = symmetric,
                                       .calculation_method = iterative_linear});
    // benchmarker.run_benchmark(option, {.calculation_type = state_estimation,
    //                                    .calculation_symmetry = asymmetric,
    //                                    .calculation_method = newton_raphson});
    benchmarker.run_benchmark(option, {.calculation_type = state_estimation,
                                       .calculation_symmetry = asymmetric,
                                       .calculation_method = iterative_linear});

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(option, {.calculation_type = state_estimation,
                                       .calculation_symmetry = symmetric,
                                       .calculation_method = newton_raphson});
    benchmarker.run_benchmark(option, {.calculation_type = state_estimation,
                                       .calculation_symmetry = symmetric,
                                       .calculation_method = iterative_linear});
    // benchmarker.run_benchmark(option, {.calculation_type = state_estimation,
    //                                    .calculation_symmetry = asymmetric,
    //                                    .calculation_method = newton_raphson});
    benchmarker.run_benchmark(option, {.calculation_type = state_estimation,
                                       .calculation_symmetry = asymmetric,
                                       .calculation_method = iterative_linear});

    std::cout << "\n\n##### BENCHMARK SHORT CIRCUIT #####\n\n";
    option.has_measurements = false;
    option.has_fault = true;
    option.has_tap_changer = false;

    // radial
    option.has_mv_ring = false;
    option.has_lv_ring = false;
    benchmarker.run_benchmark(
        option, {.calculation_type = short_circuit, .calculation_symmetry = symmetric, .calculation_method = iec60909},
        batch_size);
    benchmarker.run_benchmark(option,
                              {.calculation_type = short_circuit,
                               .calculation_symmetry = symmetric,
                               .calculation_method = iec60909,
                               .threading = 6},
                              batch_size);
    benchmarker.run_benchmark(
        option, {.calculation_type = short_circuit, .calculation_symmetry = symmetric, .calculation_method = iec60909});

    // with meshed ring
    option.has_mv_ring = true;
    option.has_lv_ring = true;
    benchmarker.run_benchmark(
        option,
        {.calculation_type = short_circuit, .calculation_symmetry = asymmetric, .calculation_method = iec60909});

    return 0;
}
