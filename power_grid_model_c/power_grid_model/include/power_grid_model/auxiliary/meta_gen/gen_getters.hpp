// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../common/component_list.hpp"
#include "../../common/counting_iterator.hpp"
#include "../meta_data.hpp"

namespace power_grid_model::meta_data::meta_data_gen {

// getter for meta attribute
template <class StructType, auto member_ptr, size_t offset, auto attribute_name_getter> struct get_meta_attribute {
    using ValueType = typename trait_pointer_to_member<decltype(member_ptr)>::value_type;

    static constexpr MetaAttribute value{
        .name = attribute_name_getter(),
        .ctype = ctype_v<ValueType>,
        .offset = offset,
        .size = sizeof(ValueType),
        .component_size = sizeof(StructType),
        .check_nan = [](RawDataConstPtr buffer_ptr, Idx pos) -> bool {
            return is_nan((reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr);
        },
        .check_all_nan = [](RawDataConstPtr buffer_ptr, Idx size) -> bool {
            return std::all_of(IdxCount{0}, IdxCount{size}, [buffer_ptr](Idx i) {
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
template <class StructType, auto component_name_getter> struct get_meta_component {
    static constexpr MetaComponent value{
        .name = component_name_getter(),
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
template <auto dataset_name_getter, template <class> class struct_getter, class comp_list> struct get_meta_dataset;
template <auto dataset_name_getter, template <class> class struct_getter, class... ComponentType>
struct get_meta_dataset<dataset_name_getter, struct_getter, ComponentList<ComponentType...>> {
    static constexpr size_t n_components = sizeof...(ComponentType);
    static constexpr std::array<MetaComponent, n_components> components{
        get_meta_component<typename struct_getter<ComponentType>::type, [] { return ComponentType::name; }>::value...};
    static constexpr MetaDataset value{
        .name = dataset_name_getter(),
        .components = components,
    };
};

// get meta data
template <auto dataset_name_getter, template <class> class struct_getter> struct dataset_mark;
template <class comp_list, class... T> struct get_meta_data;
template <class comp_list, auto... dataset_name_getter, template <class> class... struct_getter>
struct get_meta_data<comp_list, dataset_mark<dataset_name_getter, struct_getter>...> {
    static constexpr std::array<MetaDataset, sizeof...(dataset_name_getter)> datasets{
        get_meta_dataset<dataset_name_getter, struct_getter, comp_list>::value...};
    static constexpr MetaData value{
        .datasets = datasets,
    };
};

} // namespace power_grid_model::meta_data::meta_data_gen
