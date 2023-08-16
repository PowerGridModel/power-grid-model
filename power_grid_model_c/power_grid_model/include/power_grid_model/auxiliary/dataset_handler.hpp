// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_DATASET_HANDLER_HPP
#define POWER_GRID_MODEL_AUXILIARY_DATASET_HANDLER_HPP

// handle dataset and buffer related stuff

#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "meta_data.hpp"
#include "meta_data_gen.hpp"

#include <string_view>

namespace power_grid_model::meta_data {

class DatasetHandler {
  public:
    DatasetHandler(bool data_mutable, bool indptr_mutable, bool is_batch, Idx batch_size, std::string_view dataset)
        : data_mutable_{data_mutable},
          indptr_mutable_{indptr_mutable},
          is_batch_{is_batch},
          batch_size_{batch_size},
          dataset_{&meta_data().get_dataset(dataset)} {}

    bool is_batch() { return is_batch_; }
    Idx batch_size() { return batch_size_; }

    void set_mutable_buffer(std::string_view component, Idx elements_per_scenario, Idx total_elements,
                            Idx const* indptr, void* data);
    void set_const_buffer(std::string_view component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                          void const* data);

    // void set_buffer_to_be_parsed(std::string_view component, Idx* indptr, void* data);

  private:
    struct ComponentInfo {
        PGM_MetaComponent const* component;
        Idx elements_per_scenario;
        Idx total_elements;
    };

    bool data_mutable_;
    bool indptr_mutable_;
    bool is_batch_;
    Idx batch_size_;
    PGM_MetaDataset const* dataset_;
    std::vector<ComponentInfo> component_info_;
    std::vector<void*> mutable_data_;
    std::vector<Idx*> mutable_indptr_;
    std::vector<void const*> const_data_;
    std::vector<Idx const*> const_indptr_;
};

} // namespace power_grid_model::meta_data

#endif