// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_cpp.hpp>

#include <filesystem>
#include <map>
#include <ostream>
#include <set>
#include <vector>

namespace power_grid_model_cpp {

struct CLIResult {
    int exit_code;
    bool should_exit;

    operator bool() const { return should_exit || exit_code != 0; }
};

struct ClIOptions {
    std::filesystem::path input_file;
    std::vector<std::filesystem::path> batch_update_file;
    std::filesystem::path output_file;
    PGM_SerializationFormat input_serialization_format{PGM_json};
    std::vector<PGM_SerializationFormat> batch_update_serialization_format;
    bool is_batch{false};

    double system_frequency{50.0};

    Idx calculation_type{PGM_power_flow};
    Idx calculation_method{PGM_default_method};
    bool symmetric_calculation{PGM_symmetric};
    double error_tolerance{1e-8};
    Idx max_iterations{20};
    Idx threading{-1};
    Idx short_circuit_voltage_scaling{PGM_short_circuit_voltage_scaling_maximum};
    Idx tap_changing_strategy{PGM_tap_changing_strategy_disabled};

    bool use_msgpack_output_serialization{false};
    Idx output_json_indent{2};
    bool use_compact_serialization{false};

    std::string output_dataset_name;
    MetaDataset const* output_dataset{nullptr};
    std::map<MetaComponent const*, std::set<MetaAttribute const*>> output_component_attribute_filters;

    bool verbose{false};

    friend std::ostream& operator<<(std::ostream& os, ClIOptions const& options);
};

CLIResult parse_cli_options(int argc, char** argv, ClIOptions& options);

void pgm_calculation(ClIOptions const& cli_options);

} // namespace power_grid_model_cpp
