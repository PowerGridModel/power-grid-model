// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>

#include <cassert>
#include <cmath>
#include <concepts>
#include <functional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace power_grid_model::validation::rules {
namespace detail {

template <class View>
concept indexed_sized_view = requires(View const& view, std::size_t idx) {
    { view.size() } -> std::convertible_to<std::size_t>;
    view[idx];
};

template <class Value>
    requires requires(Value const& value) {
        { is_nan(value) } -> std::convertible_to<bool>;
    }
bool is_missing(Value const& value) {
    return is_nan(value);
}

template <indexed_sized_view Values, indexed_sized_view Ids, class Predicate>
std::vector<ID> find_ids(Values const& values, Ids const& ids, Predicate const& predicate) {
    if (values.size() == 0) {
        return {}; // absent column, or no elements
    }
    if (ids.size() != values.size()) {
        assert(false && "id column absent or size mismatch");
        return {};
    }

    std::vector<ID> result;
    for (std::size_t idx{}; idx != values.size(); ++idx) {
        if (std::invoke(predicate, values[idx])) {
            result.push_back(ids[idx]);
        }
    }
    return result;
}

template <indexed_sized_view Ids> std::vector<ID> all_ids(Ids const& ids) {
    std::vector<ID> result;
    result.reserve(ids.size());
    for (std::size_t idx{}; idx != ids.size(); ++idx) {
        result.push_back(ids[idx]);
    }
    return result;
}

} // namespace detail

template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void required(Values const& values, Ids const& ids, std::string_view component, std::string_view field,
              Issues& issues) {
    if (values.size() == 0) {
        // absent column: every element is missing (no-op when there are no elements)
        add_issue(issues, "missing_value", component, field, detail::all_ids(ids));
        return;
    }
    add_issue(issues, "missing_value", component, field,
              detail::find_ids(values, ids, [](auto const& value) { return detail::is_missing(value); }));
}

template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void finite(Values const& values, Ids const& ids, std::string_view component, std::string_view field, Issues& issues) {
    using Value = std::remove_cvref_t<decltype(values[std::size_t{}])>;
    static_assert(std::floating_point<Value>);
    add_issue(issues, "infinity", component, field,
              detail::find_ids(values, ids, [](Value const& value) { return std::isinf(value); }));
}

template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void greater_than_zero(Values const& values, Ids const& ids, std::string_view component, std::string_view field,
                       Issues& issues) {
    add_issue(issues, "not_greater_than_zero", component, field,
              detail::find_ids(values, ids, [](auto const& value) { return value <= 0; }));
}

template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void boolean(Values const& values, Ids const& ids, std::string_view component, std::string_view field, Issues& issues) {
    add_issue(issues, "not_boolean", component, field,
              detail::find_ids(values, ids, [](auto const& value) { return value != 0 && value != 1; }));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void unique(Range const& /*rows*/, std::string_view /*component*/, std::string_view /*field*/,
            FieldAccessor const& /*field_accessor*/, IdAccessor const& /*id_accessor*/, Issues& /*issues*/) {
    throw NotImplementedError{};
}

// TODO(figueroa1395): Change below functions to columnar based implementation
template <class Record, std::ranges::input_range Range, class FirstAccessor, class SecondAccessor, class IdAccessor>
void not_both_zero(Range const& /*rows*/, std::string_view /*component*/, std::string_view /*first_field*/,
                   std::string_view /*second_field*/, FirstAccessor const& /*first_accessor*/,
                   SecondAccessor const& /*second_accessor*/, IdAccessor const& /*id_accessor*/, Issues& /*issues*/) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void valid_id_reference(Range const& /*rows*/, std::string_view /*component*/, std::string_view /*field*/,
                        std::string_view /*reference_component*/, FieldAccessor const& /*field_accessor*/,
                        IdAccessor const& /*id_accessor*/, std::unordered_set<ID> const& /*valid_ids*/,
                        Issues& /*issues*/) {
    throw NotImplementedError{};
}

inline void cross_unique(std::unordered_map<std::string_view, std::vector<ID>> const& /*ids_by_component*/,
                         Issues& /*issues*/) {
    throw NotImplementedError{};
}

} // namespace power_grid_model::validation::rules
