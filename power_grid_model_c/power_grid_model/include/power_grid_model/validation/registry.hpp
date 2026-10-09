#pragma once

#include "context.hpp"
#include "line.hpp"
#include "node.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/exception.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace power_grid_model::validation {

using SupportedComponents = ComponentList<Node, Line>;

template <class... Components>
void validate_supported_components(ConstDataset const& dataset, std::string_view operation,
                                   ComponentList<Components...>) {
    for (Idx component_idx{}; component_idx < dataset.n_components(); ++component_idx) {
        auto const component_name = std::string_view{dataset.get_component_info(component_idx).component->name};
        if (!(... || (component_name == Components::name))) {
            throw InvalidArguments{std::string{operation} + " supports only registered components; received '" +
                                   std::string{component_name} + "'.\n"};
        }
    }
}

template <class Component>
void collect_component_ids(ConstDataset const& dataset, ValidationContext& context) {
    dataset.for_each_component<meta_data::input_getter_s, Component>([&context](auto const& rows) {
        std::size_t row{};
        for (auto const& value : rows) {
            rules::detail::visit_record<typename Component::InputType>(value, [&context, row](auto const& record) {
                auto const component_name = std::string_view{Component::name};
                context.ids_by_component[component_name].push_back(record.id);
                context.objects_by_component[component_name].push_back(
                    {.component = component_name, .id = record.id, .row = row});
                if (record.id != na_IntID) {
                    context.valid_ids_by_component[component_name].insert(record.id);
                }
            });
            ++row;
        }
        auto& objects = context.objects_by_component[std::string_view{Component::name}];
        std::unordered_map<ID, std::size_t> id_counts;
        for (auto const& object : objects) {
            ++id_counts[object.id];
        }
        for (auto& object : objects) {
            if (object.id != na_IntID && id_counts.at(object.id) == 1) {
                object.row.reset();
            }
        }
    });
}

template <class... Components>
ValidationContext build_validation_context(ConstDataset const& dataset, ComponentList<Components...>) {
    ValidationContext context;
    (collect_component_ids<Components>(dataset, context), ...);
    return context;
}

template <class... Components, class Function>
void for_each_component_type(ComponentList<Components...>, Function&& function) {
    (std::invoke(function, std::type_identity<Components>{}), ...);
}

template <class... Components>
void validate_registered_components(ConstDataset const& dataset, bool symmetric, ValidationContext const& context,
                                    Issues& issues, ComponentList<Components...> components) {
    for_each_component_type(components, [&dataset, symmetric, &context, &issues](auto component_type) {
        using Component = typename decltype(component_type)::type;
        InputValidator<Component>::validate(dataset, symmetric, context, issues);
    });
}

} // namespace power_grid_model::validation