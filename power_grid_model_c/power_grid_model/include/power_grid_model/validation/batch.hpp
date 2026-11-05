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
#include "rule_adapters.hpp"

#include "power_grid_model/auxiliary/dataset.hpp"

#include <functional>
#include <string_view>
#include <utility>

namespace power_grid_model::validation::detail {
template <class ComponentsTuple = AllComponentsTuple>
inline ValidationReport
validate_batch_with_registry(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                             DataValidatorOptions const& options, ValidationRegistry const& registry) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    ValidationContext context{.input = input_dataset, .options = options};
    Issues issues;

    check_input_ids<ComponentsTuple>(context, issues);
    // if there are issues with input IDs, return early
    if (!issues.empty()) {
        return {.type = ValidationType::input, .scopes = {make_validation_scope(std::move(issues))}};
    }

    for (auto const& component_rules : registry) {
        // TODO: Validate optional update IDs in the dedicated ID-checking rules.
        if (should_validate_component(input_dataset, component_rules.component)) {
            for (auto const& rule : component_rules.batch_rules) {
                rule.apply(context, issues);
            }
        }
    }
    return {.type = ValidationType::batch, .scopes = {make_validation_scope(std::move(issues))}};
}

} // namespace power_grid_model::validation::detail
