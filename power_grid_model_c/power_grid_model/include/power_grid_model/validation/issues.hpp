// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/exception.hpp"
#include "report.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class RuleCode : std::uint8_t { missing_value, infinity, not_greater_than_zero, not_boolean };

constexpr char const* to_string(RuleCode rule) {
    switch (rule) {
    case RuleCode::missing_value:
        return "missing_value";
    case RuleCode::infinity:
        return "infinity";
    case RuleCode::not_greater_than_zero:
        return "not_greater_than_zero";
    case RuleCode::not_boolean:
        return "not_boolean";
    }
    throw MissingCaseForEnumError{"validation::to_string", rule};
}

using Issues = std::vector<ValidationIssue>;

inline void add_issue(Issues& issues, std::string_view rule, std::string_view component, std::string_view field,
                      std::vector<ID> ids = {}) {
    if (ids.empty()) {
        return;
    }
    issues.push_back({.rule = std::string{rule},
                      .component = std::string{component},
                      .field = std::string{field},
                      .ids = std::move(ids)});
}

inline void add_issue(Issues& issues, RuleCode rule, std::string_view component, std::string_view field,
                      std::vector<ID> ids = {}) {
    add_issue(issues, to_string(rule), component, field, std::move(ids));
}

} // namespace power_grid_model::validation
