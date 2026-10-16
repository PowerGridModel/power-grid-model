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

inline void validate_node(ConstDataset const& dataset, Issues& issues) {
    using Record = Node::InputType;

    Record const default_values{};
    auto const ids = dataset.get_attribute_view<Node, ID>("id", default_values.id);
    auto const u_rated = dataset.get_attribute_view<Node, double>("u_rated", default_values.u_rated);

    rules::required(u_rated, ids, Node::name, "u_rated", issues);
    rules::finite(u_rated, ids, Node::name, "u_rated", issues);
    rules::greater_than_zero(u_rated, ids, Node::name, "u_rated", issues);
}

} // namespace power_grid_model::validation
