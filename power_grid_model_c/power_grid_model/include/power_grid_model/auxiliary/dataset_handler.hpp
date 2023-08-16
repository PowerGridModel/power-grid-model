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

#include <span>
#include <string_view>

namespace power_grid_model::meta_data {

struct DatasetDescription {
    struct ComponentInfo {
        MetaComponent const* component;
        Idx elements_per_scenario;
        Idx total_elements;
    };

    bool is_batch;
    Idx batch_size;
    MetaDataset const* dataset;
    std::vector<ComponentInfo> component_info;
};

template <bool data_mutable, bool indptr_mutable>
    requires(data_mutable || !indptr_mutable)
struct DatasetHandler {
    using Data = std::conditional_t<data_mutable, void, void const>;
    using Indptr = std::conditional_t<indptr_mutable, Idx, Idx const>;
    struct Buffer {
        Data* data;
        std::span<Indptr> indptr;
    };

    DatasetDescription description;
    std::vector<Buffer> buffers;

    Idx n_component() const { return static_cast<Idx>(buffers.size()); }

    void add_buffer(std::string_view component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    void const* data);
};

} // namespace power_grid_model::meta_data

#endif