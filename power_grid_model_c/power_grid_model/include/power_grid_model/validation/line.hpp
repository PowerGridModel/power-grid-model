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

inline void validate_line(ConstDataset const& dataset, Issues& issues) {
    LineInput const default_values{};
    auto const ids = dataset.get_attribute_view<Line, ID>("id", default_values.id);
    auto const from_status = dataset.get_attribute_view<Line, IntS>("from_status", default_values.from_status);
    auto const r1 = dataset.get_attribute_view<Line, double>("r1", default_values.r1);

    rules::required(from_status, ids, Line::name, "from_status", issues);
    rules::required(r1, ids, Line::name, "r1", issues);
    rules::finite(r1, ids, Line::name, "r1", issues);
    rules::boolean(from_status, ids, Line::name, "from_status", issues);
}

} // namespace power_grid_model::validation
