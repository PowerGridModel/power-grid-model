// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "report.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <utility>

namespace power_grid_model::validation {

inline nlohmann::json to_json(ValidationIssue const& issue) {
    return {{"rule", issue.rule}, {"component", issue.component}, {"field", issue.field}, {"ids", issue.ids}};
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
        if (!scope.issues.empty()) {
            scopes.push_back(to_json(scope));
        }
    }

    auto const json_report = nlohmann::json{{"is_batch", report.is_batch}, {"results", std::move(scopes)}};
    return json_report.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}

} // namespace power_grid_model::validation
