// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_options.hpp"

namespace power_grid_model_cpp {

int parse_cli_options(int argc, char** argv, ClIOptions& options) {
    (void)argc;
    (void)argv;
    (void)options;
    return 0;
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
