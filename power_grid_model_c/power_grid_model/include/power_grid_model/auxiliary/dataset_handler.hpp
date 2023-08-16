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

    void add_component_info(std::string_view component, Idx elements_per_scenario, Idx total_elements) {
        MetaComponent const* const component_ptr = &description.dataset->get_component(component);
        auto const found = std::find_if(description.component_info.cbegin(), description.component_info.cend(),
                                        [component_ptr](auto const& x) { return x.component == component_ptr; });
        if (found != description.component_info.cend()) {
            throw DatasetError{"Cannot have duplicated components!\n"};
        }
        description.component_info.push_back({component_ptr, elements_per_scenario, total_elements});
        buffers.push_back(Buffer{});
    }

    void add_buffer(std::string_view component, Idx elements_per_scenario, Idx total_elements, Indptr* indptr,
                    Data* data) {
        add_component_info(component, elements_per_scenario, total_elements);
        buffers.back().data = data;
        buffers.back().indptr = {indptr, static_cast<size_t>(description.batch_size + 1)};
    }

    void set_buffer(std::string_view component, Indptr* indptr, Data* data) {
        MetaComponent const* const component_ptr = &description.dataset->get_component(component);
        auto const found = std::find_if(description.component_info.cbegin(), description.component_info.cend(),
                                        [component_ptr](auto const& x) { return x.component == component_ptr; });
        if (found == description.component_info.cend()) {
            throw DatasetError{"Cannot find component to set buffer!\n"};
        }
        Idx const idx = std::distance(description.component_info.cbegin(), found);
        buffers[idx].data = data;
        buffers[idx].indptr = {indptr, static_cast<size_t>(description.batch_size + 1)};
    }
};

template struct DatasetHandler<true, true>;

using ConstDatasetHandler = DatasetHandler<false, false>;
using MutableDatasetHandler = DatasetHandler<true, false>;
using WritableDatasetHandler = DatasetHandler<true, true>;

} // namespace power_grid_model::meta_data

#endif