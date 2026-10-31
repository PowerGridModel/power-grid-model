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
#include <functional>
#include <string_view>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

template <class Component, auto AttributeType, class RuleType>
RuleDefinition make_unary_rule(std::string_view attribute, RuleType rule,
                               ValidationDatasetType source = ValidationDatasetType::input) {
    using Input = Component::InputType;

    return {.code = RuleType::code,
            .fields = {attribute},
            .apply = [attribute, source, rule = std::move(rule)](ValidationContext const& context, Issues& issues) {
                auto const& dataset = context.dataset(source);
                Input const default_values{};
                auto const apply_to_component = [&](auto const& component_span) {
                    auto const ids = meta_data::get_attribute_span(component_span, &Input::id, "id");
                    assert(ids.size() == static_cast<std::size_t>(component_span.size()));
                    auto const attribute_span = meta_data::get_attribute_span(component_span, AttributeType, attribute);
                    assert(attribute_span.size() == 0 || attribute_span.size() == ids.size());
                    if (attribute_span.size() == 0 && ids.size() > 0) {
                        if (std::invoke(rule, default_values.*AttributeType)) {
                            std::vector<ID> invalid_ids(ids.begin(), ids.end());
                            add_issue(issues, RuleType::code, Component::name, attribute, std::move(invalid_ids));
                        }
                        return;
                    }
                    rules::apply(rule, attribute_span, ids, Component::name, attribute, issues);
                };
                dataset.for_each_component<meta_data::input_getter_s, Component>(apply_to_component);
            }};
}

} // namespace power_grid_model::validation
