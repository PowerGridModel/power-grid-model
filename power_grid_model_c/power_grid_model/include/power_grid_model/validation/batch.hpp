// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "dataset.hpp"
#include "input.hpp"

#include <utility>

namespace power_grid_model::validation::detail {

inline ValidationReport validate_batch_with_registry(ConstDataset const& input_dataset,
                                                     ConstDataset const& update_dataset, bool symmetric,
                                                     CalculationType calculation_type,
                                                     ValidationRegistry const& registry) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    ValidationContext const context{input_dataset, &update_dataset, symmetric, calculation_type};
    Issues issues;
    for (auto const& component_rules : registry) {
        apply_rules(component_rules.batch_rules, context, issues);
    }
    return {.type = ValidationType::batch, .scopes = {make_validation_scope(std::move(issues))}};
}

} // namespace power_grid_model::validation::detail

namespace power_grid_model::validation {

inline ValidationReport validate_batch_report(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                              bool symmetric, CalculationType calculation_type) {
    return detail::validate_batch_with_registry(input_dataset, update_dataset, symmetric, calculation_type,
                                                default_registry());
}

} // namespace power_grid_model::validation
