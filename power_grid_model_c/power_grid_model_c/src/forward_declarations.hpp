// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "power_grid_model_c/basics.h"

#include <power_grid_model/auxiliary/dataset_fwd.hpp>

#include <type_traits>

// forward declare all referenced struct/class in C++ core
// alias them in the root namespace

namespace power_grid_model {

class MainModel;

namespace meta_data {

struct MetaAttribute;
struct MetaComponent;
struct MetaDataset;
class Serializer;
class Deserializer;

template <dataset_type_tag dataset_type> class Dataset;
using ConstDataset = Dataset<const_dataset_t>;
using MutableDataset = Dataset<mutable_dataset_t>;
using WritableDataset = Dataset<writable_dataset_t>;

struct DatasetInfo;

} // namespace meta_data

} // namespace power_grid_model

namespace power_grid_model_c {

namespace detail {

template <class c_type_input, class cpp_type_input> struct c_cpp_type_map {
    using c_type = c_type_input;
    using cpp_type = cpp_type_input;
    static_assert(!std::is_same_v<c_type, cpp_type>, "C and C++ types in type map cannot be the same!");
};

template <class... type_maps> struct type_mapping_list_impl {
    template <class c_type> using get_cpp_type_t = void;
    template <class cpp_type> using get_c_type_t = void;
};
template <class first_map, class... rest_maps> struct type_mapping_list_impl<first_map, rest_maps...> {

    template <class c_type>
    using get_cpp_type_t =
        std::conditional_t<std::is_same_v<typename first_map::c_type, c_type>, typename first_map::cpp_type,
                           typename type_mapping_list_impl<rest_maps...>::template get_cpp_type_t<c_type>>;

    template <class cpp_type>
    using get_c_type_t =
        std::conditional_t<std::is_same_v<typename first_map::cpp_type, cpp_type>, typename first_map::c_type,
                           typename type_mapping_list_impl<rest_maps...>::template get_c_type_t<cpp_type>>;
};

using type_mapping_list = type_mapping_list_impl<
    c_cpp_type_map<PGM_PowerGridModel, power_grid_model::MainModel>,
    c_cpp_type_map<PGM_MetaAttribute, power_grid_model::meta_data::MetaAttribute>,
    c_cpp_type_map<PGM_MetaComponent, power_grid_model::meta_data::MetaComponent>,
    c_cpp_type_map<PGM_MetaDataset, power_grid_model::meta_data::MetaDataset>,
    c_cpp_type_map<PGM_Serializer, power_grid_model::meta_data::Serializer>,
    c_cpp_type_map<PGM_Deserializer, power_grid_model::meta_data::Deserializer>,
    c_cpp_type_map<PGM_ConstDataset, power_grid_model::meta_data::Dataset<power_grid_model::const_dataset_t>>,
    c_cpp_type_map<PGM_MutableDataset, power_grid_model::meta_data::Dataset<power_grid_model::mutable_dataset_t>>,
    c_cpp_type_map<PGM_WritableDataset, power_grid_model::meta_data::Dataset<power_grid_model::writable_dataset_t>>,
    c_cpp_type_map<PGM_DatasetInfo, power_grid_model::meta_data::DatasetInfo>>;

template <class CTypePtr> struct convert_ptr_to_cpp {
    static constexpr bool is_const = std::is_const_v<std::remove_pointer_t<CTypePtr>>;
    using base_c_type = std::remove_const_t<std::remove_pointer_t<CTypePtr>>;
    using mapped_cpp_raw_type = type_mapping_list::get_cpp_type_t<base_c_type>;
    using mapped_cpp_type = std::conditional_t<is_const, std::add_const_t<mapped_cpp_raw_type>, mapped_cpp_raw_type>;
    using type = mapped_cpp_type*;
};
template <class CTypePtr> using convert_ptr_to_cpp_t = typename convert_ptr_to_cpp<CTypePtr>::type;

template <class CPPTypePtr> struct convert_ptr_to_c {
    static constexpr bool is_const = std::is_const_v<std::remove_pointer_t<CPPTypePtr>>;
    using base_cpp_type = std::remove_const_t<std::remove_pointer_t<CPPTypePtr>>;
    using mapped_c_raw_type = type_mapping_list::get_c_type_t<base_cpp_type>;
    using mapped_c_type = std::conditional_t<is_const, std::add_const_t<mapped_c_raw_type>, mapped_c_raw_type>;
    using type = mapped_c_type*;
};
template <class CPPTypePtr> using convert_ptr_to_c_t = typename convert_ptr_to_c<CPPTypePtr>::type;

template <class CTypePtr> auto cast_to_cpp(CTypePtr ptr) {
    return reinterpret_cast<convert_ptr_to_cpp_t<CTypePtr>>(ptr);
}

template <class CPPTypePtr> auto cast_to_c(CPPTypePtr ptr) {
    return reinterpret_cast<convert_ptr_to_c_t<CPPTypePtr>>(ptr);
}

} // namespace detail

using detail::cast_to_c;
using detail::cast_to_cpp;

} // namespace power_grid_model_c
