// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "dataset.hpp"
#include "line.hpp"
#include "node.hpp"
#include "report.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>

#include <utility>

namespace power_grid_model::validation {

inline ValidationReport validate_input_dataset(ConstDataset const& dataset, bool /*symmetric*/,
                                               CalculationType /*calculation_type*/) {
    validate_dataset_type(dataset);

    Issues issues;
    validate_node(dataset, issues);
    validate_line(dataset, issues);
    ValidationScope scope{.status = issues.empty() ? ValidationStatus::passed : ValidationStatus::failed,
                          .issues = std::move(issues)};
    return {.type = ValidationType::input, .scopes = {std::move(scope)}};
}

} // namespace power_grid_model::validation
