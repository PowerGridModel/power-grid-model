// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_cpp.hpp>

#include <filesystem>
#include <map>
#include <set>
#include <vector>
#include <ostream>

namespace power_grid_model_cpp {

struct ClIOptions {
    std::filesystem::path input_file;
    std::filesystem::path batch_update_file;
    std::filesystem::path output_file;

    Idx calculation_type{PGM_power_flow};
    Idx calculation_method{PGM_default_method};
    Idx symmetric_calculation{PGM_symmetric};
    double error_tolerance{1e-8};
    Idx max_iterations{20};
    Idx threading{-1};
    Idx short_circuit_voltage_scaling{PGM_short_circuit_voltage_scaling_maximum};
    Idx tap_changing_strategy{PGM_tap_changing_strategy_disabled};
    
    bool use_msgpack_output_serialization{false};
    Idx output_json_indent{2};
    bool use_compact_serialization{false};

    std::map<PGM_MetaComponent const*, std::set<PGM_MetaAttribute const*>> output_component_attribute_filters;

    friend std::ostream& operator<<(std::ostream& os, ClIOptions const& options);
};

int parse_cli_options(int argc, char** argv, ClIOptions& options);

} // namespace power_grid_model_cpp
