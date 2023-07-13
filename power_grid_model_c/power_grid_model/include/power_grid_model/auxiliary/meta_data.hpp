// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_META_DATA_HPP
#define POWER_GRID_MODEL_META_DATA_HPP

#include "../enum.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

#include <bit>
#include <string>

namespace power_grid_model::meta_data {

// pointer to member
template <class T>
struct trait_pointer_to_member;
template <class StructType, class ValueType>
struct trait_pointer_to_member<ValueType StructType::*> {
    using value_type = ValueType;
};
template <class StructType, auto member_ptr>
inline size_t get_offset() {
    StructType const obj{};
    return (size_t)(&(obj.*member_ptr)) - (size_t)&obj;
}

// empty template functor classes to generate attributes list
template <class T>
struct get_attributes_list;
template <class T>
struct get_component_nan;

// ctype string
template <class T>
struct ctype_t;
template <>
struct ctype_t<double> {
    static constexpr CType value = CType::c_double;
};
template <>
struct ctype_t<int32_t> {
    static constexpr CType value = CType::c_int32;
};
template <>
struct ctype_t<int8_t> {
    static constexpr CType value = CType::c_int8;
};

template <>
struct ctype_t<RealValue<false>> {
    static constexpr CType value = CType::c_double3;
};
template <class T>
requires std::is_enum_v<T>
struct ctype_t<T> : ctype_t<std::underlying_type_t<T>> {
};
template <class T>
constexpr CType ctype_v = ctype_t<T>::value;

// set nan
inline void set_nan(double& x) {
    x = nan;
}
inline void set_nan(IntS& x) {
    x = na_IntS;
}
inline void set_nan(ID& x) {
    x = na_IntID;
}
inline void set_nan(RealValue<false>& x) {
    x = RealValue<false>{nan};
}
template <class Enum>
requires std::same_as<std::underlying_type_t<Enum>, IntS>
inline void set_nan(Enum& x) {
    x = static_cast<Enum>(na_IntS);
}

using RawDataPtr = void*;             // raw mutable data ptr
using RawDataConstPtr = void const*;  // raw read-only data ptr

// meta attribute
template <class StructType, auto member_ptr>
struct MetaAttributeImpl {
    using ValueType = typename trait_pointer_to_member<decltype(member_ptr)>::value_type;
    static bool check_nan(RawDataConstPtr buffer_ptr, Idx pos) {
        return is_nan((reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr);
    }
    static void set_value(RawDataPtr buffer_ptr, RawDataConstPtr value_ptr, Idx pos) {
        (reinterpret_cast<StructType*>(buffer_ptr) + pos)->*member_ptr = *reinterpret_cast<ValueType const*>(value_ptr);
    }
    static void get_value(RawDataConstPtr buffer_ptr, RawDataPtr value_ptr, Idx pos) {
        *reinterpret_cast<ValueType*>(value_ptr) = (reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr;
    }
    static bool compare_value(RawDataConstPtr ptr_x, RawDataConstPtr ptr_y, double atol, double rtol, Idx pos) {
        ValueType const& x = (reinterpret_cast<StructType const*>(ptr_x) + pos)->*member_ptr;
        ValueType const& y = (reinterpret_cast<StructType const*>(ptr_y) + pos)->*member_ptr;
        if constexpr (std::same_as<ValueType, double>) {
            return std::abs(y - x) < (std::abs(x) * rtol + atol);
        }
        else if constexpr (std::same_as<ValueType, RealValue<false>>) {
            return (abs(y - x) < (abs(x) * rtol + atol)).all();
        }
        else {
            return x == y;
        }
    }
};

}  // namespace power_grid_model::meta_data

// attribute in global namespace
struct PGM_MetaAttribute {
    using Idx = power_grid_model::Idx;
    using CType = power_grid_model::CType;
    template <class T>
    using trait_pointer_to_member = power_grid_model::meta_data::trait_pointer_to_member<T>;
    template <class StructType, auto member_ptr>
    using MetaAttributeImpl = power_grid_model::meta_data::MetaAttributeImpl<StructType, member_ptr>;
    using RawDataConstPtr = power_grid_model::meta_data::RawDataConstPtr;
    using RawDataPtr = power_grid_model::meta_data::RawDataPtr;
    template <class T>
    static constexpr CType ctype_v = power_grid_model::meta_data::ctype_v<T>;

    template <class StructType, auto member_ptr,
              class ValueType = typename trait_pointer_to_member<decltype(member_ptr)>::value_type>
    PGM_MetaAttribute(MetaAttributeImpl<StructType, member_ptr>, std::string const& attr_name)
        : name{attr_name},
          ctype{ctype_v<ValueType>},
          offset{power_grid_model::meta_data::get_offset<StructType, member_ptr>()},
          size{sizeof(ValueType)},
          component_size{sizeof(StructType)},
          check_nan{MetaAttributeImpl<StructType, member_ptr>::check_nan},
          set_value{MetaAttributeImpl<StructType, member_ptr>::set_value},
          get_value{MetaAttributeImpl<StructType, member_ptr>::get_value},
          compare_value{MetaAttributeImpl<StructType, member_ptr>::compare_value} {
    }

    // meta data
    std::string name;
    CType ctype{};
    size_t offset{};
    size_t size{};
    size_t component_size{};

    // function pointers
    std::add_pointer_t<bool(RawDataConstPtr, Idx)> check_nan{};
    std::add_pointer_t<void(RawDataPtr, RawDataConstPtr, Idx)> set_value{};
    std::add_pointer_t<void(RawDataConstPtr, RawDataPtr, Idx)> get_value{};
    std::add_pointer_t<bool(RawDataConstPtr, RawDataConstPtr, double, double, Idx)> compare_value{};
};

namespace power_grid_model::meta_data {

// include inside meta data namespace
using MetaAttribute = PGM_MetaAttribute;

// meta component
template <class StructType>
struct MetaComponentImpl {
    static RawDataPtr create_buffer(Idx size) {
        return new StructType[size];
    }
    static void destroy_buffer(RawDataConstPtr buffer_ptr) {
        delete[] reinterpret_cast<StructType const*>(buffer_ptr);
    }
    static void set_nan(RawDataPtr buffer_ptr, Idx pos, Idx size) {
        static StructType const nan_value = get_component_nan<StructType>{}();
        StructType* ptr = reinterpret_cast<StructType*>(buffer_ptr);
        std::fill(ptr + pos, ptr + pos + size, nan_value);
    }
};

}  // namespace power_grid_model::meta_data

// component in global name space
struct PGM_MetaComponent {
    template <class StructType>
    using MetaComponentImpl = power_grid_model::meta_data::MetaComponentImpl<StructType>;
    using MetaAttribute = power_grid_model::meta_data::MetaAttribute;
    using Idx = power_grid_model::Idx;
    using RawDataConstPtr = power_grid_model::meta_data::RawDataConstPtr;
    using RawDataPtr = power_grid_model::meta_data::RawDataPtr;

    template <class StructType>
    PGM_MetaComponent(MetaComponentImpl<StructType>, std::string const& comp_name)
        : name{comp_name},
          size{sizeof(StructType)},
          alignment{alignof(StructType)},
          attributes{power_grid_model::meta_data::get_attributes_list<StructType>{}()},
          set_nan{MetaComponentImpl<StructType>::set_nan},
          create_buffer{MetaComponentImpl<StructType>::create_buffer},
          destroy_buffer{MetaComponentImpl<StructType>::destroy_buffer} {
    }

    // meta data
    std::string name;
    size_t size;
    size_t alignment;
    std::vector<MetaAttribute> attributes;

    // function pointers
    std::add_pointer_t<void(RawDataPtr, Idx, Idx)> set_nan;
    std::add_pointer_t<RawDataPtr(Idx)> create_buffer;
    std::add_pointer_t<void(RawDataConstPtr)> destroy_buffer;

    Idx n_attributes() const {
        return static_cast<Idx>(attributes.size());
    }

    MetaAttribute const& get_attribute(std::string const& attribute_name) const {
        Idx const found = find_attribute(attribute_name);
        if (found < 0) {
            throw std::out_of_range{"Cannot find attribute with name: " + attribute_name + "!\n"};
        }
        return attributes[found];
    }

    Idx find_attribute(std::string const& attribute_name) const {
        for (Idx i = 0; i != n_attributes(); ++i) {
            if (attributes[i].name == attribute_name) {
                return i;
            }
        }
        return -1;
    }

    Idx has_attribute(std::string const& attribute_name) const {
        return find_attribute(attribute_name) >= 0;
    }
};

namespace power_grid_model::meta_data {

using MetaComponent = PGM_MetaComponent;

}  // namespace power_grid_model::meta_data

// meta dataset in global namespace
struct PGM_MetaDataset {
    using MetaComponent = power_grid_model::meta_data::MetaComponent;
    using Idx = power_grid_model::Idx;

    std::string name;
    std::vector<MetaComponent> components;

    Idx n_components() const {
        return static_cast<Idx>(components.size());
    }

    MetaComponent const& get_component(std::string const& component_name) const {
        for (auto const& component : components) {
            if (component.name == component_name) {
                return component;
            }
        }
        throw std::out_of_range{"Cannot find component with name: " + component_name + "!\n"};
    }
};

namespace power_grid_model::meta_data {

using MetaDataset = PGM_MetaDataset;

// meta data
struct MetaData {
    std::vector<MetaDataset> datasets;

    Idx n_datasets() const {
        return static_cast<Idx>(datasets.size());
    }

    MetaDataset const& get_dataset(std::string const& dataset_name) const {
        for (auto const& dataset : datasets) {
            if (dataset.name == dataset_name) {
                return dataset;
            }
        }
        throw std::out_of_range{"Cannot find dataset with name: " + dataset_name + "!\n"};
    }
};

// little endian
constexpr bool is_little_endian() {
    return std::endian::native == std::endian::little;
}

}  // namespace power_grid_model::meta_data

#endif
