// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "all_components.hpp"
#include "registry.hpp"
#include "rule_adapters.hpp"
#include "rules.hpp"

namespace power_grid_model::validation {

inline ComponentRules const& node_rule_table() {
    using Node = components::Node;
    using NodeInput = Node::InputType;

    static ComponentRules const component_rules{
        .component = Node::name,
        .input_rules = {make_unary_rule<Node, &NodeInput::u_rated>("u_rated", rules::is_required),
                        make_unary_rule<Node, &NodeInput::u_rated>("u_rated", rules::is_finite),
                        make_unary_rule<Node, &NodeInput::u_rated>("u_rated", rules::is_greater_than_zero)},
        .batch_rules = {}};
    return component_rules;
}

} // namespace power_grid_model::validation
