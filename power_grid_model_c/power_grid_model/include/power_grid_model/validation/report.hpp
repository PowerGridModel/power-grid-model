#pragma once

#include "issues.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace power_grid_model::validation {

enum class ValidationType { input, batch };
enum class ValidationStatus { passed, failed, blocked };

struct ValidationScope {
    std::optional<std::size_t> scenario;
    ValidationStatus status;
    Issues issues;
};

struct ValidationReport {
    ValidationType type;
    std::vector<ValidationScope> results;
};

} // namespace power_grid_model::validation