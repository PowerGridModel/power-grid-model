// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"

#include <power_grid_model/common/common.hpp>

#include <algorithm>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

struct ValidationScope {
    Idx scenario{na_Idx};
    std::vector<ValidationIssue> issues;
};

struct ValidationReport {
    bool is_batch{};
    std::vector<ValidationScope> scopes;

    [[nodiscard]] bool valid() const noexcept {
        return std::ranges::all_of(scopes, [](ValidationScope const& scope) { return scope.issues.empty(); });
    }
};

inline ValidationScope make_validation_scope(std::vector<ValidationIssue> issues, Idx scenario = na_Idx) {
    return {.scenario = scenario, .issues = std::move(issues)};
}

} // namespace power_grid_model::validation
