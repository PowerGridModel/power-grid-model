#pragma once

#include "issues.hpp"

#include <power_grid_model/common/common.hpp>

#include <cmath>
#include <concepts>
#include <functional>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace power_grid_model_c::validation::rules {
namespace detail {

template <class Value> bool is_missing(Value const& value) {
    using ValueType = std::remove_cvref_t<Value>;
    if constexpr (std::floating_point<ValueType>) {
        return std::isnan(value);
    } else if constexpr (std::same_as<ValueType, power_grid_model::ID>) {
        return value == power_grid_model::na_IntID;
    } else if constexpr (std::same_as<ValueType, power_grid_model::IntS>) {
        return value == power_grid_model::na_IntS;
    } else {
        return false;
    }
}

template <class Value> struct ValueHash {
    std::size_t operator()(Value const& value) const noexcept {
        if constexpr (std::floating_point<Value>) {
            if (std::isnan(value)) {
                return 0x9e3779b9;
            }
        }
        return std::hash<Value>{}(value);
    }
};

template <class Value> struct ValueEqual {
    bool operator()(Value const& first, Value const& second) const noexcept {
        if constexpr (std::floating_point<Value>) {
            if (std::isnan(first) || std::isnan(second)) {
                return std::isnan(first) && std::isnan(second);
            }
        }
        return first == second;
    }
};

// TODO (nitbharambe) think of optional ids and columnar data later
template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor, class Predicate>
std::vector<power_grid_model::ID> find_ids(Range const& rows, FieldAccessor const& field_accessor,
                                           IdAccessor const& id_accessor, Predicate const& predicate) {
    std::vector<power_grid_model::ID> ids;
    for (auto const& value : rows) {
        Record const record = static_cast<Record>(value);
        if (std::invoke(predicate, std::invoke(field_accessor, record))) {
            ids.push_back(std::invoke(id_accessor, record));
        }
    }
    return ids;
}

} // namespace detail

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void required(Range const& rows, std::string_view component, std::string_view field,
              FieldAccessor const& field_accessor, IdAccessor const& id_accessor, Issues& issues) {
    auto const is_missing = [](auto const& value) { return detail::is_missing(value); };
    add_issue(issues, Rule::missing_value, component, field,
              detail::find_ids<Record>(rows, field_accessor, id_accessor, is_missing));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void finite(Range const& rows, std::string_view component, std::string_view field, FieldAccessor const& field_accessor,
            IdAccessor const& id_accessor, Issues& issues) {
    using Value = std::remove_cvref_t<std::invoke_result_t<FieldAccessor, Record const&>>;
    static_assert(std::floating_point<Value>);
    auto const is_infinite = [](Value value) { return std::isinf(value); };
    add_issue(issues, Rule::infinity, component, field,
              detail::find_ids<Record>(rows, field_accessor, id_accessor, is_infinite));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void unique(Range const& rows, std::string_view component, std::string_view field, FieldAccessor const& field_accessor,
            IdAccessor const& id_accessor, Issues& issues) {
    using Value = std::remove_cvref_t<std::invoke_result_t<FieldAccessor, Record const&>>;
    std::unordered_map<Value, std::size_t, detail::ValueHash<Value>, detail::ValueEqual<Value>> counts;
    for (auto const& value : rows) {
        Record const record = static_cast<Record>(value);
        ++counts[std::invoke(field_accessor, record)];
    }

    auto const is_duplicate = [&counts](Value const& value) { return counts.at(value) > 1; };
    add_issue(issues, Rule::not_unique, component, field,
              detail::find_ids<Record>(rows, field_accessor, id_accessor, is_duplicate));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void greater_than_zero(Range const& rows, std::string_view component, std::string_view field,
                       FieldAccessor const& field_accessor, IdAccessor const& id_accessor, Issues& issues) {
    auto const is_not_positive = [](auto value) { return value <= 0; };
    add_issue(issues, Rule::not_greater_than_zero, component, field,
              detail::find_ids<Record>(rows, field_accessor, id_accessor, is_not_positive));
}

} // namespace power_grid_model_c::validation::rules
