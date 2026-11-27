// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../auxiliary/input.hpp"
#include "../../common/enum.hpp"
#include "../registry.hpp"
#include "../rule_adapters.hpp"
#include "../rules.hpp"

namespace power_grid_model::validation::components {

inline ComponentRules const& transformer_rule_table() {
    static ComponentRules const component_rules{
        .component = "transformer",
        .input_rules =
            {
                make_unary_rule<&TransformerInput::from_node>("from_node", rules::is_required),
                make_unary_rule<&TransformerInput::to_node>("to_node", rules::is_required),
                make_unary_rule<&TransformerInput::from_status>("from_status", rules::is_required),
                make_unary_rule<&TransformerInput::from_status>("from_status", rules::is_boolean),
                make_unary_rule<&TransformerInput::to_status>("to_status", rules::is_required),
                make_unary_rule<&TransformerInput::to_status>("to_status", rules::is_boolean),
                make_unary_rule<&TransformerInput::u1>("u1", rules::is_required),
                make_unary_rule<&TransformerInput::u1>("u1", rules::is_finite),
                make_unary_rule<&TransformerInput::u1>("u1", rules::is_greater_than_zero),
                make_unary_rule<&TransformerInput::u2>("u2", rules::is_required),
                make_unary_rule<&TransformerInput::u2>("u2", rules::is_finite),
                make_unary_rule<&TransformerInput::u2>("u2", rules::is_greater_than_zero),
                make_unary_rule<&TransformerInput::sn>("sn", rules::is_required),
                make_unary_rule<&TransformerInput::sn>("sn", rules::is_finite),
                make_unary_rule<&TransformerInput::sn>("sn", rules::is_greater_than_zero),
                make_unary_rule<&TransformerInput::uk>("uk", rules::is_required),
                make_unary_rule<&TransformerInput::uk>("uk", rules::is_greater_than_zero),
                make_unary_rule<&TransformerInput::uk>("uk", rules::is_less_than_one),
                make_unary_rule<&TransformerInput::pk>("pk", rules::is_required),
                make_unary_rule<&TransformerInput::pk>("pk", rules::is_finite),
                make_unary_rule<&TransformerInput::pk>("pk", rules::is_greater_than_or_equal_to_zero),
                make_unary_rule<&TransformerInput::i0>("i0", rules::is_required),
                make_unary_rule<&TransformerInput::i0>("i0", rules::is_less_than_one),
                make_unary_rule<&TransformerInput::p0>("p0", rules::is_required),
                make_unary_rule<&TransformerInput::p0>("p0", rules::is_finite),
                make_unary_rule<&TransformerInput::p0>("p0", rules::is_greater_than_or_equal_to_zero),
                make_unary_rule<&TransformerInput::i0_zero_sequence>("i0_zero_sequence", rules::is_less_than_one),
                make_unary_rule<&TransformerInput::p0_zero_sequence>("p0_zero_sequence", rules::is_finite),
                make_unary_rule<&TransformerInput::p0_zero_sequence>("p0_zero_sequence",
                                                                     rules::is_greater_than_or_equal_to_zero),
                make_unary_rule<&TransformerInput::winding_from>("winding_from", rules::is_required),
                make_unary_rule<&TransformerInput::winding_from>("winding_from", rules::is_enum<WindingType>),
                make_unary_rule<&TransformerInput::winding_to>("winding_to", rules::is_required),
                make_unary_rule<&TransformerInput::winding_to>("winding_to", rules::is_enum<WindingType>),
                make_unary_rule<&TransformerInput::clock>("clock", rules::is_required),
                make_unary_rule<&TransformerInput::clock>("clock", rules::is_in_inclusive_range<-12, 12>),
                make_unary_rule<&TransformerInput::tap_side>("tap_side", rules::is_required),
                make_unary_rule<&TransformerInput::tap_side>("tap_side", rules::is_enum<BranchSide>),
                make_unary_rule<&TransformerInput::tap_min>("tap_min", rules::is_required),
                make_unary_rule<&TransformerInput::tap_max>("tap_max", rules::is_required),
                make_unary_rule<&TransformerInput::tap_size>("tap_size", rules::is_required),
                make_unary_rule<&TransformerInput::tap_size>("tap_size", rules::is_finite),
                make_unary_rule<&TransformerInput::tap_size>("tap_size", rules::is_greater_than_or_equal_to_zero),
                make_unary_rule<&TransformerInput::uk_min>("uk_min", rules::is_greater_than_zero),
                make_unary_rule<&TransformerInput::uk_min>("uk_min", rules::is_less_than_one),
                make_unary_rule<&TransformerInput::uk_max>("uk_max", rules::is_greater_than_zero),
                make_unary_rule<&TransformerInput::uk_max>("uk_max", rules::is_less_than_one),
                make_unary_rule<&TransformerInput::pk_min>("pk_min", rules::is_finite),
                make_unary_rule<&TransformerInput::pk_min>("pk_min", rules::is_greater_than_or_equal_to_zero),
                make_unary_rule<&TransformerInput::pk_max>("pk_max", rules::is_finite),
                make_unary_rule<&TransformerInput::pk_max>("pk_max", rules::is_greater_than_or_equal_to_zero),
                make_unary_rule<&TransformerInput::r_grounding_from>("r_grounding_from", rules::is_finite),
                make_unary_rule<&TransformerInput::x_grounding_from>("x_grounding_from", rules::is_finite),
                make_unary_rule<&TransformerInput::r_grounding_to>("r_grounding_to", rules::is_finite),
                make_unary_rule<&TransformerInput::x_grounding_to>("x_grounding_to", rules::is_finite),
            },
        .batch_rules = {make_unary_rule<&TransformerInput::from_status>("from_status", rules::is_required),
                        make_unary_rule<&TransformerInput::from_status>("from_status", rules::is_boolean),
                        make_unary_rule<&TransformerInput::to_status>("to_status", rules::is_required),
                        make_unary_rule<&TransformerInput::to_status>("to_status", rules::is_boolean)}};
    return component_rules;
}

} // namespace power_grid_model::validation::components
