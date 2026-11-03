// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "dataset.hpp"
#include "issues.hpp"
#include "options.hpp"
#include "registry.hpp"
#include "report.hpp"

#include "power_grid_model/all_components.hpp"
#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/component_list.hpp"
#include "power_grid_model/common/counting_iterator.hpp"
#include "power_grid_model/common/typing.hpp"
#include "power_grid_model/main_core/core_utils.hpp"

#include <algorithm>
#include <functional>
#include <ranges>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace power_grid_model::validation::detail {

template <typename... ComponentTypes> std::tuple<ComponentTypes...> to_tuple(ComponentList<ComponentTypes...>);
using AllComponentsTuple = decltype(to_tuple(AllComponents{}));

struct ComponentElement {
    std::string_view component_type;
    Idx element_idx;
};

using IDToComponentElementMap = std::unordered_map<ID, ComponentElement>;

inline IDToComponentElementMap make_id_to_component_element_map(ConstDataset const& input_dataset) {
    auto const components_sizes_view = input_dataset.get_description().component_info |
                                       std::views::transform(&meta_data::ComponentInfo::total_elements);
    Idx const total_elements = std::ranges::fold_left(components_sizes_view, Idx{0}, std::plus<>{});
    IDToComponentElementMap id_to_component_element_map;
    id_to_component_element_map.reserve(total_elements);

    main_core::utils::run_functor_with_tuple_return_void<AllComponentsTuple>(
        [&id_to_component_element_map, &input_dataset]<typename ComponentType>() {
            input_dataset.for_each_component<meta_data::input_getter_s, ComponentType>([&id_to_component_element_map](
                                                                                           auto const buffer_span) {
                auto const ids_view = meta_data::get_attribute_span(buffer_span, &ComponentType::InputType::id, "id");

                for (auto const& [idx, id] : std::views::zip(IdxRange{std::ssize(ids_view)}, ids_view)) {
                    id_to_component_element_map.insert(
                        {id, ComponentElement{.component_type = ComponentType::name, .element_idx = idx}});
                }
            });
        });

    return id_to_component_element_map;
}

inline ValidationReport validate_batch_with_registry(ConstDataset const& input_dataset,
                                                     ConstDataset const& update_dataset,
                                                     DataValidatorOptions const& options,
                                                     ValidationRegistry const& registry) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    ValidationContext const context{.input = input_dataset, .options = options};
    Issues issues;
    for (auto const& component_rules : registry) {
        // TODO: Validate optional update IDs in the dedicated ID-checking rules.
        if (should_validate_component(input_dataset, component_rules.component)) {
            for (auto const& rule : component_rules.batch_rules) {
                rule.apply(context, issues);
            }
        }
    }
    return {.type = ValidationType::batch, .scopes = {make_validation_scope(std::move(issues))}};
}

} // namespace power_grid_model::validation::detail
