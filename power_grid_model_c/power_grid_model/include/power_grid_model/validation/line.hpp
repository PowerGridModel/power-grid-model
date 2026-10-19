// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/component/line.hpp>

namespace power_grid_model::validation {
namespace detail {
inline void validate_line_impl(ConstDataset const& dataset, Issues& issues,
                               Idx scenario_idx = ConstDataset::invalid_index) {
    dataset.for_each_component<meta_data::input_getter_s, Line>(
        [&issues](auto const& elements) {
            auto const ids = meta_data::get_attribute_span(elements, &LineInput::id, "id");
            auto const from_status = meta_data::get_attribute_span(elements, &LineInput::from_status, "from_status");
            auto const r1 = meta_data::get_attribute_span(elements, &LineInput::r1, "r1");

            rules::required(from_status, ids, Line::name, "from_status", issues);
            rules::required(r1, ids, Line::name, "r1", issues);
            rules::finite(r1, ids, Line::name, "r1", issues);
            rules::boolean(from_status, ids, Line::name, "from_status", issues);
        },
        scenario_idx);
}
} // namespace detail

inline void validate_line(ConstDataset const& dataset, Issues& issues) {
    dataset.for_each_component<meta_data::input_getter_s, Line>([&issues](auto const& elements) {
        auto const ids = meta_data::get_attribute_span(elements, &LineInput::id, "id");
        auto const from_status = meta_data::get_attribute_span(elements, &LineInput::from_status, "from_status");
        auto const r1 = meta_data::get_attribute_span(elements, &LineInput::r1, "r1");

        rules::required(from_status, ids, Line::name, "from_status", issues);
        rules::required(r1, ids, Line::name, "r1", issues);
        rules::finite(r1, ids, Line::name, "r1", issues);
        rules::boolean(from_status, ids, Line::name, "from_status", issues);
    });
}
inline void validate_line(ConstDataset const& input_dataset, ConstDataset const& /*update_dataset*/, Issues& issues,
                          Idx scenario_idx = ConstDataset::invalid_index) {
    (void)scenario_idx;
    input_dataset.for_each_component<meta_data::input_getter_s, Line>([&issues](auto const& elements) {
        auto const ids = meta_data::get_attribute_span(elements, &LineInput::id, "id");
        auto const from_status = meta_data::get_attribute_span(elements, &LineInput::from_status, "from_status");

        rules::required(from_status, ids, Line::name, "from_status", issues);
        rules::boolean(from_status, ids, Line::name, "from_status", issues);
    });
}

} // namespace power_grid_model::validation
