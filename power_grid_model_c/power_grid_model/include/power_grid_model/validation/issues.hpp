#pragma once

#include <power_grid_model/common/common.hpp>

#include <algorithm>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class Rule {
    missing_value,
    infinity,
    not_unique,
    not_greater_than_zero,
    not_boolean,
    two_values_zero,
    invalid_id_reference,
    cross_component_not_unique,
};

struct FieldReference {
    std::string_view component;
    std::string_view field;

    friend bool operator==(FieldReference const&, FieldReference const&) = default;
};

struct ObjectReference {
    std::string_view component;
    ID id;

    friend bool operator==(ObjectReference const&, ObjectReference const&) = default;
};

struct Issue {
    Rule rule;
    std::string_view component;
    std::string_view field;
    std::vector<ID> ids;
    std::vector<FieldReference> fields;
    std::vector<ObjectReference> objects;
    std::string_view reference_component;

    friend bool operator==(Issue const&, Issue const&) = default;
};

using Issues = std::vector<Issue>;

inline void add_issue(Issues& issues, Rule rule, std::string_view component, std::string_view field,
                      std::vector<ID> ids) {
    if (ids.empty()) {
        return;
    }
    std::ranges::sort(ids);
    Issue issue{.rule = rule,
                .component = component,
                .field = field,
                .ids = std::move(ids),
                .fields = {},
                .objects = {},
                .reference_component = {}};
    issue.fields.push_back({.component = component, .field = field});
    for (ID id : issue.ids) {
        issue.objects.push_back({.component = component, .id = id});
    }
    issues.push_back(std::move(issue));
}

inline void add_multi_field_issue(Issues& issues, Rule rule, std::string_view component,
                                  std::vector<std::string_view> fields, std::vector<ID> ids) {
    if (ids.empty()) {
        return;
    }
    std::ranges::sort(ids);
    Issue issue{.rule = rule,
                .component = component,
                .field = fields.front(),
                .ids = std::move(ids),
                .fields = {},
                .objects = {},
                .reference_component = {}};
    for (std::string_view field : fields) {
        issue.fields.push_back({.component = component, .field = field});
    }
    for (ID id : issue.ids) {
        issue.objects.push_back({.component = component, .id = id});
    }
    issues.push_back(std::move(issue));
}

inline void add_reference_issue(Issues& issues, std::string_view component, std::string_view field,
                                std::string_view reference_component, std::vector<ID> ids) {
    if (ids.empty()) {
        return;
    }
    add_issue(issues, Rule::invalid_id_reference, component, field, std::move(ids));
    issues.back().reference_component = reference_component;
}

inline void add_cross_component_unique_issue(Issues& issues, std::vector<ID> duplicate_ids,
                                             std::string_view first_component, std::string_view second_component) {
    if (duplicate_ids.empty()) {
        return;
    }
    std::ranges::sort(duplicate_ids);
    duplicate_ids.erase(std::unique(duplicate_ids.begin(), duplicate_ids.end()), duplicate_ids.end());
    Issue issue{
        .rule = Rule::cross_component_not_unique,
        .component = first_component,
        .field = "id",
        .ids = {},
        .fields = {{.component = first_component, .field = "id"}, {.component = second_component, .field = "id"}},
        .objects = {},
        .reference_component = {}};
    for (ID id : duplicate_ids) {
        issue.objects.push_back({.component = first_component, .id = id});
        issue.objects.push_back({.component = second_component, .id = id});
    }
    std::ranges::sort(issue.objects, [](ObjectReference const& first, ObjectReference const& second) {
        return std::tie(first.component, first.id) < std::tie(second.component, second.id);
    });
    issues.push_back(std::move(issue));
}

} // namespace power_grid_model::validation
