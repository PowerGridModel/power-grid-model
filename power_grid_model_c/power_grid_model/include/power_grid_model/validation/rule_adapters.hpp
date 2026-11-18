// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "issues.hpp"
#include "registry.hpp"
#include "rules.hpp"
#include "utils.hpp"

#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/counting_iterator.hpp"
#include "power_grid_model/common/exception.hpp"
#include "power_grid_model/main_core/core_utils.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <set>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace power_grid_model::validation {
namespace detail {

template <class> struct member_pointer_traits;
template <class InputOrUpdateComponentType, class AttributeType>
struct member_pointer_traits<AttributeType InputOrUpdateComponentType::*> {
    using struct_type = InputOrUpdateComponentType;
    using value_type = AttributeType;
};

template <InputOrUpdateComponentType>
inline auto get_ids_span(auto const component_span) {
    auto const ids = meta_data::get_attribute_span(component_span, &InputOrUpdateComponentType::id, "id");
    // TODO(figueroa1395): Handle validation of the absent IDs correctly
    // For now a buffer without IDs will result in an empty span
    if (!ids.empty()) {
        assert(ids.size() == static_cast<std::size_t>(component_span.size()));
    }
    return ids;
}

template <class InputOrUpdateComponentType, class RuleType, class... Args>
inline RuleDefinition unary_rule_factory(std::string_view attribute, ValidationDatasetType source,
                                         functor_c auto callable, Args&&... extra_args) {
    return {.code = RuleType::code,
            .fields = {attribute},
            .apply = [&source, callable, ... extra_args = std::forward<Args>(extra_args)](
                         std::string_view component_name, ValidationContext const& context, Issues& issues, Idx scenario_idx) {

                            auto const& dataset = [&context, &source]() {
                                if (source == ValidationDatasetType::input) {
                                    return context.get_input();
                                }
                                    assert(source == ValidationDatasetType::update);
                                    return context.get_update();
                            }();
                dataset.for_each_component<InputOrUpdateComponentType>(
                    component_name,
                    [&context, &issues, callable, component_name, &extra_args...](auto const& component_span) {
                        std::invoke(callable, context, issues, component_span, component_name, extra_args...);
                    }, scenario_idx);
            }};
}

} // namespace detail

// TODO(figueroa1395): Remove source from the signature, it's not needed anymore
template <auto InputAttribute, class RuleType>
inline RuleDefinition make_unary_rule(std::string_view attribute_name, RuleType rule,
                                      ValidationDatasetType source = ValidationDatasetType::input) {
    using InputTrait = detail::member_pointer_traits<decltype(InputAttribute)>;
    using InputComponentType = typename InputTrait::struct_type;
    using InputAttributeType = typename InputTrait::value_type;

    auto const apply_to_component = [attribute_name, rule](ValidationContext const& /*context*/, Issues& issues,
                                                      auto const& component_span, std::string_view component_name) {
        auto const attribute_span = meta_data::get_attribute_span(component_span, InputAttribute, attribute_name);
        auto const ids = detail::get_ids_span<InputComponentType>(component_span);
        assert(attribute_span.size() == 0 || attribute_span.size() == ids.size());
        if (attribute_span.size() == 0 && ids.size() > 0) {
            if (std::invoke(rule, meta_data::nan_value<InputAttributeType>())) {
                std::vector<ID> invalid_ids(ids.begin(), ids.end());
                add_issue(issues, RuleType::code, component_name, attribute_name, std::move(invalid_ids));
            }
            return;
        }
        rules::apply(rule, attribute_span, ids, component_name, attribute_name, issues);
    };

    return detail::unary_rule_factory<InputComponentType, RuleType>(attribute_name, source, apply_to_component);
}

template <auto InputAttribute, auto UpdateAttribute, class RuleType>
inline RuleDefinition make_unary_rule(std::string_view attribute_name, RuleType rule,
                                      ValidationDatasetType source = ValidationDatasetType::input_with_update) {
    using InputTrait = detail::member_pointer_traits<decltype(InputAttribute)>;
    using InputComponentType = typename InputTrait::struct_type;
    using InputAttributeType = typename InputTrait::value_type;

    using UpdateTrait = detail::member_pointer_traits<decltype(UpdateAttribute)>;
    using UpdateComponentType = typename UpdateTrait::struct_type;
    using UpdateAttributeType = typename UpdateTrait::value_type;

    constexpr bool is_update = std::same_as<StructType, Update>;
    constexpr bool is_input = std::same_as<StructType, Input>;
    static_assert(is_update || is_input, "Attribute must be a member of the component's InputType or UpdateType");
    using Getter = std::conditional_t<is_update, meta_data::update_getter_s, meta_data::input_getter_s>;

    auto const apply_to_component = [attribute_name, &rule](ValidationContext& context, Issues& issues,
                                                      auto const& component_span, , std::string_view component_name) {
        auto const attribute_span = meta_data::get_attribute_span(component_span, UpdateAttribute, attribute_name);
        auto& id_map = context.id_to_component_element_map;

        auto const apply_to_ids = [&context, &issues, &id_map, &attribute_span, component_name, attribute_name,
                                    &rule](auto const& ids) {
            assert(!ids.empty()); // empty components are skipped and optional IDs fallback to input IDs
            assert(std::ranges::all_of(ids, [&id_map](ID const id) { return id_map.contains(id); }));
            assert(attribute_span.size() == 0 || attribute_span.size() == ids.size());
            // defer to input sweep if attribute is not present in the update
            if (attribute_span.size() == 0 && ids.size() > 0) {
                for (auto const id : ids) {
                    // "flag" for input sweep
                    context.component_to_attribute_to_idx_map[component_name][attribute_name] = id_map[id].element_idx;
                }
                return;
            }
            rules::apply(rule, attribute_span, ids, component_name, attribute_name, issues);
        };

        auto const ids = [](){
            auto const update_ids = detail::get_ids_span<UpdateComponentType>(component_span);
            if (!update_ids.empty()) {
                return update_ids;
            }
            // optional IDs fallback: it defaults to the respective input IDs
            return detail::get_ids_span<InputComponentType>(component_span);
        }();
        return apply_to_ids(ids);
    };

    return detail::unary_rule_factory<UpdateComponentType, RuleType>(attribute_name, source, apply_to_component);
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
