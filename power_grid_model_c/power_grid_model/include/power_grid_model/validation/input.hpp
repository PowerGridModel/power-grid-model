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

#include <utility>

namespace power_grid_model::validation::detail {

template <class ComponentsTuple = AllComponentsTuple>
inline ValidationReport validate_input_with_registry(ConstDataset const& dataset, DataValidatorOptions const& options,
                                                     ValidationRegistry const& registry) {
    validate_dataset_type(dataset);

    ValidationContext context{.input = dataset, .options = options};
    Issues issues;

    check_input_ids<ComponentsTuple>(context, issues);
    // if there are issues with input IDs, return early
    if (!issues.empty()) {
        return {.type = ValidationType::input, .scopes = {make_validation_scope(std::move(issues))}};
    }

    for (auto const& component_rules : registry) {
        if (!should_validate_component(dataset, component_rules.component)) {
            continue;
        }
        for (auto const& rule : component_rules.input_rules) {
            rule.apply(context, issues);
        }
    }
    ValidationReport report{.is_batch = false, .scopes = {}};
    if (!issues.empty()) {
        report.scopes.push_back({.scenario = na_Idx, .issues = std::move(issues)});
    }
    return report;
}

} // namespace power_grid_model::validation::detail
