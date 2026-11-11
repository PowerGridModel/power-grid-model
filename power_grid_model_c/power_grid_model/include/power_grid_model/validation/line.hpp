// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "registry.hpp"
#include "rule_adapters.hpp"
#include "rules.hpp"

#include "power_grid_model/auxiliary/input.hpp"
#include "power_grid_model/component/line.hpp"

namespace power_grid_model::validation {

inline ComponentRules const& line_rule_table() {
    static ComponentRules const component_rules{
        .component = Line::name,
        .input_rules = {make_unary_rule<Line, &LineInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<Line, &LineInput::r1>("r1", rules::is_required),
                        make_unary_rule<Line, &LineInput::r1>("r1", rules::is_finite),
                        make_unary_rule<Line, &LineInput::from_status>("from_status", rules::is_boolean)},
        .batch_rules = {make_unary_rule<Line, &LineInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<Line, &LineInput::from_status>("from_status", rules::is_boolean)}};
    return component_rules;
}

} // namespace power_grid_model::validation
