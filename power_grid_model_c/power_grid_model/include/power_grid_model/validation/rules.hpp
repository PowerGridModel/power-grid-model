// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "power_grid_model/common/common.hpp"

#include <power_grid_model/common/exception.hpp>

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string_view>
#include <type_traits>
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

struct NotBothZero {
    template <detail::indexed_sized_view FirstValues, detail::indexed_sized_view SecondValues,
              detail::indexed_sized_view Ids>
    void operator()(FirstValues const& /*unused*/, SecondValues const& /*unused*/, Ids const& /*unused*/,
                    std::string_view /*unused*/, std::string_view /*unused*/, std::string_view /*unused*/,
                    Issues& /*unused*/) const {
        throw NotImplementedError{};
    }
};

} // namespace power_grid_model::validation::rules
