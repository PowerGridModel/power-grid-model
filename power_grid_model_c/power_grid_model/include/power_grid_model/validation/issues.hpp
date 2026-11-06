// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/common/common.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class RuleCode : std::uint8_t { missing_value, infinity, not_greater_than_zero, not_boolean };

struct ValidationIssue {
    RuleCode rule;
    std::string component;
    std::string field;
    std::vector<ID> ids;
};

using Issues = std::vector<ValidationIssue>;

struct ValidationScope {
    Idx scenario{na_Idx};
    Issues issues;
};

struct ValidationReport {
    bool is_batch{};
    std::vector<ValidationScope> scopes;

    [[nodiscard]] bool valid() const noexcept {
        return std::ranges::all_of(scopes, [](ValidationScope const& scope) { return scope.issues.empty(); });
    }
};

inline void add_issue(Issues& issues, RuleCode rule, std::string_view component, std::string_view field,
                      std::vector<ID> ids = {}) {
    if (ids.empty()) {
        return;
    }
    issues.push_back(
        {.rule = rule, .component = std::string{component}, .field = std::string{field}, .ids = std::move(ids)});
}

} // namespace power_grid_model::validation
