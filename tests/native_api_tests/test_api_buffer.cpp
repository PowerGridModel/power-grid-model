// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_cpp/buffer.hpp>
#include <power_grid_model_cpp/meta_data.hpp>
#include <power_grid_model_cpp/utils.hpp>

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
template <typename T, std::convertible_to<T> U> constexpr T as_type(U const& value) {
    if constexpr (std::same_as<T, U>) {
        return value;
    } else {
        return static_cast<T>(value);
    }
}
template <std::same_as<std::array<double, 3>> T, std::convertible_to<double> U> constexpr T as_type(U const& value) {
    auto const d_value = as_type<double>(value);
    return {d_value, d_value, d_value};
}

void check_array_get_value(MetaComponent const* component, MetaAttribute const* attribute, Idx size) {
    pgm_type_func_selector(attribute, [component, attribute, size]<typename T>() {
        std::vector<T> ref_buffer(size);

        Buffer buffer{component, size};
        buffer.set_nan();

        buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
        for (Idx idx = 0; idx < size; ++idx) {
            REQUIRE(is_nan(ref_buffer[idx]));
        }
    });
}

void check_array_set_value(MetaComponent const* component, MetaAttribute const* attribute, Idx size) {
    pgm_type_func_selector(attribute, [component, attribute, size]<typename T>() {
        std::vector<T> source_buffer(size);
        std::vector<T> ref_buffer(size);
        for (Idx idx = 0; idx < size; ++idx) {
            source_buffer[idx] = as_type<T>(idx);
        }

        Buffer buffer{component, size};
        buffer.set_nan();

        buffer.set_value(attribute, source_buffer.data(), sizeof(T));
        buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
        for (Idx idx = 0; idx < size; ++idx) {
            REQUIRE(ref_buffer[idx] == as_type<T>(idx));
        }
    });
}

void check_sub_array_get_value(MetaComponent const* component, MetaAttribute const* attribute, Idx size) {
    auto const check_sub_offset_size = [component, attribute, size]<typename T>(Idx offset, Idx sub_size) {
        std::vector<T> ref_buffer(size);

        Buffer buffer{component, size};
        buffer.set_nan();

        buffer.get_value(attribute, ref_buffer.data(), offset, sub_size, sizeof(T));
        for (Idx idx = 0; idx < size; ++idx) {
            if (idx >= offset && idx < offset + sub_size) {
                REQUIRE(is_nan(ref_buffer[idx]));
            } else {
                REQUIRE(ref_buffer[idx] == T{});
            }
        }
    };
    for (Idx sub_size = 0; sub_size < size; ++sub_size) {
        CAPTURE(sub_size);
        for (Idx offset = 0; offset < size - sub_size; ++offset) {
            CAPTURE(offset);
            pgm_type_func_selector(attribute, check_sub_offset_size, offset, sub_size);
        }
    }
}

void check_sub_array_set_value(MetaComponent const* component, MetaAttribute const* attribute, Idx size) {
    auto const check_sub_offset_size = [component, attribute, size]<typename T>(Idx offset, Idx sub_size) {
        std::vector<T> ref_buffer(size);
        std::vector<T> source_buffer(size);
        for (Idx idx = 0; idx < size; ++idx) {
            source_buffer[idx] = as_type<T>(idx);
        }

        Buffer buffer{component, size};
        buffer.set_nan();
        buffer.set_value(attribute, source_buffer.data(), offset, sub_size, sizeof(T));
        buffer.get_value(attribute, ref_buffer.data(), sizeof(T));
        for (Idx idx = 0; idx < size; ++idx) {
            if (idx >= offset && idx < offset + sub_size) {
                REQUIRE(ref_buffer[idx] == as_type<T>(idx));
            } else {
                REQUIRE(is_nan(ref_buffer[idx]));
            }
        }
    };
    for (Idx sub_size = 0; sub_size < size; ++sub_size) {
        CAPTURE(sub_size);
        for (Idx offset = 0; offset < size - sub_size; ++offset) {
            CAPTURE(offset);
            pgm_type_func_selector(attribute, check_sub_offset_size, offset, sub_size);
        }
    }
}

void check_single_get_value(MetaComponent const* component, MetaAttribute const* attribute, Idx size) {
    pgm_type_func_selector(attribute, [component, attribute, size]<typename T>() {
        T ref_value{};

        Buffer buffer{component, size};
        buffer.set_nan();

        for (Idx idx = 0; idx < size; ++idx) {
            buffer.get_value(attribute, &ref_value, idx, 0);
            if (size > 0) {
                REQUIRE(is_nan(ref_value));
            } else {
                REQUIRE(ref_value == T{});
            }
        }
    });
}

void check_single_set_value(MetaComponent const* component, MetaAttribute const* attribute, Idx size) {
    pgm_type_func_selector(attribute, [component, attribute, size]<typename T>() {
        T const source_value{1};
        T ref_value{};

        Buffer buffer{component, size};
        buffer.set_nan();

        for (Idx idx = 0; idx < size; ++idx) {
            buffer.set_value(attribute, &source_value, idx, 0);
            buffer.get_value(attribute, &ref_value, idx, 0);
            if (size > 0) {
                REQUIRE(ref_value == source_value);
            } else {
                REQUIRE(ref_value == T{});
            }
        }
    });
}
} // namespace

TEST_CASE("API Buffer") {
    auto const loop_datasets_components_attributes = []<typename Func>(Func func) {
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

                    func(component, attribute);
                }
            }
        }
    };

    SUBCASE("Array buffer access") {
        SUBCASE("get value") {
            loop_datasets_components_attributes([](MetaComponent const* component, MetaAttribute const* attribute) {
                for (Idx size = 0; size < 4; ++size) {
                    check_array_get_value(component, attribute, size);
                }
            });
        }
        SUBCASE("set value") {
            loop_datasets_components_attributes([](MetaComponent const* component, MetaAttribute const* attribute) {
                for (Idx size = 0; size < 4; ++size) {
                    check_array_set_value(component, attribute, size);
                }
            });
        }
    }
    SUBCASE("Sub-array buffer access") {
        SUBCASE("get value") {
            loop_datasets_components_attributes([](MetaComponent const* component, MetaAttribute const* attribute) {
                for (Idx size = 0; size < 4; ++size) {
                    check_sub_array_get_value(component, attribute, size);
                }
            });
        }
        SUBCASE("set value") {
            loop_datasets_components_attributes([](MetaComponent const* component, MetaAttribute const* attribute) {
                for (Idx size = 0; size < 4; ++size) {
                    check_sub_array_set_value(component, attribute, size);
                }
            });
        }
    }
    SUBCASE("Single buffer access") {
        SUBCASE("get value") {
            loop_datasets_components_attributes([](MetaComponent const* component, MetaAttribute const* attribute) {
                for (Idx size = 0; size < 4; ++size) {
                    check_single_get_value(component, attribute, size);
                }
            });
        }
        SUBCASE("set value") {
            loop_datasets_components_attributes([](MetaComponent const* component, MetaAttribute const* attribute) {
                for (Idx size = 0; size < 4; ++size) {
                    check_single_set_value(component, attribute, size);
                }
            });
        }
    }
}

} // namespace power_grid_model_cpp
