#pragma once

#include "issues.hpp"
#include "registry.hpp"
#include "updates.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

#include <string>
#include <string_view>
#include <utility>

namespace power_grid_model::validation {

template <class List = SupportedComponents>
Issues validate_input_dataset(ConstDataset const& dataset, bool symmetric, List components = {}) {
    if (std::string_view{dataset.dataset().name} != "input") {
        throw InvalidArguments{"Input validation requires dataset type 'input'.\n"};
    }
    if (dataset.is_batch()) {
        throw InvalidArguments{"Input validation does not support batch datasets.\n"};
    }

    auto const context = build_validation_context(dataset, components);
    Issues issues = validate_ids(dataset, context, components);
    Issues value_issues = validate_values(dataset, symmetric, context, components);
    append_issues(issues, std::move(value_issues));
    return issues;
}

template <class... Components>
Issues validate_ids(ConstDataset const& dataset, ValidationContext const& context,
                    ComponentList<Components...> components) {
    Issues issues;
    for_each_component_type(components, [&dataset, &issues](auto component_type) {
        using Component = typename decltype(component_type)::type;
        using Record = typename Component::InputType;
        auto const get_id = [](Record const& record) { return record.id; };
        dataset.for_each_component<meta_data::input_getter_s, Component>([&](auto const& rows) {
            rules::required<Record>(rows, Component::name, "id", get_id, get_id, issues);
            rules::unique<Record>(rows, Component::name, "id", get_id, get_id, issues);
        });
    });
    rules::cross_unique(context.objects_by_component, issues);
    return issues;
}

template <class... Components>
Issues validate_values(ConstDataset const& dataset, bool symmetric, ValidationContext const& context,
                       ComponentList<Components...> components) {
    Issues issues;
    validate_component_values(dataset, symmetric, context, issues, components);
    return issues;
}

} // namespace power_grid_model::validation
