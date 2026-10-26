// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "dataset.hpp"
#include "power_grid_model/auxiliary/dataset.hpp"
#include "power_grid_model/common/enum.hpp"
#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "power_grid_model/validation/registry.hpp"
#include "power_grid_model/validation/report.hpp"

#include <utility>

namespace power_grid_model::validation::detail {

inline ValidationReport validate_batch_with_registry(ConstDataset const& input_dataset,
                                                     ConstDataset const& update_dataset, bool symmetric,
                                                     CalculationType calculation_type,
                                                     ValidationRegistry const& registry) {
    validate_batch_dataset_type(input_dataset, update_dataset);

    ValidationContext const context{
        .input = input_dataset, .symmetric = symmetric, .calculation_type = calculation_type};
    Issues issues;
    for (auto const& component_rules : registry) {
        if (!should_validate_component(input_dataset, component_rules.component)) {
            continue;
        }
        apply_rules(component_rules.batch_rules, context, issues);
    }
    return {.type = ValidationType::batch, .scopes = {make_validation_scope(std::move(issues))}};
}

} // namespace power_grid_model::validation::detail
