// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../dataset.hpp"

#include <concepts>
#include <ranges>
#include <span>

namespace power_grid_model::meta_data::detail {

struct row_based_t {};
struct columnar_t {};
constexpr row_based_t row_based{};
constexpr columnar_t columnar{};

template <typename T>
concept row_based_or_columnar_c = std::derived_from<T, row_based_t> || std::derived_from<T, columnar_t>;

template <row_based_or_columnar_c T> constexpr bool is_row_based_v = std::derived_from<T, row_based_t>;
template <row_based_or_columnar_c T> constexpr bool is_columnar_v = std::derived_from<T, columnar_t>;

// obtain attribute buffers of a columnar dataset buffer, ordered by the provided meta attributes.
//
// If none of the provided meta attributes are present in the provided attribute buffers, the result is empty.
// Otherwise, returns a list of attribute buffers with the same order as the input attribute order.
// The attribute buffers are copies of the associated attribute buffers, when provided, and otherwise empty.
template <typename BufferType>
    requires requires(BufferType const& b) {
        { b.attributes } -> std::convertible_to<std::vector<AttributeBuffer<typename BufferType::Data>>>;
    }
std::vector<AttributeBuffer<typename BufferType::Data>>
reordered_attribute_buffers(BufferType& buffer, std::span<MetaAttribute const* const> attribute_order) {
    using Data = typename BufferType::Data;
    using AttributeBufferType = AttributeBuffer<Data>;

    assert(buffer.data == nullptr);

    std::vector<AttributeBufferType> result(attribute_order.size());
    std::ranges::transform(
        attribute_order, result.begin(), [&buffer](auto const* const attribute) -> AttributeBufferType {
            if (auto it = std::ranges::find_if(buffer.attributes,
                                               [&attribute](auto const& attribute_buffer) {
                                                   return attribute_buffer.meta_attribute == attribute;
                                               });
                it != buffer.attributes.end()) {
                return *it;
            }
            return AttributeBuffer<void>{};
        });

    if (std::ranges::all_of(result, [](auto const& attribute_buffer) { return attribute_buffer.data == nullptr; })) {
        result = {};
    }

    return result;
}

} // namespace power_grid_model::meta_data::detail
