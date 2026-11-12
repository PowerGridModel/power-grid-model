// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "issues.hpp"
#include "registry.hpp"
#include "rules.hpp"
#include "utils.hpp"

#include "power_grid_model/all_components.hpp"
#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/component_list.hpp"
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
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace power_grid_model::validation {
namespace detail {

template <class> struct member_pointer_traits;
template <class InputOrUpdateComponentType, class AttributeType>
struct member_pointer_traits<AttributeType InputOrUpdateComponentType::*> {
    using class_type = InputOrUpdateComponentType;
    using value_type = AttributeType;
};

template <class ComponentType, class StructType = typename ComponentType::InputType>
inline auto get_ids_span(auto const component_span) {
    auto const ids = meta_data::get_attribute_span(component_span, &StructType::id, "id");
    // TODO(figueroa1395): Handle validation of the absent IDs correctly
    // For now a buffer without IDs will result in an empty span
    if (!ids.empty()) {
        assert(ids.size() == static_cast<std::size_t>(component_span.size()));
    }
    return ids;
}

template <class ComponentType, class Getter, class RuleType, class... Args>
inline RuleDefinition unary_rule_factory(std::string_view attribute, functor_c auto callable, Args&&... extra_args) {
    return {.code = RuleType::code,
            .fields = {attribute},
            .apply = [callable, ... extra_args = std::forward<Args>(extra_args)](ValidationContext& context,
                                                                                 Issues& issues, Idx scenario_idx) {
                auto const dataset = [&context]() {
                    if constexpr (std::same_as<Getter, meta_data::input_getter_s>) {
                        return context.get_input();
                    } else {
                        return context.get_update();
                    }
                }();

                dataset.template for_each_component<Getter, ComponentType>(
                    [&context, &issues, callable, &extra_args...](auto const& component_span) {
                        std::invoke(callable, context, issues, component_span, extra_args...);
                    },
                    scenario_idx);
            }};
}

} // namespace detail

template <class ComponentType, auto AttributeType, class RuleType>
inline RuleDefinition make_unary_rule(std::string_view attribute, RuleType rule,
                                      ValidationDatasetType source = ValidationDatasetType::input) {
    (void)source;
    using Traits = detail::member_pointer_traits<decltype(AttributeType)>;
    using StructType = typename Traits::class_type;
    using ValueType = typename Traits::value_type;
    using Input = typename ComponentType::InputType;
    using Update = typename ComponentType::UpdateType;

    constexpr bool is_update = std::same_as<StructType, Update>;
    constexpr bool is_input = std::same_as<StructType, Input>;
    static_assert(is_update || is_input, "Attribute must be a member of the component's InputType or UpdateType");
    using Getter = std::conditional_t<is_update, meta_data::update_getter_s, meta_data::input_getter_s>;

    auto const apply_to_component = [attribute, rule](ValidationContext& context, Issues& issues,
                                                      auto const& component_span) {
        auto const attribute_span = meta_data::get_attribute_span(component_span, AttributeType, attribute);
        std::string_view const component = ComponentType::name;

        if constexpr (is_input) {
            auto const ids = detail::get_ids_span<ComponentType>(component_span);
            assert(ids.size() == static_cast<std::size_t>(component_span.size()));
            assert(attribute_span.empty() || attribute_span.size() == ids.size());

            if (attribute_span.size() == 0 && ids.size() > 0) {
                if (std::invoke(rule, meta_data::nan_value<ValueType>())) {
                    std::vector<ID> invalid_ids(ids.begin(), ids.end());
                    add_issue(issues, RuleType::code, component, attribute, std::move(invalid_ids));
                }
                return;
            }
            rules::apply(rule, attribute_span, ids, component, attribute, issues);
        } else {

            auto& id_map = context.id_to_component_element_map;

            auto const apply_to_ids = [&context, &issues, &id_map, &attribute_span, &component, &attribute,
                                       &rule](auto const& ids) {
                assert(!ids.empty()); // empty components are skipped and optional IDs are fallback to input IDs
                assert(std::ranges::all_of(ids, [&id_map](ID const id) { return id_map.contains(id); }));

                // defer to input sweep if attribute is not present in the update
                assert(attribute_span.size() == 0 || attribute_span.size() == ids.size());
                if (attribute_span.size() == 0 && ids.size() > 0) {
                    for (auto const id : ids) {
                        // flag for input sweep
                        context.component_to_attribute_to_idx_map[component][attribute] = id_map[id].element_idx;
                    }
                    return;
                }
                rules::apply(rule, attribute_span, ids, component, attribute, issues);
            };

            auto const update_ids = detail::get_ids_span<ComponentType, Update>(component_span);
            if (update_ids.empty()) { // optional IDs defaults to input IDs
                context.get_input().template for_each_component<meta_data::input_getter_s, ComponentType>(
                    [&apply_to_ids](auto const& input_component_span) {
                        apply_to_ids(detail::get_ids_span<ComponentType>(input_component_span));
                    });
            }
            return apply_to_ids(update_ids);
        }
    };

    return detail::unary_rule_factory<ComponentType, Getter, RuleType>(attribute, apply_to_component);
}

template <class... ComponentTypes> std::tuple<ComponentTypes...> to_tuple(ComponentList<ComponentTypes...>);
using AllComponentsTuple = decltype(to_tuple(AllComponents{}));

template <class ComponentsTuple = AllComponentsTuple>
inline void check_input_ids(ValidationContext& context, Issues& issues) {
    using RuleType = rules::Unique;
    // TODO(figueroa1395): Replace strings as keys in the map, here and elsewhere
    using ComponentTypeToBadIDsMap = std::unordered_map<std::string_view, std::set<ID>>;

    Idx const total_elements = detail::get_total_input_elements(context.get_input());
    auto& id_to_component_element_map = context.id_to_component_element_map;
    id_to_component_element_map.reserve(total_elements);
    ComponentTypeToBadIDsMap component_type_to_bad_ids_map;

    auto const apply_to_all_components = [&context, &issues, &id_to_component_element_map,
                                          &component_type_to_bad_ids_map]<class ComponentType>() {
        auto const apply_to_component = [&id_to_component_element_map, &component_type_to_bad_ids_map](
                                            ValidationContext const& /*context*/, Issues& /*issues*/,
                                            auto const& component_span) {
            auto const ids = detail::get_ids_span<ComponentType>(component_span);
            // TODO(figueroa1395): Handle the case where the component has no IDs correctly
            // for now skip the component in the validation
            if (ids.empty()) {
                return;
            }
            std::string_view const component_type_name = ComponentType::name;
            for (auto const [idx, id] : std::views::zip(IdxRange{std::ssize(ids)}, ids)) {
                auto const [it, inserted] = id_to_component_element_map.try_emplace(
                    id, detail::ComponentElement{.component_type = component_type_name, .element_idx = idx});
                if (std::invoke(RuleType{}, inserted)) {
                    component_type_to_bad_ids_map[it->second.component_type].insert(id);
                    component_type_to_bad_ids_map[component_type_name].insert(id);
                }
            }
        };
        detail::unary_rule_factory<ComponentType, meta_data::input_getter_s, RuleType>("id", apply_to_component)
            .apply(context, issues, na_Idx);
    };
    main_core::utils::run_functor_with_tuple_return_void<ComponentsTuple>(apply_to_all_components);

    auto const report_for_all_components = [&issues, &component_type_to_bad_ids_map]<class ComponentType>() {
        std::string_view const component_type_name = ComponentType::name;
        auto const it = component_type_to_bad_ids_map.find(component_type_name);
        if (it == component_type_to_bad_ids_map.end()) {
            return;
        }
        add_issue(issues, RuleType::code, component_type_name, "id",
                  std::vector<ID>(it->second.begin(), it->second.end()));
    };
    main_core::utils::run_functor_with_tuple_return_void<ComponentsTuple>(report_for_all_components);
}

} // namespace power_grid_model::validation
