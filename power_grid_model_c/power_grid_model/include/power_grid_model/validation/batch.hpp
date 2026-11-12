// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "context.hpp"
#include "dataset.hpp"
#include "issues.hpp"
#include "options.hpp"
#include "registry.hpp"
#include "rule_adapters.hpp"

#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/counting_iterator.hpp"

#include <functional>
#include <string_view>
#include <utility>

namespace power_grid_model::validation::detail {
template <class ComponentsTuple = AllComponentsTuple>
inline ValidationReport
validate_batch_with_registry(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                             DataValidatorOptions const& options, ValidationRegistry const& registry) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    ValidationContext context{.input = input_dataset, .update = update_dataset, .options = options};
    Issues issues;
    ValidationReport report{.is_batch = true, .scopes = {}};
    Idx const batch_size = update_dataset.batch_size();

    // input IDs sweep
    check_input_ids<ComponentsTuple>(context, issues);
    // TODO(figueroa1395): don't exit early - think this through first
    // if there are issues with input IDs, return early
    if (!issues.empty()) {
        report.scopes.push_back({.scenario = na_Idx, .issues = std::move(issues)});
        return report;
    }

    // batch unary rules sweep
    for (auto const scenario_idx : IdxRange{batch_size}) {
        for (auto const& component_rules : registry) {
            // TODO: Validate optional update IDs in the dedicated ID-checking rules.
            if (should_validate_component(update_dataset, component_rules.component)) {
                for (auto const& rule : component_rules.batch_rules) {
                    rule.apply(context, issues, scenario_idx);
                }
            }
        }
    }

    if (!issues.empty()) {
        report.scopes.emplace_back(na_Idx, std::move(issues));
    }
    report.is_valid = report.scopes.empty();
    return report;
}

} // namespace power_grid_model::validation::detail
