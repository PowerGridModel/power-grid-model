// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/component/line.hpp>
#include <power_grid_model/component/node.hpp>

namespace power_grid_model::validation {

inline ComponentRules const& line_rule_table() {
    static ComponentRules const component_rules{
        .component = Line::name,
        .input_rules = {make_range_rule<Line, &LineInput::id>("id", rules::Unique{}),
                        make_id_reference_rule<Line, &LineInput::from_node>("from_node", Node::name),
                        make_unary_rule<Line, &LineInput::from_status>("from_status", rules::Required{}),
                        make_unary_rule<Line, &LineInput::r1>("r1", rules::Required{}),
                        make_unary_rule<Line, &LineInput::r1>("r1", rules::Finite{}),
                        make_unary_rule<Line, &LineInput::from_status>("from_status", rules::Boolean{})},
        .batch_rules = {make_unary_rule<Line, &LineInput::from_status>("from_status", rules::Required{}),
                        make_unary_rule<Line, &LineInput::from_status>("from_status", rules::Boolean{})},
        .build_input_id_lookup = detail::index_input_component_ids<Line>};
    return component_rules;
}

} // namespace power_grid_model::validation
