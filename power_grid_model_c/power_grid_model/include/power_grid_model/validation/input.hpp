#pragma once

#include "issues.hpp"
#include "line.hpp"
#include "node.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

#include <string>
#include <string_view>
#include <unordered_set>
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

} // namespace power_grid_model::validation
