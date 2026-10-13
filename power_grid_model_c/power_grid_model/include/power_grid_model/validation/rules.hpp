#pragma once

#include "issues.hpp"

#include <power_grid_model/common/exception.hpp>

#include <ranges>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace power_grid_model::validation::rules {

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void required(Range const&, std::string_view, std::string_view, FieldAccessor const&, IdAccessor const&, Issues&) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void finite(Range const&, std::string_view, std::string_view, FieldAccessor const&, IdAccessor const&, Issues&) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void unique(Range const&, std::string_view, std::string_view, FieldAccessor const&, IdAccessor const&, Issues&) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void greater_than_zero(Range const&, std::string_view, std::string_view, FieldAccessor const&, IdAccessor const&,
                       Issues&) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void boolean(Range const&, std::string_view, std::string_view, FieldAccessor const&, IdAccessor const&, Issues&) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FirstAccessor, class SecondAccessor, class IdAccessor>
void not_both_zero(Range const&, std::string_view, std::string_view, std::string_view, FirstAccessor const&,
                   SecondAccessor const&, IdAccessor const&, Issues&) {
    throw NotImplementedError{};
}

template <class Record, std::ranges::input_range Range, class FieldAccessor, class IdAccessor>
void valid_id_reference(Range const&, std::string_view, std::string_view, std::string_view, FieldAccessor const&,
                        IdAccessor const&, std::unordered_set<ID> const&, Issues&) {
    throw NotImplementedError{};
}

inline void cross_unique(std::unordered_map<std::string_view, std::vector<ID>> const&, Issues&) {
    throw NotImplementedError{};
}

} // namespace power_grid_model::validation::rules