// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/component/node.hpp>

#include <ranges>

namespace power_grid_model::validation {

inline void validate_node(ConstDataset const& dataset, Issues& issues) {
    using Record = Node::InputType;

    Record const default_values{};
    auto const validate_attributes = [&issues](auto const& ids, auto const& u_rated) {
        rules::required(u_rated, ids, Node::name, "u_rated", issues);
        rules::finite(u_rated, ids, Node::name, "u_rated", issues);
        rules::greater_than_zero(u_rated, ids, Node::name, "u_rated", issues);
    };

    dataset.for_each_component<meta_data::input_getter_s, Node>([&](auto const& rows) {
        if constexpr (requires { rows.template get_attribute_view<ID>("id", default_values.id); }) {
            auto const ids = rows.template get_attribute_view<ID>("id", default_values.id);
            auto const u_rated = rows.template get_attribute_view<double>("u_rated", default_values.u_rated);
            validate_attributes(ids, u_rated);
        } else {
            auto const ids = rows | std::views::transform(&Record::id);
            auto const u_rated = rows | std::views::transform(&Record::u_rated);
            validate_attributes(ids, u_rated);
        }
    });
}

} // namespace power_grid_model::validation
