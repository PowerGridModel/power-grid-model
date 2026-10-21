// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"

#include <power_grid_model/common/exception.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::validation::rules {
namespace detail {

template <class View>
concept indexed_sized_view = requires(View const& view, std::size_t idx) {
    { view.size() } -> std::convertible_to<std::size_t>;
    view[idx];
};

template <class Value> bool is_missing(Value const& value) {
    using ValueType = std::remove_cvref_t<Value>;
    if constexpr (std::floating_point<ValueType>) {
        return std::isnan(value);
    } else if constexpr (std::same_as<ValueType, ID>) {
        return value == na_IntID;
    } else if constexpr (std::same_as<ValueType, IntS>) {
        return value == na_IntS;
    } else {
        return false;
    }
}

template <class Value> struct value_hash {
    std::size_t operator()(Value const& value) const {
        if constexpr (std::floating_point<Value>) {
            if (std::isnan(value)) {
                return 0;
            }
        }
        return std::hash<Value>{}(value);
    }
};

template <class Value> struct value_equal {
    bool operator()(Value const& left, Value const& right) const {
        if constexpr (std::floating_point<Value>) {
            if (std::isnan(left) || std::isnan(right)) {
                return std::isnan(left) && std::isnan(right);
            }
        }
        return left == right;
    }
};

template <indexed_sized_view Values, indexed_sized_view Ids, class Rule>
std::vector<ID> find_ids(Values const& values, Ids const& ids, Rule const& rule) {
    assert(values.size() == ids.size());

    std::vector<ID> result;
    for (std::size_t idx{}; idx != values.size(); ++idx) {
        if (std::invoke(rule, values[idx])) {
            result.push_back(ids[idx]);
        }
    }
    return result;
}

} // namespace detail

struct Required {
    static constexpr RuleCode code{RuleCode::missing_value};

    template <class Value> bool operator()(Value const& value) const { return detail::is_missing(value); }
};

struct Finite {
    static constexpr RuleCode code{RuleCode::infinity};

    template <class Value> bool operator()(Value const& value) const {
        static_assert(std::floating_point<std::remove_cvref_t<Value>>);
        return std::isinf(value);
    }
};

struct GreaterThanZero {
    static constexpr RuleCode code{RuleCode::not_greater_than_zero};

    template <class Value> bool operator()(Value const& value) const { return value <= 0; }
};

struct Boolean {
    static constexpr RuleCode code{RuleCode::not_boolean};

    template <class Value> bool operator()(Value const& value) const { return value != 0 && value != 1; }
};

template <class Rule, detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void apply(Rule const& rule, Values const& values, Ids const& ids, std::string_view component, std::string_view field,
           Issues& issues) {
    add_issue(issues, Rule::code, component, field, detail::find_ids(values, ids, rule));
}

struct Unique {
    static constexpr RuleCode code{RuleCode::not_unique};

    template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
    std::vector<ID> operator()(Values const& values, Ids const& ids) const {
        assert(values.size() == ids.size());

        using Value = std::remove_cvref_t<decltype(values[std::size_t{}])>;
        std::unordered_map<Value, std::size_t, detail::value_hash<Value>, detail::value_equal<Value>> counts;
        counts.reserve(values.size());
        for (std::size_t idx{}; idx != values.size(); ++idx) {
            Value const value = values[idx];
            ++counts[value];
        }

        std::vector<ID> duplicate_ids;
        for (std::size_t idx{}; idx != values.size(); ++idx) {
            Value const value = values[idx];
            if (counts.at(value) > 1) {
                duplicate_ids.push_back(ids[idx]);
            }
        }
        return duplicate_ids;
    }
};

struct NotBothZero {
    template <detail::indexed_sized_view FirstValues, detail::indexed_sized_view SecondValues,
              detail::indexed_sized_view Ids>
    void operator()(FirstValues const&, SecondValues const&, Ids const&, std::string_view, std::string_view,
                    std::string_view, Issues&) const {
        throw NotImplementedError{};
    }
};

struct ValidIdReference {
    static constexpr RuleCode code{RuleCode::invalid_id};

    template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
    std::vector<ID> operator()(Values const& values, Ids const& ids, std::unordered_set<ID> const& valid_ids) const {
        using Value = std::remove_cvref_t<decltype(values[std::size_t{}])>;
        static_assert(std::same_as<Value, ID>);
        return detail::find_ids(values, ids, [&valid_ids](ID id) { return !valid_ids.contains(id); });
    }
};

struct ComponentDuplicateIds {
    std::string_view component;
    std::vector<ID> ids;
};

struct CrossUnique {
    static constexpr RuleCode code{RuleCode::not_unique_across_components};

    template <class IdsByComponent>
    std::vector<ComponentDuplicateIds> operator()(IdsByComponent const& ids_by_component) const {
        std::unordered_map<ID, std::unordered_set<std::string_view>> components_by_id;
        for (auto const& [component, ids] : ids_by_component) {
            for (ID id : ids) {
                if (id != na_IntID) {
                    components_by_id[id].insert(component);
                }
            }
        }

        std::unordered_map<std::string_view, std::vector<ID>> duplicate_ids_by_component;
        for (auto const& [id, components] : components_by_id) {
            if (components.size() > 1) {
                for (std::string_view component : components) {
                    duplicate_ids_by_component[component].push_back(id);
                }
            }
        }

        std::vector<std::string_view> components;
        components.reserve(duplicate_ids_by_component.size());
        for (auto const& [component, ids] : duplicate_ids_by_component) {
            components.push_back(component);
        }
        std::ranges::sort(components);
        std::vector<ComponentDuplicateIds> result;
        for (std::string_view component : components) {
            auto& duplicate_ids = duplicate_ids_by_component.at(component);
            std::ranges::sort(duplicate_ids);
            result.push_back({.component = component, .ids = std::move(duplicate_ids)});
        }
        return result;
    }
};

} // namespace power_grid_model::validation::rules
