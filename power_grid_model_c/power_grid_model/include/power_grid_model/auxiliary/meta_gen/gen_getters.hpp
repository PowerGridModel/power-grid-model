// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../common/component_list.hpp"
#include "../../common/counting_iterator.hpp"
#include "../meta_data.hpp"

namespace power_grid_model::meta_data::meta_data_gen {

// pointer to member
template <class T> struct trait_pointer_to_member;
template <class StructType, class ValueType> struct trait_pointer_to_member<ValueType StructType::*> {
    using value_type = ValueType;
    using struct_type = StructType;
};

// getter for meta attribute
template <auto member_ptr, class MemberPtr = decltype(member_ptr)>
constexpr MetaAttribute get_meta_attribute(size_t offset, char const* attribute_name) {
    using ValueType = typename trait_pointer_to_member<MemberPtr>::value_type;
    using StructType = typename trait_pointer_to_member<MemberPtr>::struct_type;

    return MetaAttribute{
        .name = attribute_name,
        .ctype = ctype_v<ValueType>,
        .offset = offset,
        .size = sizeof(ValueType),
        .component_size = sizeof(StructType),
        .check_nan = [](RawDataConstPtr buffer_ptr, Idx pos) -> bool {
            return is_nan((reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr);
        },
        .check_all_nan = [](RawDataConstPtr buffer_ptr, Idx size) -> bool {
            return std::ranges::all_of(IdxRange{size}, [buffer_ptr](Idx i) {
                return is_nan((reinterpret_cast<StructType const*>(buffer_ptr) + i)->*member_ptr);
            });
        },
        .set_value =
            [](RawDataPtr buffer_ptr, RawDataConstPtr value_ptr, Idx pos) {
                (reinterpret_cast<StructType*>(buffer_ptr) + pos)->*member_ptr =
                    *reinterpret_cast<ValueType const*>(value_ptr);
            },
        .get_value =
            [](RawDataConstPtr buffer_ptr, RawDataPtr value_ptr, Idx pos) {
                *reinterpret_cast<ValueType*>(value_ptr) =
                    (reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr;
            },
        .compare_value = [](RawDataConstPtr ptr_x, RawDataConstPtr ptr_y, double atol, double rtol, Idx pos) -> bool {
            ValueType const& x = (reinterpret_cast<StructType const*>(ptr_x) + pos)->*member_ptr;
            ValueType const& y = (reinterpret_cast<StructType const*>(ptr_y) + pos)->*member_ptr;
            if constexpr (std::same_as<ValueType, double>) {
                return std::abs(y - x) < (std::abs(x) * rtol + atol);
            } else if constexpr (std::same_as<ValueType, RealValue<asymmetric_t>>) {
                return (abs(y - x) < (abs(x) * rtol + atol)).all();
            } else {
                return x == y;
            }
        },
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
        .create_buffer = [](Idx size) -> RawDataPtr { return new StructType[size]; },
        .destroy_buffer = [](RawDataConstPtr buffer_ptr) { delete[] reinterpret_cast<StructType const*>(buffer_ptr); },
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
