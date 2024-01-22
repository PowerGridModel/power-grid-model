// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_META_DATA_GEN_HPP
#define POWER_GRID_MODEL_META_DATA_GEN_HPP

#include "input.hpp"
#include "meta_data.hpp"
#include "meta_gen/gen_getters.hpp"
#include "meta_gen/input.hpp"
#include "meta_gen/output.hpp"
#include "meta_gen/update.hpp"
#include "output.hpp"
#include "update.hpp"

#include "../all_components.hpp"
#include "../power_grid_model.hpp"

#include <map>
#include <string>

// generate of meta data
namespace power_grid_model::meta_data {

namespace meta_data_gen {

// list of all dataset names
template <class T> struct struct_getter_input {
    using type = typename T::InputType;
};
template <class T> struct struct_getter_update {
    using type = typename T::UpdateType;
};
template <class T> struct struct_getter_sym_output {
    using type = typename T::template OutputType<true>;
};
template <class T> struct struct_getter_asym_output {
    using type = typename T::template OutputType<false>;
};
template <class T> struct struct_getter_sc_output {
    using type = typename T::ShortCircuitOutputType;
};

// generate meta data
constexpr MetaData meta_data = get_meta_data<AllComponents, // all components list
                                             dataset_mark<[] { return "input"; }, struct_getter_input>,
                                             dataset_mark<[] { return "update"; }, struct_getter_update>,
                                             dataset_mark<[] { return "sym_output"; }, struct_getter_sym_output>,
                                             dataset_mark<[] { return "asym_output"; }, struct_getter_asym_output>,
                                             dataset_mark<[] { return "sc_output"; }, struct_getter_sc_output>
                                             // end list of all marks
                                             >::value;

} // namespace meta_data_gen

constexpr MetaData meta_data = meta_data_gen::meta_data;

} // namespace power_grid_model::meta_data

#endif
