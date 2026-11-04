// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

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

constexpr std::string_view to_string(RuleCode rule) {
    using enum RuleCode;
    switch (rule) {
    case missing_value:
        return "missing_value";
    case infinity:
        return "infinity";
    case not_greater_than_zero:
        return "not_greater_than_zero";
    case not_boolean:
        return "not_boolean";
    default:
        throw MissingCaseForEnumError{"validation::to_string", rule};
    }
}

inline void add_issue(Issues& issues, RuleCode rule, std::string_view component, std::string_view field,
                      std::vector<ID> ids = {}) {
    if (ids.empty()) {
        return;
    }
    issues.push_back(
        {.rule = rule, .component = std::string{component}, .field = std::string{field}, .ids = std::move(ids)});
}

} // namespace power_grid_model::validation
