// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_META_DATA_HPP
#define POWER_GRID_MODEL_META_DATA_HPP

#include <bit>
#include <string>

#include "../enum.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

namespace meta_data {

template <class T>
struct trait_pointer_to_member;
template <class StructType, class ValueType>
struct trait_pointer_to_member<ValueType StructType::*> {
    using struct_type = StructType;
    using value_type = ValueType;
};

using SetNaNFunc = std::add_pointer_t<void(void*)>;
using CheckNaNFunc = std::add_pointer_t<bool(void const*)>;
using SetValueFunc = std::add_pointer_t<void(void*, void const*)>;
using CompareValueFunc = std::add_pointer_t<bool(void const* ptr_x, void const* ptr_y, double atol, double rtol)>;

template <class T>
void set_value_template(void* dest, void const* src) {
    *reinterpret_cast<T*>(dest) = *reinterpret_cast<T const*>(src);
}

constexpr std::array<size_t, 1> three_phase_dimension{3};

template <class T, bool is_enum = std::is_enum_v<T>>
struct data_type;

template <>
struct data_type<double, false> {
    static constexpr const char* numpy_type = "f8";
    static constexpr const char* ctype = "double";
    static constexpr size_t ndim = 0;
    static constexpr size_t const* dims = nullptr;
    static constexpr SetNaNFunc set_nan = [](void* ptr) {
        *reinterpret_cast<double*>(ptr) = nan;
    };
    static constexpr CheckNaNFunc check_nan = [](void const* ptr) -> bool {
        return is_nan(*reinterpret_cast<double const*>(ptr));
    };
    static constexpr SetValueFunc set_value = set_value_template<double>;
    static constexpr CompareValueFunc compare_value = [](void const* ptr_x, void const* ptr_y, double atol,
                                                         double rtol) -> bool {
        double const x = *reinterpret_cast<double const*>(ptr_x);
        double const y = *reinterpret_cast<double const*>(ptr_y);
        return std::abs(y - x) < (std::abs(x) * rtol + atol);
    };
};

template <>
struct data_type<int32_t, false> {
    static constexpr const char* numpy_type = "i4";
    static constexpr const char* ctype = "int32_t";
    static constexpr size_t ndim = 0;
    static constexpr size_t const* dims = nullptr;
    static constexpr SetNaNFunc set_nan = [](void* ptr) {
        *reinterpret_cast<int32_t*>(ptr) = na_IntID;
    };
    static constexpr CheckNaNFunc check_nan = [](void const* ptr) -> bool {
        return *reinterpret_cast<int32_t const*>(ptr) == na_IntID;
    };
    static constexpr SetValueFunc set_value = set_value_template<int32_t>;
    static constexpr CompareValueFunc compare_value = [](void const* ptr_x, void const* ptr_y, double, double) -> bool {
        return *reinterpret_cast<int32_t const*>(ptr_x) == *reinterpret_cast<int32_t const*>(ptr_y);
    };
};

template <>
struct data_type<int8_t, false> {
    static constexpr const char* numpy_type = "i1";
    static constexpr const char* ctype = "int8_t";
    static constexpr size_t ndim = 0;
    static constexpr size_t const* dims = nullptr;
    static constexpr SetNaNFunc set_nan = [](void* ptr) {
        *reinterpret_cast<int8_t*>(ptr) = na_IntS;
    };
    static constexpr CheckNaNFunc check_nan = [](void const* ptr) -> bool {
        return *reinterpret_cast<int8_t const*>(ptr) == na_IntS;
    };
    static constexpr SetValueFunc set_value = set_value_template<int8_t>;
    static constexpr CompareValueFunc compare_value = [](void const* ptr_x, void const* ptr_y, double, double) -> bool {
        return *reinterpret_cast<int8_t const*>(ptr_x) == *reinterpret_cast<int8_t const*>(ptr_y);
    };
};

template <>
struct data_type<RealValue<false>, false> {
    static constexpr const char* numpy_type = "f8";
    static constexpr const char* ctype = "double[3]";
    static constexpr size_t ndim = 1;
    static constexpr size_t const* dims = three_phase_dimension.data();
    static constexpr SetNaNFunc set_nan = [](void* ptr) {
        *reinterpret_cast<RealValue<false>*>(ptr) = RealValue<false>{nan, nan, nan};
    };
    static constexpr CheckNaNFunc check_nan = [](void const* ptr) -> bool {
        return is_nan(*reinterpret_cast<RealValue<false> const*>(ptr));
    };
    static constexpr SetValueFunc set_value = set_value_template<RealValue<false>>;
    static constexpr CompareValueFunc compare_value = [](void const* ptr_x, void const* ptr_y, double atol,
                                                         double rtol) -> bool {
        RealValue<false> const x = *reinterpret_cast<RealValue<false> const*>(ptr_x);
        RealValue<false> const y = *reinterpret_cast<RealValue<false> const*>(ptr_y);
        return (abs(y - x) < (abs(x) * rtol + atol)).all();
    };
};

template <class T>
struct data_type<T, true> : data_type<std::underlying_type_t<T>> {};

struct DataAttribute {
    std::string name;
    std::string numpy_type;
    std::string ctype;
    std::vector<size_t> dims;
    size_t offset;
    size_t size;
    SetNaNFunc set_nan;
    CheckNaNFunc check_nan;
    SetValueFunc set_value;
    CompareValueFunc compare_value;
};

template <auto member_ptr>
inline size_t get_offset() {
    using struct_type = typename trait_pointer_to_member<decltype(member_ptr)>::struct_type;
    struct_type const obj{};
    return (size_t)(&(obj.*member_ptr)) - (size_t)&obj;
}

constexpr bool is_little_endian() {
    return std::endian::native == std::endian::little;
}

template <auto member_ptr>
inline DataAttribute get_data_attribute(std::string const& name) {
    using value_type = typename trait_pointer_to_member<decltype(member_ptr)>::value_type;
    using single_data_type = data_type<value_type>;
    DataAttribute attr{};
    attr.name = name;
    attr.numpy_type = single_data_type::numpy_type;
    attr.ctype = single_data_type::ctype;
    attr.offset = get_offset<member_ptr>();
    attr.size = sizeof(value_type);
    if constexpr (single_data_type::ndim > 0) {
        attr.dims = std::vector<size_t>(single_data_type::dims, single_data_type::dims + single_data_type::ndim);
    }
    attr.set_nan = single_data_type::set_nan;
    attr.check_nan = single_data_type::check_nan;
    attr.set_value = single_data_type::set_value;
    attr.compare_value = single_data_type::compare_value;
    return attr;
}

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

    void* get_position(void* ptr, Idx position) const {
        return reinterpret_cast<char*>(ptr) + position * size;
    }
    void const* get_position(void const* ptr, Idx position) const {
        return reinterpret_cast<char const*>(ptr) + position * size;
    }

    // set nan for all attributes
    void set_nan(void* ptr, Idx position = 0) const {
        ptr = get_position(ptr, position);
        for (DataAttribute const& attr : attributes) {
            void* const offset_ptr = reinterpret_cast<char*>(ptr) + attr.offset;
            attr.set_nan(offset_ptr);
        }
    }
    // check nan for a attribute
    bool check_nan(void const* ptr, DataAttribute const& attr, Idx position = 0) const {
        ptr = get_position(ptr, position);
        void const* const offset_ptr = reinterpret_cast<char const*>(ptr) + attr.offset;
        return attr.check_nan(offset_ptr);
    }
    // set value of one attribute
    void set_attr(void* ptr, void const* value_ptr, DataAttribute const& attr, Idx position = 0) const {
        ptr = get_position(ptr, position);
        void* const offset_ptr = reinterpret_cast<char*>(ptr) + attr.offset;
        attr.set_value(offset_ptr, value_ptr);
    }
    // get value of one attribute
    void get_attr(void const* ptr, void* value_ptr, DataAttribute const& attr, Idx position = 0) const {
        ptr = get_position(ptr, position);
        void const* const offset_ptr = reinterpret_cast<char const*>(ptr) + attr.offset;
        attr.set_value(value_ptr, offset_ptr);
    }
    // compare value of one attribute
    bool compare_attr(void const* ptr_x, void const* ptr_y, double atol, double rtol, DataAttribute const& attr,
                      Idx position = 0) const {
        ptr_x = get_position(ptr_x, position);
        ptr_y = get_position(ptr_y, position);
        void const* const attr_ptr_x = reinterpret_cast<char const*>(ptr_x) + attr.offset;
        void const* const attr_ptr_y = reinterpret_cast<char const*>(ptr_y) + attr.offset;
        return attr.compare_value(attr_ptr_x, attr_ptr_y, atol, rtol);
    }
};

using PowerGridMetaData = std::map<std::string, MetaData>;
using AllPowerGridMetaData = std::map<std::string, PowerGridMetaData>;

// empty template class for meta data generation functor
template <class T>
struct get_meta;

}  // namespace meta_data

}  // namespace power_grid_model

#endif