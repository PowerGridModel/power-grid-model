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
template <class Record, std::ranges::input_range Range, class IdAccessor, class Predicate>
std::vector<ObjectReference> find_objects(Range const& rows, std::string_view component,
                                          IdAccessor const& id_accessor, Predicate const& predicate) {
    std::vector<ObjectReference> objects;
    std::unordered_map<ID, std::size_t, ValueHash<ID>, ValueEqual<ID>> id_counts;
    std::size_t row{};
    for (auto const& value : rows) {
        visit_record<Record>(value, [&](Record const& record) {
            ID const id = std::invoke(id_accessor, record);
            ++id_counts[id];
            if (std::invoke(predicate, record)) {
                objects.push_back({.component = component, .id = id, .row = row});
            }
        });
        ++row;
    }
    for (auto& object : objects) {
        if (object.id != na_IntID && id_counts.at(object.id) == 1) {
            object.row.reset();
        }
    }
    return objects;
}

} // namespace detail

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void required(Range const& rows, std::string_view component, std::string_view field,
              FieldAccessor const& field_accessor, IdAccessor const& id_accessor, Issues& issues) {
    auto const is_missing = [&field_accessor](Record const& record) {
        return detail::is_missing(std::invoke(field_accessor, record));
    };
    add_issue_from_objects(issues, Rule::missing_value, component, field,
                           detail::find_objects<Record>(rows, component, id_accessor, is_missing));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void finite(Range const& rows, std::string_view component, std::string_view field, FieldAccessor const& field_accessor,
            IdAccessor const& id_accessor, Issues& issues) {
    using Value = std::remove_cvref_t<std::invoke_result_t<FieldAccessor, Record const&>>;
    static_assert(std::floating_point<Value>);
    auto const is_infinite = [&field_accessor](Record const& record) {
        return std::isinf(std::invoke(field_accessor, record));
    };
    add_issue_from_objects(issues, Rule::infinity, component, field,
                           detail::find_objects<Record>(rows, component, id_accessor, is_infinite));
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

    auto const is_duplicate = [&counts, &field_accessor](Record const& record) {
        return counts.at(std::invoke(field_accessor, record)) > 1;
    };
    add_issue_from_objects(issues, Rule::not_unique, component, field,
                           detail::find_objects<Record>(rows, component, id_accessor, is_duplicate));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void greater_than_zero(Range const& rows, std::string_view component, std::string_view field,
                       FieldAccessor const& field_accessor, IdAccessor const& id_accessor, Issues& issues) {
    auto const is_not_positive = [&field_accessor](Record const& record) {
        return std::invoke(field_accessor, record) <= 0;
    };
    add_issue_from_objects(issues, Rule::not_greater_than_zero, component, field,
                           detail::find_objects<Record>(rows, component, id_accessor, is_not_positive));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void boolean(Range const& rows, std::string_view component, std::string_view field, FieldAccessor const& field_accessor,
             IdAccessor const& id_accessor, Issues& issues) {
    auto const is_not_boolean = [&field_accessor](Record const& record) {
        auto const value = std::invoke(field_accessor, record);
        return value != 0 && value != 1;
    };
    add_issue_from_objects(issues, Rule::not_boolean, component, field,
                           detail::find_objects<Record>(rows, component, id_accessor, is_not_boolean));
}

template <class Record, std::ranges::input_range Range, class FirstAccessor, class SecondAccessor, class IdAccessor>
void not_both_zero(Range const& rows, std::string_view component, std::string_view first_field,
                   std::string_view second_field, FirstAccessor const& first_accessor,
                   SecondAccessor const& second_accessor, IdAccessor const& id_accessor, Issues& issues) {
    auto const both_zero = [&first_accessor, &second_accessor](Record const& record) {
        return std::invoke(first_accessor, record) == 0 && std::invoke(second_accessor, record) == 0;
    };
    add_multi_field_issue_from_objects(issues, Rule::two_values_zero, component, {first_field, second_field},
                                       detail::find_objects<Record>(rows, component, id_accessor, both_zero));
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void valid_id_reference(Range const& rows, std::string_view component, std::string_view field,
                        std::string_view reference_component, FieldAccessor const& field_accessor,
                        IdAccessor const& id_accessor, std::unordered_set<ID> const& valid_ids, Issues& issues) {
    auto const is_invalid_reference = [&valid_ids, &field_accessor](Record const& record) {
        return !valid_ids.contains(std::invoke(field_accessor, record));
    };
    add_reference_issue_from_objects(issues, component, field, reference_component,
                                     detail::find_objects<Record>(rows, component, id_accessor, is_invalid_reference));
}

template <class ComponentRecords> void cross_unique(ComponentRecords const& records_by_component, Issues& issues) {
    std::unordered_map<ID, std::vector<ObjectReference>> objects_by_id;
    for (auto const& [component, records] : records_by_component) {
        for (auto const& record : records) {
            ObjectReference object;
            if constexpr (std::same_as<std::remove_cvref_t<decltype(record)>, ObjectReference>) {
                object = record;
            } else {
                object = {.component = component, .id = record};
            }
            if (object.id != na_IntID) {
                objects_by_id[object.id].push_back(object);
            }
        }
    }

    std::vector<ObjectReference> duplicate_objects;
    for (auto const& [id, objects] : objects_by_id) {
        std::unordered_set<std::string_view> components;
        for (auto const& object : objects) {
            components.insert(object.component);
        }
        if (components.size() > 1) {
            duplicate_objects.insert(duplicate_objects.end(), objects.begin(), objects.end());
        }
    }
    add_cross_component_unique_issue(issues, std::move(duplicate_objects));
}

} // namespace power_grid_model::validation::rules
