// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"

#include <power_grid_model/common/exception.hpp>

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

template <class Record, class Element, class Function> void visit_record(Element const& element, Function&& function) {
    if constexpr (std::same_as<std::remove_cvref_t<Element>, Record>) {
        std::invoke(std::forward<Function>(function), element);
    } else {
        auto const record = static_cast<Record>(element);
        std::invoke(std::forward<Function>(function), record);
    }
}

template <class Record, std::ranges::input_range Range, class IdAccessor, class Predicate>
std::vector<ID> find_ids(Range const& rows, IdAccessor const& id_accessor, Predicate const& predicate) {
    std::vector<ID> ids;
    for (auto const& value : rows) {
        visit_record<Record>(value, [&predicate, &id_accessor, &ids](Record const& record) {
            if (std::invoke(predicate, record)) {
                ids.push_back(std::invoke(id_accessor, record));
            }
        });
    }
    return ids;
}

template <indexed_sized_view Values, indexed_sized_view Ids, class Predicate>
std::vector<ID> find_ids(Values const& values, Ids const& ids, Predicate const& predicate) {
    assert(values.size() == ids.size());

    std::vector<ID> result;
    for (std::size_t idx{}; idx != values.size(); ++idx) {
        if (std::invoke(predicate, values[idx])) {
            result.push_back(ids[idx]);
        }
    }
    return result;
}

} // namespace detail

template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void required(Values const& values, Ids const& ids, std::string_view component, std::string_view field,
              Issues& issues) {
    add_issue(issues, "missing_value", component, field,
              detail::find_ids(values, ids, [](auto const& value) { return detail::is_missing(value); }));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void required(Range const& rows, std::string_view component, std::string_view field,
              FieldAccessor const& field_accessor, IdAccessor const& id_accessor, Issues& issues) {
    auto const missing = [&field_accessor](Record const& record) {
        return detail::is_missing(std::invoke(field_accessor, record));
    };
    add_issue(issues, "missing_value", component, field, detail::find_ids<Record>(rows, id_accessor, missing));
}

template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void finite(Values const& values, Ids const& ids, std::string_view component, std::string_view field, Issues& issues) {
    using Value = std::remove_cvref_t<decltype(values[std::size_t{}])>;
    static_assert(std::floating_point<Value>);
    add_issue(issues, "infinity", component, field,
              detail::find_ids(values, ids, [](Value const& value) { return std::isinf(value); }));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void finite(Range const& rows, std::string_view component, std::string_view field, FieldAccessor const& field_accessor,
            IdAccessor const& id_accessor, Issues& issues) {
    using Value = std::remove_cvref_t<std::invoke_result_t<FieldAccessor, Record const&>>;
    static_assert(std::floating_point<Value>);
    auto const infinite = [&field_accessor](Record const& record) {
        return std::isinf(std::invoke(field_accessor, record));
    };
    add_issue(issues, "infinity", component, field, detail::find_ids<Record>(rows, id_accessor, infinite));
}

template <detail::indexed_sized_view Values, detail::indexed_sized_view Ids>
void greater_than_zero(Values const& values, Ids const& ids, std::string_view component, std::string_view field,
                       Issues& issues) {
    add_issue(issues, "not_greater_than_zero", component, field,
              detail::find_ids(values, ids, [](auto const& value) { return value <= 0; }));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void unique(Range const& /*rows*/, std::string_view /*component*/, std::string_view /*field*/,
            FieldAccessor const& /*field_accessor*/, IdAccessor const& /*id_accessor*/, Issues& /*issues*/) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void greater_than_zero(Range const& rows, std::string_view component, std::string_view field,
                       FieldAccessor const& field_accessor, IdAccessor const& id_accessor, Issues& issues) {
    auto const not_positive = [&field_accessor](Record const& record) {
        return std::invoke(field_accessor, record) <= 0;
    };
    add_issue(issues, "not_greater_than_zero", component, field,
              detail::find_ids<Record>(rows, id_accessor, not_positive));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void boolean(Range const& /*rows*/, std::string_view /*component*/, std::string_view /*field*/,
             FieldAccessor const& /*field_accessor*/, IdAccessor const& /*id_accessor*/, Issues& /*issues*/) {
    throw NotImplementedError{};
}

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
