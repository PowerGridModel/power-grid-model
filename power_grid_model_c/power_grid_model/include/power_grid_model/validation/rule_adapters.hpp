// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "issues.hpp"
#include "registry.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/common/common.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <string_view>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

template <class Component, auto FieldMember, class Rule>
RuleDefinition make_unary_rule(std::string_view field, Rule rule,
                               ValidationDatasetType source = ValidationDatasetType::input) {
    using Input = Component::InputType;

    return {.code = Rule::code,
            .fields = {field},
            .apply = [field, source, rule = std::move(rule)](ValidationContext const& context, Issues& issues) {
                auto const& dataset = context.dataset(source);
                Input const default_values{};
                auto const apply_to_component = [&](auto const& component_span) {
                    auto const ids = meta_data::get_attribute_span(component_span, &Input::id, "id");
                    assert(ids.size() == static_cast<std::size_t>(component_span.size()));
                    auto const values = meta_data::get_attribute_span(component_span, FieldMember, field);
                    assert(values.size() == 0 || values.size() == ids.size());
                    if (values.size() == 0 && ids.size() > 0) {
                        if (std::invoke(rule, default_values.*FieldMember)) {
                            std::vector<ID> invalid_ids(ids.begin(), ids.end());
                            add_issue(issues, Rule::code, Component::name, field, std::move(invalid_ids));
                        }
                        return;
                    }
                    rules::apply(rule, values, ids, Component::name, field, issues);
                };
                dataset.for_each_component<meta_data::input_getter_s, Component>(apply_to_component);
            }};
}

} // namespace power_grid_model::validation
