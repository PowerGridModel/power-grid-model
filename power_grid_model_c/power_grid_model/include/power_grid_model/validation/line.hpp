// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "registry.hpp"
#include "rule_adapters.hpp"
#include "rules.hpp"

#include "power_grid_model/auxiliary/input.hpp"
#include "power_grid_model/component/line.hpp"
#include "power_grid_model/validation/context.hpp"

namespace power_grid_model::validation {

inline ComponentRules const& line_rule_table() {
    static ComponentRules const component_rules{
        .component = Line::name,
        .input_rules =
            {
                make_unary_rule<Line, &LineInput::from_status>("from_status", rules::is_required),
                make_unary_rule<Line, &LineInput::from_status>("from_status", rules::is_boolean),
                make_unary_rule<Line, &LineInput::r1>("r1", rules::is_required),
                make_unary_rule<Line, &LineInput::r1>("r1", rules::is_finite),
            },
            // TODO(figueroa1395): Should I do &BranchUpdate here instead?
        .batch_rules = {make_unary_rule<Line, &BranchUpdate::from_status>("from_status", rules::is_required,
                                                                       ValidationDatasetType::input_with_update),
                        make_unary_rule<Line, &BranchUpdate::from_status>("from_status", rules::is_boolean,
                                                                       ValidationDatasetType::input_with_update)}};
    return component_rules;
}

} // namespace power_grid_model::validation
