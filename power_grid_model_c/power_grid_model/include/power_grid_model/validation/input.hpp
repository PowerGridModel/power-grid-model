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

#include "../auxiliary/dataset.hpp"
#include "../common/common.hpp"

#include <string_view>
#include <utility>

namespace power_grid_model::validation::detail {
inline ValidationReport validate_input_with_registry(ConstDataset const& dataset, DataValidatorOptions const& options,
                                                     ValidationRegistry const& registry) {
    validate_dataset_type(dataset);

    ValidationContext context{.input = dataset, .options = options};
    Issues issues;
    ValidationReport report{.is_batch = false, .scopes = {}};

    check_input_ids(context, issues);

    for (auto const& component_rules : registry) {
        std::string_view const component_name = component_rules.component;
        if (!should_validate_component(dataset, component_rules.component)) {
            continue;
        }
        for (auto const& rule : component_rules.input_rules) {
            rule.apply(component_name, context, issues);
        }
    }

    if (!issues.empty()) {
        report.scopes.emplace_back(na_Idx, std::move(issues));
    }
    report.is_valid = report.scopes.empty();
    return report;
}

} // namespace power_grid_model::validation::detail
