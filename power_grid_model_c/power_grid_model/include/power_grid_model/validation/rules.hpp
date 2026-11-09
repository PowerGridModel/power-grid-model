// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>
#include <vector>

namespace power_grid_model::validation {

enum class RuleCode : std::uint8_t { missing_value, infinity, not_greater_than_zero, not_boolean };

} // namespace power_grid_model::validation

namespace power_grid_model::validation::rules {

template <class RuleType, class ValueType>
concept rule_c = requires(RuleType const& rule, ValueType const& value) {
    { RuleType::code } -> std::convertible_to<RuleCode>;
    { rule(value) } -> std::convertible_to<bool>;
};

namespace detail {

template <class View>
concept indexed_sized_view = requires(View const& view, std::size_t idx) {
    { view.size() } -> std::convertible_to<std::size_t>;
    view[idx];
};

template <indexed_sized_view AttributeSpan, indexed_sized_view IdSpan, class RuleType>
    requires rule_c<RuleType, std::remove_cvref_t<decltype(std::declval<AttributeSpan const&>()[std::size_t{}])>>
std::vector<ID> find_ids(AttributeSpan const& attribute_span, IdSpan const& ids, RuleType const& rule) {
    assert(attribute_span.size() == ids.size()); // NOSONAR(S6994) // false positive: input may not have constexpr size

    std::vector<ID> result;
    for (std::size_t idx{}; idx != attribute_span.size(); ++idx) {
        if (std::invoke(rule, attribute_span[idx])) {
            result.push_back(ids[idx]);
        }
    }
    return result;
}

} // namespace detail

struct Required {
    static constexpr RuleCode code{RuleCode::missing_value};

    template <class Value> bool operator()(Value const& value) const { return is_nan(value); }
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

template <class RuleType, detail::indexed_sized_view AttributeSpan, detail::indexed_sized_view IdSpan>
    requires rule_c<RuleType, std::remove_cvref_t<decltype(std::declval<AttributeSpan const&>()[std::size_t{}])>>
void apply(RuleType const& rule, AttributeSpan const& attribute_span, IdSpan const& ids, std::string_view component,
           std::string_view field, Issues& issues) {
    add_issue(issues, RuleType::code, component, field, detail::find_ids(attribute_span, ids, rule));
}

struct NotBothZero {
    template <detail::indexed_sized_view FirstAttributeSpan, detail::indexed_sized_view SecondAttributeSpan,
              detail::indexed_sized_view IdSpan>
    void operator()(FirstAttributeSpan const& /*unused*/, SecondAttributeSpan const& /*unused*/,
                    IdSpan const& /*unused*/, std::string_view /*unused*/, std::string_view /*unused*/,
                    std::string_view /*unused*/, Issues& /*unused*/) const {
        throw NotImplementedError{};
    }
};

} // namespace power_grid_model::validation::rules
