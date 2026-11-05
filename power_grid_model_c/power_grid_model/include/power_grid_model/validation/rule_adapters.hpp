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
#include "power_grid_model/main_core/core_utils.hpp"

#include <cassert>
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
template <class ComponentType> inline auto get_ids_span(auto const component_span) {
    auto const ids = meta_data::get_attribute_span(component_span, &ComponentType::InputType::id, "id");
    assert(ids.size() == static_cast<std::size_t>(component_span.size()));
    return ids;
}

template <class ComponentType, class RuleType, class... Args>
inline RuleDefinition unary_rule_factory(std::string_view attribute, ValidationDatasetType source,
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

template <class ComponentType, auto AttributeType, class RuleType>
inline RuleDefinition make_unary_rule(std::string_view attribute, RuleType rule,
                                      ValidationDatasetType source = ValidationDatasetType::input) {
    auto const apply_to_component = [attribute, rule](ValidationContext const& /*context*/, Issues& issues,
                                                      auto const& component_span) {
        using Input = ComponentType::InputType;
        using AttributeInputType = std::remove_cvref_t<decltype(std::declval<Input const&>().*AttributeType)>;

        auto const ids = detail::get_ids_span<ComponentType>(component_span);

        auto const attribute_span = meta_data::get_attribute_span(component_span, AttributeType, attribute);
        assert(attribute_span.size() == 0 || attribute_span.size() == ids.size());
        if (attribute_span.size() == 0 && ids.size() > 0) {
            if (std::invoke(rule, meta_data::nan_value<AttributeInputType>())) {
                std::vector<ID> invalid_ids(ids.begin(), ids.end());
                add_issue(issues, RuleType::code, ComponentType::name, attribute, std::move(invalid_ids));
            }
            return;
        }
        rules::apply(rule, attribute_span, ids, ComponentType::name, attribute, issues);
    };

    return detail::unary_rule_factory<ComponentType, RuleType>(attribute, source, apply_to_component);
}

template <class... ComponentTypes> std::tuple<ComponentTypes...> to_tuple(ComponentList<ComponentTypes...>);
using AllComponentsTuple = decltype(to_tuple(AllComponents{}));

struct ComponentElement {
    std::string_view component_type;
    Idx element_idx;
};

using IDToComponentElementMap = std::unordered_map<ID, ComponentElement>;

inline IDToComponentElementMap check_input_ids(ValidationContext const& context, Issues& issues) {
    using RuleType = rules::Unique;
    // TODO(figueroa1395): Replace strings as keys in the map, here and elsewhere
    using ComponentTypeToBadIDsMap = std::unordered_map<std::string_view, std::set<ID>>;

    Idx const total_elements = detail::get_total_input_elements(context.dataset(ValidationDatasetType::input));
    IDToComponentElementMap id_to_component_element_map;
    id_to_component_element_map.reserve(total_elements);
    ComponentTypeToBadIDsMap component_type_to_bad_ids_map;

    auto const apply_to_all_components = [&context, &issues, &id_to_component_element_map,
                                          &component_type_to_bad_ids_map]<class ComponentType>() {
        auto const apply_to_component = [&id_to_component_element_map, &component_type_to_bad_ids_map](
                                            ValidationContext const& /*context*/, Issues& /*issues*/,
                                            auto const& component_span) {
            auto const ids = detail::get_ids_span<ComponentType>(component_span);
            std::string_view const component_type_name = ComponentType::name;
            for (auto const [idx, id] : std::views::zip(IdxRange{std::ssize(ids)}, ids)) {
                auto const [it, inserted] = id_to_component_element_map.try_emplace(
                    id, ComponentElement{.component_type = component_type_name, .element_idx = idx});
                if (std::invoke(RuleType{}, inserted)) {
                    component_type_to_bad_ids_map[it->second.component_type].insert(id);
                    component_type_to_bad_ids_map[component_type_name].insert(id);
                }
            }
        };
        detail::unary_rule_factory<ComponentType, RuleType>("id", ValidationDatasetType::input, apply_to_component)
            .apply(context, issues);
    };
    main_core::utils::run_functor_with_tuple_return_void<AllComponentsTuple>(apply_to_all_components);

    auto const report_for_all_components = [&issues, &component_type_to_bad_ids_map]<class ComponentType>() {
        std::string_view const component_type_name = ComponentType::name;
        auto const it = component_type_to_bad_ids_map.find(component_type_name);
        if (it == component_type_to_bad_ids_map.end()) {
            return;
        }
        add_issue(issues, RuleType::code, component_type_name, "id",
                  std::vector<ID>(it->second.begin(), it->second.end()));
    };
    main_core::utils::run_functor_with_tuple_return_void<AllComponentsTuple>(report_for_all_components);

    return id_to_component_element_map;
}

} // namespace power_grid_model::validation
