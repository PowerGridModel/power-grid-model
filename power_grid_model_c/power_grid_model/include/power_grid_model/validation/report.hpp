// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class ValidationType : std::uint8_t { input, batch };
enum class ValidationStatus : std::uint8_t { passed, failed, blocked };

constexpr char const* to_string(ValidationType type) {
    switch (type) {
    case ValidationType::input:
        return "input";
    case ValidationType::batch:
        return "batch";
    }
    throw MissingCaseForEnumError{"ValidationType::to_string", type};
}

struct ValidationIssue {
    std::string rule;
    std::string component;
    std::string field;
    std::vector<ID> ids;
};

struct ValidationScope {
    Idx scenario{na_Idx};
    ValidationStatus status{ValidationStatus::passed};
    std::vector<ValidationIssue> issues;
};

struct ValidationReport {
    ValidationType type{ValidationType::input};
    std::vector<ValidationScope> scopes;

    [[nodiscard]] bool valid() const noexcept {
        return std::ranges::all_of(scopes, [](ValidationScope const& scope) {
            return scope.status == ValidationStatus::passed && scope.issues.empty();
        });
    }
};

inline ValidationScope make_validation_scope(std::vector<ValidationIssue> issues) {
    ValidationStatus const status = issues.empty() ? ValidationStatus::passed : ValidationStatus::failed;
    return {.scenario = na_Idx, .status = status, .issues = std::move(issues)};
}

} // namespace power_grid_model::validation
