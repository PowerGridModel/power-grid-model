// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"

#include <bit>
#include <span>
#include <string>
#include <string_view>

namespace power_grid_model::meta_data {

// primary template to get the attribute list of a component
// the specializations will contain static constexpr "value" field
//    which is a std::array
// the specializations are automatically generated
template <class T> struct get_attributes_list;

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

template <> struct ctype_t<RealValue<asymmetric_t>> {
    static constexpr CType value = CType::c_double3;
};
template <class T>
    requires std::is_enum_v<T>
struct ctype_t<T> : ctype_t<std::underlying_type_t<T>> {};
template <class T> constexpr CType ctype_v = ctype_t<T>::value;

// function selector based on ctype
// the operator() of the functor should have a single template parameter
// the selector will instantiate the operator() with relevant type
template <class Functor, class... Args> decltype(auto) ctype_func_selector(CType ctype, Functor&& f, Args&&... args) {
    using enum CType;

    switch (ctype) {
    case c_double:
        return std::forward<Functor>(f).template operator()<double>(std::forward<Args>(args)...);
    case c_double3:
        return std::forward<Functor>(f).template operator()<RealValue<asymmetric_t>>(std::forward<Args>(args)...);
    case c_int8:
        return std::forward<Functor>(f).template operator()<int8_t>(std::forward<Args>(args)...);
    case c_int32:
        return std::forward<Functor>(f).template operator()<int32_t>(std::forward<Args>(args)...);
    default:
        throw MissingCaseForEnumError{"CType selector", ctype};
    }
}

// set nan
constexpr void set_nan(double& x) { x = nan; }
constexpr void set_nan(IntS& x) { x = na_IntS; }
constexpr void set_nan(ID& x) { x = na_IntID; }
inline void set_nan(RealValue<asymmetric_t>& x) { x = RealValue<asymmetric_t>{nan}; }
template <class Enum>
    requires std::same_as<std::underlying_type_t<Enum>, IntS>
constexpr void set_nan(Enum& x) {
    x = static_cast<Enum>(na_IntS);
}
template <typename T>
    requires requires(T t) {
        { set_nan(t) };
    }
inline T const nan_value = [] {
    T v{};
    set_nan(v);
    return v;
}();

using RawDataPtr = void*;            // raw mutable data ptr
using RawDataConstPtr = void const*; // raw read-only data ptr

// meta attribute
struct MetaAttribute {
    // meta data
    char const* name{};
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
struct MetaComponent {
    // meta data
    char const* name;
    size_t size;
    size_t alignment;
    std::span<MetaAttribute const> attributes;

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

// meta dataset
struct MetaDataset {
    char const* name;
    std::span<MetaComponent const> components;

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
    std::span<MetaDataset const> datasets;

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

// list of all dataset names
struct input_getter_s {
    static constexpr char const* name = "input";
    template <class T> using type = typename T::InputType;
};
struct update_getter_s {
    static constexpr char const* name = "update";
    template <class T> using type = typename T::UpdateType;
};
struct sym_output_getter_s {
    static constexpr char const* name = "sym_output";
    template <class T> using type = typename T::template OutputType<symmetric_t>;
};
struct asym_output_getter_s {
    static constexpr char const* name = "asym_output";
    template <class T> using type = typename T::template OutputType<asymmetric_t>;
};
struct sc_output_getter_s {
    static constexpr char const* name = "sc_output";
    template <class T> using type = typename T::ShortCircuitOutputType;
};

} // namespace power_grid_model::meta_data
