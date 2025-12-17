// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_options.hpp"

#include <CLI/CLI.hpp>

namespace power_grid_model_cpp {

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
