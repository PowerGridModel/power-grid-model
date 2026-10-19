// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/component/node.hpp>

namespace power_grid_model::validation {
inline void validate_node(ConstDataset const& input_dataset, Issues& issues) {
    input_dataset.for_each_component<meta_data::input_getter_s, Node>([&issues](auto const& elements) {
        auto const ids = meta_data::get_attribute_span(elements, &NodeInput::id, "id");
        auto const u_rated = meta_data::get_attribute_span(elements, &NodeInput::u_rated, "u_rated");

        rules::required(u_rated, ids, Node::name, "u_rated", issues);
        rules::finite(u_rated, ids, Node::name, "u_rated", issues);
        rules::greater_than_zero(u_rated, ids, Node::name, "u_rated", issues);
    });
}

} // namespace power_grid_model::validation
