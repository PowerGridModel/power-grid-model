// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "dataset.hpp"
#include "issues.hpp"
#include "options.hpp"
#include "registry.hpp"
#include "report.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>

#include <utility>

namespace power_grid_model::validation::detail {

inline ValidationReport validate_batch_with_registry(ConstDataset const& input_dataset,
                                                     ConstDataset const& update_dataset,
                                                     DataValidatorOptions const& options,
                                                     ValidationRegistry const& registry) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    ValidationContext const context{.input = input_dataset, .options = options};
    Issues issues;
    for (auto const& component_rules : registry) {
        // TODO: Validate optional update IDs in the dedicated ID-checking rules.
        if (should_validate_component(input_dataset, component_rules.component)) {
            for (auto const& rule : component_rules.batch_rules) {
                rule.apply(context, issues);
            }
        }
    }
    return {.is_batch = true, .scopes = {make_validation_scope(std::move(issues))}};
}

} // namespace power_grid_model::validation::detail
