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
#include <type_traits>
#include <utility>
#include <vector>

namespace power_grid_model::validation {
namespace detail {
template <typename ComponentType> inline auto get_ids_span(auto const component_span) {
    auto const ids = meta_data::get_attribute_span(component_span, &ComponentType::InputType::id, "id");
    assert(ids.size() == static_cast<std::size_t>(component_span.size()));
    return ids;
}

template <class ComponentType, class RuleType, class... Args>
inline RuleDefinition make_unary_rule_base(std::string_view attribute, ValidationDatasetType source,
                                           functor_c auto callable, Args&&... extra_args) {
    return {.code = RuleType::code,
            .fields = {attribute},
            .apply = [source, callable, ... extra_args = std::forward<Args>(extra_args)](
                         ValidationContext const& context, Issues& issues) {
                auto const& dataset = context.dataset(source);

                dataset.for_each_component<meta_data::input_getter_s, ComponentType>(
                    [&context, &issues, callable, &extra_args...](auto const& component_span) {
                        std::invoke(callable, context, issues, component_span, extra_args...);
                    });
            }};
}

} // namespace detail

template <class Component, auto AttributeType, class RuleType>
inline RuleDefinition make_unary_rule(std::string_view attribute, RuleType rule,
                                      ValidationDatasetType source = ValidationDatasetType::input) {
    auto const apply_to_component = [attribute, rule](ValidationContext const& context, Issues& issues,
                                                      auto const& component_span) {
        using Input = Component::InputType;
        using AttributeInputType = std::remove_cvref_t<decltype(std::declval<Input const&>().*AttributeType)>;
        (void)context;

        auto const ids = detail::get_ids_span<Component>(component_span);

        auto const attribute_span = meta_data::get_attribute_span(component_span, AttributeType, attribute);
        assert(attribute_span.size() == 0 || attribute_span.size() == ids.size());
        if (attribute_span.size() == 0 && ids.size() > 0) {
            if (std::invoke(rule, meta_data::nan_value<AttributeInputType>())) {
                std::vector<ID> invalid_ids(ids.begin(), ids.end());
                add_issue(issues, RuleType::code, Component::name, attribute, std::move(invalid_ids));
            }
            return;
        }
        rules::apply(rule, attribute_span, ids, Component::name, attribute, issues);
    };

    return detail::make_unary_rule_base<Component, RuleType>(attribute, source, apply_to_component);
}

} // namespace power_grid_model::validation
