// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "report.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

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

} // namespace power_grid_model::validation
