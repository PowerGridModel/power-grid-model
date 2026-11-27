// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../auxiliary/input.hpp"
#include "../context.hpp"
#include "../registry.hpp"
#include "../rule_adapters.hpp"
#include "../rules.hpp"

namespace power_grid_model::validation::components {

inline ComponentRules const& line_rule_table() {
    using ConditionalValidationType::only_state_estimation;

    static ComponentRules const component_rules{
        .component = "line",
        .input_rules = {make_unary_rule<&LineInput::from_node>("from_node", rules::is_required),
                        make_unary_rule<&LineInput::to_node>("to_node", rules::is_required),
                        make_unary_rule<&LineInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&LineInput::from_status>("from_status", rules::is_boolean),
                        make_unary_rule<&LineInput::to_status>("to_status", rules::is_required),
                        make_unary_rule<&LineInput::to_status>("to_status", rules::is_boolean),
                        make_unary_rule<&LineInput::r1>("r1", rules::is_required),
                        make_unary_rule<&LineInput::r1>("r1", rules::is_finite),
                        make_unary_rule<&LineInput::x1>("x1", rules::is_required),
                        make_unary_rule<&LineInput::x1>("x1", rules::is_finite),
                        make_unary_rule<&LineInput::c1>("c1", rules::is_required),
                        make_unary_rule<&LineInput::c1>("c1", rules::is_finite),
                        make_unary_rule<&LineInput::tan1>("tan1", rules::is_required),
                        make_unary_rule<&LineInput::tan1>("tan1", rules::is_finite),
                        make_unary_rule<&LineInput::r0>("r0", rules::is_required, only_state_estimation),
                        make_unary_rule<&LineInput::r0>("r0", rules::is_finite, only_state_estimation),
                        make_unary_rule<&LineInput::x0>("x0", rules::is_required, only_state_estimation),
                        make_unary_rule<&LineInput::x0>("x0", rules::is_finite, only_state_estimation),
                        make_unary_rule<&LineInput::c0>("c0", rules::is_required, only_state_estimation),
                        make_unary_rule<&LineInput::c0>("c0", rules::is_finite, only_state_estimation),
                        make_unary_rule<&LineInput::tan0>("tan0", rules::is_required, only_state_estimation),
                        make_unary_rule<&LineInput::tan0>("tan0", rules::is_finite, only_state_estimation),
                        make_unary_rule<&LineInput::i_n>("i_n", rules::is_finite),
                        make_unary_rule<&LineInput::i_n>("i_n", rules::is_greater_than_zero)},
        .batch_rules = {make_unary_rule<&LineInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&LineInput::from_status>("from_status", rules::is_boolean),
                        make_unary_rule<&LineInput::to_status>("to_status", rules::is_required),
                        make_unary_rule<&LineInput::to_status>("to_status", rules::is_boolean)}};
    return component_rules;
}

} // namespace power_grid_model::validation::components
