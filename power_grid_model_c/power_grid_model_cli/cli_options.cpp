// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_options.hpp"

#include <CLI/CLI.hpp>

#include <map>

namespace power_grid_model_cpp {

using EnumMap = std::map<std::string, Idx>;

CLIResult parse_cli_options(int argc, char** argv, ClIOptions& options) {
    CLI::App app{"Power Grid Model CLI"};

    CLI::Validator existing_parent_dir_validator{
        [](std::string& input) {
            std::filesystem::path p{input};
            auto parent = p.has_parent_path() ? p.parent_path() : std::filesystem::path{"."};
            if (parent.empty() || !std::filesystem::exists(parent) || !std::filesystem::is_directory(parent)) {
                return std::string("The parent directory of the specified path does not exist or is not a directory.");
            }
            return std::string{};
        },
        "ExistingParentDirectory"};

    app.add_option("-i,--input", options.input_file, "Input file path")->required()->check(CLI::ExistingFile);
    app.add_option("-b,--batch-update", options.batch_update_file, "Batch update file path")->check(CLI::ExistingFile);
    app.add_option("-o,--output", options.output_file, "Output file path")
        ->required()
        ->check(existing_parent_dir_validator);
    app.add_option("-c,--calculation-type", options.calculation_type, "Calculation type")
        ->transform(CLI::CheckedTransformer(
            EnumMap{
                {"power_flow", PGM_power_flow},
                {"short_circuit", PGM_short_circuit},
                {"state_estimation", PGM_state_estimation},
            },
            CLI::ignore_case));
    app.add_option("-m,--calculation-method", options.calculation_method, "Calculation method")
        ->transform(CLI::CheckedTransformer(
            EnumMap{
                {"default", PGM_default_method},
                {"newton_raphson", PGM_newton_raphson},
                {"iterative_linear", PGM_iterative_linear},
                {"iterative_current", PGM_iterative_current},
                {"linear_current", PGM_linear_current},
                {"iec60909", PGM_iec60909},
            },
            CLI::ignore_case));
    app.add_flag("-s,--symmetric-calculation,!-a,!--asymmetric-calculation", options.symmetric_calculation,
                 "Use symmetric calculation (1) or not (0)");
    app.add_option("-e,--error-tolerance", options.error_tolerance, "Error tolerance for iterative calculations");
    app.add_option("-x,--max-iterations", options.max_iterations,
                   "Maximum number of iterations for iterative calculations");
    app.add_option("-t,--threading", options.threading, "Number of threads to use (-1 for automatic selection)");
    app.add_option("--short-circuit-voltage-scaling", options.short_circuit_voltage_scaling,
                   "Short circuit voltage scaling")
        ->transform(CLI::CheckedTransformer(
            EnumMap{
                {"minimum", PGM_short_circuit_voltage_scaling_minimum},
                {"maximum", PGM_short_circuit_voltage_scaling_maximum},
            },
            CLI::ignore_case));
    app.add_option("--tap-changing-strategy", options.tap_changing_strategy, "Tap changing strategy")
        ->transform(CLI::CheckedTransformer(
            EnumMap{
                {"disabled", PGM_tap_changing_strategy_disabled},
                {"any", PGM_tap_changing_strategy_any_valid_tap},
                {"min_voltage", PGM_tap_changing_strategy_min_voltage_tap},
                {"max_voltage", PGM_tap_changing_strategy_max_voltage_tap},
                {"fast_any", PGM_tap_changing_strategy_fast_any_tap},
            },
            CLI::ignore_case));
    app.add_flag("--msgpack,--use-msgpack-output-serialization", options.use_msgpack_output_serialization,
                 "Use MessagePack output serialization");
    app.add_option("--indent,--output-json-indent", options.output_json_indent,
                   "Number of spaces to indent JSON output");
    auto compact_flag =
        app.add_flag("--compact,--use-compact-serialization,!--no-compact,!--no-compact-serialization",
                     options.use_compact_serialization, "Use compact serialization (no extra whitespace)");

    app.callback([&options, compact_flag]() {
        if (compact_flag->count() == 0 && options.use_msgpack_output_serialization) {
            options.use_compact_serialization = true;
        }
    });

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return {.exit_code = app.exit(e), .should_exit = true};
    }

    return {.exit_code = 0, .should_exit = false};
}

std::ostream& operator<<(std::ostream& os, ClIOptions const& options) {
    os << "CLI Options:\n";
    os << "Input file: " << options.input_file << "\n";
    os << "Batch update file: " << options.batch_update_file << "\n";
    os << "Output file: " << options.output_file << "\n";

    os << "Calculation type: " << options.calculation_type << "\n";
    os << "Calculation method: " << options.calculation_method << "\n";
    os << "Symmetric calculation: " << options.symmetric_calculation << "\n";
    os << "Error tolerance: " << options.error_tolerance << "\n";
    os << "Max iterations: " << options.max_iterations << "\n";
    os << "Threading: " << options.threading << "\n";
    os << "Short circuit voltage scaling: " << options.short_circuit_voltage_scaling << "\n";
    os << "Tap changing strategy: " << options.tap_changing_strategy << "\n";

    os << "Use msgpack output serialization: " << options.use_msgpack_output_serialization << "\n";
    os << "Output JSON indent: " << options.output_json_indent << "\n";
    os << "Use compact serialization: " << options.use_compact_serialization << "\n";
    return os;
}

} // namespace power_grid_model_cpp
