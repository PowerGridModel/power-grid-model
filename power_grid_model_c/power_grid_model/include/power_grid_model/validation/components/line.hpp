// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../auxiliary/input.hpp"
#include "../registry.hpp"
#include "../rule_adapters.hpp"
#include "../rules.hpp"

#include <array>
#include <string_view>

namespace power_grid_model::validation::components {

inline ComponentRules const& line_rule_table() {
    static constexpr std::array<std::string_view, 1> node_components{"node"};
    static ComponentRules const component_rules{
        .component = "line",
        .input_rules = {make_unary_rule<&LineInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&LineInput::r1>("r1", rules::is_required),
                        make_unary_rule<&LineInput::r1>("r1", rules::is_finite),
                        make_unary_rule<&LineInput::from_status>("from_status", rules::is_boolean),
                        make_valid_id_rule<&LineInput::from_node>("from_node", node_components),
                        make_valid_id_rule<&LineInput::to_node>("to_node", node_components)},
        .batch_rules = {make_unary_rule<&LineInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&LineInput::from_status>("from_status", rules::is_boolean)}};
    return component_rules;
}

} // namespace power_grid_model::validation::components
