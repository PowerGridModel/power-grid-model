#pragma once

#include "issues.hpp"

#include <power_grid_model/common/common.hpp>

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::validation {

struct ValidationContext {
    std::unordered_map<std::string_view, std::vector<ObjectReference>> objects_by_component;

    std::unordered_set<ID> valid_ids(std::string_view component) const {
        std::unordered_set<ID> ids;
        auto const found = objects_by_component.find(component);
        if (found != objects_by_component.end()) {
            for (ObjectReference const& object : found->second) {
                if (object.id != na_IntID) {
                    ids.insert(object.id);
                }
            }
        }
        return ids;
    }
};

template <class Component> struct ComponentValueValidator;

} // namespace power_grid_model::validation
