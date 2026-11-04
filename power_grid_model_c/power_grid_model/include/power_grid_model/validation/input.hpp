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

inline ValidationReport validate_input_with_registry(ConstDataset const& dataset, DataValidatorOptions const& options,
                                                     ValidationRegistry const& registry) {
    validate_dataset_type(dataset);

    ValidationContext const context{.input = dataset, .options = options};
    Issues issues;
    for (auto const& component_rules : registry) {
        if (!should_validate_component(dataset, component_rules.component)) {
            continue;
        }
        for (auto const& rule : component_rules.input_rules) {
            rule.apply(context, issues);
        }
    }
    return {.is_batch = false, .scopes = {make_validation_scope(std::move(issues))}};
}

} // namespace power_grid_model::validation::detail
