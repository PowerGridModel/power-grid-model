// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "power_grid_model/auxiliary/input.hpp"
#include "power_grid_model/validation/rules.hpp"

#include <power_grid_model/component/node.hpp>

namespace power_grid_model::validation {

inline ComponentRules const& node_rule_table() {
    static ComponentRules const component_rules{
        .component = Node::name,
        .input_rules = {make_unary_rule<Node, &NodeInput::u_rated>("u_rated", rules::Required{}),
                        make_unary_rule<Node, &NodeInput::u_rated>("u_rated", rules::Finite{}),
                        make_unary_rule<Node, &NodeInput::u_rated>("u_rated", rules::GreaterThanZero{})},
        .batch_rules = {}};
    return component_rules;
}

} // namespace power_grid_model::validation
