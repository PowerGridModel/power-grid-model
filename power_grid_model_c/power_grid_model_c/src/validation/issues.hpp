#pragma once

#include <power_grid_model/common/common.hpp>

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace power_grid_model_c::validation {

enum class Rule { missing_value, infinity, not_unique, not_greater_than_zero };

struct Issue {
    Rule rule;
    std::string_view component;
    std::string_view field;
    std::vector<power_grid_model::ID> ids;

    friend bool operator==(Issue const&, Issue const&) = default;
};

using Issues = std::vector<Issue>;

inline void add_issue(Issues& issues, Rule rule, std::string_view component, std::string_view field,
                      std::vector<power_grid_model::ID> ids) {
    if (ids.empty()) {
        return;
    }
    std::ranges::sort(ids);
    issues.push_back({.rule = rule, .component = component, .field = field, .ids = std::move(ids)});
}

} // namespace power_grid_model_c::validation
