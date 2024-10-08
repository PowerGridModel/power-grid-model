// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_cpp.hpp"

#include <power_grid_model_c/dataset_definitions.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <limits>
#include <ranges>
#include <string>

namespace power_grid_model_cpp {
namespace {
constexpr double nan = std::numeric_limits<double>::quiet_NaN();
constexpr int8_t na_IntS = std::numeric_limits<int8_t>::min();
constexpr ID na_IntID = std::numeric_limits<ID>::min();

template <std::same_as<double> T> constexpr T nan_value() { return nan; }
template <std::same_as<std::array<double, 3>> T> constexpr T nan_value() { return {nan, nan, nan}; }
template <std::same_as<ID> T> constexpr T nan_value() { return na_IntID; }
template <std::same_as<int8_t> T> constexpr T nan_value() { return na_IntS; }

template <typename T, std::convertible_to<T> U> constexpr T as_type(U const& value) { return static_cast<T>(value); }
template <typename T> constexpr T as_type(T const& value) { return value; }
template <std::same_as<std::array<double, 3>> T> constexpr T as_type(double const& value) {
    return {value, value, value};
}

inline bool is_nan(double value) { return std::isnan(value); }
inline bool is_nan(std::array<double, 3> value) { return is_nan(value[0]) && is_nan(value[1]) && is_nan(value[2]); }
inline bool is_nan(ID x) { return x == std::numeric_limits<ID>::min(); }
inline bool is_nan(int8_t x) { return x == std::numeric_limits<int8_t>::min(); }

void check_buffer(MetaComponent const* component, MetaAttribute const* attribute) {
    auto check_attribute = [component, attribute]<typename T>() {
        for (Idx size = 0; size < 4; ++size) {
            T ref_value{};
            std::vector<T> ref_buffer(size);
            std::vector<T> source_buffer(size);

            Buffer buffer{component, size};
            buffer.set_nan();

            buffer.get_value(attribute, &ref_value, 0);
            if (size > 0) {
                CHECK(is_nan(ref_value));
            } else {
                CHECK(ref_value == T{});
            }

            buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
            for (Idx idx = 0; idx < size; ++idx) {
                CHECK(is_nan(ref_buffer[idx]));
            }

            for (Idx idx = 0; idx < size; ++idx) {
                source_buffer[idx] = as_type<T>(idx);
            }
            buffer.set_value(attribute, source_buffer.data(), sizeof(T));
            buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
            for (Idx idx = 0; idx < size; ++idx) {
                CHECK(ref_buffer[idx] == as_type<T>(idx));
            }
        }
    };
    switch (MetaData::attribute_ctype(attribute)) {
    case PGM_int32:
        return check_attribute.template operator()<PGM_ID>();
    case PGM_int8:
        return check_attribute.template operator()<int8_t>();
    case PGM_double:
        return check_attribute.template operator()<double>();
    case PGM_double3:
        return check_attribute.template operator()<std::array<double, 3>>();
    default:
        FAIL("Invalid ctype");
    }
}
} // namespace

TEST_CASE("API Buffer") {
    for (Idx dataset_idx = 0; dataset_idx < MetaData::n_datasets(); ++dataset_idx) {
        CAPTURE(dataset_idx);
        MetaDataset const* dataset = MetaData::get_dataset_by_idx(dataset_idx);
        CAPTURE(MetaData::dataset_name(dataset));
        for (Idx component_idx = 0; component_idx < MetaData::n_components(dataset); ++component_idx) {
            CAPTURE(component_idx);
            MetaComponent const* component = MetaData::get_component_by_idx(dataset, component_idx);
            CAPTURE(MetaData::component_name(component));

            for (Idx attribute_idx = 0; attribute_idx < MetaData::n_attributes(component); ++attribute_idx) {
                CAPTURE(attribute_idx);
                MetaAttribute const* attribute = MetaData::get_attribute_by_idx(component, attribute_idx);
                CAPTURE(MetaData::attribute_name(attribute));

                check_buffer(component, attribute);
            }
        }
    }
}

} // namespace power_grid_model_cpp
