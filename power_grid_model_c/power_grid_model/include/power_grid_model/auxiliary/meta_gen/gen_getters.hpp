// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_CXX_MODULE_PURVIEW
#include "../../common/common.hpp"
#include "../../common/component_list.hpp"
#include "../../common/counting_iterator.hpp"
#include "../../common/three_phase_tensor.hpp"
#include "../meta_data.hpp"
#endif

#ifndef PGM_CXX_MODULE_PURVIEW
#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#endif

namespace power_grid_model::meta_data::meta_data_gen {

// pointer to member
template <class T> struct trait_pointer_to_member;
template <class StructType, class ValueType> struct trait_pointer_to_member<ValueType StructType::*> {
    using value_type = ValueType;
    using struct_type = StructType;
};

// The functions stored in MetaAttribute are static member functions of a class template that only depends on the
// struct type, value type and offset. They must not depend on a pointer-to-member template argument: clang (22.1.3)
// crashes when mangling such entities if the struct type comes from an imported module.
template <class StructType, class ValueType, size_t offset> struct meta_attribute_functions {
    static ValueType const& value_at(RawDataConstPtr ptr, Idx pos) {
        return *reinterpret_cast<ValueType const*>(
            reinterpret_cast<char const*>(static_cast<StructType const*>(ptr) + pos) + offset);
    }
    static ValueType& value_at(RawDataPtr ptr, Idx pos) {
        return *reinterpret_cast<ValueType*>(reinterpret_cast<char*>(static_cast<StructType*>(ptr) + pos) + offset);
    }

    static bool check_nan(RawDataConstPtr buffer_ptr, Idx pos) { return is_nan(value_at(buffer_ptr, pos)); }
    static bool check_all_nan(RawDataConstPtr buffer_ptr, Idx size) {
        return std::ranges::all_of(IdxRange{size}, [buffer_ptr](Idx i) { return is_nan(value_at(buffer_ptr, i)); });
    }
    static void set_value(RawDataPtr buffer_ptr, RawDataConstPtr value_ptr, Idx pos) {
        value_at(buffer_ptr, pos) = *reinterpret_cast<ValueType const*>(value_ptr);
    }
    static void get_value(RawDataConstPtr buffer_ptr, RawDataPtr value_ptr, Idx pos) {
        *reinterpret_cast<ValueType*>(value_ptr) = value_at(buffer_ptr, pos);
    }
    static bool compare_value(RawDataConstPtr ptr_x, RawDataConstPtr ptr_y, [[maybe_unused]] double atol,
                              [[maybe_unused]] double rtol, Idx pos) {
        ValueType const& x = value_at(ptr_x, pos);
        ValueType const& y = value_at(ptr_y, pos);
        if constexpr (std::same_as<ValueType, double>) {
            return std::abs(y - x) < (std::abs(x) * rtol + atol);
        } else if constexpr (std::same_as<ValueType, RealValue<asymmetric_t>>) {
            return (abs(y - x) < (abs(x) * rtol + atol)).all();
        } else {
            return x == y;
        }
    }
};

// getter for meta attribute
// consteval: the pointer-to-member only deduces the types and must never be part of an emitted symbol
template <auto member_ptr, size_t offset, class MemberPtr = decltype(member_ptr)>
consteval MetaAttribute get_meta_attribute(char const* attribute_name) {
    using ValueType = trait_pointer_to_member<MemberPtr>::value_type;
    using StructType = trait_pointer_to_member<MemberPtr>::struct_type;
    using functions = meta_attribute_functions<StructType, ValueType, offset>;

    return MetaAttribute{
        .name = attribute_name,
        .ctype = ctype_v<ValueType>,
        .offset = offset,
        .size = sizeof(ValueType),
        .component_size = sizeof(StructType),
        .check_nan = &functions::check_nan,
        .check_all_nan = &functions::check_all_nan,
        .set_value = &functions::set_value,
        .get_value = &functions::get_value,
        .compare_value = &functions::compare_value,
    };
};
// getter for meta component
template <class StructType> constexpr MetaComponent get_meta_component(char const* component_name) {
    return MetaComponent{
        .name = component_name,
        .size = sizeof(StructType),
        .alignment = alignof(StructType),
        .attributes = get_attributes_list<StructType>::value,
        .set_nan =
            [](RawDataPtr buffer_ptr, Idx pos, Idx size) {
                auto ptr = reinterpret_cast<StructType*>(buffer_ptr);
                std::fill(ptr + pos, ptr + pos + size, StructType{});
            },
        .create_buffer = [](Idx size) -> RawDataPtr {
            return new StructType[size]; // NOSONAR(S5025)
        },
        .destroy_buffer =
            [](RawDataConstPtr buffer_ptr) {
                delete[] reinterpret_cast<StructType const*>(buffer_ptr); // NOSONAR(S5025)
            },
    };
};

// getter for meta dataset
template <class struct_getter, class comp_list> struct get_meta_dataset;
template <class struct_getter, class... ComponentType>
struct get_meta_dataset<struct_getter, ComponentList<ComponentType...>> {
    static constexpr size_t n_components = sizeof...(ComponentType);
    static constexpr std::array<MetaComponent, n_components> components{
        get_meta_component<typename struct_getter::template type<ComponentType>>(ComponentType::name)...};
    static constexpr MetaDataset value{
        .name = struct_getter::name,
        .components = components,
    };
};

// get meta data
template <class comp_list, class... struct_getter> struct get_meta_data {
    static constexpr std::array<MetaDataset, sizeof...(struct_getter)> datasets{
        get_meta_dataset<struct_getter, comp_list>::value...};
    static constexpr MetaData value{
        .datasets = datasets,
    };
};

} // namespace power_grid_model::meta_data::meta_data_gen
