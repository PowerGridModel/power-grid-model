#pragma once

#include "input.hpp"
#include "report.hpp"

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/exception.hpp>

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
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
                        std::string_view component, Issues& issues, std::vector<std::size_t>& indices) {
    if (updates.empty()) {
        return true;
    }
    if (input.empty()) {
        std::vector<ObjectReference> objects;
        objects.reserve(updates.size());
        for (std::size_t row{}; row < updates.size(); ++row) {
            objects.push_back({.component = component, .id = updates[row].id, .row = row});
        }
        add_update_addressing_issue(issues, component, std::move(objects));
        return false;
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

    std::unordered_map<ID, std::size_t> input_index;
    std::unordered_set<ID> ambiguous_input_ids;
    input_index.reserve(input.size());
    for (std::size_t index{}; index < input.size(); ++index) {
        if (input[index].id != na_IntID) {
            auto const [position, inserted] = input_index.try_emplace(input[index].id, index);
            if (!inserted) {
                ambiguous_input_ids.insert(position->first);
            }
        }
    }

    std::vector<ObjectReference> invalid_ids;
    std::vector<ObjectReference> ambiguous_ids;
    std::vector<ObjectReference> missing_ids;
    indices.reserve(updates.size());
    for (std::size_t row{}; row < updates.size(); ++row) {
        UpdateRecord const& update = updates[row];
        if (update.id == na_IntID) {
            missing_ids.push_back({.component = component, .id = update.id, .row = row});
            continue;
        }
        if (ambiguous_input_ids.contains(update.id)) {
            ambiguous_ids.push_back({.component = component, .id = update.id, .row = row});
            continue;
        }
        auto const found = input_index.find(update.id);
        if (found == input_index.end()) {
            invalid_ids.push_back({.component = component, .id = update.id, .row = row});
        } else {
            indices.push_back(found->second);
        }
    }
    bool const invalid_addressing = !missing_ids.empty() || !ambiguous_ids.empty() || !invalid_ids.empty();
    if (!missing_ids.empty()) {
        add_update_addressing_issue(issues, component, std::move(missing_ids));
    }
    if (!ambiguous_ids.empty()) {
        add_update_addressing_issue(issues, component, std::move(ambiguous_ids));
    }
    if (!invalid_ids.empty()) {
        add_id_not_in_dataset_issue_from_objects(issues, component, std::move(invalid_ids));
    }
    return !invalid_addressing;
}

template <class Component, class ComponentList> struct ComponentIndex;

template <class Component, class... Components>
struct ComponentIndex<Component, ComponentList<Component, Components...>> : std::integral_constant<std::size_t, 0> {};

template <class Component, class First, class... Components>
struct ComponentIndex<Component, ComponentList<First, Components...>>
    : std::integral_constant<std::size_t, 1 + ComponentIndex<Component, ComponentList<Components...>>::value> {};

template <class Component> struct ComponentIndex<Component, ComponentList<>> {
    static_assert(!std::is_same_v<Component, Component>, "Component is not in the validation list.");
};

template <class ComponentList> struct InputRecordStorage;

template <class... Components> struct InputRecordStorage<ComponentList<Components...>> {
    using List = ComponentList<Components...>;
    std::tuple<std::vector<typename Components::InputType>...> records;

    template <class Component> auto& get() { return std::get<ComponentIndex<Component, List>::value>(records); }
    template <class Component> auto const& get() const { return std::get<ComponentIndex<Component, List>::value>(records); }
};

template <class ComponentList> struct ScenarioUpdateStorage;

template <class... Components> struct ScenarioUpdateStorage<ComponentList<Components...>> {
    using List = ComponentList<Components...>;
    template <class> using IndexVector = std::vector<std::size_t>;
    std::tuple<std::vector<typename Components::UpdateType>...> updates;
    std::tuple<IndexVector<Components>...> indices;

    template <class Component> auto& records() { return std::get<ComponentIndex<Component, List>::value>(updates); }
    template <class Component> auto const& records() const { return std::get<ComponentIndex<Component, List>::value>(updates); }
    template <class Component> auto& row_indices() { return std::get<ComponentIndex<Component, List>::value>(indices); }
    template <class Component> auto const& row_indices() const { return std::get<ComponentIndex<Component, List>::value>(indices); }
};

template <class... Components>
InputRecordStorage<ComponentList<Components...>> collect_input_records(ConstDataset const& dataset,
                                                                       ComponentList<Components...> components) {
    InputRecordStorage<ComponentList<Components...>> storage;
    for_each_component_type(components, [&dataset, &storage](auto component_type) {
        using Component = typename decltype(component_type)::type;
        storage.template get<Component>() =
            collect_records<meta_data::input_getter_s, Component, typename Component::InputType>(dataset);
    });
    return storage;
}

template <class... Components>
bool collect_scenario_updates(ConstDataset const& scenario_dataset, InputRecordStorage<ComponentList<Components...>> const& input,
                              ScenarioUpdateStorage<ComponentList<Components...>>& updates, Issues& issues,
                              ComponentList<Components...> components) {
    bool valid = true;
    for_each_component_type(components, [&scenario_dataset, &input, &updates, &issues, &valid](auto component_type) {
        using Component = typename decltype(component_type)::type;
        auto& component_updates = updates.template records<Component>();
        component_updates = collect_records<meta_data::update_getter_s, Component, typename Component::UpdateType>(
            scenario_dataset);
        bool const component_valid = get_update_indices(
            component_updates, input.template get<Component>(), Component::name, issues,
            updates.template row_indices<Component>());
        valid = component_valid && valid;
    });
    return valid;
}

template <class... Components>
void apply_scenario_updates(InputRecordStorage<ComponentList<Components...>>& input,
                            ScenarioUpdateStorage<ComponentList<Components...>> const& updates,
                            meta_data::MetaData const& metadata, ComponentList<Components...> components) {
    for_each_component_type(components, [&input, &updates, &metadata](auto component_type) {
        using Component = typename decltype(component_type)::type;
        apply_component_updates<Component>(input.template get<Component>(), updates.template records<Component>(),
                                           updates.template row_indices<Component>(), metadata);
    });
}

template <class... Components>
void add_input_buffers(ConstDataset& dataset, InputRecordStorage<ComponentList<Components...>> const& input,
                       ComponentList<Components...> components) {
    for_each_component_type(components, [&dataset, &input](auto component_type) {
        using Component = typename decltype(component_type)::type;
        auto const& records = input.template get<Component>();
        auto const size = static_cast<Idx>(records.size());
        dataset.add_buffer(Component::name, size, size, nullptr, records.data());
    });
}

} // namespace batch_detail

template <class... Components>
ValidationReport validate_batch_report(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                       bool symmetric, ComponentList<Components...> components) {
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

    validate_supported_components(input_dataset, "Input validation", components);
    validate_supported_components(update_dataset, "Batch input validation", components);
    ValidationReport report{.type = ValidationType::batch, .results = {}};
    if (update_dataset.batch_size() == 0) {
        return report;
    }

    auto const input = batch_detail::collect_input_records(input_dataset, components);
    Issues input_identity_issues = validate_input_identity_dataset(input_dataset, components);
    bool const input_identity_valid = input_identity_issues.empty();
    report.results.push_back({.scenario = std::nullopt,
                              .status = input_identity_valid ? ValidationStatus::passed : ValidationStatus::failed,
                              .issues = std::move(input_identity_issues)});

    report.results.reserve(static_cast<std::size_t>(update_dataset.batch_size()) + 1);
    for (Idx scenario{}; scenario < update_dataset.batch_size(); ++scenario) {
        auto const scenario_dataset = update_dataset.get_individual_scenario(scenario);
        Issues scenario_issues;
        batch_detail::ScenarioUpdateStorage<ComponentList<Components...>> updates;
        bool const addressing_valid =
            batch_detail::collect_scenario_updates(scenario_dataset, input, updates, scenario_issues, components);
        if (!input_identity_valid || !addressing_valid) {
            report.results.push_back({.scenario = static_cast<std::size_t>(scenario),
                                      .status = ValidationStatus::blocked,
                                      .issues = std::move(scenario_issues)});
            continue;
        }

        auto merged_input = input;
        batch_detail::apply_scenario_updates(merged_input, updates, input_dataset.meta_data(), components);
        ConstDataset merged_dataset{false, 1, "input", input_dataset.meta_data()};
        batch_detail::add_input_buffers(merged_dataset, merged_input, components);
        Issues scenario_validation_issues = validate_input_dataset(merged_dataset, symmetric, components);
        auto const status = scenario_validation_issues.empty() ? ValidationStatus::passed : ValidationStatus::failed;
        report.results.push_back({.scenario = static_cast<std::size_t>(scenario), .status = status,
                                  .issues = std::move(scenario_validation_issues)});
    }
    return report;
}

template <class... Components>
BatchIssues validate_batch_dataset(ConstDataset const& input_dataset, ConstDataset const& update_dataset, bool symmetric,
                                   ComponentList<Components...> components) {
    ValidationReport const report = validate_batch_report(input_dataset, update_dataset, symmetric, components);
    BatchIssues issues;
    for (auto const& result : report.results) {
        if (result.scenario.has_value()) {
            issues.push_back(result.issues);
        }
    }
    return issues;
}

template <class... Components>
ValidationReport validate_batch_report(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                       bool symmetric) {
    return validate_batch_report(input_dataset, update_dataset, symmetric, SupportedComponents{});
}

inline BatchIssues validate_batch_dataset(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                          bool symmetric) {
    return validate_batch_dataset(input_dataset, update_dataset, symmetric, SupportedComponents{});
}

} // namespace power_grid_model::validation