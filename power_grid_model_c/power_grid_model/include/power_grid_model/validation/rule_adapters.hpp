// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "issues.hpp"
#include "registry.hpp"
#include "rules.hpp"
#include "utils.hpp"

#include "../auxiliary/dataset.hpp"
#include "../auxiliary/meta_data.hpp"
#include "../common/common.hpp"
#include "../common/counting_iterator.hpp"

#include <cassert>
#include <cstddef>
#include <functional>
#include <ranges>
#include <set>
#include <span>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace power_grid_model::validation {
namespace detail {
template <class> struct member_pointer_traits;
template <class InputComponentType, class AttributeType>
struct member_pointer_traits<AttributeType InputComponentType::*> {
    using struct_type = InputComponentType;
    using value_type = AttributeType;
};

template <class InputComponentType> inline auto get_ids_span(auto const component_span) {
    auto const ids = meta_data::get_attribute_span(component_span, &InputComponentType::id, "id");
    // TODO(figueroa1395): Handle validation of the absent IDs correctly
    // For now a buffer without IDs will result in an empty span
    if (!ids.empty()) {
        assert(ids.size() == static_cast<std::size_t>(component_span.size()));
    }
    return ids;
}

template <class InputComponentType, class RuleType, class... Args>
inline RuleDefinition unary_rule_factory(std::string_view attribute, ValidationDatasetType source,
                                         functor_c auto callable, Args&&... extra_args) {
    return {.code = RuleType::code,
            .fields = {attribute},
            .apply = [source, callable, ... extra_args = std::forward<Args>(extra_args)](
                         std::string_view component_name, ValidationContext const& context, Issues& issues) {
                context.dataset(source).for_each_component<InputComponentType>(
                    component_name,
                    [&context, &issues, callable, component_name, &extra_args...](auto const& component_span) {
                        std::invoke(callable, context, issues, component_span, component_name, extra_args...);
                    });
            }};
}

} // namespace detail

template <auto Attribute, class RuleType>
inline RuleDefinition make_unary_rule(std::string_view attribute, RuleType rule,
                                      ValidationDatasetType source = ValidationDatasetType::input) {
    using Trait = detail::member_pointer_traits<decltype(Attribute)>;
    using InputComponentType = Trait::struct_type;
    using AttributeType = Trait::value_type;

    auto const apply_to_component = [attribute, rule](ValidationContext const& /*context*/, Issues& issues,
                                                      auto const& component_span, std::string_view component_name) {
        auto const attribute_span = meta_data::get_attribute_span(component_span, Attribute, attribute);
        auto const ids = detail::get_ids_span<InputComponentType>(component_span);
        assert(attribute_span.size() == 0 || attribute_span.size() == ids.size());
        if (attribute_span.size() == 0 && ids.size() > 0) {
            if (std::invoke(rule, meta_data::nan_value<AttributeType>())) {
                std::vector<ID> invalid_ids(ids.begin(), ids.end());
                add_issue(issues, RuleType::code, component_name, attribute, std::move(invalid_ids));
            }
            return;
        }
        rules::apply(rule, attribute_span, ids, component_name, attribute, issues);
    };

    return detail::unary_rule_factory<InputComponentType, RuleType>(attribute, source, apply_to_component);
}

template <auto InputAttribute>
inline RuleDefinition make_valid_id_rule(std::string_view attribute_name,
                                         std::span<std::string_view const> ref_components) {
    using InputTrait = detail::member_pointer_traits<decltype(InputAttribute)>;
    using InputComponentType = typename InputTrait::struct_type;

    auto const apply_to_component = [attribute_name](ValidationContext const& context, Issues& issues,
                                                     auto const& component_span, std::string_view component_name,
                                                     std::span<std::string_view const> ref_components_arg) {
        rules::ValidId const rule{.id_to_component_element_map = context.id_to_component_element_map,
                                  .ref_components = ref_components_arg};
        auto const attribute_span = meta_data::get_attribute_span(component_span, InputAttribute, attribute_name);
        auto const ids = detail::get_ids_span<InputComponentType>(component_span);
        if (attribute_span.size() == 0) {
            // Attribtue should not be optional and not passing this rule.
            return;
        }
        assert(attribute_span.size() == ids.size());
        rules::apply(rule, attribute_span, ids, component_name, attribute_name, issues);
    };

    return detail::unary_rule_factory<InputComponentType, rules::ValidId>(attribute_name, ValidationDatasetType::input,
                                                                          apply_to_component, ref_components);
}
}

inline void check_input_ids(ValidationContext& context, Issues& issues) {
    using RuleType = rules::Unique;
    // TODO(figueroa1395): Replace strings as keys in the map, here and elsewhere
    using ComponentTypeToBadIDsMap = std::unordered_map<std::string_view, std::set<ID>>;

    auto const& dataset = context.dataset(ValidationDatasetType::input);
    Idx const total_elements = detail::get_total_input_elements(dataset);
    auto& id_to_component_element_map = context.id_to_component_element_map;
    id_to_component_element_map.reserve(total_elements);
    ComponentTypeToBadIDsMap component_type_to_bad_ids_map;

    auto const component_names =
        dataset.get_description().component_info | std::views::transform([](meta_data::ComponentInfo const& info) {
            return std::string_view{info.component->name};
        });

    // check each ID for uniqueness within and accross components
    // as well as update the global ID to component element map
    for (std::string_view const component_name : component_names) {
        // TODO(figueroa1395): Handle the case where the component has no IDs correctly
        auto const ids = meta_data::get_component_ids_view(dataset, component_name);
        for (auto const [idx, id] : std::views::zip(IdxRange{std::ssize(ids)}, ids)) {
            auto const [it, inserted] = id_to_component_element_map.try_emplace(
                id, detail::ComponentElement{.component_type = component_name, .element_idx = idx});
            if (std::invoke(RuleType{}, inserted)) {
                component_type_to_bad_ids_map[it->second.component_type].insert(id);
                component_type_to_bad_ids_map[component_name].insert(id);
            }
        }
    }

    // report issues for each component if there are any bad IDs
    for (std::string_view const component_name : component_names) {
        if (auto const it = component_type_to_bad_ids_map.find(component_name);
            it != component_type_to_bad_ids_map.end()) {
            add_issue(issues, RuleType::code, component_name, "id",
                      std::vector<ID>(it->second.begin(), it->second.end()));
        }
    }
}

} // namespace power_grid_model::validation
