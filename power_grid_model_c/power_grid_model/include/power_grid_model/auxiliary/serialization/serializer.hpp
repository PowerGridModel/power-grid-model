// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_SERIALIZER_HPP
#define POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_SERIALIZER_HPP

#include "../../exception.hpp"
#include "../../power_grid_model.hpp"
#include "../meta_data.hpp"
#include "../meta_data_gen.hpp"

#include <nlohmann/json.hpp>

#include <msgpack.hpp>

#include <span>
#include <string_view>

namespace power_grid_model::meta_data {

class Serializer {
  public:
    // not copyable
    Serializer(Serializer const&) = delete;
    Serializer& operator=(Serializer const&) = delete;
    // movable
    Serializer(Serializer&&) = default;
    Serializer& operator=(Serializer&&) = default;
    // destructor
    ~Serializer() = default;

    Serializer(std::string_view dataset, bool is_batch, Idx batch_size, Idx n_components, char const** components,
               Idx const* elements_per_scenario, Idx const** indptrs, void const** data)
        : dataset_{&meta_data().get_dataset(dataset)},
          is_batch_{is_batch},
          batch_size_{batch_size},
          n_components_{n_components} {}

  private:
    MetaDataset const* dataset_;
    bool is_batch_;
    Idx batch_size_;
    Idx n_components_;
};

} // namespace power_grid_model::meta_data

#endif
