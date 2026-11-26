// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../registry.hpp"
#include "../rule_adapters.hpp"
#include "power_grid_model/auxiliary/input.hpp"
#include "power_grid_model/common/enum.hpp"

#include <array>
#include <span>
#include <string_view>
#include <utility>

namespace power_grid_model::validation::components {

inline ComponentRules const& sym_power_sensor_rule_table() {
    using enum MeasuredTerminalType;

    static constexpr std::array<std::string_view, 12> measured_objects{
        "node",   "line",  "asym_line", "generic_branch", "transformer", "three_winding_transformer",
        "source", "shunt", "sym_load",  "asym_load",      "sym_gen",     "asym_gen"};
    static constexpr std::array<std::string_view, 4> branches{"line", "asym_line", "generic_branch", "transformer"};
    static constexpr std::array<std::string_view, 1> sources{"source"};
    static constexpr std::array<std::string_view, 1> shunts{"shunt"};
    static constexpr std::array<std::string_view, 2> loads{"sym_load", "asym_load"};
    static constexpr std::array<std::string_view, 2> generators{"sym_gen", "asym_gen"};
    static constexpr std::array<std::string_view, 1> three_winding_transformers{"three_winding_transformer"};
    static constexpr std::array<std::string_view, 1> nodes{"node"};

    static constexpr std::array<std::pair<MeasuredTerminalType, std::span<std::string_view const>>, 10>
        terminal_type_to_ref_components{{
            {branch_from, branches},
            {branch_to, branches},
            {source, sources},
            {shunt, shunts},
            {load, loads},
            {generator, generators},
            {branch3_1, three_winding_transformers},
            {branch3_2, three_winding_transformers},
            {branch3_3, three_winding_transformers},
            {node, nodes},
        }};

    static ComponentRules const component_rules{
        .component = "sym_power_sensor",
        .input_rules = {make_valid_id_rule<&SymPowerSensorInput::measured_object>("measured_object", measured_objects),
                        make_filtered_valid_id_rule<&SymPowerSensorInput::measured_object,
                                                    &SymPowerSensorInput::measured_terminal_type>(
                            "measured_object", "measured_terminal_type", terminal_type_to_ref_components)},
        .batch_rules = {}};
    return component_rules;
}

} // namespace power_grid_model::validation::components
