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

#include <algorithm>
#include <cassert>
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

// non-owning view over the ids of a component, for both row-based and columnar buffers
// empty if the component is absent or a columnar buffer has no "id" attribute buffer
// full non-templated and runtime id retrieval
inline auto get_component_ids(ConstDataset const& dataset, std::string_view component) {
    char const* first_id = nullptr;
    Idx stride{};
    Idx size{};

    if (Idx const component_idx = dataset.find_component(component, false); component_idx >= 0) {
        auto const& info = dataset.get_component_info(component_idx);
        auto const& buffer = dataset.get_buffer(component_idx);

        if (dataset.is_row_based(buffer)) {
            auto const& id_attribute = info.component->get_attribute("id");
            assert(id_attribute.ctype == meta_data::ctype_v<ID>);
            first_id = static_cast<char const*>(buffer.data) + id_attribute.offset;
            assert(first_id != nullptr);
            stride = static_cast<Idx>(info.component->size);
            size = info.total_elements;
        } else if (auto const it =
                       std::ranges::find_if(buffer.attributes,
                                            [](auto const& attribute_buffer) {
                                                return std::string_view{attribute_buffer.meta_attribute->name} == "id";
                                            });
                   it != buffer.attributes.end()) {
            assert(it->meta_attribute->ctype == meta_data::ctype_v<ID>);
            first_id = static_cast<char const*>(it->data);
            stride = static_cast<Idx>(sizeof(ID));
            // this needs to be set inside the branches or optional IDs crash
            size = info.total_elements;
            assert(first_id != nullptr || size == 0);
        }
    }
    return IdxRange{size} | std::views::transform([first_id, stride](Idx idx) {
               return *reinterpret_cast<ID const*>(first_id + idx * stride);
           });
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
    using InputComponentType = typename Trait::struct_type;
    using AttributeType = typename Trait::value_type;

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
        auto const ids = detail::get_component_ids(dataset, component_name);
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
