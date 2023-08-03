// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_META_DATA_GEN_HPP
#define POWER_GRID_MODEL_META_DATA_GEN_HPP

#include "input.hpp"
#include "meta_data.hpp"
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
namespace power_grid_model {

namespace meta_data {

using DatasetMap = std::map<std::string, MetaComponent>;
using AllDatasetMap = std::map<std::string, DatasetMap>;

// template function to add meta data
template <class CT>
void add_meta_data(AllDatasetMap& meta) {
    meta["input"].try_emplace(CT::name, MetaComponentImpl<typename CT::InputType>{}, CT::name);
    meta["update"].try_emplace(CT::name, MetaComponentImpl<typename CT::UpdateType>{}, CT::name);
    meta["sym_output"].try_emplace(CT::name, MetaComponentImpl<typename CT::template OutputType<true>>{}, CT::name);
    meta["asym_output"].try_emplace(CT::name, MetaComponentImpl<typename CT::template OutputType<false>>{}, CT::name);
    meta["sc_output"].try_emplace(CT::name, MetaComponentImpl<typename CT::ShortCircuitOutputType>{}, CT::name);
}

template <class T>
struct MetaDataGeneratorImpl;

template <class... ComponentType>
struct MetaDataGeneratorImpl<ComponentList<ComponentType...>> {
    using FuncPtr = std::add_pointer_t<void(AllDatasetMap& meta)>;
    static constexpr std::array<FuncPtr, sizeof...(ComponentType)> func_arr{&add_meta_data<ComponentType>...};

    static MetaData create_meta() {
        // get all dataset map
        AllDatasetMap all_map{};
        for (auto const func : func_arr) {
            func(all_map);
        }

        // create meta data set
        MetaData meta{};
        for (auto const* const dataset_name : {"input", "update", "sym_output", "asym_output", "sc_output"}) {
            DatasetMap const& single_map = all_map.at(dataset_name);
            MetaDataset meta_dataset{};
            meta_dataset.name = dataset_name;
            for (auto const& [component_name, meta_component] : single_map) {
                meta_dataset.components.push_back(meta_component);
            }
            meta.datasets.push_back(meta_dataset);
        }
        return meta;
    }
};

using MetaDataGenerator = MetaDataGeneratorImpl<AllComponents>;

inline MetaData const& meta_data() {
    static MetaData const meta_data = MetaDataGenerator::create_meta();
    return meta_data;
}

}  // namespace meta_data

}  // namespace power_grid_model

#endif
