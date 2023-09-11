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
#include <string_view>

namespace power_grid_model::meta_data {

// pointer to member
template <class T> struct trait_pointer_to_member;
template <class StructType, class ValueType> struct trait_pointer_to_member<ValueType StructType::*> {
    using value_type = ValueType;
};
template <class StructType, auto member_ptr> inline size_t get_offset() {
    StructType const obj{};
    return (size_t)(&(obj.*member_ptr)) - (size_t)&obj;
}

// empty template functor classes to generate attributes list
template <class T> struct get_attributes_list;
template <class T> struct get_component_nan;

// ctype string
template <class T> struct ctype_t;
template <> struct ctype_t<double> {
    static constexpr CType value = CType::c_double;
};
template <> struct ctype_t<int32_t> {
    static constexpr CType value = CType::c_int32;
};
template <> struct ctype_t<int8_t> {
    static constexpr CType value = CType::c_int8;
};

template <> struct ctype_t<RealValue<false>> {
    static constexpr CType value = CType::c_double3;
};
template <class T>
    requires std::is_enum_v<T>
struct ctype_t<T> : ctype_t<std::underlying_type_t<T>> {};
template <class T> constexpr CType ctype_v = ctype_t<T>::value;

// function selector based on ctype
// the operator() of the functor should have a single template parameter
// the selector will instantiate the operator() with relevant type
template <class Functor, class... Args> decltype(auto) ctype_func_selector(CType ctype, Functor f, Args&&... args) {
    using enum CType;

    switch (ctype) {
    case c_double:
        return f.template operator()<double>(std::forward<Args>(args)...);
    case c_double3:
        return f.template operator()<RealValue<false>>(std::forward<Args>(args)...);
    case c_int8:
        return f.template operator()<int8_t>(std::forward<Args>(args)...);
    case c_int32:
        return f.template operator()<int32_t>(std::forward<Args>(args)...);
    default:
        throw MissingCaseForEnumError{"CType selector", ctype};
    }
}

// set nan
inline void set_nan(double& x) { x = nan; }
inline void set_nan(IntS& x) { x = na_IntS; }
inline void set_nan(ID& x) { x = na_IntID; }
inline void set_nan(RealValue<false>& x) { x = RealValue<false>{nan}; }
template <class Enum>
    requires std::same_as<std::underlying_type_t<Enum>, IntS>
inline void set_nan(Enum& x) {
    x = static_cast<Enum>(na_IntS);
}

using RawDataPtr = void*;            // raw mutable data ptr
using RawDataConstPtr = void const*; // raw read-only data ptr

// meta attribute
template <class StructType, auto member_ptr> struct MetaAttributeImpl {
    using ValueType = typename trait_pointer_to_member<decltype(member_ptr)>::value_type;
    static bool check_nan(RawDataConstPtr buffer_ptr, Idx pos) {
        return is_nan((reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr);
    }
    static bool check_all_nan(RawDataConstPtr buffer_ptr, Idx size) {
        return std::all_of(IdxCount{0}, IdxCount{size}, [buffer_ptr](Idx i) { return check_nan(buffer_ptr, i); });
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
        } else if constexpr (std::same_as<ValueType, RealValue<false>>) {
            return (abs(y - x) < (abs(x) * rtol + atol)).all();
        } else {
            return x == y;
        }
    }
};

struct MetaAttribute {
    template <class StructType, auto member_ptr,
              class ValueType = typename trait_pointer_to_member<decltype(member_ptr)>::value_type>
    MetaAttribute(MetaAttributeImpl<StructType, member_ptr> /* attribute_data */, std::string attr_name)
        : name{std::move(attr_name)},
          ctype{ctype_v<ValueType>},
          offset{get_offset<StructType, member_ptr>()},
          size{sizeof(ValueType)},
          component_size{sizeof(StructType)},
          check_nan{MetaAttributeImpl<StructType, member_ptr>::check_nan},
          check_all_nan{MetaAttributeImpl<StructType, member_ptr>::check_all_nan},
          set_value{MetaAttributeImpl<StructType, member_ptr>::set_value},
          get_value{MetaAttributeImpl<StructType, member_ptr>::get_value},
          compare_value{MetaAttributeImpl<StructType, member_ptr>::compare_value} {}

    // meta data
    std::string name;
    CType ctype{};
    size_t offset{};
    size_t size{};
    size_t component_size{};

    // function pointers
    std::add_pointer_t<bool(RawDataConstPtr, Idx)> check_nan{};
    std::add_pointer_t<bool(RawDataConstPtr, Idx)> check_all_nan{};
    std::add_pointer_t<void(RawDataPtr, RawDataConstPtr, Idx)> set_value{};
    std::add_pointer_t<void(RawDataConstPtr, RawDataPtr, Idx)> get_value{};
    std::add_pointer_t<bool(RawDataConstPtr, RawDataConstPtr, double, double, Idx)> compare_value{};

    // get attribute by offsetting the pointer
    template <class T> T& get_attribute(std::conditional_t<std::is_const_v<T>, RawDataConstPtr, RawDataPtr> ptr) const {
        assert(ctype_v<std::remove_cv_t<T>> == ctype);
        using CharType = std::conditional_t<std::is_const_v<T>, char const*, char*>;
        return *reinterpret_cast<T*>(reinterpret_cast<CharType>(ptr) + offset);
    }
};

// meta component
template <class StructType> struct MetaComponentImpl {
    static RawDataPtr create_buffer(Idx size) { return new StructType[size]; }
    static void destroy_buffer(RawDataConstPtr buffer_ptr) { delete[] reinterpret_cast<StructType const*>(buffer_ptr); }
    static void set_nan(RawDataPtr buffer_ptr, Idx pos, Idx size) {
        static StructType const nan_value = get_component_nan<StructType>{}();
        auto ptr = reinterpret_cast<StructType*>(buffer_ptr);
        std::fill(ptr + pos, ptr + pos + size, nan_value);
    }
};

struct MetaComponent {
    template <class StructType>
    MetaComponent(MetaComponentImpl<StructType> /* component_data */, std::string comp_name)
        : name{std::move(comp_name)},
          size{sizeof(StructType)},
          alignment{alignof(StructType)},
          attributes{get_attributes_list<StructType>{}()},
          set_nan{MetaComponentImpl<StructType>::set_nan},
          create_buffer{MetaComponentImpl<StructType>::create_buffer},
          destroy_buffer{MetaComponentImpl<StructType>::destroy_buffer} {}

    // meta data
    std::string name;
    size_t size;
    size_t alignment;
    std::vector<MetaAttribute> attributes;

    // function pointers
    std::add_pointer_t<void(RawDataPtr, Idx, Idx)> set_nan;
    std::add_pointer_t<RawDataPtr(Idx)> create_buffer;
    std::add_pointer_t<void(RawDataConstPtr)> destroy_buffer;

    Idx n_attributes() const { return static_cast<Idx>(attributes.size()); }

    MetaAttribute const& get_attribute(std::string_view attribute_name) const {
        Idx const found = find_attribute(attribute_name);
        if (found < 0) {
            throw std::out_of_range{"Cannot find attribute with name: " + std::string(attribute_name) + "!\n"};
        }
        return attributes[found];
    }

    Idx find_attribute(std::string_view attribute_name) const {
        for (Idx i = 0; i != n_attributes(); ++i) {
            if (attributes[i].name == attribute_name) {
                return i;
            }
        }
        return -1;
    }

    bool has_attribute(std::string_view attribute_name) const { return find_attribute(attribute_name) >= 0; }

    RawDataPtr advance_ptr(RawDataPtr ptr, Idx difference) const {
        return reinterpret_cast<char*>(ptr) + difference * size;
    }
    RawDataConstPtr advance_ptr(RawDataConstPtr ptr, Idx difference) const {
        return reinterpret_cast<char const*>(ptr) + difference * size;
    }
};

struct MetaDataset {
    std::string name;
    std::vector<MetaComponent> components;

    Idx n_components() const { return static_cast<Idx>(components.size()); }

    MetaComponent const& get_component(std::string_view component_name) const {
        for (auto const& component : components) {
            if (component.name == component_name) {
                return component;
            }
        }
        throw std::out_of_range{"Cannot find component with name: " + std::string{component_name} + "!\n"};
    }
};

// meta data
struct MetaData {
    std::vector<MetaDataset> datasets;

    Idx n_datasets() const { return static_cast<Idx>(datasets.size()); }

    MetaDataset const& get_dataset(std::string_view dataset_name) const {
        for (auto const& dataset : datasets) {
            if (dataset.name == dataset_name) {
                return dataset;
            }
        }
        throw std::out_of_range{"Cannot find dataset with name: " + std::string{dataset_name} + "!\n"};
    }
};

// little endian
constexpr bool is_little_endian() { return std::endian::native == std::endian::little; }

} // namespace power_grid_model::meta_data

#endif
