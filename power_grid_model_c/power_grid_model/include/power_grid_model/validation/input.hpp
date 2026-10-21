// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "dataset.hpp"
#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "registry.hpp"
#include "report.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>

#include <utility>

namespace power_grid_model::validation::detail {

inline ValidationReport validate_input_with_registry(ConstDataset const& dataset, bool symmetric,
                                                     CalculationType calculation_type,
                                                     ValidationRegistry const& registry) {
    validate_dataset_type(dataset);

    ValidationContext const context{
        .input = dataset, .update = nullptr, .symmetric = symmetric, .calculation_type = calculation_type};
    Issues issues;
    for (auto const& component_rules : registry) {
        apply_rules(component_rules.input_rules, context, issues);
    }
    return {.type = ValidationType::input, .scopes = {make_validation_scope(std::move(issues))}};
}

} // namespace power_grid_model::validation::detail

namespace power_grid_model::validation {

inline ValidationReport validate_input_dataset(ConstDataset const& dataset, bool symmetric,
                                               CalculationType calculation_type) {
    return detail::validate_input_with_registry(dataset, symmetric, calculation_type, default_registry());
}

} // namespace power_grid_model::validation
