// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"

#include "../common/common.hpp"
#include "../common/enum_utils.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class RuleCode : std::uint8_t {
    missing_value,
    infinity,
    not_greater_than_zero,
    not_less_than_one,
    not_greater_than_or_equal_to_zero,
    not_in_inclusive_range,
    not_boolean,
    not_valid_enum,
    not_unique
};

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
inline std::vector<ID> find_ids(AttributeSpan const& attribute_span, IdSpan const& ids, RuleType const& rule) {
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

    template <class Value> constexpr bool operator()(Value const& value) const { return is_nan(value); }
};
inline constexpr Required is_required;

struct Finite {
    static constexpr RuleCode code{RuleCode::infinity};

    template <class Value> constexpr bool operator()(Value const& value) const {
        static_assert(std::floating_point<std::remove_cvref_t<Value>>);
        return std::isinf(value);
    }
};
inline constexpr Finite is_finite;

struct GreaterThanZero {
    static constexpr RuleCode code{RuleCode::not_greater_than_zero};

    template <class Value> constexpr bool operator()(Value const& value) const { return value <= Value{0}; }
};
inline constexpr GreaterThanZero is_greater_than_zero;

struct LessThanOne {
    static constexpr RuleCode code{RuleCode::not_less_than_one};

    template <class Value> constexpr bool operator()(Value const& value) const { return value >= Value{1}; }
};
inline constexpr LessThanOne is_less_than_one;

struct GreaterThanOrEqualToZero {
    static constexpr RuleCode code{RuleCode::not_greater_than_or_equal_to_zero};

    template <class Value> constexpr bool operator()(Value const& value) const { return value < Value{0}; }
};
inline constexpr GreaterThanOrEqualToZero is_greater_than_or_equal_to_zero;

template <std::integral auto lower_bound, std::integral auto upper_bound>
    requires(std::same_as<decltype(lower_bound), decltype(upper_bound)> && (lower_bound < upper_bound))
struct InInclusiveRange {
    static constexpr RuleCode code{RuleCode::not_in_inclusive_range};

    template <class Value> constexpr bool operator()(Value const& value) const {
        using LowerBoundType = decltype(lower_bound);
        if constexpr ((std::integral<Value> && std::integral<LowerBoundType>) ||
                      (std::floating_point<Value> && std::floating_point<LowerBoundType>)) {
            return std::cmp_less(value, lower_bound) || std::cmp_greater(value, upper_bound);
        } else {
            static_assert(false, "Value and bounds type conversion is not safe");
        }
    }
};
template <std::integral auto lower_bound, std::integral auto upper_bound>
    requires(std::same_as<decltype(lower_bound), decltype(upper_bound)> && (lower_bound < upper_bound))
inline constexpr InInclusiveRange<lower_bound, upper_bound> is_in_inclusive_range;

struct Boolean {
    static constexpr RuleCode code{RuleCode::not_boolean};

    template <class Value> constexpr bool operator()(Value const& value) const {
        return value != Value{0} && value != Value{1};
    }
};
inline constexpr Boolean is_boolean;

template <typename EnumType>
    requires std::is_enum_v<EnumType>
struct Enum {
    static constexpr RuleCode code{RuleCode::not_valid_enum};

    template <class Value> constexpr bool operator()(Value const& value) const {
        static_assert(std::is_same_v<Value, EnumType> || std::is_same_v<Value, std::underlying_type_t<EnumType>>,
                      "We are providing the wrong data type. This is a bug in PGM.");
        using enum_utils::is_valid;
        return !is_valid(static_cast<EnumType>(value));
    }
};
template <typename EnumType>
    requires(std::is_enum_v<EnumType>)
inline constexpr Enum<EnumType> is_enum;

struct Unique {
    static constexpr RuleCode code{RuleCode::not_unique};

    constexpr bool operator()(bool is_unique) const { return !is_unique; }
};

template <class RuleType, detail::indexed_sized_view AttributeSpan, detail::indexed_sized_view IdSpan>
    requires rule_c<RuleType, std::remove_cvref_t<decltype(std::declval<AttributeSpan const&>()[std::size_t{}])>>
inline void apply(RuleType const& rule, AttributeSpan const& attribute_span, IdSpan const& ids,
                  std::string_view component, std::string_view field, Issues& issues) {
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
