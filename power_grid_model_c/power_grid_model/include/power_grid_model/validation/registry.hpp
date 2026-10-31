// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"

#include <functional>
#include <string_view>
#include <vector>

namespace power_grid_model::validation {

struct ValidationContext;

struct RuleDefinition {
    RuleCode code;
    std::vector<std::string_view> fields;
    std::function<void(ValidationContext const&, Issues&)> apply;
};

struct ComponentRules {
    std::string_view component;
    std::vector<RuleDefinition> input_rules;
    std::vector<RuleDefinition> batch_rules;
};

using ValidationRegistry = std::vector<ComponentRules>;

} // namespace power_grid_model::validation
