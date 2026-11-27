// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../auxiliary/input.hpp"
#include "../registry.hpp"
#include "../rule_adapters.hpp"
#include "../rules.hpp"

namespace power_grid_model::validation::components {

inline ComponentRules const& generic_branch_rule_table() {
    static ComponentRules const component_rules{
        .component = "generic_branch",
        .input_rules = {make_unary_rule<&GenericBranchInput::from_node>("from_node", rules::is_required),
                        make_unary_rule<&GenericBranchInput::to_node>("to_node", rules::is_required),
                        make_unary_rule<&GenericBranchInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&GenericBranchInput::from_status>("from_status", rules::is_boolean),
                        make_unary_rule<&GenericBranchInput::to_status>("to_status", rules::is_required),
                        make_unary_rule<&GenericBranchInput::to_status>("to_status", rules::is_boolean),
                        make_unary_rule<&GenericBranchInput::r1>("r1", rules::is_required),
                        make_unary_rule<&GenericBranchInput::r1>("r1", rules::is_finite),
                        make_unary_rule<&GenericBranchInput::x1>("x1", rules::is_required),
                        make_unary_rule<&GenericBranchInput::x1>("x1", rules::is_finite),
                        make_unary_rule<&GenericBranchInput::g1>("g1", rules::is_required),
                        make_unary_rule<&GenericBranchInput::g1>("g1", rules::is_finite),
                        make_unary_rule<&GenericBranchInput::b1>("b1", rules::is_required),
                        make_unary_rule<&GenericBranchInput::b1>("b1", rules::is_finite),
                        make_unary_rule<&GenericBranchInput::k>("k", rules::is_finite),
                        make_unary_rule<&GenericBranchInput::k>("k", rules::is_greater_than_zero),
                        make_unary_rule<&GenericBranchInput::theta>("theta", rules::is_finite),
                        make_unary_rule<&GenericBranchInput::sn>("sn", rules::is_finite),
                        make_unary_rule<&GenericBranchInput::sn>("sn", rules::is_greater_than_or_equal_to_zero)},
        .batch_rules = {make_unary_rule<&GenericBranchInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&GenericBranchInput::from_status>("from_status", rules::is_boolean),
                        make_unary_rule<&GenericBranchInput::to_status>("to_status", rules::is_required),
                        make_unary_rule<&GenericBranchInput::to_status>("to_status", rules::is_boolean)}};
    return component_rules;
}

} // namespace power_grid_model::validation::components
