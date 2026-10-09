#pragma once

#include "issues.hpp"

#include <power_grid_model/common/common.hpp>

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::validation {

struct ValidationContext {
    std::unordered_map<std::string_view, std::vector<ID>> ids_by_component;
    std::unordered_map<std::string_view, std::vector<ObjectReference>> objects_by_component;
    std::unordered_map<std::string_view, std::unordered_set<ID>> valid_ids_by_component;

    std::unordered_set<ID> const& valid_ids(std::string_view component) const {
        static std::unordered_set<ID> const empty_ids;
        auto const found = valid_ids_by_component.find(component);
        return found == valid_ids_by_component.end() ? empty_ids : found->second;
    }
};

template <class Component> struct InputValidator;

} // namespace power_grid_model::validation