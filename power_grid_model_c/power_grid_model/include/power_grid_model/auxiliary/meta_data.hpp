// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_META_DATA_HPP
#define POWER_GRID_MODEL_META_DATA_HPP

#include <bit>
#include <memory>
#include <string>

#include "../enum.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

namespace meta_data {

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

// ctype string
template <class T, bool is_enum = std::is_enum_v<T>>
struct ctype_t;
template <>
struct ctype_t<double, false> {
    static constexpr const char* value = "double";
};
template <>
struct ctype_t<int32_t, false> {
    static constexpr const char* value = "int32_t";
};
template <>
struct ctype_t<int8_t, false> {
    static constexpr const char* value = "int8_t";
};

template <>
struct ctype_t<RealValue<false>, false> {
    static constexpr const char* value = "double[3]";
};
template <class T>
struct ctype_t<T, true> : ctype_t<std::underlying_type_t<T>> {};
template <class T>
constexpr const char* ctype_v = ctype_t<T>::value;

using RawDataPtr = void*;             // raw mutable data ptr
using RawDataConstPtr = void const*;  // raw read-only data ptr

// meta attribute
struct MetaAttribute {
    std::string name;
    std::string ctype;
    size_t offset;
    size_t size;
    size_t component_size;

    // virtual functions
    virtual bool check_nan(RawDataConstPtr buffer_ptr, Idx pos) = 0;
    virtual void set_value(RawDataPtr buffer_ptr, RawDataConstPtr value_ptr, Idx pos) = 0;
    virtual void get_value(RawDataConstPtr buffer_ptr, RawDataPtr value_ptr, Idx pos) = 0;
    virtual bool compare_value(RawDataConstPtr buffer_ptr, RawDataPtr value_ptr, double atol, double rtol, Idx pos) = 0;
    virtual ~MetaAttribute() = default;
};

template <class StructType, auto member_ptr>
struct MetaAttributeImpl : MetaAttribute {
    using ValueType = typename trait_pointer_to_member<decltype(member_ptr)>::value_type;
    MetaAttributeImpl(std::string attr_name) :
        MetaAttribute{
            .name = attr_name,
            .ctype = ctype,
            .offset = get_offset<StructType, member_ptr>(),
            .size = sizeof(ValueType),
            .component_size = sizeof(StructType)
        }
    {}

    // virtual functions
    bool check_nan(RawDataConstPtr buffer_ptr, Idx pos) final {
        return is_nan((reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr);
    }
    void set_value(RawDataPtr buffer_ptr, RawDataConstPtr value_ptr, Idx pos) final {
        (reinterpret_cast<StructType*>(buffer_ptr) + pos)->*member_ptr = *reinterpret_cast<ValueType const*>(value_ptr);
    }
    void get_value(RawDataConstPtr buffer_ptr, RawDataPtr value_ptr, Idx pos) final {
        *reinterpret_cast<ValueType*>(value_ptr) = (reinterpret_cast<StructType const*>(buffer_ptr) + pos)->*member_ptr;
    }
    bool compare_value(RawDataConstPtr buffer_ptr, RawDataConstPtr value_ptr, double atol, double rtol, Idx pos) final {
        ValueType const& x = (reinterpret_cast<StructType*>(buffer_ptr) + pos)->*member_ptr;
        ValueType const& y = *reinterpret_cast<ValueType const*>(value_ptr);
        if constexpr (std::is_same_v<ValueType, double>) {
            return std::abs(y - x) < (std::abs(x) * rtol + atol);
        }
        else if constexpr (std::is_same_v<ValueType, RealValue<false>>) {
            return (abs(y - x) < (abs(x) * rtol + atol)).all();
        }
        else {
            return x == y;
        }
    }
};

// struct for meta data per type (input/update/output)
struct MetaData {
    std::string name;
    size_t size;
    size_t alignment;
    std::vector<DataAttribute> attributes;

    DataAttribute const& get_attr(std::string const& attr_name) const {
        Idx const found = find_attr(attr_name);
        if (found >= 0) {
            return attributes[found];
        }
        throw std::out_of_range{std::string("Unknown attribute name: ") + attr_name + "!\n"};
    }

    Idx find_attr(std::string const& attr_name) const {
        for (Idx i = 0; i != (Idx)attributes.size(); ++i) {
            if (attributes[i].name == attr_name) {
                return i;
            }
        }
        return -1;
    }

    bool has_attr(std::string const& attr_name) const {
        return find_attr(attr_name) >= 0;
    }

    // set nan for all attributes
    void set_nan(RawDataPtr ptr, Idx position = 0) const {
        ptr = get_position(ptr, position);
        for (DataAttribute const& attr : attributes) {
            void* const offset_ptr = reinterpret_cast<char*>(ptr) + attr.offset;
            attr.set_nan(offset_ptr);
        }
    }
};

using PowerGridMetaData = std::map<std::string, MetaData>;
using AllPowerGridMetaData = std::map<std::string, PowerGridMetaData>;

constexpr bool is_little_endian() {
    return std::endian::native == std::endian::little;
}

// empty template class for meta data generation functor
template <class T>
struct get_meta;

}  // namespace meta_data

}  // namespace power_grid_model

#endif