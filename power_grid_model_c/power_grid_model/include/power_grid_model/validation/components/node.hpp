// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../auxiliary/input.hpp"
#include "../registry.hpp"
#include "../rule_adapters.hpp"
#include "../rules.hpp"

namespace power_grid_model::validation::components {

inline ComponentRules const& node_rule_table() {
    static ComponentRules const component_rules{
        .component = "node",
        .input_rules = {make_unary_rule<&NodeInput::u_rated>("u_rated", rules::is_required),
                        make_unary_rule<&NodeInput::u_rated>("u_rated", rules::is_finite),
                        make_unary_rule<&NodeInput::u_rated>("u_rated", rules::is_greater_than_zero)},
        .batch_rules = {}};
    return component_rules;
}

} // namespace power_grid_model::validation::components
