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
            std::vector<T> source_buffer(size);
            std::vector<T> ref_buffer(size);

            Buffer buffer{component, size};

            // array get value
            buffer.set_nan();
            buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
            for (Idx idx = 0; idx < size; ++idx) {
                REQUIRE(is_nan(ref_buffer[idx]));
            }

            // array set value
            buffer.set_nan();
            for (Idx idx = 0; idx < size; ++idx) {
                source_buffer[idx] = as_type<T>(idx);
            }
            buffer.set_value(attribute, source_buffer.data(), sizeof(T));
            buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
            for (Idx idx = 0; idx < size; ++idx) {
                REQUIRE(ref_buffer[idx] == as_type<T>(idx));
            }
            // array set value with fixed size
            for (Idx sub_size = 0; sub_size < size; ++sub_size) {
                for (Idx offset = 0; offset < size - sub_size; ++offset) {
                    std::ranges::fill(ref_buffer, T{});
                    // get value
                    buffer.set_nan();
                    buffer.get_value(attribute, ref_buffer.data(), offset, sub_size, sizeof(T));
                    for (Idx idx = 0; idx < size; ++idx) {
                        if (idx >= offset && idx < offset + sub_size) {
                            REQUIRE(is_nan(ref_buffer[idx]));
                        } else {
                            REQUIRE(ref_buffer[idx] == T{});
                        }
                    }

                    // set value
                    buffer.set_nan();
                    for (Idx idx = 0; idx < size; ++idx) {
                        source_buffer[idx] = as_type<T>(idx);
                    }
                    buffer.set_value(attribute, source_buffer.data(), offset, sub_size, sizeof(T));
                    buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
                    for (Idx idx = 0; idx < size; ++idx) {
                        if (idx >= offset && idx < offset + sub_size) {
                            REQUIRE(ref_buffer[idx] == as_type<T>(idx));
                        } else {
                            REQUIRE(is_nan(ref_buffer[idx]));
                        }
                    }
                }
            }

            // single access
            T source_value{};
            T ref_value{};
            buffer.set_nan();
            for (Idx idx = 0; idx < size; ++idx) {
                // single get value
                buffer.get_value(attribute, &ref_value, idx, 0);
                if (size > 0) {
                    REQUIRE(is_nan(ref_value));
                } else {
                    REQUIRE(ref_value == T{});
                }
                // single set value
                buffer.set_value(attribute, &source_value, idx, 0);
                buffer.get_value(attribute, &ref_value, idx, 0);
                if (size > 0) {
                    REQUIRE(ref_value == source_value);
                } else {
                    REQUIRE(ref_value == T{});
                }
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
