// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../auxiliary/input.hpp"
#include "../registry.hpp"
#include "../rule_adapters.hpp"
#include "../rules.hpp"

namespace power_grid_model::validation::components {

inline ComponentRules const& link_rule_table() {
    static ComponentRules const component_rules{
        .component = "link",
        .input_rules = {make_unary_rule<&LinkInput::from_node>("from_node", rules::is_required),
                        make_unary_rule<&LinkInput::to_node>("to_node", rules::is_required),
                        make_unary_rule<&LinkInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&LinkInput::from_status>("from_status", rules::is_boolean),
                        make_unary_rule<&LinkInput::to_status>("to_status", rules::is_required),
                        make_unary_rule<&LinkInput::to_status>("to_status", rules::is_boolean)},
        .batch_rules = {make_unary_rule<&LinkInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&LinkInput::from_status>("from_status", rules::is_boolean),
                        make_unary_rule<&LinkInput::to_status>("to_status", rules::is_required),
                        make_unary_rule<&LinkInput::to_status>("to_status", rules::is_boolean)}};
    return component_rules;
}

} // namespace power_grid_model::validation::components
