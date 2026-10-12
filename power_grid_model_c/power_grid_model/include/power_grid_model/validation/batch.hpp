#pragma once

#include "input.hpp"
#include "report.hpp"

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/exception.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace power_grid_model::validation {
namespace batch_detail {

template <class Getter, class Component, class Record>
std::vector<Record> collect_records(ConstDataset const& dataset) {
    std::vector<Record> records;
    dataset.for_each_component<Getter, Component>([&records](auto const& rows) {
        for (auto const& value : rows) {
            rules::detail::visit_record<Record>(value, [&records](Record const& record) { records.push_back(record); });
        }
    });
    return records;
}

template <class UpdateRecord, class InputRecord>
bool get_update_indices(std::vector<UpdateRecord> const& updates, std::vector<InputRecord> const& input,
                        std::unordered_map<ID, std::size_t> const& input_index, std::string_view component,
                        Issues& issues, std::vector<std::size_t>& indices) {
    if (updates.empty()) {
        return true;
    }

    bool const positional_ids =
        std::ranges::all_of(updates, [](UpdateRecord const& update) { return update.id == na_IntID; });
    if (positional_ids) {
        if (updates.size() != input.size()) {
            std::vector<ObjectReference> objects;
            objects.reserve(updates.size());
            for (std::size_t row{}; row < updates.size(); ++row) {
                objects.push_back({.component = component, .id = updates[row].id, .row = row});
            }
            add_update_addressing_issue(issues, component, std::move(objects));
            return false;
        }
        indices.reserve(updates.size());
        for (std::size_t index{}; index < updates.size(); ++index) {
            indices.push_back(index);
        }
        return true;
    }

    std::vector<ObjectReference> missing_ids;
    std::vector<ObjectReference> invalid_ids;
    indices.reserve(updates.size());
    for (std::size_t row{}; row < updates.size(); ++row) {
        UpdateRecord const& update = updates[row];
        if (update.id == na_IntID) {
            missing_ids.push_back({.component = component, .id = update.id, .row = row});
            continue;
        }
        auto const found = input_index.find(update.id);
        if (found == input_index.end()) {
            invalid_ids.push_back({.component = component, .id = update.id, .row = row});
        } else {
            indices.push_back(found->second);
        }
    }
    bool const invalid_addressing = !missing_ids.empty() || !invalid_ids.empty();
    if (!missing_ids.empty()) {
        add_update_addressing_issue(issues, component, std::move(missing_ids));
    }
    if (!invalid_ids.empty()) {
        add_id_not_in_dataset_issue(issues, component, std::move(invalid_ids));
    }
    return !invalid_addressing;
}

template <class Component> struct BaseComponentData {
    using InputRecord = typename Component::InputType;

    std::vector<InputRecord> records;
    std::unordered_map<ID, std::size_t> index_by_id;
    std::vector<InputUpdateAttributePair> update_attributes;
};

template <class Component> struct ScenarioData {
    using InputRecord = typename Component::InputType;
    using UpdateRecord = typename Component::UpdateType;

    std::vector<UpdateRecord> updates;
    std::vector<std::size_t> update_indices;
    std::optional<std::vector<InputRecord>> merged_records;
};

template <class... Components> using BaseComponentDataTuple = std::tuple<BaseComponentData<Components>...>;
template <class... Components> using ScenarioDataTuple = std::tuple<ScenarioData<Components>...>;

template <class... Components>
BaseComponentDataTuple<Components...> collect_base_data(ConstDataset const& dataset,
                                                        ComponentList<Components...> components) {
    BaseComponentDataTuple<Components...> base_data;
    for_each_component_type(components, [&dataset, &base_data](auto component_type) {
        using Component = typename decltype(component_type)::type;
        auto& component_data = std::get<BaseComponentData<Component>>(base_data);
        component_data.records =
            collect_records<meta_data::input_getter_s, Component, typename Component::InputType>(dataset);
        component_data.update_attributes = component_update_attributes<Component>(dataset.meta_data());
        component_data.index_by_id.reserve(component_data.records.size());
        for (std::size_t row{}; row < component_data.records.size(); ++row) {
            ID const id = component_data.records[row].id;
            if (id != na_IntID) {
                component_data.index_by_id.try_emplace(id, row);
            }
        }
    });
    return base_data;
}

template <class... Components> ScenarioDataTuple<Components...> make_scenario_data(ComponentList<Components...>) {
    return {};
}

template <class... Components>
bool collect_scenario_updates(ConstDataset const& scenario_dataset,
                              BaseComponentDataTuple<Components...> const& base_data,
                              ScenarioDataTuple<Components...>& scenario_data, Issues& issues,
                              ComponentList<Components...> components) {
    bool valid = true;
    for_each_component_type(components, [&scenario_dataset, &base_data, &scenario_data, &issues,
                                         &valid](auto component_type) {
        using Component = typename decltype(component_type)::type;
        auto const& base_component = std::get<BaseComponentData<Component>>(base_data);
        auto& scenario_component = std::get<ScenarioData<Component>>(scenario_data);
        scenario_component.merged_records.reset();
        scenario_component.updates =
            collect_records<meta_data::update_getter_s, Component, typename Component::UpdateType>(scenario_dataset);
        scenario_component.update_indices.clear();
        bool const component_valid =
            get_update_indices(scenario_component.updates, base_component.records, base_component.index_by_id,
                               Component::name, issues, scenario_component.update_indices);
        valid = component_valid && valid;
    });
    return valid;
}

template <class... Components>
void apply_scenario_updates(BaseComponentDataTuple<Components...> const& base_data,
                            ScenarioDataTuple<Components...>& scenario_data, ComponentList<Components...> components) {
    for_each_component_type(components, [&base_data, &scenario_data](auto component_type) {
        using Component = typename decltype(component_type)::type;
        auto const& base_component = std::get<BaseComponentData<Component>>(base_data);
        auto& scenario_component = std::get<ScenarioData<Component>>(scenario_data);
        if (!scenario_component.updates.empty()) {
            scenario_component.merged_records = base_component.records;
            apply_component_updates<Component>(*scenario_component.merged_records, scenario_component.updates,
                                               scenario_component.update_indices, base_component.update_attributes);
        }
    });
}

template <class... Components>
void add_input_buffers(ConstDataset& dataset, BaseComponentDataTuple<Components...> const& base_data,
                       ScenarioDataTuple<Components...> const& scenario_data, ComponentList<Components...> components) {
    for_each_component_type(components, [&dataset, &base_data, &scenario_data](auto component_type) {
        using Component = typename decltype(component_type)::type;
        auto const& base_component = std::get<BaseComponentData<Component>>(base_data);
        auto const& scenario_component = std::get<ScenarioData<Component>>(scenario_data);
        auto const& records =
            scenario_component.merged_records ? *scenario_component.merged_records : base_component.records;
        auto const size = static_cast<Idx>(records.size());
        dataset.add_buffer(Component::name, size, size, nullptr, records.data());
    });
}

} // namespace batch_detail

template <class List = SupportedComponents>
ValidationReport validate_batch_report(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                       bool symmetric, List components = {}) {
    if (std::string_view{input_dataset.dataset().name} != "input") {
        throw InvalidArguments{"Batch input validation requires dataset type 'input'.\n"};
    }
    if (input_dataset.is_batch()) {
        throw InvalidArguments{"Batch input validation requires a non-batch input dataset.\n"};
    }
    if (std::string_view{update_dataset.dataset().name} != "update") {
        throw InvalidArguments{"Batch input validation requires dataset type 'update'.\n"};
    }
    if (!update_dataset.is_batch()) {
        throw InvalidArguments{"Batch input validation requires a batch update dataset.\n"};
    }

    ValidationReport report{.type = ValidationType::batch, .results = {}};
    if (update_dataset.batch_size() == 0) {
        return report;
    }

    auto const input_context = build_validation_context(input_dataset, components);
    Issues input_id_issues = validate_ids(input_dataset, input_context, components);
    bool const input_ids_valid = input_id_issues.empty();
    Issues input_value_issues = validate_values(input_dataset, symmetric, input_context, components);
    bool const input_valid = input_ids_valid && input_value_issues.empty();
    append_issues(input_id_issues, std::move(input_value_issues));
    report.results.push_back({.scenario = std::nullopt,
                              .status = input_valid ? ValidationStatus::passed : ValidationStatus::failed,
                              .issues = std::move(input_id_issues)});

    report.results.reserve(static_cast<std::size_t>(update_dataset.batch_size()) + 1);
    if (!input_ids_valid) {
        for (Idx scenario{}; scenario < update_dataset.batch_size(); ++scenario) {
            report.results.push_back(
                {.scenario = static_cast<std::size_t>(scenario), .status = ValidationStatus::blocked, .issues = {}});
        }
        return report;
    }

    auto const base_data = batch_detail::collect_base_data(input_dataset, components);
    auto scenario_data = batch_detail::make_scenario_data(components);
    for (Idx scenario{}; scenario < update_dataset.batch_size(); ++scenario) {
        auto const scenario_dataset = update_dataset.get_individual_scenario(scenario);
        Issues scenario_issues;
        bool const addressing_valid = batch_detail::collect_scenario_updates(scenario_dataset, base_data, scenario_data,
                                                                             scenario_issues, components);
        if (!addressing_valid) {
            report.results.push_back({.scenario = static_cast<std::size_t>(scenario),
                                      .status = ValidationStatus::blocked,
                                      .issues = std::move(scenario_issues)});
            continue;
        }

        batch_detail::apply_scenario_updates(base_data, scenario_data, components);
        ConstDataset merged_dataset{false, 1, "input", input_dataset.meta_data()};
        batch_detail::add_input_buffers(merged_dataset, base_data, scenario_data, components);
        Issues scenario_validation_issues = validate_values(merged_dataset, symmetric, input_context, components);
        auto const status = scenario_validation_issues.empty() ? ValidationStatus::passed : ValidationStatus::failed;
        report.results.push_back({.scenario = static_cast<std::size_t>(scenario),
                                  .status = status,
                                  .issues = std::move(scenario_validation_issues)});
    }
    return report;
}

} // namespace power_grid_model::validation
