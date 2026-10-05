#pragma once

#include "issues.hpp"

#include <power_grid_model/common/common.hpp>

#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::validation::rules {
namespace detail {

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

template <class Record, class Element, class Func> void visit_record(Element const& element, Func&& func) {
    if constexpr (std::same_as<std::remove_cvref_t<Element>, Record>) {
        std::invoke(func, element);
    } else {
        Record const record = static_cast<Record>(element);
        std::invoke(func, record);
    }
}

// TODO (nitbharambe) think of optional ids and columnar data later
template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor, class Predicate>
std::vector<ID> find_ids(Range const& rows, FieldAccessor const& field_accessor, IdAccessor const& id_accessor,
                         Predicate const& predicate) {
    std::vector<ID> ids;
    for (auto const& value : rows) {
        visit_record<Record>(value, [&](Record const& record) {
            if (std::invoke(predicate, std::invoke(field_accessor, record))) {
                ids.push_back(std::invoke(id_accessor, record));
            }
        });
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
        detail::visit_record<Record>(value,
                                     [&](Record const& record) { ++counts[std::invoke(field_accessor, record)]; });
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

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void boolean(Range const& rows, std::string_view component, std::string_view field, FieldAccessor const& field_accessor,
             IdAccessor const& id_accessor, Issues& issues) {
    auto const is_not_boolean = [](auto value) { return value != 0 && value != 1; };
    add_issue(issues, Rule::not_boolean, component, field,
              detail::find_ids<Record>(rows, field_accessor, id_accessor, is_not_boolean));
}

template <class Record, std::ranges::input_range Range, class FirstAccessor, class SecondAccessor, class IdAccessor>
void not_both_zero(Range const& rows, std::string_view component, std::string_view first_field,
                   std::string_view second_field, FirstAccessor const& first_accessor,
                   SecondAccessor const& second_accessor, IdAccessor const& id_accessor, Issues& issues) {
    std::vector<ID> ids;
    for (auto const& value : rows) {
        detail::visit_record<Record>(value, [&](Record const& record) {
            if (std::invoke(first_accessor, record) == 0 && std::invoke(second_accessor, record) == 0) {
                ids.push_back(std::invoke(id_accessor, record));
            }
        });
    }
    add_multi_field_issue(issues, Rule::two_values_zero, component, {first_field, second_field}, std::move(ids));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void valid_id_reference(Range const& rows, std::string_view component, std::string_view field,
                        std::string_view reference_component, FieldAccessor const& field_accessor,
                        IdAccessor const& id_accessor, std::unordered_set<ID> const& valid_ids, Issues& issues) {
    auto const is_invalid_reference = [&valid_ids](ID id) { return !valid_ids.contains(id); };
    add_reference_issue(issues, component, field, reference_component,
                        detail::find_ids<Record>(rows, field_accessor, id_accessor, is_invalid_reference));
}

template <std::ranges::input_range NodeIds, std::ranges::input_range LineIds>
void cross_unique(NodeIds const& node_ids, LineIds const& line_ids, Issues& issues) {
    std::unordered_set<ID> nodes;
    std::unordered_set<ID> lines;
    for (ID id : node_ids) {
        nodes.insert(id);
    }
    for (ID id : line_ids) {
        if (nodes.contains(id)) {
            lines.insert(id);
        }
    }
    add_cross_component_unique_issue(issues, {lines.begin(), lines.end()}, "node", "line");
}

} // namespace power_grid_model::validation::rules
