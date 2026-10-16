// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "line.hpp"
#include "node.hpp"
#include "report.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

#include <string_view>
#include <utility>

namespace power_grid_model::validation {

namespace detail {
inline void validate_dataset_type(const power_grid_model::ConstDataset& dataset) {
    if (std::string_view{dataset.dataset().name} != "input") {
        throw InvalidArguments{"Input validation requires dataset type 'input'.\n"};
    }
    if (dataset.is_batch()) {
        throw InvalidArguments{"Input validation does not support batch datasets.\n"};
    }
}
} // namespace detail

inline ValidationReport validate_input_dataset(ConstDataset const& dataset, bool /*symmetric*/,
                                               CalculationType /*calculation_type*/) {
    detail::validate_dataset_type(dataset);

    Issues issues;
    validate_node(dataset, issues);
    validate_line(dataset, issues);
    ValidationScope scope{.status = issues.empty() ? ValidationStatus::passed : ValidationStatus::failed,
                          .issues = std::move(issues)};
    return {.type = ValidationType::input, .scopes = {std::move(scope)}};
}

} // namespace power_grid_model::validation
