// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_functions.hpp"

#include <CLI/CLI.hpp>

#include <algorithm>
#include <fstream>
#include <map>
#include <numeric>
#include <ranges>
#include <string>

namespace power_grid_model_cpp {

using EnumMap = std::map<std::string, Idx>;

struct CLIPostCallback {
    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
    ClIOptions& options;
    CLI::Option* msgpack_flag;
    CLI::Option* compact_flag;
    std::vector<std::string> const& output_components;
    std::vector<std::string> const& output_attributes;
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

    void operator()() {
        set_default_values();
        set_output_dataset();
        add_component_output_filter();
        add_attribute_output_filter();
    }

    static PGM_SerializationFormat get_serialization_format(std::string const& argument_type,
                                                            std::filesystem::path const& path) {
        std::ifstream file{path, std::ios::binary};
        if (!file.is_open()) {
            throw CLI::ValidationError(argument_type, "Unable to open file: " + path.string());
        }
        uint8_t header;
        file.read(reinterpret_cast<char*>(&header), 1);
        if (!file) {
            throw CLI::ValidationError(argument_type, "Unable to read from file: " + path.string());
        }
        // Check for fixmap (0x80-0x8f), map16 (0xde), or map32 (0xdf)
        bool const is_msgpack = (header >= 0x80 && header <= 0x8f) || header == 0xde || header == 0xdf;
        return is_msgpack ? PGM_msgpack : PGM_json;
    }

    void set_default_values() {
        // detect if input file is msgpack
        options.input_serialization_format = get_serialization_format("input", options.input_file);
        // detect if batch update file is provided
        options.is_batch = !options.batch_update_file.empty();
        // detect if batch update file is msgpack
        options.batch_update_serialization_format.resize(options.batch_update_file.size());
        std::transform(options.batch_update_file.cbegin(), options.batch_update_file.cend(),
                       options.batch_update_serialization_format.begin(),
                       [](auto const& path) { return get_serialization_format("batch-update", path); });
        // default msgpack output if input or any of the batch updates is msgpack and user did not specify output format
        if (msgpack_flag->count() == 0 && (options.input_serialization_format == PGM_msgpack ||
                                           std::ranges::any_of(options.batch_update_serialization_format,
                                                               [](auto format) { return format == PGM_msgpack; }))) {
            options.use_msgpack_output_serialization = true;
        }
        // default compact serialization if msgpack output and user did not specify compact option
        if (compact_flag->count() == 0 && options.use_msgpack_output_serialization) {
            options.use_compact_serialization = true;
        }
    }

    void set_output_dataset() {
        if (options.calculation_type == PGM_power_flow || options.calculation_type == PGM_state_estimation) {
            if (options.symmetric_calculation) {
                options.output_dataset_name = "sym_output";
            } else {
                options.output_dataset_name = "asym_output";
            }
        } else {
            // options.calculation_type == PGM_short_circuit
            options.output_dataset_name = "sc_output";
        }
        options.output_dataset = MetaData::get_dataset_by_name(options.output_dataset_name);
    }

    void add_component_output_filter() {
        for (auto const& comp_name : output_components) {
            try {
                auto const component = MetaData::get_component_by_name(options.output_dataset_name, comp_name);
                options.output_component_attribute_filters[component] = {};
            } catch (PowerGridError const&) {
                throw CLI::ValidationError("output-component", "Component '" + comp_name + "' not found in dataset '" +
                                                                   options.output_dataset_name + "'.");
            }
        }
    }

    void add_attribute_output_filter() {
        for (auto const& attr_full_name : output_attributes) {
            auto dot_pos = attr_full_name.find('.');
            if (dot_pos == std::string::npos || dot_pos == 0 || dot_pos == attr_full_name.size() - 1) {
                throw CLI::ValidationError("output-attribute", "Attribute '" + attr_full_name +
                                                                   "' is not in the format 'component.attribute'.");
            }
            auto comp_name = attr_full_name.substr(0, dot_pos);
            auto attr_name = attr_full_name.substr(dot_pos + 1);
            MetaComponent const* component = nullptr;
            try {
                component = MetaData::get_component_by_name(options.output_dataset_name, comp_name);
            } catch (PowerGridError const&) {
                throw CLI::ValidationError("output-attribute", "Component '" + comp_name + "' not found in dataset '" +
                                                                   options.output_dataset_name + "'.");
            }
            MetaAttribute const* attribute = nullptr;
            try {
                attribute = MetaData::get_attribute_by_name(options.output_dataset_name, comp_name, attr_name);
            } catch (PowerGridError const&) {
                throw CLI::ValidationError("output-attribute",
                                           "Attribute '" + attr_name + "' not found in component '" + comp_name +
                                               "' of dataset '" + options.output_dataset_name + "'.");
            }
            options.output_component_attribute_filters[component].insert(attribute);
        }
    }
};

CLIResult parse_cli_options(int argc, char** argv, ClIOptions& options) {
    std::string const version_str = std::string("Power Grid Model CLI\n Version: ") + PGM_version();
    CLI::App app{version_str};

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

    app.set_version_flag("-v,--version", PGM_version());
    app.add_option("-i,--input", options.input_file, "Input file path")->required()->check(CLI::ExistingFile);
    app.add_option("-b,--batch-update", options.batch_update_file,
                   "Batch update file path. Can be specified multiple times.\n"
                   "If multiple files are specified, the core will intepret them as the cartesian product of all "
                   "combinations of all scenarios in the list of batch datasets.")
        ->check(CLI::ExistingFile);
    app.add_option("-o,--output", options.output_file, "Output file path")
        ->required()
        ->check(existing_parent_dir_validator);
    app.add_option("-f,--system-frequency", options.system_frequency, "System frequency in Hz, default is 50.0 Hz.");
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
    auto msgpack_flag =
        app.add_flag("--msgpack,--use-msgpack-output-serialization,!--json,!--use-json-output-serialization",
                     options.use_msgpack_output_serialization, "Use MessagePack output serialization");
    app.add_option("--indent,--output-json-indent", options.output_json_indent,
                   "Number of spaces to indent JSON output");
    auto compact_flag =
        app.add_flag("--compact,--use-compact-serialization,!--no-compact,!--no-compact-serialization",
                     options.use_compact_serialization, "Use compact serialization (no extra whitespace)");
    std::vector<std::string> output_components;
    app.add_option("--oc,--output-component", output_components,
                   "Filter output to only include specified components (can be specified multiple times)");
    std::vector<std::string> output_attributes;
    app.add_option("--oa,--output-attribute", output_attributes,
                   "Filter output to only include specified attributes, in the format `component.attribute` (can be "
                   "specified multiple times)");

    app.callback(CLIPostCallback{.options = options,
                                 .msgpack_flag = msgpack_flag,
                                 .compact_flag = compact_flag,
                                 .output_components = output_components,
                                 .output_attributes = output_attributes});

    try {
        app.parse(argc, argv);
    } catch (CLI::ParseError const& e) {
        return {.exit_code = app.exit(e), .should_exit = true};
    }

    return {.exit_code = 0, .should_exit = false};
}

std::ostream& operator<<(std::ostream& os, ClIOptions const& options) {
    os << "CLI Options:\n";
    os << "Input file: " << options.input_file << "\n";
    os << "Batch update file: \n";
    for (auto const& file : options.batch_update_file) {
        os << '\t' << file << "\n";
    }
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
