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

#include <functional>
#include <string_view>
#include <utility>

namespace power_grid_model::validation::detail {
inline ValidationReport
validate_batch_with_registry(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                             DataValidatorOptions const& options, ValidationRegistry const& registry) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    ValidationContext context{.input = input_dataset, .options = options};
    Issues issues;
    ValidationReport report{.is_batch = true, .scopes = {}};

    check_input_ids(context, issues);

    for (auto const& component_rules : registry) {
        std::string_view const component_name = component_rules.component;
        // TODO: Validate optional update IDs in the dedicated ID-checking rules.
        if (should_validate_component(input_dataset, component_rules.component)) {
            for (auto const& rule : component_rules.batch_rules) {
                rule.apply(component_name, context, issues);
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
