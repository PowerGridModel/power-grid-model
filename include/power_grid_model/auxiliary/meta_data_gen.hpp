// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_META_DATA_GEN_HPP
#define POWER_GRID_MODEL_META_DATA_GEN_HPP

#include <map>
#include <string>

#include "../all_components.hpp"
#include "../power_grid_model.hpp"
#include "input.hpp"
#include "meta_data.hpp"
#include "output.hpp"

// generate of meta data
namespace power_grid_model {

namespace meta_data {

template <class T>
struct MetaDataGeneratorImpl;

template <class... ComponentType>
struct MetaDataGeneratorImpl<ComponentList<ComponentType...>> {
    MetaDataGeneratorImpl() {
        static constexpr std::array func_arr{&retrieve_single_type<ComponentType>...};
        for (auto const func : func_arr) {
            func(*this);
        }
    }

    template <class CompType>
    static void retrieve_single_type(MetaDataGeneratorImpl& inst) {
        inst.meta_data["input"][CompType::name] = CompType::InputType::get_meta();
        inst.meta_data["update"][CompType::name] = CompType::UpdateType::get_meta();
        inst.meta_data["sym_output"][CompType::name] = CompType::template OutputType<true>::get_meta();
        inst.meta_data["asym_output"][CompType::name] = CompType::template OutputType<false>::get_meta();
    }

    AllPowerGridMetaData meta_data;
};

using MetaDataGenerator = MetaDataGeneratorImpl<AllComponents>;

inline AllPowerGridMetaData const& meta_data() {
    static MetaDataGenerator const meta{};
    return meta.meta_data;
}

}  // namespace meta_data

}  // namespace power_grid_model

#endif
