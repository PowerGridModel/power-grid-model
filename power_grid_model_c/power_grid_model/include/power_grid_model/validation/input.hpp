#pragma once

#include "issues.hpp"
#include "line.hpp"
#include "node.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

inline Issues validate_input_dataset(ConstDataset const& dataset, bool symmetric) {
    if (std::string_view{dataset.dataset().name} != "input") {
        throw InvalidArguments{"Input validation requires dataset type 'input'.\n"};
    }
    if (dataset.is_batch()) {
        throw InvalidArguments{"Input validation does not support batch datasets.\n"};
    }

    bool has_node = false;
    bool has_line = false;
    for (Idx component_idx{}; component_idx < dataset.n_components(); ++component_idx) {
        auto const component_name = dataset.get_component_info(component_idx).component->name;
        if (std::string_view{component_name} == "node") {
            has_node = true;
        } else if (std::string_view{component_name} == "line") {
            has_line = true;
        } else {
            throw InvalidArguments{"Input validation supports only node and line components; received '" +
                                   std::string{component_name} + "'.\n"};
        }
    }

    Issues issues;
    std::vector<ID> node_ids;
    std::unordered_set<ID> valid_node_ids;
    if (has_node) {
        validate_node(dataset, issues);
        dataset.for_each_component<meta_data::input_getter_s, Node>([&](auto const& rows) {
            for (auto const& value : rows) {
                rules::detail::visit_record<Node::InputType>(value, [&](Node::InputType const& record) {
                    node_ids.push_back(record.id);
                    if (record.id != na_IntID) {
                        valid_node_ids.insert(record.id);
                    }
                });
            }
        });
    }
    if (has_line) {
        validate_line(dataset, symmetric, valid_node_ids, issues);
    }
    if (has_node && has_line) {
        std::vector<ID> line_ids;
        dataset.for_each_component<meta_data::input_getter_s, Line>([&](auto const& rows) {
            for (auto const& value : rows) {
                rules::detail::visit_record<Line::InputType>(
                    value, [&](Line::InputType const& record) { line_ids.push_back(record.id); });
            }
        });
        rules::cross_unique(node_ids, line_ids, issues);
    }
    return issues;
}

namespace batch_detail {

inline std::pair<bool, bool> get_supported_components(ConstDataset const& dataset, std::string_view operation) {
    bool has_node = false;
    bool has_line = false;
    for (Idx component_idx{}; component_idx < dataset.n_components(); ++component_idx) {
        auto const component_name = dataset.get_component_info(component_idx).component->name;
        if (std::string_view{component_name} == "node") {
            has_node = true;
        } else if (std::string_view{component_name} == "line") {
            has_line = true;
        } else {
            throw InvalidArguments{std::string{operation} + " supports only node and line components; received '" +
                                   component_name + "'.\n"};
        }
    }
    return {has_node, has_line};
}

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
                        bool has_input_component, std::string_view component, Issues& issues,
                        std::vector<std::size_t>& indices) {
    if (updates.empty()) {
        return true;
    }
    if (!has_input_component || input.empty()) {
        add_nullable_id_issue(issues, Rule::invalid_update_id, component, {});
        return false;
    }

    bool const positional_ids =
        std::ranges::all_of(updates, [](UpdateRecord const& update) { return update.id == na_IntID; });
    if (positional_ids) {
        if (updates.size() != input.size()) {
            add_nullable_id_issue(issues, Rule::invalid_update_id, component, {});
            return false;
        }
        indices.reserve(updates.size());
        for (std::size_t index{}; index < updates.size(); ++index) {
            indices.push_back(index);
        }
        return true;
    }

    std::unordered_map<ID, std::size_t> input_index;
    input_index.reserve(input.size());
    for (std::size_t index{}; index < input.size(); ++index) {
        input_index.try_emplace(input[index].id, index);
    }

    std::vector<ID> invalid_ids;
    indices.reserve(updates.size());
    for (UpdateRecord const& update : updates) {
        auto const found = input_index.find(update.id);
        if (found == input_index.end()) {
            invalid_ids.push_back(update.id);
        } else {
            indices.push_back(found->second);
        }
    }
    if (!invalid_ids.empty()) {
        add_id_not_in_dataset_issue(issues, component, std::move(invalid_ids));
        return false;
    }
    return true;
}

} // namespace batch_detail

inline BatchIssues validate_batch_dataset(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                          bool symmetric) {
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

    auto const [has_input_node, has_input_line] =
        batch_detail::get_supported_components(input_dataset, "Input validation");
    static_cast<void>(batch_detail::get_supported_components(update_dataset, "Batch input validation"));

    auto const input_nodes = batch_detail::collect_records<meta_data::input_getter_s, Node, NodeInput>(input_dataset);
    auto const input_lines = batch_detail::collect_records<meta_data::input_getter_s, Line, LineInput>(input_dataset);
    std::vector<ID> node_ids;
    std::vector<ID> line_ids;
    node_ids.reserve(input_nodes.size());
    line_ids.reserve(input_lines.size());
    for (auto const& node : input_nodes) {
        node_ids.push_back(node.id);
    }
    for (auto const& line : input_lines) {
        line_ids.push_back(line.id);
    }

    Issues base_issues;
    if (has_input_node && has_input_line) {
        rules::cross_unique(node_ids, line_ids, base_issues);
    }

    BatchIssues batch_issues;
    batch_issues.reserve(static_cast<std::size_t>(update_dataset.batch_size()));
    for (Idx scenario{}; scenario < update_dataset.batch_size(); ++scenario) {
        auto const scenario_dataset = update_dataset.get_individual_scenario(scenario);
        auto const node_updates =
            batch_detail::collect_records<meta_data::update_getter_s, Node, Node::UpdateType>(scenario_dataset);
        auto const line_updates =
            batch_detail::collect_records<meta_data::update_getter_s, Line, Line::UpdateType>(scenario_dataset);

        Issues scenario_issues = base_issues;
        std::vector<std::size_t> node_indices;
        std::vector<std::size_t> line_indices;
        bool const node_ids_valid = batch_detail::get_update_indices(node_updates, input_nodes, has_input_node, "node",
                                                                     scenario_issues, node_indices);
        bool const line_ids_valid = batch_detail::get_update_indices(line_updates, input_lines, has_input_line, "line",
                                                                     scenario_issues, line_indices);
        if (!node_ids_valid || !line_ids_valid) {
            batch_issues.push_back(std::move(scenario_issues));
            continue;
        }

        auto merged_nodes = input_nodes;
        auto merged_lines = input_lines;
        for (std::size_t index{}; index < line_updates.size(); ++index) {
            auto const& update = line_updates[index];
            auto& line = merged_lines[line_indices[index]];
            if (update.from_status != na_IntS) {
                line.from_status = update.from_status;
            }
            if (update.to_status != na_IntS) {
                line.to_status = update.to_status;
            }
        }

        ConstDataset merged_dataset{false, 1, "input", input_dataset.meta_data()};
        if (has_input_node) {
            auto const size = static_cast<Idx>(merged_nodes.size());
            merged_dataset.add_buffer("node", size, size, nullptr, merged_nodes.data());
        }
        if (has_input_line) {
            auto const size = static_cast<Idx>(merged_lines.size());
            merged_dataset.add_buffer("line", size, size, nullptr, merged_lines.data());
        }
        batch_issues.push_back(validate_input_dataset(merged_dataset, symmetric));
    }
    return batch_issues;
}

} // namespace power_grid_model::validation
