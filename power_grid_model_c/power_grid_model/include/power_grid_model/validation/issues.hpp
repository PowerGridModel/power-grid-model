#pragma once

#include <power_grid_model/common/common.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
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
    id_not_in_dataset,
    invalid_update_id,
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
    std::optional<std::size_t> row{};

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

inline void append_issues(Issues& issues, Issues&& additional_issues) {
    issues.reserve(issues.size() + additional_issues.size());
    for (Issue& issue : additional_issues) {
        issues.push_back(std::move(issue));
    }
}

inline void add_issue(Issues& issues, Rule rule, std::string_view component, std::string_view field,
                      std::vector<ObjectReference> objects, bool sort_objects = true) {
    if (objects.empty()) {
        return;
    }
    if (sort_objects) {
        std::ranges::sort(objects, [](ObjectReference const& first, ObjectReference const& second) {
            return std::tie(first.component, first.id, first.row) < std::tie(second.component, second.id, second.row);
        });
    }
    std::vector<ID> ids;
    ids.reserve(objects.size());
    for (auto const& object : objects) {
        ids.push_back(object.id);
    }
    Issue issue{.rule = rule,
                .component = component,
                .field = field,
                .ids = std::move(ids),
                .fields = {{.component = component, .field = field}},
                .objects = std::move(objects),
                .reference_component = {}};
    issues.push_back(std::move(issue));
}

inline void add_update_addressing_issue(Issues& issues, std::string_view component,
                                        std::vector<ObjectReference> objects) {
    add_issue(issues, Rule::invalid_update_id, component, "id", std::move(objects), false);
}

inline void add_id_not_in_dataset_issue(Issues& issues, std::string_view component,
                                        std::vector<ObjectReference> objects) {
    if (objects.empty()) {
        return;
    }
    std::vector<ID> ids;
    ids.reserve(objects.size());
    for (auto const& object : objects) {
        ids.push_back(object.id);
    }
    Issue issue{.rule = Rule::id_not_in_dataset,
                .component = component,
                .field = "id",
                .ids = std::move(ids),
                .fields = {{.component = component, .field = "id"}},
                .objects = std::move(objects),
                .reference_component = {}};
    issues.push_back(std::move(issue));
}

inline void add_multi_field_issue(Issues& issues, Rule rule, std::string_view component,
                                  std::vector<std::string_view> fields, std::vector<ObjectReference> objects) {
    if (objects.empty()) {
        return;
    }
    std::ranges::sort(objects, [](ObjectReference const& first, ObjectReference const& second) {
        return std::tie(first.component, first.id, first.row) < std::tie(second.component, second.id, second.row);
    });
    std::ranges::sort(fields);
    std::vector<ID> ids;
    ids.reserve(objects.size());
    for (auto const& object : objects) {
        ids.push_back(object.id);
    }
    Issue issue{.rule = rule,
                .component = component,
                .field = fields.front(),
                .ids = std::move(ids),
                .fields = {},
                .objects = std::move(objects),
                .reference_component = {}};
    for (std::string_view field : fields) {
        issue.fields.push_back({.component = component, .field = field});
    }
    issues.push_back(std::move(issue));
}

inline void add_reference_issue(Issues& issues, std::string_view component, std::string_view field,
                                std::string_view reference_component, std::vector<ObjectReference> objects) {
    if (objects.empty()) {
        return;
    }
    add_issue(issues, Rule::invalid_id_reference, component, field, std::move(objects));
    issues.back().reference_component = reference_component;
}

inline void add_cross_component_unique_issue(Issues& issues, std::vector<ObjectReference> objects) {
    if (objects.empty()) {
        return;
    }

    std::ranges::sort(objects, [](ObjectReference const& first, ObjectReference const& second) {
        return std::tie(first.component, first.id, first.row) < std::tie(second.component, second.id, second.row);
    });
    std::vector<FieldReference> fields;
    for (auto const& object : objects) {
        FieldReference const field{.component = object.component, .field = "id"};
        if (fields.empty() || fields.back() != field) {
            fields.push_back(field);
        }
    }

    Issue issue{.rule = Rule::cross_component_not_unique,
                .component = fields.front().component,
                .field = "id",
                .ids = {},
                .fields = std::move(fields),
                .objects = std::move(objects),
                .reference_component = {}};
    issues.push_back(std::move(issue));
}

} // namespace power_grid_model::validation
