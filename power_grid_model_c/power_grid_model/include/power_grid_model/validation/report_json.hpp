// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/common/exception.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <utility>

namespace power_grid_model::validation {

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

inline nlohmann::json to_json(ValidationIssue const& issue) {
    return {{"rule", std::string{to_string(issue.rule)}},
            {"component", issue.component},
            {"field", issue.field},
            {"ids", issue.ids}};
}

inline nlohmann::json to_json(ValidationScope const& scope) {
    nlohmann::json scenario = nullptr;
    if (scope.scenario != na_Idx) {
        scenario = scope.scenario;
    }

    auto issues = nlohmann::json::array();
    for (ValidationIssue const& issue : scope.issues) {
        issues.push_back(to_json(issue));
    }

    return {{"scenario", std::move(scenario)}, {"issues", std::move(issues)}};
}

inline std::string to_json_report(ValidationReport const& report) {
    auto scopes = nlohmann::json::array();
    for (ValidationScope const& scope : report.scopes) {
        scopes.push_back(to_json(scope));
    }

    constexpr int schema_version = 0;
    auto const json_report = nlohmann::json{{"schema_version", schema_version},
                                            {"is_batch", report.is_batch},
                                            {"valid", report.valid()},
                                            {"results", std::move(scopes)}};
    return json_report.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}

} // namespace power_grid_model::validation
