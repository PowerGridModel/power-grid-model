#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/component/node.hpp>

namespace power_grid_model::validation {

inline void validate_node(ConstDataset const& dataset, Issues& issues) {
    using Record = Node::InputType;

    auto const get_id = [](Record const& record) { return record.id; };
    auto const get_u_rated = [](Record const& record) { return record.u_rated; };

    dataset.for_each_component<meta_data::input_getter_s, Node>([&issues, &get_id, &get_u_rated](auto const& rows) {
        rules::required<Record>(rows, "node", "id", get_id, get_id, issues);
        rules::required<Record>(rows, "node", "u_rated", get_u_rated, get_id, issues);
        rules::finite<Record>(rows, "node", "u_rated", get_u_rated, get_id, issues);
        rules::unique<Record>(rows, "node", "id", get_id, get_id, issues);
        rules::greater_than_zero<Record>(rows, "node", "u_rated", get_u_rated, get_id, issues);
    });
}

} // namespace power_grid_model::validation
