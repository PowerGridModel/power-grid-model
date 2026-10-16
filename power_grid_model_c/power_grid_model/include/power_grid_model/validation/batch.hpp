// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "dataset.hpp"
#include "input.hpp"

namespace power_grid_model::validation {

inline ValidationReport validate_batch_report(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                              bool /*symmetric*/, CalculationType /*calculation_type*/) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    Issues issues;
    validate_line(input_dataset, update_dataset, issues);
    ValidationScope scope{.status = issues.empty() ? ValidationStatus::passed : ValidationStatus::failed,
                          .issues = std::move(issues)};
    return {.type = ValidationType::batch, .scopes = {std::move(scope)}};
}

} // namespace power_grid_model::validation
