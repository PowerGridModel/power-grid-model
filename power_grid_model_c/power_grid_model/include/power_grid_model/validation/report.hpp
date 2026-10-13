#pragma once

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

#include <string>
#include <vector>

namespace power_grid_model::validation {

enum class ValidationType { input, batch };
enum class ValidationStatus { passed, failed, blocked };

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
        for (ValidationScope const& scope : scopes) {
            if (scope.status != ValidationStatus::passed || !scope.issues.empty()) {
                return false;
            }
        }
        return true;
    }
};

} // namespace power_grid_model::validation
