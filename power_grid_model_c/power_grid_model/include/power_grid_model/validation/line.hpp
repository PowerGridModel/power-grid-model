#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/component/line.hpp>

#include <unordered_set>

namespace power_grid_model::validation {

inline void validate_line(ConstDataset const& dataset, bool symmetric, std::unordered_set<ID> const& node_ids,
                          Issues& issues) {
    using Record = Line::InputType;

    auto const get_id = [](Record const& record) { return record.id; };
    auto const get_from_node = [](Record const& record) { return record.from_node; };
    auto const get_to_node = [](Record const& record) { return record.to_node; };
    auto const get_from_status = [](Record const& record) { return record.from_status; };
    auto const get_to_status = [](Record const& record) { return record.to_status; };
    auto const get_r1 = [](Record const& record) { return record.r1; };
    auto const get_x1 = [](Record const& record) { return record.x1; };
    auto const get_c1 = [](Record const& record) { return record.c1; };
    auto const get_tan1 = [](Record const& record) { return record.tan1; };
    auto const get_r0 = [](Record const& record) { return record.r0; };
    auto const get_x0 = [](Record const& record) { return record.x0; };
    auto const get_c0 = [](Record const& record) { return record.c0; };
    auto const get_tan0 = [](Record const& record) { return record.tan0; };
    auto const get_i_n = [](Record const& record) { return record.i_n; };

    dataset.for_each_component<meta_data::input_getter_s, Line>([&](auto const& rows) {
        rules::required<Record>(rows, "line", "id", get_id, get_id, issues);
        rules::required<Record>(rows, "line", "from_node", get_from_node, get_id, issues);
        rules::required<Record>(rows, "line", "to_node", get_to_node, get_id, issues);
        rules::required<Record>(rows, "line", "from_status", get_from_status, get_id, issues);
        rules::required<Record>(rows, "line", "to_status", get_to_status, get_id, issues);
        rules::required<Record>(rows, "line", "r1", get_r1, get_id, issues);
        rules::required<Record>(rows, "line", "x1", get_x1, get_id, issues);
        rules::required<Record>(rows, "line", "c1", get_c1, get_id, issues);
        rules::required<Record>(rows, "line", "tan1", get_tan1, get_id, issues);
        if (!symmetric) {
            rules::required<Record>(rows, "line", "r0", get_r0, get_id, issues);
            rules::required<Record>(rows, "line", "x0", get_x0, get_id, issues);
            rules::required<Record>(rows, "line", "c0", get_c0, get_id, issues);
            rules::required<Record>(rows, "line", "tan0", get_tan0, get_id, issues);
        }

        rules::finite<Record>(rows, "line", "r1", get_r1, get_id, issues);
        rules::finite<Record>(rows, "line", "x1", get_x1, get_id, issues);
        rules::finite<Record>(rows, "line", "c1", get_c1, get_id, issues);
        rules::finite<Record>(rows, "line", "tan1", get_tan1, get_id, issues);
        rules::finite<Record>(rows, "line", "r0", get_r0, get_id, issues);
        rules::finite<Record>(rows, "line", "x0", get_x0, get_id, issues);
        rules::finite<Record>(rows, "line", "c0", get_c0, get_id, issues);
        rules::finite<Record>(rows, "line", "tan0", get_tan0, get_id, issues);
        rules::finite<Record>(rows, "line", "i_n", get_i_n, get_id, issues);

        rules::unique<Record>(rows, "line", "id", get_id, get_id, issues);
        rules::valid_id_reference<Record>(rows, "line", "from_node", "node", get_from_node, get_id, node_ids, issues);
        rules::valid_id_reference<Record>(rows, "line", "to_node", "node", get_to_node, get_id, node_ids, issues);
        rules::boolean<Record>(rows, "line", "from_status", get_from_status, get_id, issues);
        rules::boolean<Record>(rows, "line", "to_status", get_to_status, get_id, issues);
        rules::not_both_zero<Record>(rows, "line", "r1", "x1", get_r1, get_x1, get_id, issues);
        rules::not_both_zero<Record>(rows, "line", "r0", "x0", get_r0, get_x0, get_id, issues);
        rules::greater_than_zero<Record>(rows, "line", "i_n", get_i_n, get_id, issues);
    });
}

} // namespace power_grid_model::validation
